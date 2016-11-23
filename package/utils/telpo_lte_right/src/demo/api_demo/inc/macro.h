/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: macro.h 4965 2015-04-27 22:23:30Z nizajerk $
**
** macro.h
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** This file contains macros for commonly used procedures
**
*/

/* 
** Function pointer macros
** Note: Presume pState->ProObj is the channel structure 
*/

#define Delay           pState->ProObj->deviceId->ctrlInterface->Delay_fptr
#define pProTimer       pState->ProObj->deviceId->ctrlInterface->hTimer
#define pProHW          pState->ProObj->deviceId->ctrlInterface->hCtrl
#define pChannel        pState->ProObj->channel
#define WriteReg        pState->ProObj->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg         pState->ProObj->deviceId->ctrlInterface->ReadRegister_fptr
#define WriteRAM        pState->ProObj->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM         pState->ProObj->deviceId->ctrlInterface->ReadRAM_fptr
#define SetSemaphore    pState->ProObj->deviceId->ctrlInterface->Semaphore_fptr
#define TimeElapsed     pState->ProObj->deviceId->ctrlInterface->timeElapsed_fptr
#define GetTime         pState->ProObj->deviceId->ctrlInterface->getTime_fptr


/*
** Expression macros
*/
#define TOGGLE(x)               (x ^= (1 << (x&0x0001)))
#define GET_ENABLED_STATE(x)    ((x == 0)?"disabled":"enabled")
#define GET_ACTIVE_STATE(x)     ((x == 0)?"inactive":"active")
#define GET_ABRUPT_STATE(x)     ((x == 0)?"smooth":"abrupt")

/*
** CID
*/
#define FSK_DEPTH_TRIG  4

/*
** Bitmasks
*/
#define DIAG_SEL_HR_VTIPC		0x11
#define DIAG_SEL_HR_VRINGC		0x12
#define DIAG_SEL_HR_VBAT		0x13
#define DIAG_SEL_HR_VDC			0x14
#define DIAG_SEL_HR_ILONG		0x16
#define DIAG_SEL_HR_VLONG		0x17
#define DIAG_SEL_HR_ILOOP		0x1A
