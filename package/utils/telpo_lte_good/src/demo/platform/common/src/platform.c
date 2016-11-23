/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories
 *
 * $Id: platform.c 4904 2015-04-23 15:41:51Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 * File Description:
 *
 * This file implements common VMB/VMB2 functions needed for the various demos.
 *
 */

#include <stdio.h>
#include "proslic.h"
#include "spi_main.h"
#include "proslic_timer.h"

/***********************************/

static void vmbReset(controlInterfaceType *ctrlInterface)
{
    /* Setup code keeps part in reset at prompt.... */
    /*ctrlInterface->Reset_fptr(ctrlInterface->hCtrl,1);
    ctrlInterface->Delay_fptr(ctrlInterface->hTimer,250); */

    ctrlInterface->Reset_fptr(ctrlInterface->hCtrl,0);

    /* For ISI, we needed to extend the delay to access the part - 
       somewhere between 1 - 2 seconds is needed.  
       Normally this delay can be shorter. */
    ctrlInterface->Delay_fptr(ctrlInterface->hTimer,1500);
}


/************************************/
#ifdef VMB2
void vmbSetup(controlInterfaceType *ProHWIntf)
{
  uInt8 m;
  char resp[8] = {0};
  int sclk_freq;
  int pcm_src;
  int pclk_freq;
  uInt16 firmware_version;
  int isISI=0;

  printf("Demo: VMB2 VCP Connected via %s\n", SPI_GetPortNum(ProHWIntf));
  firmware_version = GetFirmwareID();
  printf("Demo: VMB2 Firmware Version %d.%d\n", ((firmware_version >> 8)&0x00FF),(firmware_version & 0x00FF));

  /* Set some defaults */
  SPI_setSCLKFreq(VMB2_SCLK_1000);  
  PCM_setPCLKFreq(VMB2_PCLK_2048);

	printf("\nChange Default VMB2 Settings (y/n) ??   ->");
	gets(resp);

  if(toupper(resp[0]) == 'Y')
	{
    /* Releases prior to firmware rev 1.3 did not support ISI */
    if( firmware_version >= 0x0103)
    {
      printf("\nDo you want to use SPI or ISI (s/i) ?? ->");
      gets(resp);
      printf("\n\n");
      fflush(stdout);
    }
    else
    {
      *resp = 'S';
    }

    if(toupper(*resp) == 'S')
    {
      printf("\n\n");
      printf("-----------------\n");
      printf("Select SCLK Freq\n");
      printf("-----------------\n");
      printf("0. Default \n");
      printf("1. 1MHz\n");
      printf("2. 2MHz\n");
      printf("3. 4MHz\n");
      printf("4. 8MHz\n");
      printf("5. 12MHz\n");
      printf("-----------------\n");
      printf("Selection -> ");

      gets(resp);
      printf("\n\n");
      fflush(stdout);

      switch(resp[0])
      {
        case '0':
          sclk_freq = VMB2_SCLK_1000;
          break;
        case '1': 
          sclk_freq = VMB2_SCLK_1000;
            break;
        case '2': 
          sclk_freq = VMB2_SCLK_2000;
            break;
        case '3': 
          sclk_freq = VMB2_SCLK_4000;
            break;
        case '4': 
          sclk_freq = VMB2_SCLK_8000;
            break;
        case '5': 
          sclk_freq = VMB2_SCLK_12000;
            break;
        default:
          sclk_freq = VMB2_SCLK_1000;
      }	

      SPI_setSCLKFreq(sclk_freq);
    }
    else
    {
      SPI_SelectFormat(2);
      printf("VMB2 now set for ISI.\n"); 
      isISI = 1;
    }

    printf("-----------------\n");
    printf("Select PCM Source\n");
    printf("-----------------\n");
    printf("0. Internal \n");
    printf("1. External\n");
    printf("-----------------\n");
    printf("Selection -> ");

    gets(resp);
    printf("\n\n");
    fflush(stdout);

    switch(resp[0])
    {
          case '1': 
            pcm_src = 0;
            if(isISI == 1)
            {
              printf("NOTE: ISI needs to have a 2.048 MHz PCLK\n");
              break;
            }
          default:
            pcm_src = 1;  /* internal */
            break;
    }

    /* Setup hardware */
    PCM_setSource(pcm_src);

    if( (pcm_src == 1) && (isISI == 0))
    {
      printf("\n\n");
      printf("-----------------\n");
      printf("Select PCLK Freq\n");
      printf("-----------------\n");
      printf("0. Default \n");
      printf("1. 8192kHz\n");
      printf("2. 4096kHz\n");
      printf("3. 2048kHz\n");
      printf("4. 1024kHz\n");
      printf("5. 512kHz\n");
      printf("6. 1536kHz\n");
      printf("7. 768kHz\n");
      printf("8. 1544kHz\n");		
      printf("-----------------\n");
      printf("Selection -> ");

      gets(resp);
      m = (uInt8)atoi(resp);
      printf("\n\n");
      fflush(stdout);

      if((m < 1)||(m > 8)) pclk_freq = 3;
      else 				 pclk_freq = m-1;
      
      printf("-----------------\n");
      printf("Select Framesync type\n");
      printf("-----------------\n");
      printf ("0. FSYNC Short\n");
      printf ("1. FSYNC Long\n");
      printf("-----------------\n");
      printf("Selection -> ");

      gets(resp);
      m = (uInt8)atoi(resp);
      printf("\n\n");
      fflush(stdout);

      if(m == 1)
      {
        PCM_setFsyncType(VMB2_FSYNC_LONG);
      } 
      else
      {
        PCM_setFsyncType(VMB2_FSYNC_SHORT);
      }
      PCM_setPCLKFreq(pclk_freq);
    }
  } /* end of if change = 'Y' */
}

#else
#include "DLLWrapper.h"
void vmbSetup(controlInterfaceType *ProHWIntf)
{
	char resp[8] = {0};

    printf("Demo: VMB Connected\n");

	printf("\nChange Default VMB Settings (y/n) ??   ->");
	gets(resp);
	if(toupper(resp[0]) == 'Y')
	{
	    int pcm_src, m;
	    int pcm_enums[] = {128,64,32,16,8,4,512,12,24};

		printf("\n\n");
		printf("-----------------\n");
		printf("Select PCM Source\n");
		printf("-----------------\n");
		printf("0. Internal \n");
		printf("1. External\n");
		printf("-----------------\n");
		printf("Selection -> ");

		gets(resp);
		printf("\n\n");
		fflush(stdout);

		switch(*resp)
		{
			case '0': 
				pcm_src = 1;
			    break;
			case '1': 
				pcm_src = 0;
				m = 3;
			    break;
			default:
				pcm_src = 1;  /* internal */
				break;
		}	

		if(pcm_src == 1)
		{

			printf("\n\n");
			printf("-----------------\n");
			printf("Select PCLK Freq\n");
			printf("-----------------\n");
			printf("0. Default \n");
			printf("1. 8192kHz\n");
			printf("2. 4096kHz\n");
			printf("3. 2048kHz\n");
			printf("4. 1024kHz\n");
			printf("5. 512kHz\n");
			printf("6. 256kHz\n");
			printf("7. 32768kHz\n");
			printf("8. 768kHz\n");
			printf("9. 1536kHz\n");

			printf("-----------------\n");
			printf("Selection -> ");

			gets(resp);
			m = (uInt8)atoi(resp);
			printf("\n\n");
			fflush(stdout);
			
			if(m == 0)
			{
				m = 3;/* 2.048 MHz */
			}

			if((m <0) || (m>9))
			{
				printf("Invalid choice, defaulting to 2.048 Mhz\n");
				m = 3;
			}
		}
		printf("pcm_src = %d m = %d\n", pcm_src, m);
		setPcmSourceExp(pcm_src, pcm_enums[m-1], 0);
	}
}
#endif

/************************************/
void initControlInterfaces(controlInterfaceType *ProHWIntf, void *spiIf, void *timerObj)
{

  SiVoice_setControlInterfaceCtrlObj (ProHWIntf, spiIf);
	SiVoice_setControlInterfaceReset (ProHWIntf, ctrl_ResetWrapper);

	SiVoice_setControlInterfaceWriteRegister (ProHWIntf, ctrl_WriteRegisterWrapper);
	SiVoice_setControlInterfaceReadRegister (ProHWIntf, ctrl_ReadRegisterWrapper);		
	SiVoice_setControlInterfaceWriteRAM (ProHWIntf, ctrl_WriteRAMWrapper);
	SiVoice_setControlInterfaceReadRAM (ProHWIntf, ctrl_ReadRAMWrapper);

	SiVoice_setControlInterfaceTimerObj (ProHWIntf, timerObj);
	SiVoice_setControlInterfaceDelay (ProHWIntf, time_DelayWrapper);
	SiVoice_setControlInterfaceTimeElapsed (ProHWIntf, time_TimeElapsedWrapper);
	SiVoice_setControlInterfaceGetTime (ProHWIntf, time_GetTimeWrapper);

	SiVoice_setControlInterfaceSemaphore (ProHWIntf, NULL);

  //ctrl_ResetWrapper(ProHWIntf,1); /* Since we may be changing clocks in vmbSetup() , keep the part in reset */

  /*
    	** Configuration of SPI and PCM internal/external 
	*/
	//vmbSetup(ProHWIntf);

	/*
   ** Call host/system reset function (omit if ProSLIC tied to global reset)
   */
	printf("Demo: Resetting device...\n");
  //vmbReset(ProHWIntf);

}

/************************************/
void destroyControlInterfaces(controlInterfaceType *ProHWIntf)
{
#ifdef VMB2
  SPI_Close(ProHWIntf->hCtrl);
#endif

}

