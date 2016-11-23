/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: audio.c 4965 2015-04-27 22:23:30Z nizajerk $
**
** audio.c
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** audio resources
**
*/

#include "api_demo.h"
#include "macro.h"

/**************************************/
static uInt8 checkSum(char *str)
{
    int i=0;
    uInt8 sum = 0;

    while(str[i] != 0)
    {
        sum += str[i++];
    }

    return -sum;
}

/**************************************/
/* Wait for FSK buffer to be available... */
static waitForCIDDone(SiVoiceChanType_ptr pChan)
{
    int tmp = 0;

    do
    {
        ProSLIC_CheckCIDBuffer(pChan, &tmp);
    } while(tmp == 0);

}

/**************************************/
/* 
** Sequential (blocking) example of CID transmission
*/
void sendCIDStream(chanState *pState)
{
uInt8 cid_preamble[] = {'U','U','U','U','U','U','U','U'};
#if 1
    int8 cid_msg[] =
            "\x80"           /* MDMF Type */
            "\x27"           /* Message Length */
            "\x01"           /* Date/Time Param */
            "\x08"           /* 8-byte Date/Time */
            "07040815"       /* July 4th 08:15 am */
            "\x02"           /* Calling Number Param */
            "\x0A"           /* 10-byte Calling Number */
            "5124168500"     /* Phone Number */
            "\x07"           /* Calling Name Param */
            "\x0F"           /* 15-byte Calling Name */
            "Nice"           /* Calling Name */
            "\x20"           /* Calling Name (whitespace) */
            "ProSLIC!!!"     /* Calling Name */
            "\x00"           /* Placeholder for Checksum */
            "\x00"           /* Markout */
    ;
#else
	int8 cid_msg[50];
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);
	char timebuf[8];
	cid_msg[0]= 0x04;
	cid_msg[1]= 19;
	//sprintf(&cid_msg[2], "%02d%02d%02d%02d", p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min);	
	strcpy(&cid_msg[2], "11010808");
	strcpy(&cid_msg[10],"13016054035");
	cid_msg[10+11]= '\x00';
	cid_msg[11+11]= '\x00';
#endif

    uInt8 csum;
    uInt8 reg_tmp;
    int cid_remainder;
    int baud_rate = 1200;
    int bits = 10;  /* 1 start + 8 data + 1 stop */
    int buff_timeout;
    int i;

    buff_timeout = ((8 - FSK_DEPTH_TRIG) * (10000*bits)/baud_rate)/10;

    ProSLIC_RingSetup(pState->ProObj,1);
    ProSLIC_FSKSetup(pState->ProObj,0);
    reg_tmp =ReadReg(pProHW,pState->ProObj->channel,RINGCON);
    WriteReg(pProHW,pState->ProObj->channel,RINGCON,reg_tmp&0xF0);

    /* Ensure OFFHOOK Active */
    ProSLIC_SetLinefeedStatus(pState->ProObj,LF_FWD_ACTIVE);
    Delay(pProTimer,500);

    /* 1st Ring Burst */
    ProSLIC_SetLinefeedStatus(pState->ProObj,LF_RINGING);
    Delay(pProTimer,2000);

    /* OHT */
    ProSLIC_SetLinefeedStatus(pState->ProObj,LF_FWD_OHT);
    Delay(pProTimer,1400);   /* Delay 250 to 3600ms */

    /* Enable FSKBUF interrupt so we can check it later. */
    WriteReg(pProHW,pState->ProObj->channel,IRQEN1,0x40);
  
    /*
     ** SEND CID HERE
     */

     /* Clear FSK buffer */
     if((cid_msg[1]+3)%FSK_DEPTH_TRIG)
     {
        cid_remainder = 1;
     }
     else
     {
        cid_remainder = 0;
     }

     ProSLIC_EnableCID(pState->ProObj);
     Delay(pProTimer,133);   /* > 130ms of mark bits */
            
     /* Send preamble */
     for(i=0;i<30;i+=FSK_DEPTH_TRIG)
     {
        if(i >= 8)
		//if(i >= 8)
        {
            waitForCIDDone(pState->ProObj);
        }
        ProSLIC_SendCID(pState->ProObj,cid_preamble,FSK_DEPTH_TRIG);
     }

     if (30%FSK_DEPTH_TRIG)
     {
        waitForCIDDone(pState->ProObj);
     }

     ProSLIC_SendCID(pState->ProObj,cid_preamble,30%FSK_DEPTH_TRIG);
     waitForCIDDone(pState->ProObj);

     /* Delay > 130ms for idle mark bits */
     Delay(pProTimer,133);
            
     /* Send Message */
     csum = checkSum(cid_msg);
     cid_msg[cid_msg[1]+2] = csum;

    for(i=0;i<(cid_msg[1]+3);i+=FSK_DEPTH_TRIG)
    {
        //if(i>=8)
		if(i>=4)
        {
            waitForCIDDone(pState->ProObj);
        }
        ProSLIC_SendCID(pState->ProObj,&(cid_msg[i]),FSK_DEPTH_TRIG);
    }

    if(cid_remainder)
    {
        waitForCIDDone(pState->ProObj);
        
        ProSLIC_SendCID(pState->ProObj,&(cid_msg[((cid_msg[1]+3)/FSK_DEPTH_TRIG)*FSK_DEPTH_TRIG]),(cid_msg[1]+3)%FSK_DEPTH_TRIG);
    }

    waitForCIDDone(pState->ProObj);

    /* Make sure the last byte is shifted out prior to disabling CID */
    Delay(pProTimer,buff_timeout);
    ProSLIC_DisableCID(pState->ProObj);
}


/**************************************/
/* 
** Sequential (blocking) example of VMWI transmission
*/
void sendVMWIStream(chanState *pState,int enable_flag)
{
    uInt8 vmwi_preamble[] = {'U','U','U','U','U','U','U','U'};
    uInt8 *vmwi_msg;

    // VMWI Message 
    int8 vmwi_msg_enable[] =
            "\x06"           /* SDMF VMWI Type */
            "\x03"           /* Message Length (always 3 for SDMF VMWI) */
            "\x42"           /* VMWI On */
            "\x42"           /* VMWI On */
            "\x42"           /* VMWI On */
            "\x00"           /* Placeholder for Checksum */
            "\x00"            /* Markout */
        ;

    int8 vmwi_msg_disable[] =
        
            "\x06"           /* SDMF VMWI Type */
            "\x03"           /* Message Length (always 3 for SDMF VMWI) */
            "\x6f"           /* VMWI Off */
            "\x6f"           /* VMWI Off */
            "\x6f"           /* VMWI Off */
            "\x00"           /* Placeholder for Checksum */
            "\x00"           /* Markout */
        ;
    
    uInt8 csum;
    int remainder;
    int baud_rate = 1200;
    int bits = 10;  /* 1 start + 8 data + 1 stop */
    int buff_timeout;
    int i;

    if(enable_flag)
    {
        vmwi_msg = vmwi_msg_enable;
    }
    else
    {
        vmwi_msg = vmwi_msg_disable;
    }

    buff_timeout = ((8 - FSK_DEPTH_TRIG) * (10000*bits)/baud_rate)/10;
    ProSLIC_FSKSetup(pState->ProObj,0);

    /* OHT */
    ProSLIC_SetLinefeedStatus(pState->ProObj,LF_FWD_OHT);
    Delay(pProTimer,1400);   /* Delay 250 to 3600ms (1400) */

    /* Enable FSKBUF interrupt so we can check it later. */
    WriteReg(pProHW,pState->ProObj->channel,IRQEN1,0x40);
 
    /*
     ** SEND MSG HERE
     */

    /* Clear FSK buffer */
    if((vmwi_msg[1]+3)%FSK_DEPTH_TRIG)
    {
        remainder = 1;
    }
    else
    {
        remainder = 0;
    }

    ProSLIC_EnableCID(pState->ProObj);
    Delay(pProTimer,120);   /* 18 bytes of mark */
            
    /* Send preamble */
    for(i=0;i<30;i+=FSK_DEPTH_TRIG)
    {
        if(i >= 8)
        {
            waitForCIDDone(pState->ProObj);
        }
        ProSLIC_SendCID(pState->ProObj,vmwi_preamble,FSK_DEPTH_TRIG);
    }

    if (30%FSK_DEPTH_TRIG)
    {
        waitForCIDDone(pState->ProObj);
    }

    ProSLIC_SendCID(pState->ProObj,vmwi_preamble,30%FSK_DEPTH_TRIG);

    waitForCIDDone(pState->ProObj);

    /* Delay for idle mark bits */
    Delay(pProTimer,100);
            
    /* Send Message */
    csum = checkSum(vmwi_msg);
    vmwi_msg[vmwi_msg[1]+2] = csum;

    for(i=0;i<(vmwi_msg[1]+3);i+=FSK_DEPTH_TRIG)
    {
        if(i>=8)
        {
            waitForCIDDone(pState->ProObj);
        }
        ProSLIC_SendCID(pState->ProObj,&(vmwi_msg[i]),FSK_DEPTH_TRIG);
    }

    if(remainder)
    {
        waitForCIDDone(pState->ProObj);
            
        ProSLIC_SendCID(pState->ProObj,&(vmwi_msg[((vmwi_msg[1]+3)/FSK_DEPTH_TRIG)*FSK_DEPTH_TRIG]),(vmwi_msg[1]+3)%FSK_DEPTH_TRIG);
    }

    waitForCIDDone(pState->ProObj);

    /* Make sure the last byte is shifted out prior to disabling CID */
    Delay(pProTimer,buff_timeout);
    ProSLIC_DisableCID(pState->ProObj);
}


/**************************************/
void audioMenu(chanState *pState)
{
    char cmd[8]={0};
    int presetNum, nums = 0;
    int32 rxgain, txgain;
    uInt16 rxcount, txcount;

    do
    {
        printf("\n\n");
        printf("---------------------------------------------------------------\n");
        printf("------------------------   AUDIO MENU -------------------------\n");
        printf("---------------------------------------------------------------\n");  
        printf("0. Change Audio Gains\n");
        printf("1. Load Zsynth Preset\n");
        printf("2. Send Caller ID Stream\n");
        printf("3. Send VMWI Enable\n");
        printf("4. Send VMWI Disable\n");
        printf("5. Load PCM Preset\n");
        printf("6. Set PCM RX/TX Counts\n");
        printf("7. Enable PCM Bus\n");
        printf("8. Disable PCM Bus\n");
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

       // gets(cmd);

      //  switch(cmd[0]) 
		switch('2') 
        {
            case '0':
                presetNum = selectUserInt("Enter Zsynth Preset Number : ");
                ProSLIC_ZsynthSetup(pState->ProObj,presetNum);
                rxgain = (int32) selectUserInt("Enter Desired RX Gain (dB) : ");
                txgain = (int32) selectUserInt("Enter Desired TX Gain (dB) : ");
                ProSLIC_AudioGainSetup(pState->ProObj,rxgain,txgain,presetNum);
                printf("\n\n");
                printf("RX Gain = %d\n", rxgain);
                printf("TX Gain = %d\n", txgain);
                printf("Preset = %d\n", presetNum);
                break;

            case '1':
                printf("\n\n");
                presetNum = selectUserInt("Enter Zsynth Preset Number : ");
                ProSLIC_ZsynthSetup(pState->ProObj, presetNum);
                break;
            
            case '2':
                printf("\n\n");
                printf("Sending CID Stream...");
                sendCIDStream(pState);
				nums++;
                printf("nums = %d\n", nums);
                break;
                
            case '3':
                printf("\n\n");
                printf("Sending VMWI Enable...");
                sendVMWIStream(pState,1);
                printf("\n");
                break;
                
            case '4':
                printf("\n\n");
                printf("Sending VMWI Disable...");
                sendVMWIStream(pState,0);
                printf("\n");
                break;            
                
            case '5':
                printf("\n\n");            
                presetNum = selectUserInt("Enter PCM Preset Number : ");
                ProSLIC_PCMSetup(pState->ProObj,presetNum);
                printf("\n");
                break;    
                
            case '6':
                printf("\n\n");            
                rxcount = selectUserInt("Enter RX Count : ");
                txcount = selectUserInt("Enter TX Count : ");                
                ProSLIC_PCMTimeSlotSetup(pState->ProObj,rxcount,txcount);
                printf("\n");
                break;                

            case '7':
                printf("\n\n");
                printf("Starting PCM Bus\n");
                ProSLIC_PCMStart(pState->ProObj);
                printf("\n");                
                break;
                
            case '8':
                printf("\n\n");
                printf("Stopping PCM Bus\n");
                ProSLIC_PCMStop(pState->ProObj);
                printf("\n");                
                break;    

            default:
                break;                
                
        }
   } while(toupper(cmd[0]) != 'Q');
}
