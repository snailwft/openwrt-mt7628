/*
** Copyright (c) 2013-2014 by Silicon Laboratories, Inc.
**
** $Id: ringing.c 4722 2015-02-09 22:38:05Z nizajerk $
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** ringing resources
**
*/

#include "api_demo.h"
#include "macro.h"


/********************************************/
int get_ring_osc_state(chanState *pState)
{
uInt8 regtmp;

    regtmp = ReadReg(pProHW,pState->ProObj->channel,RINGCON);
    if(regtmp & 0x18)
        return 1;
    else
        return 0;
}


/********************************************/
void get_ring_osc_timers(chanState *pState, int *ton, int *toff)
{
uInt16 tmp;

    tmp = (ReadReg(pProHW,pState->ProObj->channel,RINGTAHI)&0x00FF)<<8;
    tmp |= ReadReg(pProHW,pState->ProObj->channel,RINGTALO);
    *ton = (int)(tmp / 8);

    tmp = (ReadReg(pProHW,pState->ProObj->channel,RINGTIHI)&0x00FF)<<8;
    tmp |= ReadReg(pProHW,pState->ProObj->channel,RINGTILO);
    *toff = (int)(tmp / 8);
}


/********************************************/
void set_ring_osc_timers(chanState *pState, int ton, int toff)
{
int tmp;
uInt8 reg_val;

    /* Active Time */
    tmp = ton * 8;
    reg_val = (uInt8)(tmp&0x00FF);
    WriteReg(pProHW,pState->ProObj->channel, RINGTALO, reg_val);
    reg_val = (uInt8)((tmp&0xFF00)>>8);
    WriteReg(pProHW,pState->ProObj->channel, RINGTAHI, reg_val);
    /* Inactive Time */
    tmp = toff * 8;
    reg_val = (uInt8)(tmp&0x00FF);
    WriteReg(pProHW,pState->ProObj->channel, RINGTILO, reg_val);
    reg_val = (uInt8)((tmp&0xFF00)>>8);
    WriteReg(pProHW,pState->ProObj->channel, RINGTIHI, reg_val);
}

/********************************************/
int toggle_ring_osc(chanState *pState)
{
uInt8 regtmp;

    regtmp = ReadReg(pProHW,pState->ProObj->channel,RINGCON);
    if(regtmp & 0x18)
    {
        WriteReg(pProHW,pState->ProObj->channel,RINGCON,regtmp&~0x18);
        return 0;
    }
    else
    {
        WriteReg(pProHW,pState->ProObj->channel,RINGCON,regtmp|0x18);
        return 1;
    }
}



/***************************************/
void ringingMenu(chanState *pState)
{
    char cmd[8]={0};
    int val;
    int ton = 2000;
    int toff = 4000;
    uInt8 ring_osc_en = 0;
    
	do
    {
        ring_osc_en = get_ring_osc_state(pState);
        get_ring_osc_timers(pState,&ton,&toff);

        printf("\n\n");
        printf("---------------------------------------------------------------\n");
        printf("-----------------------   RINGING MENU ------------------------\n");
        printf("---------------------------------------------------------------\n");  
        printf("0. Start Ringing\n");
        printf("1. Stop Ringing\n");
        printf("2. Load Ringing Preset\n"); 
        printf("3. Toggle Ring Cadence Timers (%s)\n", GET_ENABLED_STATE(ring_osc_en));
        printf("4. Set Ringing Cadence (%d ms, %d ms)\n",ton,toff); 
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);

        printf("\n\n");
        switch(cmd[0]) 
        {
        case '0':
			ProSLIC_RingStart(pState->ProObj);
            break;

        case '1':
            ProSLIC_RingStop(pState->ProObj);
            break;

        case '2':
            val = selectUserInt("Enter Ringing Preset to Load : ");      
            ProSLIC_RingSetup(pState->ProObj,val);
            break;

        case '3':
            toggle_ring_osc(pState);
            break;

        case '4':
            ton = selectUserInt("Enter Ring Active Time (ms) : ");      
            toff = selectUserInt("Enter Ring Inactive Time (ms) : ");  
            set_ring_osc_timers(pState,ton,toff);
            break;

        default:
            break;

       }
   } while(toupper(cmd[0]) != 'Q');
}


