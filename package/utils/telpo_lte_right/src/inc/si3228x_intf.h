/*
** Copyright (c) 2015 by Silicon Laboratories
**
** $Id: si3228x_intf.h 4969 2015-04-28 00:26:38Z nizajerk $
**
** Si3228x_Intf.h
** Si3228x ProSLIC interface header file
**
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

#ifndef SI3228X_INTF_H
#define SI3228X_INTF_H

/*
** Calibration Constants
*/
#define SI3228X_CAL_STD_CALR1               0xC0    /* FF */
#define SI3228X_CAL_STD_CALR2               0x18    /* F8 */

/* Timeouts in 10s of ms */
#define SI3228X_TIMEOUT_DCDC_UP             200
#define SI3228X_TIMEOUT_DCDC_DOWN           200

/* The following macros are for backward compatibility */
#define Si3228x_DCFeedSetup(PCHAN,PRESET)    Si3228x_DCFeedSetupCfg((PCHAN),Si3228x_DCfeed_Presets,(PRESET))
#define Si3228x_LoadPatch                    ProSLIC_LoadPatch
#define Si3228x_ReadHookStatus               ProSLIC_ReadHookStatus
#define Si3228x_SetPowersaveMode             ProSLIC_SetPowersaveMode
#define Si3228x_VerifyPatch                  ProSLIC_VerifyPatch
#define Si3228x_Init(PCHAN,SZ)               Si3228x_Init_with_Options((PCHAN),(SZ),INIT_NO_OPT)


/* DC Feed */
#ifndef DISABLE_DCFEED_SETUP
extern Si3228x_DCfeed_Cfg Si3228x_DCfeed_Presets[];
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
int Si3228x_ShutdownChannel (proslicChanType_ptr hProslic);

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
int Si3228x_Init_MultiBOM (proslicChanType_ptr *hProslic,int size,int preset);

/*
** Function: Si3228x_Init_with_Options
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
int Si3228x_Init_with_Options (proslicChanType_ptr *pProslic, int size, initOptionsType init_opt);

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
int Si3228x_VerifyControlInterface (proslicChanType_ptr hProslic);

/*
** Function: Si3228x_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
** Returns:
** int (error)
**
*/
int Si3228x_PowerUpConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3228x_PowerDownConverter
**
** Description: 
** Power down DCDC converter (selected channel only)
**
** Returns:
** int (error)
**
*/
int Si3228x_PowerDownConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3228x_Calibrate
**
** Description: 
** Generic calibration function for Si3228x
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
int Si3228x_Calibrate (proslicChanType_ptr *hProslic, int size, uInt8 *calr, int maxTime);

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
int Si3228x_EnableInterrupts (proslicChanType_ptr hProslic);

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
int Si3228x_RingSetup (proslicChanType *pProslic, int preset);

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
int Si3228x_ToneGenSetup (proslicChanType *pProslic, int preset);

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
int Si3228x_FSKSetup (proslicChanType *pProslic, int preset);


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
int Si3228x_DTMFDecodeSetup (proslicChanType *pProslic, int preset);

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
int Si3228x_ZsynthSetup (proslicChanType *pProslic, int preset);

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
int Si3228x_TXAudioGainSetup (proslicChanType *pProslic, int preset);
int Si3228x_RXAudioGainSetup (proslicChanType *pProslic, int preset);
#define Si3228x_AudioGainSetup ProSLIC_AudioGainSetup
int Si3228x_TXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);
int Si3228x_RXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);

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
int Si3228x_DCFeedSetupCfg (proslicChanType *pProslic,ProSLIC_DCfeed_Cfg *cfg,int preset);

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
int Si3228x_PCMSetup (proslicChanType *pProslic, int preset);

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
int Si3228x_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed);

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
int Si3228x_SetLinefeedStatusBroadcast (proslicChanType *pProslic, uInt8 newLinefeed);

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
int Si3228x_MWISetup (proslicChanType *pProslic,uInt16 vpk_mag,uInt16 lcrmask_mwi);

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
int Si3228x_MWIEnable (proslicChanType *pProslic);

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
int Si3228x_MWIDisable (proslicChanType *pProslic);

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
int Si3228x_SetMWIState (proslicChanType *pProslic,uInt8 flash_on);


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
int Si3228x_GetMWIState (proslicChanType *pProslic);

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
int Si3228x_MWI (proslicChanType *pProslic,uInt8 lampOn);

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
int Si3228x_PulseMeterSetup (proslicChanType *pProslic, int preset);

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
int Si3228x_LBCal (proslicChanType_ptr *pProslic, int size);

/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3228x_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset);

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3228x_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset);


/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3228x_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset);


/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provisionary function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/
int Si3228x_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset);

/*
** Function: PROSLIC_dbgSetRXGain
**
** Description: 
** Provisionary function for setting up
** RX path gain.
*/
int Si3228x_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);

/*
** Function: PROSLIC_dbgSetTXGain
**
** Description: 
** Provisionary function for setting up
** TX path gain.
*/
int Si3228x_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);


/*
** Function: PROSLIC_LineMonitor
**
** Description: 
** Monitor line voltages and currents
*/
int Si3228x_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor);


/*
** Function: PROSLIC_PSTNCheck
**
** Description: 
** Continuous monitor of ilong to detect hot pstn line 
*/
int Si3228x_PSTNCheck(proslicChanType *pProslic, proslicPSTNCheckObjType *pstnCheckObj);

/*
** Function: PROSLIC_DiffPSTNCheck
**
** Description: 
** Detection of foreign PSTN 
*/
int Si3228x_DiffPSTNCheck (proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pPSTNCheck);

/*
** Function: PROSLIC_SetPowersaveMode
**
** Description: 
** Enable or Disable powersave mode 
*/
int Si3228x_SetPowersaveMode(proslicChanType *pProslic, int pwrsave);

/*
** Function: PROSLIC_ReadMADCScaled
**
** Description: 
** ReadMADC (or other sensed voltage/currents) and
** return scaled value in int32 format 
*/
int32 Si3228x_ReadMADCScaled(proslicChanType *pProslic, uInt16 addr, int32 scale);


#endif

