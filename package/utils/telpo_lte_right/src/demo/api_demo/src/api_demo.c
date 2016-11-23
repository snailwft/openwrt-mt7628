/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: api_demo.c 4897 2015-04-21 23:33:59Z nizajerk $
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API
**
*/
#include "stdio.h"
#include "proslic.h"
#include "api_demo.h"
#include "proslic_timer.h"
#include "proslic_tstin_limits.h"
#include "si32xxx_multibom_constants.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if 1
#include "gsm.h"
#include "asoundlib.h"
#endif

/*
** Functional Blocks
*/
#include "debug.h" 
#include "monitor.h"
#include "linefeed.h"
#include "converter.h"
#include "ringing.h"
#include "audio.h"
#include "tonegen.h"
#include "interrupt.h"
#include "pm_mwi.h"
#ifdef MLT_ENABLED
#include "proslic_mlt_demo.h"
#endif
/*
** VMB1/2 Includes 
*/
#include "spi_main.h"

/*
** Device Specific Includes
*/
#ifdef SI321X
#include "si321x.h"
#endif
#ifdef SI322X
#include "si3226.h"
#endif
#ifdef SI3217X
#include "si3217x.h"
#include "vdaa.h"
extern Si3217x_General_Cfg Si3217x_General_Configuration;
#endif
#ifdef SI3218X
#include "si3218x.h"
extern Si3218x_General_Cfg Si3218x_General_Configuration;
#endif
#ifdef SI3226X
#include "si3226x.h"
#endif
#ifdef SI3228X
#include "si3228x.h"
#endif
#ifdef SI324X
#include "si324x.h"
#endif

/************************************/
uInt8 channelSelect(chanState *pState)
{
uInt8 m;
char resp[8]={0};
    do
    {
        printf("\nPlease Enter Channel:  (0->%d): ", (pState->numberOfChan -1));
		gets(resp);
		m = resp[0];
        m = ((uInt8)m - 48);
		fflush(stdin);
    }while(m<0 || m>(pState->numberOfChan-1));

    return m;
}

/***********************************/
/*
** Function: API_Demo
**
** Description: 
** Example implementation of ProSLIC API hardware initialization and config
**
** Input Parameters: 
** 
** Return:
** none
*/
uInt16 API_Demo (){
	ctrl_S spiGciObj;              /* Link to host spi obj (os specific) */
  	systemTimer_S timerObj;        /* Link to host timer obj (os specific)*/
	controlInterfaceType *ProHWIntf; /* proslic hardware interface object */
	ProslicDeviceType *ProSLICDevices[MAX_NUMBER_OF_DEVICE]; /* proslic device object */
	proslicChanType_ptr arrayOfProslicChans[MAX_NUMBER_OF_CHAN]; /* used for initialization only */
	chanState ports[MAX_NUMBER_OF_CHAN];  /* declare channel state structures */
	uInt8 i=0;
	proslicChanType *pSlic;
	int status, fdpcm = -1;
	pcm_in = get_pcm_in();
	pcm_out = get_pcm_out();	
#if 0
	fdpcm = open("/dev/pcm", O_RDWR);
	if (fdpcm < 0)
	{
		printf("open failed\n");
	}
#endif
	fd4 = open("/dev/slic_gpio", O_RDWR); // 电话指示灯
	if (fd4 < 0)
	{
#ifdef _DBUG_
		printf("open fd4 failed\n");
#endif
		dbug_log(__FILE__,__LINE__,"open file failed\n");
		return 0;
	}
    /* 
    ** --------------------------------------------------------------------
    ** Initialize host SPI interface object (optional - platform dependent)
    ** --------------------------------------------------------------------
    */
    printf("\nDemo:  Connecting to Host -> %s ...\n",VMB_INFO);
	  if(SPI_Init(&spiGciObj) == FALSE)
    {
      printf("Demo: Cannot connect to %s\n",VMB_INFO);
      getchar();
      return 0;	
    }
    
    /* 
    ** ----------------------------------------------------------------------
    ** Initialize host TIMER interface object (optional - platform dependent)
    ** ----------------------------------------------------------------------
    */
    printf("Demo: Initializing system timer...\n");
	TimerInit(&timerObj);

	/* 
	** This demo supports single device/BOM option only
	*/

#ifdef SI3217X
	ports[0].deviceType     = SI3217X_TYPE;
	ports[0].numberOfDevice = SI3217XB_NUMBER_OF_DEVICE;
	ports[0].numberOfChan   = SI3217XB_NUMBER_OF_CHAN;
	ports[0].chanPerDevice  = SI3217X_CHAN_PER_DEVICE;
#endif
#ifdef SI3218X
	ports[0].deviceType     = SI3218X_TYPE;
	ports[0].numberOfDevice = SI3218X_NUMBER_OF_DEVICE;
	ports[0].numberOfChan   = SI3218X_NUMBER_OF_CHAN;
	ports[0].chanPerDevice  = SI3218X_CHAN_PER_DEVICE;
#endif
#ifdef SI3226X
	ports[0].deviceType     = SI3226X_TYPE;
	ports[0].numberOfDevice = SI3226X_NUMBER_OF_DEVICE;
	ports[0].numberOfChan   = SI3226X_NUMBER_OF_CHAN;
	ports[0].chanPerDevice  = SI3226X_CHAN_PER_DEVICE;
#endif
#ifdef SI3228X
	ports[0].deviceType     = SI3228X_TYPE;
	ports[0].numberOfDevice = SI3228X_NUMBER_OF_DEVICE;
	ports[0].numberOfChan   = SI3228X_NUMBER_OF_CHAN;
	ports[0].chanPerDevice  = SI3228X_CHAN_PER_DEVICE;
#endif
#ifdef SI322X	
	ports[0].deviceType     = SI3226_TYPE;
	ports[0].numberOfDevice = SI322X_NUMBER_OF_DEVICE;
	ports[0].numberOfChan   = SI322X_NUMBER_OF_CHAN;
	ports[0].chanPerDevice  = SI322X_CHAN_PER_DEVICE;
#endif
#ifdef SI324X
	ports[0].deviceType     = SI324X_TYPE;
	ports[0].numberOfDevice = SI324X_NUMBER_OF_DEVICE;
	ports[0].numberOfChan   = SI324X_NUMBER_OF_CHAN;
	ports[0].chanPerDevice  = SI324X_CHAN_PER_DEVICE;
#endif
#ifdef SI321X
	ports[0].deviceType     = SI321X_TYPE;
	ports[0].numberOfDevice = SI321X_NUMBER_OF_DEVICE;
	ports[0].numberOfChan   = SI321X_NUMBER_OF_CHAN;
	ports[0].chanPerDevice  = SI321X_CHAN_PER_DEVICE;
#endif

    /*
    ** Print demo header
    */
    printf("------------------------------------------\n");
    printf("\tProSLIC API Implemtation Demo\n");
    printf("\tAPI Version %s\n", ProSLIC_Version());
#ifdef MLT_ENABLED
    printf("\tMLT Version: %s\n", ProSLIC_mlt_version());
#endif
    printf("\tNumber of devices: %d Number of channels: %d\n", 
		ports[0].numberOfDevice,
		ports[0].numberOfChan);
    printf("------------------------------------------\n");


    /*
    ** Create control interface object (required)
    */ 
	printf("Demo: Creating Control Interface...\n");
	SiVoice_createControlInterface(&ProHWIntf);

    /*
    ** Create device object(s) (required)
    */
	printf("Demo: Creating Device Obj...\n");
	for (i=0;i<ports[0].numberOfDevice;i++)
		SiVoice_createDevice (&(ProSLICDevices[i]));
	

    /*
    ** Create channel object(s) and allocate
    ** memory for them (required)
    */
	printf("Demo: Creating Chan Obj...\n");
	for (i=0;i<ports[0].numberOfChan;i++)
	{
		SiVoice_createChannel(&(ports[i].ProObj));
		SiVoice_SWInitChan (ports[i].ProObj,i,ports[0].deviceType, ProSLICDevices[i/ports[0].chanPerDevice], ProHWIntf);
	}

    /*
    ** Establish linkage between host objects/functions
    ** and ProSLIC API (required)
    */
	printf("Demo: Linking function pointers...\n");
  	initControlInterfaces(ProHWIntf, &spiGciObj, &timerObj);
	SiVoice_Reset(ports[0].ProObj); // ProHWIntf
	#ifdef TSTIN_SUPPORT
	/*
    ** Setup optional inward test facilities
	*/
	printf("Demo: Creating Inward Test Obj...\n");
	for(i=0;i<ports[0].numberOfChan;i++)
	{
		ProSLIC_createTestInObj(&(ports[i].pTstin));
	}

	/*
    ** Configure test objects
	*/
	printf("Demo: Configuring Inward Tests...\n");
	for(i=0;i<ports[0].numberOfChan;i++)
	{
		ProSLIC_testInPcmLpbkSetup(ports[i].pTstin, &ProSLIC_testIn_PcmLpbk_Test);
		ProSLIC_testInDcFeedSetup(ports[i].pTstin, &ProSLIC_testIn_DcFeed_Test);
		ProSLIC_testInRingingSetup(ports[i].pTstin, &ProSLIC_testIn_Ringing_Test);
		ProSLIC_testInBatterySetup(ports[i].pTstin, &ProSLIC_testIn_Battery_Test);
		ProSLIC_testInAudioSetup(ports[i].pTstin, &ProSLIC_testIn_Audio_Test);
	}
#endif

		


#ifdef SI3217X
  /* If we have a '178, we have 2 channels (default), otherwise, we have 1,
     we're assuming ALL '17x's are the same...
   */
  i = ProSLIC_ReadReg(ports[0].ProObj,0);
  if(( (i>>3) & 7) != 6)
  {
    ports[0].numberOfChan--;
    ports[0].chanPerDevice--;
  }
#endif
    /*
    ** Enable debug mode per channel (optional)
    ** Create array of channel pointers to streamline
    ** the initialization process (required)
    */
	for (i=0;i<ports[0].numberOfChan;i++)
	{
		arrayOfProslicChans[i] = ports[i].ProObj; 
		SiVoice_setSWDebugMode (ports[i].ProObj,TRUE);
	}

	/* ----------------------------
	** Main ProSLIC Initialization
	** ----------------------------
	*/
	printf("Demo: Initializing ProSLIC...\n");

	if (ProSLIC_Init(arrayOfProslicChans,ports[0].numberOfChan))
	{
		printf ("\nDemo:  ERROR : Initialization failed\n");
		printf ("\n\tHit any key to abort ->");
		(void)getchar();
		printf("\n");
		return 0;
	}
#ifdef VDAA_SUPPORT
	if(Si3217x_General_Configuration.daa_cntl!= 0)
	{
		printf("Demo: Initializing FXO...\n");
		if((ports[0].deviceType == SI3217X_TYPE)&&(ports[0].deviceRev == 1)) /* Rev B Only */
		{
			/* Initialize any DAA channels that are present */
			if (Vdaa_Init(arrayOfProslicChans,ports[0].numberOfChan))
			{
				printf ("\nDemo:  ERROR : DAA Initialization Failed\n");
				printf ("\n\tHit any key to abort ->");
				(void)getchar();
				printf("\n");
				return 0;
			}	
		}
	}
#endif
#ifndef SKIP_LBCAL
    /*
    ** Longitudinal Balance Calibration
    **
	*/
    printf ("Demo: Starting Longitudinal Balance Calibration...\n");
    status = ProSLIC_LBCal(arrayOfProslicChans,ports[0].numberOfChan);
	if(status != RC_NONE)
	{
		printf("\nDemo: LB CAL ERROR : %d\n", status);
	}
#endif

	printf("Demo: Loading Presets...\n");
    /*
    ** Load custom presets (required if used)
    */
	for (i=0;i<ports[0].numberOfChan;i++)
	{
		pSlic = ports[i].ProObj;
		ProSLIC_DCFeedSetup(pSlic,0);
		ProSLIC_RingSetup(pSlic,0);
    	ProSLIC_ZsynthSetup(pSlic,0);
	}
#ifdef GSM_CFG
	if (national_standard_match_handler(ports[0]) > 0) //阻抗匹配，输入输出音量大小初始化 国家标准等初始化
		printf("impedance_match SUCCESS\n");
	else
		printf("impedance_match FAILED\n");
#endif

    /*
    ** Initialize Demo
    */
	for (i=0;i<ports[0].numberOfChan;i++)
	{		
		pSlic = ports[i].ProObj;
		ProSLIC_SetLinefeedStatus(pSlic,LF_FWD_ACTIVE);
		ProSLIC_EnableInterrupts(pSlic);	
	}

#ifdef VDAA_SUPPORT
	if( Si3217x_General_Configuration.daa_cntl && 
			(ports[0].deviceType == SI3217X_TYPE)&&(ports[0].deviceRev == 1))
	{
		Vdaa_InitLineInUseCheck(&(ports[1].liuCfg),35,4,14);
	}

#endif

	printf("Demo: Initialization Complete\n");

#ifdef GSM_CFG
	char ttybuf[20] = {0};
	if (access("/dev/ttyUSB0", F_OK) == -1) // 不存在
	{
		if (access("/dev/ttyUSB2", F_OK) != -1)
		{
			remove("/dev/ttyUSB2");
		}
	}
	for (i = 0; i < 6; i++)
	{
		sprintf(ttybuf, "/dev/ttyUSB%d", i);
		ttybuf[strlen(ttybuf)] = '\0';
		if (access(ttybuf, F_OK) == -1) //检测文件是否存在，如果不存在就等待
		{
			perror("");
			simcom_enable(ports[0].ProObj); //使能4G 模块
			sleep(25);
			break;
		}
	}
	pthread_t gsm_init_deal, gsm_recver, gsm_send, user_hand,
		gsm_write_audio, gsm_read_audio, proslic_interrupt, interface_active, slic_lamp_hand, send_sms_deal;

	init_machine();	
	gsm_init();
	user_init();
	slic_init();
	interacter_init();
	init_dtmf_rx_buff();
	signal(SIGALRM, time_handler);
	set_timer(); 														// 时间设置好之后到点就会发出SIGALRM信号	
	fd = uart_open("/dev/ttyUSB2");						//AT指令
	fd2 = pcm_channel_open("/dev/ttyUSB4");
	sem_init(&gsm_sem, 0, 0); 
	sem_init(&gsm_pcm_sem, 0, 0);
	sem_init(&gsm_pcm_sem2, 0, 1);
	pthread_mutex_init(&audio_mutex, NULL);
	pthread_mutex_init(&cid_mutex, NULL);//
	pthread_mutex_init(&dtmf_mutex, NULL);//
	pthread_mutex_init(&atorder_mutex, NULL);//
	pthread_mutex_init(&send_atorder_mutex, NULL);
	pthread_mutex_init(&pcm_mutex, NULL);
	pthread_mutex_init(&pcm_recv_mutex, NULL);
	pthread_mutex_init(&pcm_send_mutex, NULL);
	
	pthread_create(&gsm_init_deal, NULL, gsm_init_handler, NULL);
	pthread_create(&gsm_recver, NULL, gsm_recv_handler, NULL);		
	pthread_create(&user_hand, NULL, user_hanlder, (void *)&ports[0]);
	//pthread_create(&proslic_interrupt, NULL, proslic_interrupt_handler, &ports[0]);
	//pthread_create(&interface_active,NULL,interface_active_handler,NULL);
	pthread_create(&slic_lamp_hand, NULL, slic_lamp_handler, NULL);
	pthread_create(&gsm_write_audio, NULL, gsm_audio_write_thread, NULL);
	pthread_create(&gsm_read_audio, NULL, gsm_audio_read_thread, NULL);

#ifdef SMS_ENABLE
	//pthread_create(&send_sms_deal, NULL, send_sms_handler, NULL);
#endif

	pthread_join(gsm_init_deal,NULL);
	pthread_join(gsm_recver, NULL);
	pthread_join(user_hand, NULL);	
	pthread_join(slic_lamp_hand, NULL);
	pthread_join(gsm_write_audio, NULL);
	pthread_join(gsm_read_audio, NULL);
	pthread_join(proslic_interrupt, NULL);
	//pthread_join(signal_hand, NULL);
	//pthread_join(interface_active,NULL);
	pthread_mutex_destroy(&audio_mutex);  //删除互斥锁
	pthread_mutex_destroy(&cid_mutex);
	pthread_mutex_destroy(&dtmf_mutex);
	pthread_mutex_destroy(&atorder_mutex);
	pthread_mutex_destroy(&send_atorder_mutex);
	pthread_mutex_destroy(&pcm_mutex);
	pthread_mutex_destroy(&pcm_recv_mutex);
	pthread_mutex_destroy(&pcm_send_mutex);
	pthread_attr_destroy(&attr);
	sem_destroy(&gsm_sem);
	sem_destroy(&gsm_pcm_sem);
	sem_destroy(&gsm_pcm_sem2);
	close(fd);
	close(fd2);
#endif


	/*
	** Main Demo Menu
	*/
    ApiTopMenu(&(ports[0]));


	/*
	** Shutdown, free memory, reset 
	*/
	for (i=0;i<ports[0].numberOfChan;i++)
	{
        ProSLIC_ShutdownChannel(ports[i].ProObj);
	}

  SiVoice_Reset(ports[0].ProObj);

	for (i=0;i<ports[0].numberOfChan;i++)
	{
#ifdef TSTIN_SUPPORT
		ProSLIC_destroyTestInObj(&(ports[i].pTstin));
#endif
		ProSLIC_destroyChannel(&(ports[i].ProObj));
	}

	for(i=0;i<ports[0].numberOfDevice;i++)
	{
		ProSLIC_destroyDevice(&(ProSLICDevices[i]));
	}

  /* Close any system resources */
  destroyControlInterfaces(&ProHWIntf);

  /* Free the memory for the control interfaces */
  SiVoice_destroyControlInterface(&ProHWIntf);

	return 1;
}

#ifdef VDAA_SUPPORT

/*
** 
** API TEST Functions
**
*/
char *ReadHookStatus(chanState *pState) {
	int offset;

	if(pState[0].ProObj->channelType == DAA)
		offset = 0;
	else
		offset = 1;

    switch(Vdaa_GetHookStatus(pState[offset].ProObj)) {
        case VDAA_ONHOOK:
            return "ONHOOK";
        break;
        case VDAA_ONHOOK_MONITOR:
            return "ONHOOK_MONITOR";
        break;
        case VDAA_OFFHOOK:
            return "OFFHOOK";
        break;
        default:
            return "UNKNOWN";
    }

}

#define WriteVDAAReg pState[1]->ProObj->deviceId->ctrlInterface->WriteRegister_fptr;
#define ReadVDAAReg pState[1]->ProObj->deviceId->ctrlInterface->ReadRegister_fptr;

void daaMenu(chanState *pState)
{
    char cmd[0]={0};
    uInt8 regData;
    uInt32 ramData;
    uInt8 m=0;
    int ovl = 0;
    int8 vloop;
    int16 iloop;
    int i;
    int offset;

	/* Allow for standalone 3050 teting */
	if(pState[0].ProObj->channelType == DAA)
		offset = 0;
	else 
		offset = 1;

	do
	{
        printf("\n\n");

        printf("-----------------------------\n");
        printf("------   DAA MAIN MENU ------\n");
        printf("-----------------------------\n");  
        printf("1. Toggle Hook State (currently %s)\n",ReadHookStatus(pState));
        printf("2. Line Monitor\n");
        printf("3. Ring Detect\n");
        printf("4. Register Dump\n");
        printf("5. Line In Use Check\n");
        printf("-----------------------------\n"); 
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);

        switch(cmd[0])
        {
            case '1':
                regData = Vdaa_GetHookStatus(pState[offset].ProObj);
                switch(regData) {
                    case VDAA_ONHOOK:
                    case VDAA_ONHOOK_MONITOR:
                        Vdaa_SetHookStatus(pState[offset].ProObj, VDAA_OFFHOOK);
                    break;
                    case VDAA_OFFHOOK:
                        Vdaa_SetHookStatus(pState[offset].ProObj, VDAA_ONHOOK);
                    break;
                }
        
            break;
            
            case '2':
                ovl = Vdaa_ReadLinefeedStatus (pState[offset].ProObj,&vloop, &iloop);
                printf ("\n\nVDAA LINE MONITOR\n");
                printf ("-----------------------\n");
                printf ("VTR      = %d  v\n", vloop);
                printf ("ILOOP    = %d  mA\n", iloop);
				if(ovl == RC_VDAA_ILOOP_OVLD)
					printf("#### OVL ####\n");
            break;

            case '3':
                printf("\n\nWORK IN PROGRESS....\n\n");
            break;

            case '4':
                for(i=1;i<60;i++) {
                    regData = pState[offset].ProObj->deviceId->ctrlInterface->ReadRegister_fptr(pState[offset].ProObj,offset,i);
                    printf("REG %2d = 0x%02X\n", i,regData);
                }
            break;


            case '5':
                i = Vdaa_CheckForLineInUse(pState[offset].ProObj, &(pState[offset].liuCfg));
                printf("\n\n");
                printf("Measured VLOOP = %d\n", pState[offset].liuCfg.measured_vloop);
                printf("Measured ILOOP = %d\n", pState[offset].liuCfg.measured_iloop);
                if(i==VDAA_ONHOOK)
                    printf("Hook Status = ONHOOK\n");
                else
                    printf("Hook Status = OFFHOOK\n");

                if(pState[offset].liuCfg.status == PAR_HANDSET_DETECTED)
                    printf("Parallel Handset DETECTED\n");
                else
                    printf("Parallel Handset NOT DETECTED\n");
            break;

			default:
				break;
        }
    }while(toupper(cmd[0]) != 'Q');

}
#endif /* VDAA SUPPORT */


/***********************************/
void ApiTopMenu(chanState *pState)
{
uInt8 chan=0;
char resp[32] = {0};
nums = 0;

	chan = channelSelect(pState);
  do
	{
    printf("\n\n");
    printf("---------------------------------------------------------------\n");
    printf("------------------   API TEST MAIN MENU -----------------------\n");
    printf("---------------------------------------------------------------\n");  
    printf("0.  Select Channel\n");
    printf("1.  Debug Menu\n");
    printf("2.  Test & Monitor Menu\n");  
    printf("3.  Linefeed Menu\n");
    printf("4.  DC/DC Converter Menu\n");
    printf("5.  Ringing Menu\n");
    printf("6.  Audio Menu\n");
    printf("7.  Tone Generator Menu\n");
    printf("8.  MWI/Pulse Metering Menu\n");
    printf("9.  Interrupt Menu\n");
#ifdef MLT_ENABLED
    printf("a.  MLT test Menu\n");
#endif
    printf("---------------------------------------------------------------\n\n");
    printf("Select Menu Item or 'q' to quit: ");
				
		//gets(resp);
		//fflush(stdin); 
        
    //switch(tolower(*resp)) 
    switch('6')
    {
        case '0':
            chan = channelSelect(pState);
            break;
        case '1':
            debugMenu(&(pState[chan]));
            break;
        case '2':
            testMonitorMenu(&(pState[chan]));
            break;
        case '3':
            linefeedMenu(&(pState[chan]));
            break;
        case '4':
            converterMenu(&(pState[chan]));
            break;
        case '5':
            ringingMenu(&(pState[chan]));
            break;
        case '6':
            audioMenu(&(pState[chan]));
            break;
        case '7':
            toneGenMenu(&(pState[chan]));
            break;
        case '8':
            pmMwiMenu(&(pState[chan]));
            break;			
        case '9':
            interruptMenu(&(pState[chan]));
            break;
#ifdef MLT_ENABLED
        case 'a':
            mltMenu(&(pState[chan]));
#endif
        default:
            break;
		}/* switch */
	}while(toupper(resp[0]) != 'Q');/* while */
}
