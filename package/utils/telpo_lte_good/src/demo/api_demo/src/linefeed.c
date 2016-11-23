/*
** Copyright (c) 2013-2014 by Silicon Laboratories, Inc.
**
** $Id: linefeed.c 4609 2014-12-08 17:04:16Z nizajerk $
**
** linefeed.c
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** linefeed resources
**
*/

#include "api_demo.h"
#include "proslic.h"
#include "macro.h"

const char *lfStateStr[] = {
    "LF_OPEN",
    "LF_FWD_ACTIVE",
    "LF_FWD_OHT",
    "LF_TIP_OPEN",
    "LF_RINGING",
    "LF_REV_ACTIVE",
    "LF_REV_OHT",
    "LF_RING_OPEN"
};

/********************************************/
void linefeedMenu(chanState *pState)
{
char cmd[8]={0};
char n[2] = {0};
uInt8 reg;
uInt8 newLfState;
int presetNum = 0;
uInt8 abrupt = 0;

    do
    {
        printf("\n\n");
        printf("---------------------------------------------------------------\n");
        printf("----------------------   LINEFEED MENU ------------------------\n");
        printf("---------------------------------------------------------------\n");  
        printf("0. Read Linefeed State\n");
        printf("1. Change Linefeed State\n");
        printf("2. Load DC Feed Preset\n");  
        printf("3. POLREV (%s}\n", GET_ACTIVE_STATE(((ReadReg(pProHW,pChannel,31)&0x06))==0x02)); 
        printf("4. WINK (%s}\n", GET_ACTIVE_STATE(((ReadReg(pProHW,pChannel,31)&0x06))==0x06));
		printf("5. Toggle POLREV Type (%s)\n", GET_ABRUPT_STATE(abrupt));
		printf("6. Toggle PWRSAVE Mode (%s)\n", GET_ENABLED_STATE((ReadReg(pProHW,pChannel,ENHANCE)&0x10)));
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);
        
        switch(cmd[0]) 
        {
            case '0':
                printf("\n\n");
                reg = ReadReg(pProHW,pChannel,30);
                printf("LF State = %s (0x%02X)\n", lfStateStr[reg&0x0F],reg);
                break;

		    case '1':
                printf("\n\n");
                printf("0.   OPEN\n");
                printf("1.   FWD ACTIVE\n");
                printf("2.   FWD OHT\n");
                printf("3.   TIP OPEN\n");
                printf("4.   RINGING\n");
                printf("5.   REV ACTIVE\n");
                printf("6.   REV OHT\n");
                printf("7.   RING OPEN\n");
	            printf("----------------\n");
                printf("Select LINEFEED State or 'q' to cancel: ");
                gets(n);
		        newLfState = atoi(n);  
                if((newLfState >=0)&&(newLfState <= 7))
                    ProSLIC_SetLinefeedStatus(pState->ProObj,newLfState);
				break;

            case '2':
                printf("\n\n");
                presetNum = selectUserInt("Enter DC Feed Preset : ");
                ProSLIC_DCFeedSetup(pState->ProObj, presetNum);
                break;

            case '3':
                printf("\n\n");
                reg = ReadReg(pProHW,pChannel,31);
                if(reg&0x02)
                {
                    printf("POLLREV STOP\n");
                    ProSLIC_PolRev(pState->ProObj,abrupt,POLREV_STOP);
                }
                else
                {
                    printf("POLLREV START\n");
                    ProSLIC_PolRev(pState->ProObj,abrupt,POLREV_START);
                }
                break;

            case '4':
                printf("\n\n");
                reg = ReadReg(pProHW,pChannel,31);
                if((reg&0x06)==0x06)
                {
                    printf("WINK STOP\n");
                    ProSLIC_PolRev(pState->ProObj,abrupt,WINK_STOP);
                }
                else
                {
                    printf("WINK START\n");
                    ProSLIC_PolRev(pState->ProObj,abrupt,WINK_START);
                }
                break;

			case '5':
				if(abrupt == 0)
					abrupt = 1;
				else
					abrupt = 0;
				break;

			case '6':
				if(ReadReg(pProHW,pState->ProObj->channel,ENHANCE)&0x10)
				{
					ProSLIC_SetPowersaveMode(pState->ProObj,PWRSAVE_DISABLE);
				}
				else
				{
					ProSLIC_SetPowersaveMode(pState->ProObj,PWRSAVE_ENABLE);
				}
				break;

            default:
                break;

        }
   }while(toupper(cmd[0]) != 'Q');
}
