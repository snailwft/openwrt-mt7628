/*
** Copyright (c) 2007-2015 by Silicon Laboratories
**
** $Id: si3226x_intf.h 4969 2015-04-28 00:26:38Z nizajerk $
**
** Si3226x_Intf.h
** Si3226x ProSLIC interface header file
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

#ifndef SI3226X_INTF_H
#define SI3226X_INTF_H

/*
**
** Si3226x General Constants
**
*/
#define SI3226X_REVA                0
#define SI3226X_REVB                1
#define SI3226X_REVC                3    /* This is revC bug - shows revD revision code */

/*
** Calibration Constants
*/
#define SI3226X_CAL_STD_CALR1               0xC0    /* FF */
#define SI3226X_CAL_STD_CALR2               0x18    /* F8 */

/* Timeouts in 10s of ms */
#define SI3226X_TIMEOUT_DCDC_UP             200
#define SI3226X_TIMEOUT_DCDC_DOWN           200

/* The following macros are for backward compatibility */
#define Si3226x_Cal(PCHAN,INTARG)            ProSLIC_UnsupportedFeatureNoArg((PCHAN),"Si3226x_Cal")
#define Si3226x_CheckCIDBuffer               ProSLIC_CheckCIDBuffer
#define Si3226x_DCFeedSetup(PCHAN,PRESET)    Si3226x_DCFeedSetupCfg((PCHAN),Si3226x_DCfeed_Presets,(PRESET))
#define Si3226x_DisableCID                   ProSLIC_DisableCID
#define Si3226x_DisableInterrupts            ProSLIC_DisableInterrupts
#define Si3226x_DTMFReadDigit                ProSLIC_DTMFReadDigit
#define Si3226x_EnableCID                    ProSLIC_EnableCID
#define Si3226x_GetInterrupts                ProSLIC_GetInterrupts
#define Si3226x_GetLBCalResult               ProSLIC_GetLBCalResult
#define Si3226x_GetPLLFreeRunStatus          ProSLIC_GetPLLFreeRunStatus
#define Si3226x_GPIOControl                  ProSLIC_GPIOControl
#define Si3226x_Init(PCHAN,SZ)               Si3226x_Init_with_Options((PCHAN),(SZ),INIT_NO_OPT)
#define Si3226x_LoadPatch                    ProSLIC_LoadPatch
#define Si3226x_LoadPreviousLBCal            ProSLIC_LoadPreviousLBCal
#define Si3226x_LoadPreviousLBCalPacked      ProSLIC_LoadPreviousLBCalPacked
#define Si3226x_LoadRegTables                ProSLIC_LoadRegTables
#define Si3226x_ModemDetSetup(PCHAN,PRESET)  ProSLIC_UnsupportedFeatureNoArg((PCHAN),"Si3226x_ModemDetSetup")
#define Si3226x_ModifyCIDStartBits           ProSLIC_ModifyCIDStartBits
#define Si3226x_PCMStart                     ProSLIC_PCMStart
#define Si3226x_PCMStop                      ProSLIC_PCMStop
#define Si3226x_PCMTimeSlotSetup             ProSLIC_PCMTimeSlotSetup
#define Si3226x_PLLFreeRunStart              ProSLIC_PLLFreeRunStart
#define Si3226x_PLLFreeRunStop               ProSLIC_PLLFreeRunStop
#define Si3226x_PolRev                       ProSLIC_PolRev
#define Si3226x_PringDebugRAM                ProSLIC_PrintDebugRAM
#define Si3226x_PrintDebugData               ProSLIC_PrintDebugData
#define Si3226x_PrintDebugReg                ProSLIC_PrintDebugReg
#define Si3226x_PulseMeterDisable            ProSLIC_PulseMeterDisable
#define Si3226x_PulseMeterEnable             ProSLIC_PulseMeterEnable
#define Si3226x_PulseMeterStart              ProSLIC_PulseMeterStart
#define Si3226x_PulseMeterStop               ProSLIC_PulseMeterDisable
#define Si3226x_ReadHookStatus               ProSLIC_ReadHookStatus
#define Si3226x_ReadRAM                      ProSLIC_ReadRAM
#define Si3226x_ReadReg                      ProSLIC_ReadReg
#define Si3226x_Reset                        SiVoice_Reset
#define Si3226x_RingStart                    ProSLIC_RingStart
#define Si3226x_RingStop                     ProSLIC_RingStop
#define Si3226x_SendCID                      ProSLIC_SendCID
#define Si3226x_SetLoopbackMode              ProSLIC_SetLoopbackMode
#define Si3226x_SetMuteStatus                ProSLIC_SetMuteStatus
#define Si3226x_SetPowersaveMode             ProSLIC_SetPowersaveMode
#define Si3226x_SetProfile(PCHAN,PRESET)     ProSLIC_UnsupportedFeatureNoArg((PCHAN),"Si3226x_SetProfile")
#define Si3226x_ToneGenStart                 ProSLIC_ToneGenStart
#define Si3226x_ToneGenStop                  ProSLIC_ToneGenStop
#define Si3226x_VerifyPatch ProSLIC_VerifyPatch
#define Si3226x_WriteRAM                     ProSLIC_WriteRAM
#define Si3226x_WriteReg                     ProSLIC_WriteReg

/* DC Feed */
#ifndef DISABLE_DCFEED_SETUP
extern Si3226x_DCfeed_Cfg Si3226x_DCfeed_Presets[];
#endif


/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_ShutdownChannel
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
int Si3226x_ShutdownChannel (proslicChanType_ptr hProslic);

/*
** Function: PROSLIC_Init_MultiBOM
**
** Description: 
** Initializes the ProSLIC w/ selected general parameters
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** size:     number of channels
** preset:   general configuration preset
**
** Return:
** none
*/
int Si3226x_Init_MultiBOM (proslicChanType_ptr *hProslic,int size,int preset);

/*
** Function: Si3226x_Init_with_Options
**
** Description: 
** Initializes the ProSLIC with an option.
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** size - number of continuous channels to initialize
** init_opt - which initialization type to do.
**
** Return:
** none
*/
int Si3226x_Init_with_Options (proslicChanType_ptr *pProslic, int size, initOptionsType init_opt);

/*
** Function: PROSLIC_VerifyControlInterface
**
** Description: 
** Verify SPI port read capabilities
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226x_VerifyControlInterface (proslicChanType_ptr hProslic);

/*
** Function: Si3226x_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
** Returns:
** int (error)
**
*/
int Si3226x_PowerUpConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3226x_PowerDownConverter
**
** Description: 
** Power down DCDC converter (selected channel only)
**
** Returns:
** int (error)
**
*/
int Si3226x_PowerDownConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3226x_Calibrate
**
** Description: 
** Generic calibration function for Si3226x
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object,
** size:     maximum number of channels
** calr:     array of CALRx register values
** maxTime:  cal timeout (in ms) 
**
** Return:
** int 
*/
int Si3226x_Calibrate (proslicChanType_ptr *hProslic, int size, uInt8 *calr, int maxTime);

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
int Si3226x_EnableInterrupts (proslicChanType_ptr hProslic);

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
int Si3226x_RingSetup (proslicChanType *pProslic, int preset);

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
int Si3226x_ToneGenSetup (proslicChanType *pProslic, int preset);

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
int Si3226x_FSKSetup (proslicChanType *pProslic, int preset);


/*
** Function: PROSLIC_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDTMFDec: pointer to dtmf decoder config structure
**
** Return:
** none
*/
int Si3226x_DTMFDecodeSetup (proslicChanType *pProslic, int preset);

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
int Si3226x_ZsynthSetup (proslicChanType *pProslic, int preset);

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
int Si3226x_GciCISetup (proslicChanType *pProslic, int preset);

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
int Si3226x_TXAudioGainSetup (proslicChanType *pProslic, int preset);
int Si3226x_RXAudioGainSetup (proslicChanType *pProslic, int preset);
#define Si3226x_AudioGainSetup ProSLIC_AudioGainSetup
int Si3226x_TXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);
int Si3226x_RXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);

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
int Si3226x_DCFeedSetupCfg (proslicChanType *pProslic,ProSLIC_DCfeed_Cfg *cfg,int preset);

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
int Si3226x_GPIOSetup (proslicChanType *pProslic);

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
int Si3226x_PCMSetup (proslicChanType *pProslic, int preset);

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
int Si3226x_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed);

/*
** Function: PROSLIC_SetLinefeedBroadcast
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
int Si3226x_SetLinefeedStatusBroadcast (proslicChanType *pProslic, uInt8 newLinefeed);

/*
** Function: ProSLIC_MWISetup
**
** Description: 
** Modify default MWI amplitude and switch debounce parameters
**
** Input Parameters: 
** pProslic:		pointer to Proslic object
** vpk_mag:			peak flash voltgage (vpk) - passing a 0 results
**                  in no change to VBATH_NEON
** lcmrmask_mwi:	LCR mask time (ms) after MWI state switch - passing
**                  a 0 results in no change to LCRMASK_MWI
**
** Return:
** none
*/
int Si3226x_MWISetup (proslicChanType *pProslic,uInt16 vpk_mag,uInt16 lcrmask_mwi);

/*
** Function: ProSLIC_MWIEnable
**
** Description: 
** Enable MWI feature
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226x_MWIEnable (proslicChanType *pProslic);

/*
** Function: ProSLIC_MWIDisable
**
** Description: 
** Disable MWI feature
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226x_MWIDisable (proslicChanType *pProslic);

/*
** Function: ProSLIC_SetMWIState
**
** Description: 
** Set MWI state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** flash_on: 0 = low, 1 = high (VBATH_NEON)
**
** Return:
** none
*/
int Si3226x_SetMWIState (proslicChanType *pProslic,uInt8 flash_on);


/*
** Function: ProSLIC_GetMWIState
**
** Description: 
** Read MWI state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** 0 - Flash OFF, 1 - Flash ON, RC_MWI_NOT_ENABLED
*/
int Si3226x_GetMWIState (proslicChanType *pProslic);

/*
** Function: ProSLIC_MWI
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
**
** Use Deprecated.
*/
int Si3226x_MWI (proslicChanType *pProslic,uInt8 lampOn);

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
int Si3226x_PulseMeterSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_LBCal
**
** Description: 
** Execute longitudinal balance calibration
**
** Input Parameters: 
** hProslic: pointer to array of Proslic objects
** 
** Return:
** 
*/
int Si3226x_LBCal (proslicChanType_ptr *pProslic, int size);

/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226x_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset);

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226x_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset);


/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226x_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset);


/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provisionary function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/
int Si3226x_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset);

/*
** Function: PROSLIC_dbgSetRXGain
**
** Description: 
** Provisionary function for setting up
** RX path gain.
*/
int Si3226x_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);

/*
** Function: PROSLIC_dbgSetTXGain
**
** Description: 
** Provisionary function for setting up
** TX path gain.
*/
int Si3226x_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);


/*
** Function: PROSLIC_LineMonitor
**
** Description: 
** Monitor line voltages and currents
*/
int Si3226x_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor);


/*
** Function: PROSLIC_PSTNCheck
**
** Description: 
** Continuous monitor of ilong to detect hot pstn line 
*/
int Si3226x_PSTNCheck(proslicChanType *pProslic, proslicPSTNCheckObjType *pstnCheckObj);

/*
** Function: PROSLIC_DiffPSTNCheck
**
** Description: 
** Detection of foreign PSTN 
*/
int Si3226x_DiffPSTNCheck (proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pPSTNCheck);

/*
** Function: PROSLIC_SetPowersaveMode
**
** Description: 
** Enable or Disable powersave mode 
*/
int Si3226x_SetPowersaveMode(proslicChanType *pProslic, int pwrsave);

/*
** Function: PROSLIC_ReadMADCScaled
**
** Description: 
** ReadMADC (or other sensed voltage/currents) and
** return scaled value in int32 format 
*/
int32 Si3226x_ReadMADCScaled(proslicChanType *pProslic, uInt16 addr, int32 scale);


#endif

