/*
** Copyright (c) 2013-2014 by Silicon Laboratories, Inc.
**
** $Id: pm_mwi.c 4886 2015-04-20 23:16:13Z elgeorge $
**
** pm_mwi.c
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** pulse metering and message waiting resources
**
*/
#include "api_demo.h"
#include "proslic.h"
#include "macro.h"



/********************************************/
int get_pm_state(chanState *pState)
{
uInt8 regtmp;

    regtmp = ReadReg(pProHW,pState->ProObj->channel,PMCON);
    if(regtmp & 0x01)
        return 1;
    else
        return 0;
}
/****************************************/
static int read_lcrmask_mwi(chanState *pState)
{
int32 ram_val;
uInt16 ram_addr;

	if((pState->ProObj->deviceId->chipType >= SI32260)&&(pState->ProObj->deviceId->chipType <= SI32268))
	{
		ram_addr = 786;
	}
	else
	{
		ram_addr = 987;
	}

	ram_val = ProSLIC_ReadRAM(pState->ProObj,ram_addr);
	ram_val /= 65536L;
	return ram_val;
}

/****************************************/
void pmMwiMenu(chanState *pState)
{
char cmd[8]={0};
uInt8 val;
int vpk = 85;
int mask_time = 70;
int presetNum;
int mwi_toggle_interval = 1000;  /* ms */
uInt8 mwi_enable = 0;
uInt8 mwi_state = 0;
uInt8 metering_enable = 0;


	do
    {
        metering_enable = get_pm_state(pState);

        printf("\n\n");
        printf("------------------------------------------------------------\n");
        printf("-------------------   SPECIAL TEST MENU --------------------\n");
        printf("------------------------------------------------------------\n");  
        printf("0. MWI Setup (%d v, %d ms)\n", vpk, mask_time);
        printf("1. Toggle MWI Enable (%s)\n",GET_ENABLED_STATE(mwi_enable));
        printf("2. Toggle MWI State (%s)\n", GET_ACTIVE_STATE(mwi_state));
        printf("3. Set MWI Toggle Interval (%d ms)\n", mwi_toggle_interval);
        printf("4. Run Auto MWI\n");
        printf("5. Load Pulse Metering Preset\n");
        printf("6. Toggle Pulse Metering Enable (%s)\n", GET_ENABLED_STATE(metering_enable));
        printf("7. Start Pulse Metering\n");
        printf("8. Stop Pulse Metering\n");
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

		gets(cmd);

        switch(cmd[0]) 
        {
            case '0':
                vpk = selectUserInt("Enter MWI Vpk (v) : "); 
                mask_time = selectUserInt("Enter MWI Mask Time (50 to 120 ms) : ");
                ProSLIC_MWISetup(pState->ProObj,vpk,mask_time);
                break;

            case '1':
				val = ProSLIC_ReadReg(pState->ProObj,USERSTAT);
				if(val & 0x04)
                {
					ProSLIC_MWIDisable(pState->ProObj);
                    mwi_enable = 0;
                }
				else
				{
					ProSLIC_MWIEnable(pState->ProObj);
                    mwi_enable = 1;
				}
                break;

            case '2':
				if(ProSLIC_GetMWIState(pState->ProObj) == SIL_MWI_FLASH_OFF)
                {
					ProSLIC_SetMWIState(pState->ProObj,SIL_MWI_FLASH_ON);
                    mwi_state = 1;
                }
				else
                {
					ProSLIC_SetMWIState(pState->ProObj,SIL_MWI_FLASH_OFF);
                    mwi_state = 0;
                }
				break;

            case '3':
                    mwi_toggle_interval = selectUserInt("Enter MWI Interval (ms) : ");
                break;

            case '4':
                printf("\n\nHit Any Key To Stop MWI\n\n");
				ProSLIC_MWIEnable(pState->ProObj);
				while(!(kbhit()))
				{
					ProSLIC_SetMWIState(pState->ProObj,SIL_MWI_FLASH_OFF);
                    printf("MWI Flash OFF\n");
					Delay(pProTimer,mwi_toggle_interval);
					ProSLIC_SetMWIState(pState->ProObj,SIL_MWI_FLASH_ON);
                    printf("MWI Flash ON\n");
					Delay(pProTimer,mwi_toggle_interval);
				}
				ProSLIC_MWIDisable(pState->ProObj);
                break;

            case '5':
                presetNum = selectUserInt("Enter PM Preset : "); 
				ProSLIC_PulseMeterSetup(pState->ProObj,presetNum);
                break;

            case '6':
                if(get_pm_state(pState))
                {
				    ProSLIC_PulseMeterDisable(pState->ProObj);
                }
                else
                {
				    ProSLIC_PulseMeterEnable(pState->ProObj);
                    val = ProSLIC_ReadReg(pState->ProObj,PMCON);
                    if(!(val&0x01))
                    {
                        printf("\nPulse Metering not supported\n");
                    }
                }
                break;

            case '7':
                ProSLIC_PulseMeterStart(pState->ProObj);
                break;

            case '8':
				ProSLIC_PulseMeterStop(pState->ProObj);
                break;

            default:
                break;
        }
   } while(toupper(cmd[0]) != 'Q');
}
