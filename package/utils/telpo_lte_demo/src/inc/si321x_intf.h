/*
** Copyright (c) 2007-2015 by Silicon Laboratories
**
** $Id: si321x_intf.h 4727 2015-02-10 22:35:54Z nizajerk $
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
*/

#ifndef SI321x_INTF_H
#define SI321x_INTF_H

#include "proslic.h"
#include "si321x.h"


/* The following macros are for backward compatibility */
#define Si321x_AudioEQSetup(PCHAN, PRESET)        ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_AudioEQSetup")
#define Si321x_DTMFDecodeSetup(PCHAN)             ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_DTMFDecodeSetup")
#define Si321x_GciCISetup(PCHAN,PRESET)           ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_GciCISetup")
#define Si321x_GPIOControl(PCHAN, CONTROL, RD_wr) ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_GPIOControl")
#define Si321x_GPIOSetup(PCHAN, PRESET)           ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_GPIOSetup")
#define Si321x_HybridSetup(PCHAN,PRESET)          ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_HybridSetup")
#define Si321x_LoadPatch(PCHAN, PATCH)            ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_LoadPatch")
#define Si321x_LoadRegTables                      ProSLIC_LoadRegTables
#define Si321x_ModemDetSetup(PCHAN, PRESET)       ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_ModemDetSetup")
#define Si321x_PLLFreeRunStart(PCHAN, PRESET)     ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_PLLFreeRunStart")
#define Si321x_PLLFreeRunStop(PCHAN, PRESET)      ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_PLLFreeRunStop")
#define Si321x_PrintDebugData                     ProSLIC_PrintDebugData
#define Si321x_ReadRAM                            ProSLIC_ReadRAM
#define Si321x_ReadReg                            ProSLIC_ReadReg
#define Si321x_Reset                              SiVoice_Reset
#define Si321x_RingStart                          ProSLIC_RingStart
#define Si321x_RingStop                           ProSLIC_RingStop
#define Si321x_SetProfile(PCHAN)                  ProSLIC_UnsupportedFeatureNoArg((PCHAN), "Si321x_SetProfile")
#define Si321x_WriteRAM                           ProSLIC_WriteRAM
#define Si321x_WriteReg                           ProSLIC_WriteReg

int Si321x_Init (proslicChanType_ptr *hProslic, int size);
int Si321x_InitBroadcast (proslicChanType_ptr *hProslic);
int Si321x_Cal (proslicChanType_ptr *hProslic, int size);
int Si321x_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed);
int Si321x_SetLinefeedStatusBroadcast (proslicChanType *pProslic,uInt8 newLinefeed);
int Si321x_VerifyControlInterface(proslicChanType_ptr hProslic);

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
int Si321x_EnableInterrupts (proslicChanType_ptr hProslic);
int Si321x_DisableInterrupts (proslicChanType_ptr hProslic);

int Si321x_SetLoopbackMode (proslicChanType_ptr hProslic, ProslicLoopbackModes newMode);
int Si321x_SetMuteStatus (proslicChanType_ptr hProslic, ProslicMuteModes muteEn);
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
int Si321x_RingSetup (proslicChanType *pProslic, int preset);

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
int Si321x_ToneGenSetup (proslicChanType *pProslic, int preset);

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
int Si321x_FSKSetup (proslicChanType *pProslic, int preset);

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
int Si321x_ZsynthSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_TXAudioGainSetup
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
int Si321x_TXAudioGainSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_RXAudioGainSetup
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
int Si321x_RXAudioGainSetup (proslicChanType *pProslic, int preset);

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
int Si321x_DCFeedSetup (proslicChanType *pProslic, int preset);

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
int Si321x_PulseMeterSetup (proslicChanType *pProslic, int preset);

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
int Si321x_PCMSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_PCMTimeSlotSetup
**
** Description: 
** configure pcm
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** rxcount
** txcount
**
** Return:
** none
*/
int Si321x_PCMTimeSlotSetup (proslicChanType *pProslic, uInt16 rxcount, uInt16 txcount);

/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/

/*
** Function: PROSLIC_GetInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** pIntData: pointer to interrupt info retrieved
** 
** Return:
** 
*/
int Si321x_GetInterrupts (proslicChanType_ptr hProslic, proslicIntType *pIntData);

/*
** Function: PROSLIC_ReadHookStatus
**
** Description: 
** Determine hook status
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pHookStat: current hook status
**
** Return:
** none
*/
int Si321x_ReadHookStatus (proslicChanType *pProslic,uInt8 *pHookStat);

/*
** Function: PROSLIC_PolRev
**
** Description: 
** Sets polarity reversal state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** abrupt: set this to 1 for abrupt pol rev
** newPolRevState: new pol rev state
**
** Return:
** none
*/
int Si321x_PolRev (proslicChanType *pProslic,uInt8 abrupt, uInt8 newPolRevState);

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
int Si321x_MWI (proslicChanType *pProslic,uInt8 lampOn);

/*
** Function: PROSLIC_StartGenericTone
**
** Description: 
** Initializes and start tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** timerEn: specifies whether to enable the tone generator timers
**
** Return:
** none
*/
int Si321x_ToneGenStart (proslicChanType *pProslic, uInt8 timerEn);


/*
** Function: PROSLIC_StopTone
**
** Description: 
** Stops tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_ToneGenStop (proslicChanType *pProslic);

/*
** Function: PROSLIC_EnableCID
**
** Description: 
** enable fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_EnableCID (proslicChanType *pProslic);

/*
** Function: PROSLIC_DisableCID
**
** Description: 
** disable fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_DisableCID (proslicChanType *pProslic);

int Si321x_CheckCIDBuffer (proslicChanType *pProslic,uInt8 * fsk_buf_avail);

/*
** Function: PROSLIC_SendCID
**
** Description: 
** send fsk data
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** buffer: buffer to send
** numBytes: num of bytes in the buffer
**
** Return:
** none
*/
int Si321x_SendCID (proslicChanType *pProslic, uInt8 *buffer, uInt8 numBytes);

/*
** Function: PROSLIC_StartPCM
**
** Description: 
** Starts PCM
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_PCMStart (proslicChanType *pProslic);


/*
** Function: PROSLIC_StopPCM
**
** Description: 
** Disables PCM
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_PCMStop (proslicChanType *pProslic);

/*
** Function: PROSLIC_ReadDTMFDigit
**
** Description: 
** Read DTMF digit (would be called after DTMF interrupt to collect digit)
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDigit: digit read
**
** Return:
** none
*/
int Si321x_DTMFReadDigit (proslicChanType *pProslic,uInt8 *pDigit);

/*
** Function: PROSLIC_PulseMeterStart
**
** Description: 
** start pulse meter tone
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_PulseMeterStart (proslicChanType *pProslic);

/*
** Function: PROSLIC_PulseMeterStop
**
** Description: 
** stop pulse meter tone
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_PulseMeterStop (proslicChanType *pProslic);

#define Si321x_SetAudioGain ProSLIC_SetAudiGain

/*
** Function: Si321x_CalcAG
**
** Description:
** Calculates audio gain settings.
**/
int Si321x_CalcAG(proslicChanType *pProslic, int rxgain, int txgain, int *agrx_adjust, int *agtx_adjust);

int Si321x_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset);

int Si321x_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset);

int Si321x_dbgSetGain(proslicChanType *pProslic, int32 gain, int ag_value, int audio_gain_preset);

#endif
