/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: debug.c 4722 2015-02-09 22:38:05Z nizajerk $
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** debugging resources
**
*/
#include "api_demo.h"
#include "macro.h"

#include <stdio.h>

/******************************************/
void dump_memory(chanState *pState,const char* filename)
{
FILE *fp;
int i;

	if((fp = fopen(filename,"w"))==NULL)
	{
		printf("\nERROR OPENING %s!!\n", filename);
		return;
	}
	fprintf(fp,"#############\n");
	fprintf(fp,"Registers\n");
	fprintf(fp,"#############\n");
	for(i=0; i<128; i++)
	{
		fprintf(fp,"REG %d = 0x%02X\n",(uInt8)i,ReadReg(pProHW,pState->ProObj->channel,(uInt8)i));
	}
	fprintf(fp,"#############\n");
	fprintf(fp,"RAM\n");
	fprintf(fp,"#############\n");
	for(i=0; i<1023; i++)
	{
		fprintf(fp,"RAM %d = 0x%08X\n", (uInt16)i,ReadRAM(pProHW,pState->ProObj->channel,(uInt16)i));
	}
	ProSLIC_SetUserMode(pState,1,0);
	fprintf(fp,"#############\n");
	fprintf(fp,"MMREG\n");
	fprintf(fp,"#############\n");
	for(i=1024; i<1648; i++)
	{
		fprintf(fp,"RAM %d = 0x%08X\n", (uInt16)i,ReadRAM(pProHW,pState->ProObj->channel,(uInt16)i));
	}

	fclose(fp);
}

/**************************************************/
void debugMenu(chanState *pState)
{
char cmd[8]={0};
char regNumStr[32] = {0};
char ramNumStr[32] = {0};
char regDataStr[32] = {0};
char ramDataStr[32] = {0};
uInt32 iRamData;
uInt8  iRegData;
char resp[60];


	ProSLIC_SetUserMode(pState,1,0);

	do
    {
        printf("\n\n");
        printf("---------------------------------------------------------------\n");
        printf("------------------------   DEBUG MENU -------------------------\n");
        printf("---------------------------------------------------------------\n"); 
        printf("0. Toggle Debug Mode (%s)\n", GET_ENABLED_STATE(pState->ProObj->debugMode));
		    printf("1. Read Device Information\n");	
        printf("2. Register/RAM Dump\n");
        printf("3. Read Register\n");
        printf("4. Write Register\n");
        printf("5. Read RAM\n");
        printf("6. Write RAM\n");
        printf("7. Place part into/out of free run mode (if supported)\n");
        printf("8. Reset EVB\n");
        printf("9. Change VMB1/2 settings\n");
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);

        printf("\n\n");
        fflush(stdout);

        switch(cmd[0]) 
        {
            case '0':
				if(pState->ProObj->debugMode == 0)
				{
					pState->ProObj->debugMode = 1;
				}
				else
				{
					pState->ProObj->debugMode = 0;
				}
                break;

            case '1':

                break;	
				
            case '2':
				printf("Enter Log File Name -> ");
				gets(resp);
				dump_memory(pState,resp);
                break;
				
            case '3':
				printf("Enter Register Address -> ");
				gets(regNumStr);
				iRegData = ReadReg(pState->ProObj,pState->ProObj->channel,atoi(regNumStr));
				printf("\n\nReg %d  =  0x%02x\n",atoi(regNumStr),iRegData);
                break;

            case '4':
				printf("Enter Register Address (dec) -> ");
				gets(regNumStr);
				printf("Enter Register Value (hex) -> ");
				gets(regDataStr);
                iRegData = (uInt8)strtol(regDataStr,NULL,16);
				WriteReg(pState->ProObj,pState->ProObj->channel,atoi(regNumStr),iRegData);
                break;

			case '5':
				printf("Enter RAM Address (dec) -> ");
				gets(ramNumStr);
				iRamData = ReadRAM(pState->ProObj,pState->ProObj->channel,atoi(ramNumStr));
				printf("\n\nRAM %d  =  0x%08x\n",atoi(ramNumStr),iRamData);
				break;

			case '6':
				printf("Enter RAM Address (dec) -> ");
				gets(ramNumStr);
				printf("Enter RAM Value (hex) -> ");
        gets(ramDataStr);
        iRamData = (uInt32)strtol(ramDataStr,NULL,16);
				WriteRAM(pState->ProObj,pState->ProObj->channel,atoi(ramNumStr),iRamData);
				break;

			case '7': /* Free run */
        *regNumStr = '?';
        do
        {
          printf("Place part into free run mode (y/n) -> ");
          gets(regNumStr);
        } while( (*regNumStr != 'y') && (*regNumStr != 'Y')
          && (*regNumStr != 'N') && (*regNumStr != 'n'));

        if( (*regNumStr == 'Y') || (*regNumStr == 'y'))
        {
          ProSLIC_PLLFreeRunStart(pState->ProObj);
        }
        else
        {
          ProSLIC_PLLFreeRunStop(pState->ProObj);
        }
        break;

      case '8':
        SiVoice_Reset(pState->ProObj);
        break;

      case '9':   
        fflush(stdout);
        vmbSetup(pState->ProObj->deviceId->ctrlInterface);
        break;

			default:
				break;
        }
   } while(toupper(cmd[0]) != 'Q');
}
