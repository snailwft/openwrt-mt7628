/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: tonegen.c 4722 2015-02-09 22:38:05Z nizajerk $
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** tone generator resources
**
*/

#include "api_demo.h"
#include "macro.h"

/********************************************/
int get_tonegen_osc_state(chanState *pState)
{
uInt8 regtmp;

    regtmp = ReadReg(pProHW,pState->ProObj->channel,OCON);
    if(regtmp & 0x77)
        return 1;
    else
        return 0;
}

/********************************************/
int toggle_tonegen_osc(chanState *pState)
{
    if(get_tonegen_osc_state(pState))
    {
        WriteReg(pProHW,pState->ProObj->channel,OCON,0x00);
        return 0;
    }
    else
    {
        WriteReg(pProHW,pState->ProObj->channel,OCON,0x77);
        return 1;
    }
}

/********************************************/
void get_tonegen_osc_timers(chanState *pState, int *ton, int *toff)
{
uInt16 tmp;

    /* Read only from O1 since timers identical in this demo */
    tmp = (ReadReg(pProHW,pState->ProObj->channel,O1TAHI)&0x00FF)<<8;
    tmp |= ReadReg(pProHW,pState->ProObj->channel,O1TALO);
    *ton = (int)(tmp / 8);

    tmp = (ReadReg(pProHW,pState->ProObj->channel,O1TIHI)&0x00FF)<<8;
    tmp |= ReadReg(pProHW,pState->ProObj->channel,O1TILO);
    *toff = (int)(tmp / 8);
}
/********************************************/
void set_tonegen_osc_timers(chanState *pState, int ton, int toff)
{
int tmp;
uInt8 reg_val;

    /* Active Time */
    tmp = ton * 8;
    reg_val = (uInt8)(tmp&0x00FF);
    WriteReg(pProHW,pState->ProObj->channel, O1TALO, reg_val);
    WriteReg(pProHW,pState->ProObj->channel, O2TALO, reg_val);
    reg_val = (uInt8)((tmp&0xFF00)>>8);
    WriteReg(pProHW,pState->ProObj->channel, O1TAHI, reg_val);
    WriteReg(pProHW,pState->ProObj->channel, O2TAHI, reg_val);
    /* Inactive Time */
    tmp = toff * 8;
    reg_val = (uInt8)(tmp&0x00FF);
    WriteReg(pProHW,pState->ProObj->channel, O1TILO, reg_val);
    WriteReg(pProHW,pState->ProObj->channel, O2TILO, reg_val);
    reg_val = (uInt8)((tmp&0xFF00)>>8);
    WriteReg(pProHW,pState->ProObj->channel, O1TIHI, reg_val);
    WriteReg(pProHW,pState->ProObj->channel, O2TIHI, reg_val);
}
/*************************************/
void toneGenMenu(chanState *pState)
{
    char cmd[8]={0};
    int presetNum;
    int tone_gen_timer_en = 0;
    int ton = 500;
    int toff = 500;


	do
    {
        tone_gen_timer_en = get_tonegen_osc_state(pState);
        get_tonegen_osc_timers(pState,&ton,&toff);
        printf("\n\n");
        printf("---------------------------------------------------\n");
        printf("------------------ TONEGEN MENU -------------------\n");
        printf("---------------------------------------------------\n");  
        printf("0. Load Tone Generator Preset\n");
        printf("1. Enable Tone Generator\n"); 
        printf("2. Disable Tone Generator\n"); 
        printf("3. Set Tone Gen Timers (%d ms, %d ms)\n", ton, toff);
        printf("4. Toggle Tone Gen Timers (%s)\n", GET_ENABLED_STATE(tone_gen_timer_en));
        printf("----------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);
        
        switch(cmd[0]) 
        {
            case '0':
                printf("\n\n");
                presetNum = selectUserInt("Enter ToneGen Preset : ");
                ProSLIC_ToneGenSetup(pState->ProObj, presetNum);
                break;

			case '1':
                printf("\n\n");
				ProSLIC_ToneGenStart(pState->ProObj,0);
				break;
			case '2':
                printf("\n\n");
				ProSLIC_ToneGenStop(pState->ProObj);
				break;

            case '3':
                ton = selectUserInt("Enter ToneGen Active Time (ms) : ");
                toff = selectUserInt("Enter ToneGen Inactive Time (ms) : ");
                set_tonegen_osc_timers(pState,ton,toff);
                break;

            case '4':
                toggle_tonegen_osc(pState);
                break;

            default:
                break;
		}
   } while(toupper(cmd[0]) != 'Q');
}
