/*
** Copyright (c) 2007-2015 by Silicon Laboratories
**
** $Id: si3226_intf.h 4727 2015-02-10 22:35:54Z nizajerk $
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
** This is the header file for the ProSLIC driver.
**
**
*/
#ifndef SI3226_INTF_H
#define SI3226_INTF_H


/*
**
** Si3226 General Constants
**
*/
  
/* Timeouts in 10s of ms */
#define SI3226_TIMEOUT_DCDC_UP             200
#define SI3226_TIMEOUT_DCDC_DOWN           200
 

#define SI3226_CAL_STD_CALR1               0xFF
#define SI3226_CAL_STD_CALR2               0xF8

/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/* The following macros are for backward compatibility */

#define Si3226_CheckCIDBuffer                 ProSLIC_CheckCIDBuffer
#define Si3226_DisableCID                     ProSLIC_DisableCID
#define Si3226_DisableInterrupts              ProSLIC_DisableInterrupts
#define Si3226_DTMFReadDigit                  ProSLIC_DTMFReadDigit
#define Si3226_EnableCID                      ProSLIC_EnableCID
#define Si3226_GetInterrupts                  ProSLIC_GetInterrupts 
#define Si3226_GetLBCalResult                 ProSLIC_GetLBCalResult
#define Si3226_GetLBCalResultPacked           ProSLIC_GetLBCalResultPacked
#define Si3226_GetPLLFreeRunStatus            ProSLIC_GetPLLFreeRunStatus
#define Si3226_GPIOControl                    ProSLIC_GPIOControl
#define Si3226_Init(PCHAN,SZ)                 Si3226_Init_with_Options((PCHAN), (SZ), INIT_NO_OPT)
#define Si3226_LoadPatch                      ProSLIC_LoadPatch
#define Si3226_LoadPreviousLBCal              ProSLIC_LoadPreviousLBCal
#define Si3226_LoadPreviousLBCalPacked        ProSLIC_LoadPreviousLBCalPacked
#define Si3226_LoadRegTables                  ProSLIC_LoadRegTables
#define Si3226_ModemDetSetup(PCHAN, PRESET)   ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si3226_ModemDetSetup")
#define Si3226_ModifyCIDStartBits             ProSLIC_ModifyCIDStartBits
#define Si3226_PCMStart                       ProSLIC_PCMStart
#define Si3226_PCMStop                        ProSLIC_PCMStop
#define Si3226_PCMTimeSlotSetup               ProSLIC_PCMTimeSlotSetup
#define Si3226_PLLFreeRunStart                ProSLIC_PLLFreeRunStart
#define Si3226_PLLFreeRunStop                 ProSLIC_PLLFreeRunStart
#define Si3226_PolRev                         ProSLIC_PolRev
#define Si3226_PrintData                      ProSLIC_PrintDebugData
#define Si3226_PrintDebugRAM                  ProSLIC_PrintDebugRAM
#define Si3226_PrintDebugReg                  ProSLIC_PrintDebugReg
#define Si3226_PulseMeterSetup(PCHAN, PRESET) ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si3226_PulseMeterSetup")
#define Si3226_PulseMeterStart(PCHAN)         ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si3226_PulseMeterStart")      
#define SI3226_PulseMeterStop(PCHAN)          ProSLIC_UnsupportedFeatureNoArg((PCHAN), "SI3226_PulseMeterStop")       
#define Si3226_ReadHookStatus                 ProSLIC_ReadHookStatus
#define Si3226_ReadRAM                        ProSLIC_ReadRAM
#define Si3226_ReadReg                        ProSLIC_ReadReg
#define Si3226_Reset                          SiVoice_Reset
#define Si3226_RingStart                      ProSLIC_RingStart
#define Si3226_RingStop                       ProSLIC_RingStop
#define Si3226_SendCID                        ProSLIC_SendCID
#define Si3226_SetLoopbackMode                ProSLIC_SetLoopbackMode
#define Si3226_SetMuteStatus                  ProSLIC_SetMuteStatus
#define Si3226_SetProfile(PCHAN, PRESET)      ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si3226_SetProfile")
#define Si3226_ToneGenStart                   ProSLIC_ToneGenStart
#define Si3226_ToneGenStop                    ProSLIC_ToneGenStop
#define Si3226_VerifyPatch                    ProSLIC_VerifyPatch
#define Si3226_WrieRAM                        ProSLIC_WriteRAM
#define Si3226_WrieReg                        ProSLIC_WriteReg

/*
** Function: Si3226_ShutdownChannel
**
** Description: 
** Safely shutdown channel w/o interruption to 
** other active channels
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_ShutdownChannel (proslicChanType_ptr hProslic);

/*
** Function: Si3226_Init_with_Options
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
int Si3226_Init_with_Options (proslicChanType_ptr *hProslic,int size, initOptionsType initOption);

/*
** Function: PROSLIC_Reinit
**
** Description: 
** Performs Soft Reset and reinitializes the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Reinit (proslicChanType_ptr hProslic,int size);

/*
** Function: PROSLIC_Init_MultiBOM
**
** Description: 
** Initializes the ProSLIC with supplied general configuration preset
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Init_MultiBOM (proslicChanType_ptr *hProslic,int size,int preset);

int Si3226_VerifyControlInterface (proslicChanType_ptr hProslic);

/*
** Function: Si3226_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
** Returns:
** int (error)
**
*/
int Si3226_PowerUpConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3226_PowerDownConverter
**
** Description: 
** Power down DCDC converter (selected channel only)
**
** Returns:
** int (error)
**
*/
int Si3226_PowerDownConverter(proslicChanType_ptr hProslic);

/*
** Function: PROSLIC_Calibrate
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
int Si3226_Calibrate (proslicChanType_ptr *hProslic, int size, uInt8 *calr, int maxTime);

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
int Si3226_Cal (proslicChanType_ptr *hProslic, int size);

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
int Si3226_EnableInterrupts (proslicChanType_ptr hProslic);

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
int Si3226_RingSetup (proslicChanType *pProslic, int preset);

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
int Si3226_ToneGenSetup (proslicChanType *pProslic, int preset);

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
int Si3226_FSKSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_DTMFDecodeSetup
**
** Description: 
** configure DTMF decode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDTMFDec: pointer to DTMF decoder config structure
**
** Return:
** none
*/
int Si3226_DTMFDecodeSetup (proslicChanType *pProslic, int preset);

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
int Si3226_ZsynthSetup (proslicChanType *pProslic, int preset);

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
int Si3226_GciCISetup (proslicChanType *pProslic, int preset);

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
int Si3226_TXAudioGainSetup (proslicChanType *pProslic, int preset);
int Si3226_RXAudioGainSetup (proslicChanType *pProslic, int preset);
#define Si3226_AudioGainSetup ProSLIC_AudioGainSetup
int Si3226_TXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);
int Si3226_RXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);

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
int Si3226_AudioEQSetup (proslicChanType *pProslic, int preset);

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
int Si3226_DCFeedSetup (proslicChanType *pProslic, int preset);

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
int Si3226_GPIOSetup (proslicChanType *pProslic);

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
int Si3226_PCMSetup (proslicChanType *pProslic, int preset);

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
int Si3226_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed);

/*
** Function: PROSLIC_MWI
**
** Description: 
** implements message waiting indicator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** lampOn: 0 = turn lamp off, 1 = turn lamp on
**
** Return:
** none
*/
int Si3226_MWI (proslicChanType *pProslic,uInt8 lampOn);

int Si3226_LBCal (proslicChanType_ptr *pProslic, int size);
int Si3226_SetLinefeedStatusBroadcast (proslicChanType_ptr hProslic, uInt8 newLinefeed);

/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** Provision function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset);

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** Provision function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset);


/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** Provision function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset);


/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provision function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/

int Si3226_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset);

int Si3226_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);
int Si3226_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);

int Si3226_LineMonitor (proslicChanType *pProslic, proslicMonitorType *monitor);
int Si3226_PSTNCheck (proslicChanType *pProslic,proslicPSTNCheckObjType *pPSTNCheck);

#endif

