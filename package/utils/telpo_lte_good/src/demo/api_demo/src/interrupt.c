/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: interrupt.c 4904 2015-04-23 15:41:51Z nizajerk $
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains example implementation and use of ProSLIC API 
** interrupt generating resources
**
*/
#include "api_demo.h"
#include "macro.h"
#include "user_handler.h"
#define P_HVIC_LPF P_Q1_D

enum {
    ONHOOK_IDLE,
    OFFHOOK_IDLE,
    ONHOOK_DECODE_PD,
    ONHOOK_DECODE_HF,
    OFFHOOK_DECODE_PD,
    OFFHOOK_WAIT_IDLE
};


/* Pulse Dial/Hookflash Config */
static hookChange_Cfg hook_change_cfg = { 20,    /* Min Break Time */
                                          80,    /* Max Break Time */
                                          20,    /* Min Make Time */
                                          80,    /* Max Make Time */
                                          90,    /* Min Inter-digit Delay */
                                          100,   /* Min hookflash */
                                          800,   /* Max hookflash */
                                          850 }; /* Min Idle Time */
#define MAX_INT_STRINGS 38
char *intMapStrings[] = 
{
    "IRQ_OSC1_T1",
    "IRQ_OSC1_T2",
    "IRQ_OSC2_T1",
    "IRQ_OSC2_T2",
    "IRQ_RING_T1",
    "IRQ_RING_T2",
    "IRQ_PM_T1",
    "IRQ_PM_T2",
    "IRQ_FSKBUF_AVAIL", /**< FSK FIFO depth reached */
    "IRQ_VBAT", 
    "IRQ_RING_TRIP", /**< Ring Trip detected */
    "IRQ_LOOP_STATUS",  /**< Loop Current changed */
    "IRQ_LONG_STAT",
    "IRQ_VOC_TRACK",
    "IRQ_DTMF",         /**< DTMF Detected - call @ref ProSLIC_DTMFReadDigit to decode the value */
    "IRQ_INDIRECT",     /**< Indirect/RAM access completed */
    "IRQ_TXMDM",
    "IRQ_RXMDM",
    "IRQ_PQ1",          /**< Power alarm 1 */
    "IRQ_PQ2",          /**< Power alarm 2 */
    "IRQ_PQ3",          /**< Power alarm 3 */
    "IRQ_PQ4",          /**< Power alarm 4 */
    "IRQ_PQ5",          /**< Power alarm 5 */
    "IRQ_PQ6",          /**< Power alarm 6 */
    "IRQ_RING_FAIL",
    "IRQ_CM_BAL",
    "IRQ_USER_0",
    "IRQ_USER_1",
    "IRQ_USER_2",
    "IRQ_USER_3",
    "IRQ_USER_4",
    "IRQ_USER_5",
    "IRQ_USER_6",
    "IRQ_USER_7",
    "IRQ_DSP",
    "IRQ_MADC_FS",
    "IRQ_P_HVIC",
    "IRQ_P_THERM", /**< Thermal alarm */
    "IRQ_P_OFFLD"
};


/*************************************/
void irq_demo_handle_interrupt(chanState *pState, ProslicInt interrupt)
{

    switch(interrupt)
    {
    case IRQ_LOOP_STATUS:
        ProSLIC_ReadHookStatus(pState->ProObj,&(pState->is_offhook));
        pState->hook_change = TRUE;

        break;

    default:
        break;

    }
}


/*************************************/
void irq_demo_check_interrupts(chanState *pState)
{
    proslicIntType irqs;
    ProslicInt arrayIrqs[MAX_PROSLIC_IRQS];

    irqs.irqs = arrayIrqs;

    if (ProSLIC_GetInterrupts(pState->ProObj, &irqs) != 0)
    {
        unsigned int i;
        /* Iterate through the interrupts and handle */
        for(i=0 ; i<irqs.number;  i++)
        {
            irq_demo_handle_interrupt(pState,irqs.irqs[i]);
        }
    }
}

#if 1
void interrupt_handler(chanState *arg)
{
	chanState *pState = arg;
	uInt8 reg;
	int poll_interval = 10;   /* ms */
	proslicIntType irqs;
	ProslicInt arrayIrqs[MAX_PROSLIC_IRQS];
	int onhook_state = LF_FWD_ACTIVE;
	uInt8 pDigit;
	irqs.irqs = arrayIrqs;
	irqs.number = 0;
	int i, ret = 0;
	Delay(pProTimer, poll_interval);
	//pthread_mutex_lock(&cid_mutex);
	ret = ProSLIC_GetInterrupts(pState->ProObj, &irqs);
	//pthread_mutex_unlock(&cid_mutex);
	if (ret == RC_REINIT_REQUIRED)
	{
		dbug_log(__FILE__,__LINE__,"Soft Reset Required\n");
		//exit(1); // 软件重启
		longjmp(env,1);	
		//goto error_exit;
	}
	else if (irqs.number >0)		// 表示有多少个中断产生
	{			
		for (i = 0; i < irqs.number; i++)
		{
#ifndef _DBUG_
			printf("\nInterrupt Code ->  %d", irqs.irqs[i]);        
			dbug_log(__FILE__,__LINE__, "\nInterrupt Code ->  %d", irqs.irqs[i]);
#endif			
			if (irqs.irqs[i] < MAX_INT_STRINGS)
			{
#ifndef _DBUG_
				printf(" = %s\n", intMapStrings[irqs.irqs[i]]);
				dbug_log(__FILE__,__LINE__, " = %s\n", intMapStrings[irqs.irqs[i]]);
#endif					
			}

			/* If returning onhook, set to default onhook state */
			if (irqs.irqs[i] == IRQ_LOOP_STATUS)
			{
				//proslic_read_hook_status(ports[0].ProObj,&reg);
				ProSLIC_ReadHookStatus(pState->ProObj,&reg);
				if (reg == PROSLIC_ONHOOK) // 0 表示挂机
				{
					ProSLIC_SetLinefeedStatus(pState->ProObj, onhook_state); //LF_FWD_ACTIVE
					set_slic_state(PROSLIC_ONHOOK);
#ifdef _DBUG_
					printf("ONHOOK\n");
#endif
					dbug_log(__FILE__,__LINE__, "ONHOOK\n");
					//打开指示灯，不闪
					//ioctl(fd4, LIGHT_UP);
					slic_light = LIGHT_UP;
				}
				else
				{ 
					set_slic_state(PROSLIC_OFFHOOK);	// 1表示摘机
#ifdef _DBUG_
					printf("OFFHOOK\n");
#endif
					dbug_log(__FILE__,__LINE__, "OFFHOOK\n");
					//指示灯闪
					slic_light = LIGHT_BLINK_FAST;
				}
			}         
#ifndef DTMF_DECODE
			else if (irqs.irqs[i] == IRQ_DTMF)
			{
				/*
				*  1-9 = no remapping required
				*  0   = 0xA
				*  *   = 0xB
				*  #   = 0xC
				*  A-C = 0xD-0xF
				*  D   = 0x0
				*/
				ProSLIC_DTMFReadDigit(pState->ProObj, &pDigit);	//读出dtmf数据
				pthread_mutex_lock(&dtmf_mutex);
				recv_dtmf(pDigit);
				pthread_mutex_unlock(&dtmf_mutex);								
			}
#endif
		}
	}

	return ;
}
#endif

/***************************************/

void interruptMenu(chanState *pState)
{
    char cmd[8]={0};
    char regDataStr[32] = {0};
    uInt8 reg;
    uInt8 done = 0;
    int poll_interval = 1;   /* ms */
    proslicIntType irqs;
    ProslicInt arrayIrqs[MAX_PROSLIC_IRQS];
    int onhook_state = LF_FWD_ACTIVE;
    timeStamp hook_time;
    int elapsed_time;
    int digit = 0;
    BOOLEAN valid_digit_pending = FALSE;
    BOOLEAN valid_hook_flash_pending = FALSE;
    uInt8 digit_buffer[32];
    uInt8 digit_count = 0;


    irqs.irqs = arrayIrqs;

	do
    {
        printf("\n\n");
        printf("-----------------------------------------------------\n");
        printf("------------------ INTERRUPT MENU -------------------\n");
        printf("-----------------------------------------------------\n");  
        printf("0. Read IRQ Regs\n");
        printf("1. Read IRQENx Regs\n");
        printf("2. Enable Interrupts\n"); 
        printf("3. Manually Enable/Disable Interrupts\n");
        printf("4. Disable Interrupts\n"); 
		printf("5. Wait For Interrupt (single)\n");
        printf("6. Wait For Interrupt (continuous)\n");
        printf("7. Set Continuous Poll Interval (%d ms)\n",poll_interval);
		printf("8. Set Default Onhook State (%d)\n", onhook_state);
		printf("9. Pulse Digit Decode\n");
        printf("---------------------------------------------------------------\n\n");
        printf("Select Menu Item or 'q' to quit: ");

        gets(cmd);
        
        switch(cmd[0]) 
        {
            case '0':
				printf("\n\nINTERRUPT STATUS\n");
				printf("----------------------\n");
				printf("IRQ     =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQ));
				printf("IRQ0    =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQ0));
				printf("IRQ1    =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQ1));
				printf("IRQ2    =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQ2));
				printf("IRQ3    =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQ3));
				break;

			case '1':
				printf("\n\nREADING IRQENx\n");
				printf("----------------------\n");
				printf("IRQEN1  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN1));
				printf("IRQEN2  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN2));
				printf("IRQEN3  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN3));
				break;

			case '2':
				printf("\n\nENABLING INTERRUPTS\n");
				printf("----------------------\n");
				ProSLIC_EnableInterrupts(pState->ProObj);
				printf("IRQEN1  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN1));
				printf("IRQEN2  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN2));
				printf("IRQEN3  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN3));
				break;

		    case '3':
				printf("\n\nEnter IRQEN1 Value (hex) -> ");
				gets(regDataStr);
                reg = (uInt8)strtol(regDataStr,NULL,16);
                WriteReg(pProHW,pState->ProObj->channel,IRQEN1,reg);

				printf("Enter IRQEN2 Value (hex) -> ");
				gets(regDataStr);
                reg = (uInt8)strtol(regDataStr,NULL,16);
                WriteReg(pProHW,pState->ProObj->channel,IRQEN2,reg);

				printf("Enter IRQEN3 Value (hex) -> ");
				gets(regDataStr);
                reg = (uInt8)strtol(regDataStr,NULL,16);
                WriteReg(pProHW,pState->ProObj->channel,IRQEN3,reg);

                printf("\n\nIRQEN1  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN1));
				printf("IRQEN2  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN2));
				printf("IRQEN3  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN3));
                break;

            case '4':
				printf("\n\nDISABLING INTERRUPTS\n");
				printf("----------------------\n");
				ProSLIC_DisableInterrupts(pState->ProObj);
				printf("IRQEN1  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN1));
				printf("IRQEN2  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN2));
				printf("IRQEN3  =  0x%02X \n", ReadReg(pProHW,pState->ProObj->channel,IRQEN3));
				break;

            case '5':
                done = 0;
                printf("\n\nWaiting for Interrupt\n\n");
                while((!kbhit())&&(!done))
                {
                    Delay(pProTimer,poll_interval);

                    ProSLIC_GetInterrupts(pState->ProObj,&irqs);

                    if(irqs.number > 0)
                    {
                        printf("\nInterrupt Code ->  %d", irqs.irqs[0]);
                        if(irqs.irqs[0] < MAX_INT_STRINGS)
                        {
                            printf("= %s", intMapStrings[irqs.irqs[0]]);
                        }
                        printf("\n");
                        done = 1;
                    }
                }
                break;

            case '6':
                printf("\n\nContinuously Monitoring IRQs.  Hit Any Key To Stop.\n\n");
                while(!kbhit())
                {
                    Delay(pProTimer,poll_interval);

                    ProSLIC_GetInterrupts(pState->ProObj,&irqs);

                    if(irqs.number > 0)
                    {
                        int i;
                        for(i = 0; i < irqs.number; i++)
                        {
                          printf("\nInterrupt Code ->  %d", irqs.irqs[i]);
                          
                          if(irqs.irqs[i] < MAX_INT_STRINGS)
                          {
                            printf(" = %s", intMapStrings[irqs.irqs[i]]);
                          }

                          printf("\n");

						              /* If returning onhook, set to default onhook state */
						              if(irqs.irqs[i] == IRQ_LOOP_STATUS)
                          {
                            
                            ProSLIC_ReadHookStatus(pState->ProObj,&reg);
                            if(reg == PROSLIC_ONHOOK)
                            {
                              ProSLIC_SetLinefeedStatus(pState->ProObj, onhook_state);
                              printf("ONHOOK\n");
                            }
                            else
                            {
                              printf("OFFHOOK\n");
                            }
                          }
                        }
                    }
                }
                break;

            case '7':
                poll_interval =  selectUserInt("Enter IRQ Poll Interval (ms) : "); 
                break;
				
			case '8':
			    printf("\n\n");
                printf("1.   FWD ACTIVE\n");
                printf("2.   FWD OHT\n");
                printf("3.   REV ACTIVE\n");
                printf("4.   REV OHT\n");
	            printf("----------------\n");
                printf("Select LINEFEED State -> ");
                gets(regDataStr);
		        reg = (uInt8)atoi(regDataStr);  
				
				switch(reg)
				{
					case 1:
						onhook_state = LF_FWD_ACTIVE;
						break;
					case 2:
						onhook_state = LF_FWD_OHT;
						break;
					case 3:
						onhook_state = LF_REV_ACTIVE;
						break;
					case 4:
						onhook_state = LF_REV_OHT;
						break;
					default:
						printf("\nInvalid Linefeed State Selected\n");
						break;
				}
                
				ProSLIC_ReadHookStatus(pState->ProObj,&reg);
				
				if(reg == PROSLIC_ONHOOK)
					ProSLIC_SetLinefeedStatus(pState->ProObj, onhook_state);
					
				break;
				
			case '9':
                /* Save entry hook state */
	            ProSLIC_ReadHookStatus(pState->ProObj,&reg);

                if(reg == PROSLIC_OFFHOOK)
                    pState->hook_det_state = OFFHOOK_IDLE;
                else
                    pState->hook_det_state = ONHOOK_IDLE;

                GetTime(pProTimer, &hook_time);   /* Init timer */

                /* Start Monitor Loop */
                while(!kbhit())
                {

                    /*
                    ** Display any decoded digits
                    */
                    while(digit_count > 0)
                    {
                        if(digit_buffer[digit_count-1] == 15)
                            LOGPRINT("--> HOOKFLASH\n");
                        else
                            LOGPRINT("--> DIGIT %d\n", digit_buffer[digit_count-1]);

                        digit_count--;
                    }

                    Delay(pProTimer, 5);                /* Simulate Poll Inverval of 5ms */

                    pState->hook_change = FALSE;        /* Reset hook monitor  */
                    irq_demo_check_interrupts(pState);  /* Poll for interrupts */                    
                    
                    /*
                    ** Must handle the following
                    ** 
                    ** 1.  Response to change in hook state (resets timer)
                    ** 2.  Monitor time after hook state change (does not reset timer)
                    **
                    */

                    if(pState->hook_change == TRUE)
                    {
                        switch(pState->hook_det_state)
                        {
                        case ONHOOK_IDLE:
                            pState->hook_det_state = OFFHOOK_IDLE;
                            GetTime(pProTimer, &hook_time);   /* reset timer */
                            break;

                        case OFFHOOK_IDLE:
                            pState->hook_det_state = ONHOOK_DECODE_PD;
                            GetTime(pProTimer, &hook_time);   /* reset timer */
                            break;

                        case ONHOOK_DECODE_PD:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            GetTime(pProTimer, &hook_time);  /* reset timer */
                            if((elapsed_time >= hook_change_cfg.minOnHook)&&(elapsed_time <= hook_change_cfg.maxOnHook))
                            {
                                pState->hook_det_state = OFFHOOK_DECODE_PD;
                                valid_digit_pending = TRUE;    /* Pending valid offhook time */
                            }
                            else
                            {
                                pState->hook_det_state = OFFHOOK_IDLE;
                                valid_digit_pending = FALSE;
                                digit = 0;
                            }
                            break;

                        case ONHOOK_DECODE_HF:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            GetTime(pProTimer, &hook_time); /* reset timer */
                            if((elapsed_time >= hook_change_cfg.minHookFlash)&&(elapsed_time <= hook_change_cfg.maxHookFlash))
                            {
                                pState->hook_det_state = OFFHOOK_WAIT_IDLE;
                                valid_hook_flash_pending = TRUE;
                            }
                            else
                            {
                                pState->hook_det_state = OFFHOOK_IDLE;
                                valid_hook_flash_pending = FALSE;
                            }
                            break;

                        case OFFHOOK_DECODE_PD:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            GetTime(pProTimer, &hook_time); /* reset timer */
                            if((elapsed_time >= hook_change_cfg.minOffHook)&&(elapsed_time <= hook_change_cfg.maxOffHook))
                            {
                                pState->hook_det_state = ONHOOK_DECODE_PD;
                                digit++;
                            }
                            else
                            {
                                pState->hook_det_state = ONHOOK_DECODE_PD;
                                valid_digit_pending = FALSE;
                                digit = 0;
                            }
                            break;

                        case OFFHOOK_WAIT_IDLE:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            GetTime(pProTimer, &hook_time); /* reset timer */
                            pState->hook_det_state = ONHOOK_DECODE_PD;
                            if((elapsed_time >= hook_change_cfg.minInterDigit)||(valid_hook_flash_pending))
                            {
                                if(valid_hook_flash_pending)
                                {
                                    digit_buffer[digit_count++] = 15;  /* Store Hookflash as digit 15 */
                                    valid_hook_flash_pending = FALSE;
                                }
                                else if(valid_digit_pending)
                                {
                                    digit_buffer[digit_count++] = digit;
                                    valid_digit_pending = FALSE;
                                    digit = 0;
                                }
                            }
                            else
                            {
                                valid_hook_flash_pending = FALSE;
                                valid_digit_pending = FALSE;
                                digit = 0;
                            }
                            break;

                        default:
                            LOGPRINT("INVALID STATE\n");
                            break;
                        }

                    }
                    else  /* Polling timers */
                    {
                        switch(pState->hook_det_state)
                        {
                        case ONHOOK_IDLE:
                        case OFFHOOK_IDLE:
                            /* Do Nothing */
                            break;

                        case ONHOOK_DECODE_PD:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            if(elapsed_time > hook_change_cfg.maxOnHook)
                            {
                                pState->hook_det_state = ONHOOK_DECODE_HF;
                            }
                            break;

                        case ONHOOK_DECODE_HF:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            if(elapsed_time > hook_change_cfg.maxHookFlash)
                            {
                                pState->hook_det_state = ONHOOK_IDLE;
                            }
                            break;

                        case OFFHOOK_DECODE_PD:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            if(elapsed_time > hook_change_cfg.maxOffHook)
                            {
                                digit++;
                                valid_digit_pending = TRUE;
                                pState->hook_det_state = OFFHOOK_WAIT_IDLE;
                            }
                            break;

                        case OFFHOOK_WAIT_IDLE:
                            TimeElapsed(pProTimer, &hook_time, &elapsed_time);
                            GetTime(pProTimer, &hook_time);  /* reset timer */
                            pState->hook_det_state = OFFHOOK_IDLE;
                            if((elapsed_time >= hook_change_cfg.minInterDigit)||(valid_hook_flash_pending))
                            {
                                if(valid_hook_flash_pending)
                                {
                                    digit_buffer[digit_count++] = 15;  /* Store hookflash as digit 15 */
                                    valid_hook_flash_pending = FALSE;
                                }
                                else if(valid_digit_pending)
                                {
                                    digit_buffer[digit_count++] = digit;
                                    valid_digit_pending = FALSE;
                                    digit = 0;
                                }
                            }
                            else
                            {
                                valid_hook_flash_pending = FALSE;
                                valid_digit_pending = FALSE;
                                digit = 0;
                            }
                            break;

                        default:
                            LOGPRINT("INVALID STATE\n");
                            break;
                        }
                    }

                }
			
			
				break;

            default:
                break;
		}	
   } while(toupper(cmd[0]) != 'Q');
}
