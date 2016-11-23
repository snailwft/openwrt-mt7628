/*
** Copyright (c) 2014-2015 by Silicon Laboratories
**
** $Id: api_demo.h 4807 2015-03-10 20:56:29Z elgeorge $
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the API demo
**
*/

#ifndef API_DEMO_H
#define API_DEMO_H

#include "proslic.h"
#include "proslic_timer.h"
#include "proslic_tstin.h"
#include "vdaa.h"
#include "macro.h"
#include "util.h"
#include "si3226x_registers.h"  // use as superset of regs 

/* 
** Describe Hardware (single, standard EVB) 
** See below if using custom/partner hardware
*/

#define SI321X_NUMBER_OF_DEVICE 1
#define SI324X_NUMBER_OF_DEVICE 1
#define SI322X_NUMBER_OF_DEVICE 1
#define SI3217XB_NUMBER_OF_DEVICE 1   
#define SI3217XC_NUMBER_OF_DEVICE 1  
#define SI3218X_NUMBER_OF_DEVICE 1  
#define SI3226X_NUMBER_OF_DEVICE 1
#define SI3228X_NUMBER_OF_DEVICE 1

#define MAX_NUMBER_OF_DEVICE 16
#define MAX_NUMBER_OF_CHAN 32

#define SI321X_NUMBER_OF_CHAN (SI321X_NUMBER_OF_DEVICE*SI321X_CHAN_PER_DEVICE)
#define SI324X_NUMBER_OF_CHAN (SI324X_NUMBER_OF_DEVICE*SI324X_CHAN_PER_DEVICE)
#define SI322X_NUMBER_OF_CHAN (SI322X_NUMBER_OF_DEVICE*SI322X_CHAN_PER_DEVICE)
#define SI3217XB_NUMBER_OF_CHAN (SI3217XB_NUMBER_OF_DEVICE*SI3217X_CHAN_PER_DEVICE)
#define SI3217XC_NUMBER_OF_CHAN (SI3217XC_NUMBER_OF_DEVICE*SI3217X_CHAN_PER_DEVICE)
#define SI3218X_NUMBER_OF_CHAN (SI3218X_NUMBER_OF_DEVICE*SI3218X_CHAN_PER_DEVICE)
#define SI3226X_NUMBER_OF_CHAN (SI3226X_NUMBER_OF_DEVICE*SI3226X_CHAN_PER_DEVICE)
#define SI3228X_NUMBER_OF_CHAN (SI3228X_NUMBER_OF_DEVICE*SI3228X_CHAN_PER_DEVICE)

/*
** If using custom hardware or stacking evb's (adding more channels)
** explicitly undef and redefine number of device below.  
*/

/* eg.  4 Si3226x EVBs Stacked or 8 channel design
#undef SI3226X_NUMBER_OF_DEVICE
#define SI3226X_NUMBER_OF_DEVICE 4
*/

/*
** EVB Options
*/


/*
** CONSTANTS
*/
#define PSTN_CHECK_AVG_THRESH       5000    /* 5ma */
#define PSTN_CHECK_SINGLE_THRESH    40000   /* 40mA */
#define PSTN_CHECK_SAMPLES          8

typedef struct chanStatus chanState; //forward declaration

/*typedef void (*procState) (chanState *pState, ProslicInt eInput);*/

/*
** structure to hold state information for pbx demo
*/
struct chanStatus { 
	/*call_progress_sm_t *sm;*/
	uInt8 deviceType;
	uInt8 numberOfChan;
	uInt8 numberOfDevice;
	uInt8 chanPerDevice;
	uInt8 deviceRev;
	//uInt8 mbGenParamOffset;
	int genConf;
	vdaa_LIU_Config liuCfg;
	proslicChanType *ProObj;
	timeStamp onHookTime;
	timeStamp offHookTime;
	/*procState currentState;*/
	uInt16 digitCount;
	uInt8 digits[20];
	uInt8 ringCount;
	uInt16 connectionWith;
	uInt16 powerAlarmCount;
	BOOLEAN eventEnable;
    proslicPSTNCheckObjType *checkObj;  
    proslicDiffPSTNCheckObjType *diffCheckObj; 
    BOOLEAN validLBCalResults;
    BOOLEAN packedResultsFlag;
    int32 lbCalResultsPacked;
    int32 lbCalResults[4];  
	proslicTestInObjType *pTstin;
	uInt16 max_mmreg_addr;
	BOOLEAN is_offhook;
	BOOLEAN hook_change;
	int hook_det_state;
} ; 


/*
** Line Monitor Status
*/
typedef enum {
    MON_LINE_OK,
    MON_LINE_ERR
} monitorStatus;


/*
** API Demo Functions
*/
uInt16 API_Demo();
int deviceMenu(chanState *pChan);
int selectGenConf(uInt8 device, int *pGenConf);
uInt8 channelSelect(chanState *pState);
void ApiTopMenu(chanState *pState);
void daaMenu(chanState *pState);

#endif


