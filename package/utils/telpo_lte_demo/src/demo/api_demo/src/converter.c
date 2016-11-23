/*
** Copyright (c) 2013-2014 by Silicon Laboratories, Inc.
**
** $Id: converter.c 4609 2014-12-08 17:04:16Z nizajerk $
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
#include "macro.h"


/**********************************/
void converterMenu(chanState *pState) {
char cmd[8]={0};
int status = 0;
ramData data;

	do
    {
        printf("\n\n");
        printf("---------------------------------------------------------------\n");
        printf("----------------------   CONVERTER MENU -----------------------\n");
        printf("---------------------------------------------------------------\n");  
        printf("\n\n");
        printf("Select Converter State\n");
        printf("0. Power Up Converter\n");
        printf("1. Power Down Converter\n");
        printf("2. Read DCDC_STATUS\n"); 
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);

        switch(cmd[0])
        {
            case '0':
                status = ProSLIC_PowerUpConverter(pState->ProObj);
                if(status != RC_NONE)
                {
                    printf("\nDCDC Powerup Failed (status = %d)\n", status);
                }
                break;

            case '1':
                ProSLIC_PowerDownConverter(pState->ProObj);
                break;

            case '2':
                data = ReadRAM(pProHW,pState->ProObj->channel,DCDC_STATUS);
                printf("DCDC_STATUS = 0x%08X\n", data);
                break;

            default:
                break;
        }
    }while(toupper(cmd[0]) != 'Q');
}
