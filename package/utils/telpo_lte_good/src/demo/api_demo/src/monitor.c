/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: monitor.c 4722 2015-02-09 22:38:05Z nizajerk $
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** line monitoring resources
**
*/

#include "api_demo.h"
#include "proslic.h"
#include "macro.h"

/*******************************************************/
static void lineMonitor(chanState *pState)
{
proslicMonitorType monitor;
	ProSLIC_SetPowersaveMode(pState->ProObj,PWRSAVE_DISABLE);
    Delay(pProTimer,50);
    ProSLIC_LineMonitor((pState->ProObj), &monitor);
    printf ("\n\n ProSLIC LINE MONITOR\n");
    printf ("-----------------------\n");
    printf ("VTR      = %d.%d  v\n", monitor.vtr/1000, abs(monitor.vtr - monitor.vtr/1000*1000));
    printf ("VTIP     = %d.%d  v\n", monitor.vtip/1000, abs(monitor.vtip - monitor.vtip/1000*1000));
    printf ("VRING    = %d.%d  v\n", monitor.vring/1000, abs(monitor.vring - monitor.vring/1000*1000));
    printf ("VLONG    = %d.%d  v\n", monitor.vlong/1000, abs(monitor.vlong - monitor.vlong/1000*1000));
    printf ("VBAT     = %d.%d  v\n", monitor.vbat/1000, abs(monitor.vbat - monitor.vbat/1000*1000));
    printf ("VDC      = %d.%d  v\n", monitor.vdc/1000, abs(monitor.vdc - monitor.vdc/1000*1000));
    printf ("ITR      = %d.%d  mA\n", monitor.itr/1000, abs(monitor.itr - monitor.itr/1000*1000));
    printf ("ITIP     = %d.%d  mA\n", monitor.itip/1000, abs(monitor.itip - monitor.itip/1000*1000));
    printf ("IRING    = %d.%d  mA\n", monitor.iring/1000, abs(monitor.iring - monitor.iring/1000*1000));
    printf ("ILONG    = %d.%d  mA\n", monitor.ilong/1000, abs(monitor.ilong - monitor.ilong/1000*1000));
    printf ("P_HVIC   = %d mW\n", monitor.p_hvic);   
}

/*******************************************************/
void testMonitorMenu(chanState *pState)
{
    char cmd[8]={0};
#ifdef TSTIN_SUPPORT
    int testResult = 0;
#endif
#ifdef VDAA_SUPPORT
    int ovl;
    uInt8 vloop=0;
    uInt16 iloop=0;
    int fxo_ch_offset;

    /* Allow for standalone 3050 testing */
    if(pState[0].ProObj->channelType == DAA)
        fxo_ch_offset=0;
    else
        fxo_ch_offset=1;
#endif

	do
    {
        printf("\n\n");
        printf("---------------------------------------------------------------\n");
        printf("------------------   TEST & MONITOR MENU ----------------------\n");
        printf("---------------------------------------------------------------\n");  
        printf("0. FXS Line Monitor\n");
        printf("\n");
#ifdef TSTIN_SUPPORT
        printf("1. (Inward) Toggle PCM Loopback (%s)\n",GET_ENABLED_STATE(pState->pTstin->pcmLpbkTest.pcmLpbkEnabled));
        printf("2. (Inward) DC Feed Test\n"); 
        printf("3. (Inward) Ringing Test\n"); 
        printf("4. (Inward) Battery Test\n"); 
        printf("5. (Inward) Audio Test\n"); 
        printf("6. Print Inward Test Limits\n");
#endif
#ifdef VDAA_SUPPORT
        if(pState->deviceType == SI3217X_TYPE)
        {
            printf("---------------\n");
            printf("9. FXO Line Monitor\n");
        }
#endif
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);
        
        switch(*cmd) 
        {
            case '0':
                lineMonitor(pState);
                break;
#ifdef TSTIN_SUPPORT
		    case '1':
                printf("\n");
                if(pState->pTstin->pcmLpbkTest.pcmLpbkEnabled)
				{
				    ProSLIC_testInPCMLpbkDisable(pState->ProObj,pState->pTstin);
					printf("PCM Loopback Disabled\n");
				}
                else
				{
                    ProSLIC_testInPCMLpbkEnable(pState->ProObj,pState->pTstin);
					printf("PCM Loopback Enabled\n");
				}
				break;
			case '2':
				printf("\n");
				testResult = ProSLIC_testInDCFeed(pState->ProObj, pState->pTstin);

				if(testResult == RC_TEST_PASSED)
				{
					printf("\nTEST PASSED!!\n");
				}
				else if(testResult == RC_LINE_IN_USE)
				{	
					printf("\nLINE IN USE!!\n");
				}
				else
				{
					printf("\nTEST FAILED!!\n");
				}
				break;
		    case '3':
				printf("\n\n");
				testResult = ProSLIC_testInRinging(pState->ProObj, pState->pTstin);

				if(testResult == RC_TEST_PASSED)
				{
					printf("\nTEST PASSED!!\n");
				}
				else
				{
					printf("\nTEST FAILED!!\n");
				}
				break;
		    case '4':
				printf("\n\n");
				testResult = ProSLIC_testInBattery(pState->ProObj, pState->pTstin);

				if(testResult == RC_TEST_PASSED)
				{
					printf("\nTEST PASSED!!\n");
				}
				else
				{
					printf("\nTEST FAILED!!\n");
				}
				break;
		    case '5':
				printf("\n\n");
				testResult = ProSLIC_testInAudio(pState->ProObj, pState->pTstin);

				if(testResult == RC_TEST_PASSED)
				{
					printf("\nTEST PASSED!!\n");
				}
				else
				{
					printf("\nTEST FAILED!!\n");
				}
				break;

			case '6':
				ProSLIC_testInPrintLimits(pState->ProObj,pState->pTstin);
				break;
#endif

            case '9':
#ifdef VDAA_SUPPORT
                if(pState->ProObj->channelType == DAA)
                {
                    ovl = Vdaa_ReadLinefeedStatus (pState[fxo_ch_offset].ProObj,&vloop, &iloop);
                    printf ("\n\nVDAA LINE MONITOR\n");
                    printf ("-----------------------\n");
                    printf ("VTR      = %d  v\n", vloop);
                    printf ("ILOOP    = %d  mA\n", iloop);
				    if(ovl == RC_VDAA_ILOOP_OVLD)
					    printf("#### OVL ####\n");
                }
                else
                {
                    printf("\n\nNOT AN FXO CHANNEL\n");
                }
#endif
                break;

            default:
                break;
        }
    } while(toupper(cmd[0]) != 'Q');
}

