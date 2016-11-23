/*
** Copyright (c) 2007-2015 by Silicon Laboratories
**
** $Id: si324x_intf.h 4727 2015-02-10 22:35:54Z nizajerk $
**
** Si324x_Intf.h
** Si324x ProSLIC interface header file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the Si3224x ProSLIC driver.
**
**
*/
#ifndef SI324X_INTF_H
#define SI324X_INTF_H


/*
**
** Si324x General Constants 
**
*/
#define SCALE_V_MADC        1074    /* 1/(100*931.32e-9) mV */
#define SCALE_I_MADC        597     /* 1/(1e6*1.676e-9) uA */

/* The following macros are for backward compatibility */

#define Si224x_PrintDebugData                ProSLIC_PrintDebugData
#define Si224x_PrintDebugRAM                 ProSLIC_PrintDebugRAM
#define Si224x_PrintDebugReg                 ProSLIC_PrintDebugReg
#define Si324x_CheckCIDBuffer                ProSLIC_CheckCIDBuffer
#define Si324x_DisableCID                    ProSLIC_DisableCID
#define Si324x_DisableInterrupts             ProSLIC_DisableInterrupts
#define Si324x_DTMFReadDigit                 ProSLIC_DTMFReadDigit
#define Si324x_EnableCID                     ProSLIC_EnableCID
#define Si324x_GetInterrupts                 ProSLIC_GetInterrupts
#define Si324x_GetLBCalResult                ProSLIC_GetLBCalResult
#define Si324x_GetLBCalResultPacked          ProSLIC_GetLBCalResultPacked
#define Si324x_GPIOControl                   ProSLIC_GPIOControl
#define Si324x_LoadPatch                     ProSLIC_LoadPatch
#define Si324x_LoadPreviousLBCal             ProSLIC_LoadPreviousLBCal
#define Si324x_LoadPreviousLBCalPacked       ProSLIC_LoadPreviousLBCalPacked
#define Si324x_LoadRegTables                 ProSLIC_LoadRegTables
#define Si324x_ModemDetSetup(PCHAN,PRESET)   ProSLIC_UnsupportedFeatureNoArg((PCHAN),"Si324x_ModemDetSetup")
#define Si324x_MWI(PCHAN,ARG)                ProSLIC_UnsupportedFeatureNoArg((PCHAN),"Si324x_MWI")
#define Si324x_PCMStart                      ProSLIC_PCMStart
#define Si324x_PCMStop                       ProSLIC_PCMStop
#define Si324x_PCMTimeSlotSetup              ProSLIC_PCMTimeSlotSetup
#define Si324x_PLLFreeRunStart(PCHAN)        ProSLIC_UnsupportedFeatureNoArg((PCHAN),"Si324x_PLLFreeRunStart")
#define Si324x_PLLFreeRunStop(PCHAN)         ProSLIC_UnsupportedFeatureNoArg((PCHAN),"Si324x_PLLFreeRunStop")
#define Si324x_PolRev                        ProSLIC_PolRev
#define Si324x_PrintDebugData                ProSLIC_PrintDebugData
#define Si324x_ReadHookStatus                ProSLIC_ReadHookStatus
#define Si324x_Reset                         SiVoice_Reset
#define Si324x_RingStart                     ProSLIC_RingStart
#define Si324x_RingStop                      ProSLIC_RingStop
#define Si324x_SendCID                       ProSLIC_SendCID
#define Si324x_SetLoopbackMode               ProSLIC_SetLoopbackMode
#define Si324x_SetMuteStatus                 ProSLIC_SetMuteStatus
#define Si324x_SetProfile(PCHAN,PRESET)      ProSLIC_UnsupportedFeatureNoArgS((PCHAN),"Si324x_SetProfile")
#define Si324x_ToneGenStart                  ProSLIC_ToneGenStart
#define Si324x_ToneGenStop                   ProSLIC_ToneGenStop
#define Si324x_VerifyPatch                   ProSLIC_VerifyPatch
/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_Init
**
** Description: 
** Initializes the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si324x_Init (proslicChanType_ptr *hProslic,int size);

/*
** Function: PROSLIC_Init
**
** Description: 
** Initializes the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si324x_InitBroadcast (proslicChanType_ptr *hProslic);

/*
** Function: PROSLIC_Cal
**
** Description: 
** Calibrates the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si324x_Cal (proslicChanType_ptr *hProslic, int size);

/*
** Function: PROSLIC_EnableInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** 
** Return:
** 
*/
int Si324x_EnableInterrupts (proslicChanType_ptr hProslic);

/*
**
** PROSLIC CONFIGURATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_RingSetup
**
** Description: 
** configure ringing
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pRingSetup: pointer to ringing config structure
**
** Return:
** none
*/
int Si324x_RingSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ToneGenSetup
**
** Description: 
** configure tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pTone: pointer to tones config structure
**
** Return:
** none
*/
int Si324x_ToneGenSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_FSKSetup
**
** Description: 
** configure fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pFsk: pointer to fsk config structure
**
** Return:
** none
*/
int Si324x_FSKSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDTMFDec: pointer to DTMF decoder config structure
**
** Return:
** none
*/
int Si324x_DTMFDecodeSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ZsynthSetup
**
** Description: 
** configure impedance synthesis
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pZynth: pointer to zsynth config structure
**
** Return:
** none
*/
int Si324x_ZsynthSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_GciCISetup
**
** Description: 
** configure CI bits (GCI mode)
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pCI: pointer to ringing config structure
**
** Return:
** none
*/
int Si324x_GciCISetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pAudio: pointer to audio gains config structure
**
** Return:
** none
*/
int Si324x_TXAudioGainSetup (proslicChanType *pProslic, int preset);
int Si324x_RXAudioGainSetup (proslicChanType *pProslic, int preset);
#define Si324x_AudioGainSetup ProSLIC_AudioGainSetup

/*
** Function: PROSLIC_AudioEQSetup
**
** Description: 
** configure audio equalizers
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pAudioEQ: pointer to ringing config structure
**
** Return:
** none
*/
int Si324x_AudioEQSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_DCFeedSetup
**
** Description: 
** configure dc feed
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDcFeed: pointer to dc feed config structure
**
** Return:
** none
*/
int Si324x_DCFeedSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_GPIOSetup
**
** Description: 
** configure gpio
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pGpio: pointer to gpio config structure
**
** Return:
** none
*/
int Si324x_GPIOSetup (proslicChanType *pProslic);

/*
** Function: PROSLIC_PulseMeterSetup
**
** Description: 
** configure pulse metering
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pPulseCfg: pointer to pulse metering config structure
**
** Return:
** none
*/
int Si324x_PulseMeterSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pPcm: pointer to pcm config structure
**
** Return:
** none
*/
int Si324x_PCMSetup (proslicChanType *pProslic, int preset);

/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/

/*
** Function: PROSLIC_WriteLinefeed
**
** Description: 
** Sets linefeed state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** newLinefeed: new linefeed state
**
** Return:
** none
*/
int Si324x_SetLinefeedStatus (proslicChanType *pProslic, uInt8 newLinefeed);

/*
** Function: PROSLIC_WriteLinefeed
**
** Description: 
** Sets linefeed state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** newLinefeed: new linefeed state
**
** Return:
** none
*/
int Si324x_SetLinefeedStatusBroadcast (proslicChanType *pProslic, uInt8 newLinefeed);

int Si324x_VerifyControlInterface (proslicChanType_ptr pProslic);

int Si324x_LBCal (proslicChanType_ptr *pProslic, int size);
int Si324x_LBCalBroadcast (proslicChanType *pProslic);


/*
** Function: Si3224_dbgSetTXGain
**
** Description: 
** Provision function for setting up
** TX gain
*/
int Si324x_dbgSetTXGain (proslicChanType *pProslic, int32 gain, 
    int impedance_preset, int audio_gain_preset);

/*
** Function: Si3224_dbgSetRXGain
**
** Description: 
** Provision function for setting up
** RX gain
*/
int Si324x_dbgSetRXGain(proslicChanType *pProslic, int32 gain, 
    int impedance_preset, int audio_gain_preset);

/*
** Function: Si3224_LineMonitor
**
** Description: 
** Monitor line voltages and current
*/
int Si324x_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor);

#endif



