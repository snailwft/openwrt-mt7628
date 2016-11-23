/*
** Copyright (c) 2015 by Silicon Laboratories
**
** $Id: si3218x_intf.c 5008 2015-05-14 23:11:07Z nizajerk $
**
** SI3218X ProSLIC interface implementation file
**
** Author(s): 
** cdp
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.   
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si3218x.h"
#include "si3218x_intf.h"
#include "si3218x_registers.h"
#include "proslic_api_config.h"

#define PRAM_ADDR (334 + 0x400)
#define PRAM_DATA (335 + 0x400)
#define SI3218X_IRING_LIM_MAX 0x8B9ACAL
#define SI3218X_REVA   2

#define WriteReg        pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg         pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW          pProslic->deviceId->ctrlInterface->hCtrl
#define Reset           pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay           pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer       pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM        pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM         pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed     pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define getTime         pProslic->deviceId->ctrlInterface->getTime_fptr
#define SetSemaphore    pProslic->deviceId->ctrlInterface->Semaphore_fptr

#define WriteRegX       deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX        deviceId->ctrlInterface->ReadRegister_fptr
#define pProHWX         deviceId->ctrlInterface->hCtrl
#define DelayX          deviceId->ctrlInterface->Delay_fptr
#define pProTimerX      deviceId->ctrlInterface->hTimer
#define ReadRAMX        deviceId->ctrlInterface->ReadRAM_fptr
#define WriteRAMX       deviceId->ctrlInterface->WriteRAM_fptr

#define BROADCAST 0xff
#define DEVICE_KEY_MIN 0x6E
#define DEVICE_KEY_MAX 0x6F

/*
** Define patch parameters that can be modified by API
*/
#define SI3218X_PRAM_VBATH_NEON			995
#define SI3218X_PRAM_LCRMASK_MWI		987

#ifdef ENABLE_DEBUG
static const char LOGPRINT_PREFIX[] = "Si3218x: ";
#endif

/*
** Externs
*/

/* General Configuration */
extern Si3218x_General_Cfg Si3218x_General_Configuration;
#ifdef SIVOICE_MULTI_BOM_SUPPORT
extern const proslicPatch SI3218X_PATCH_A;
extern Si3218x_General_Cfg Si3218x_General_Configuration_MultiBOM[];
extern int si3218x_genconf_multi_max_preset;
#else
extern const proslicPatch SI3218X_PATCH_A_DEFAULT;
#endif

/* Ringing */
#ifndef DISABLE_RING_SETUP
extern Si3218x_Ring_Cfg Si3218x_Ring_Presets[];
#endif

/* Tone Generation */
#ifndef DISABLE_TONE_SETUP
extern Si3218x_Tone_Cfg Si3218x_Tone_Presets[];
#endif

/* FSK */
#ifndef DISABLE_FSK_SETUP
extern Si3218x_FSK_Cfg Si3218x_FSK_Presets[];
#endif

/* DTMF */
#ifndef DISABLE_DTMF_SETUP
extern Si3218x_DTMFDec_Cfg Si3218x_DTMFDec_Presets[];
#endif

/* Zsynth */
#ifndef DISABLE_ZSYNTH_SETUP
extern Si3218x_Impedance_Cfg Si3218x_Impedance_Presets [];
#endif

/* Audio Gain Scratch */
extern Si3218x_audioGain_Cfg Si3218x_audioGain_Presets[];

/* Pulse Metering */
#ifndef DISABLE_PULSE_SETUP
extern Si3218x_PulseMeter_Cfg Si3218x_PulseMeter_Presets [];
#endif

/* PCM */
#ifndef DISABLE_PCM_SETUP
extern Si3218x_PCM_Cfg Si3218x_PCM_Presets [];
#endif

#define SI3218X_RAM_DCDC_DCFF_ENABLE SI3218X_RAM_GENERIC_8
#define GCONF Si3218x_General_Configuration

/*
** Constants
*/
#define BIT20LSB                       1048576L
#define OITHRESH_OFFS                  900L
#define OITHRESH_SCALE                 100L
#define OVTHRESH_OFFS                  71000
#define OVTHRESH_SCALE                 3000L
#define UVTHRESH_OFFS                  4057L
#define UVTHRESH_SCALE                 187L
#define UVHYST_OFFS                    548L
#define UVHYST_SCALE                   47L


/*
** Local functions are defined first
*/


/*
** Function: getChipType
**
** Description: 
** Decode ID register to identify chip type
**
** Input Parameters: 
** ID register value
**
** Return:
** partNumberType
*/
static partNumberType getChipType(uInt8 data)
{
    /* For the parts that have a HV variant, we map to the lower voltage version,
       the actual differences are handled in the constants file
     */
   
    const uInt8 partNums[8] = {
        UNSUPPORTED_PART_NUM, UNSUPPORTED_PART_NUM, UNSUPPORTED_PART_NUM, UNSUPPORTED_PART_NUM, 
        SI32180, SI32180, SI32181, SI32181 };

    uInt8 partNum = (data & 0x38) >> 3; /* PART_NUM[2:0] = ID[5:3] */
    
    return partNums[ partNum ];
}

/*
** Function: probeDaisyChain
**
** Description: 
** Determine number of devices on chain
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** number of channels detected
*/
static int probeDaisyChain (proslicChanType *pProslic){
    uInt8 i=0;
    WriteReg(pProHW,BROADCAST,SI3218X_REG_RAM_ADDR_HI,0x40);
    while (ReadReg(pProHW,i++,SI3218X_REG_RAM_ADDR_HI) == 0x40 && (i<=32));
    WriteReg(pProHW,BROADCAST,SI3218X_REG_RAM_ADDR_HI,0x0);
    return i-1;
}

/*
** Function: isVerifiedProslic
**
** Description: 
** Determine if DAA or ProSLIC present
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** channelType
*/
static int isVerifiedProslic(proslicChanType *pProslic){
uInt8 data;
int returnCode;

    /*
    ** Note:   DIAG3 replaced DAA_CNTL on Si3218x.  Setting DIAG3.0
    **         sets PD_GR909, which powers down the HVIC.  
    */
    data = (ReadReg(pProHW,pProslic->channel,SI3218X_REG_ID)&0x02);
    if(data)
    {
        pProslic->channelType = PROSLIC;
        return RC_NONE;
    }

    data = ReadReg(pProHW,pProslic->channel,SI3218X_REG_PCMMODE); /* always 0x05 after reset */
    if ( data == 0x05) {
        pProslic->channelType = PROSLIC;
        returnCode = RC_NONE;
    }
    else {
        pProslic->channelType = UNKNOWN;
        returnCode = RC_CHANNEL_TYPE_ERR;
    }
    return returnCode;
}

/*
** Function: Si3218x_ConverterSetup
**
** Description: 
** Program revision specific settings before powering converter
**
** Specifically, from general parameters and knowledge that this
** is Si32188x, setup dcff drive, gate drive polarity, and charge pump.
**
** Returns:
** int (error)
**
*/
int Si3218x_ConverterSetup(proslicChanType_ptr pProslic)
{
    //ramData inv_on;
    ramData inv_off;

    /* Option to add a per-channel inversion for maximum flexibility */
    if(pProslic->dcdc_polarity_invert)
    {
        //inv_on  = 0x0L;
        inv_off = 0x100000L;
    }
    else
    {
        //inv_on  = 0x100000L;
        inv_off = 0x0L;
    }

    switch(Si3218x_General_Configuration.bom_option)
    {
    case BO_DCDC_LCQC_5W:
    case BO_DCDC_LCCB:
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_LIFT_EN,0x0L);      /* dcff disabled */
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_SWDRV_POL,inv_off);    /* non-inverted */
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_CPUMP,0x100000L);   /* Charge pump on */
        Delay(pProTimer,20); /* Cpump settle */
        break;

    default:
        return RC_DCDC_SETUP_ERR;
    }

    return RC_NONE;
}

/*
** Function: Si3218x_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
** Returns:
** int (error)
**
*/
int Si3218x_PowerUpConverter(proslicChanType_ptr pProslic)
{
    errorCodeType error = RC_DCDC_SETUP_ERR;
    int32 vbath,vbat;
    uInt8 reg = 0;
    int timer = 0;


    if(pProslic->channelType != PROSLIC) 
        return RC_CHANNEL_TYPE_ERR;

    /*
    ** - powerup digital dc/dc w/ OV clamping and shutdown
    ** - delay
    ** - verify no short circuits by looking for vbath/2
    ** - clear dcdc status
    ** - switch to analog converter with OV clamping only (no shutdown)
    ** - select analog dcdc and disable pwrsave
    ** - delay
    */

    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED,LF_FWD_OHT);  /* Force out of pwrsave mode if called in error */
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED,LF_OPEN);     /* Ensure open line before powering up converter */
    reg = ReadReg(pProHW,pProslic->channel,SI3218X_REG_ENHANCE);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_ENHANCE,reg&0x07);  /* Disable powersave mode */

	WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_DCDC,0x700000L); /* In case OV or UV previously occurred */

    /* 
    ** Setup converter drive polarity and charge pump enable 
    ** based on bom
    */

    error = Si3218x_ConverterSetup(pProslic);

    if(error != RC_NONE)
    {
        #ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            LOGPRINT ("%sChannel %d : DCDC initialization failed\n",LOGPRINT_PREFIX, pProslic->channel);
        }
        #endif 
        return error;
    }
    
	WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_DCDC,0x600000L);
	WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_BIAS,0x200000L);   
	WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_HVIC,0x200000L);
    Delay(pProTimer,50); 
    
    vbath = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATH_EXPECT);
    vbat = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_VBAT);
    if(vbat & 0x10000000L)
        vbat |= 0xF0000000L;
    if(vbat < (vbath / 2)) {
        pProslic->channelEnable = 0;
        error = RC_VBAT_UP_TIMEOUT;
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_DCDC, 0x300000L); /* shutdown converter */
        #ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            LOGPRINT ("%sChannel %d : DCDC Short Circuit Failure - disabling channel\n",LOGPRINT_PREFIX, pProslic->channel);
            LOGPRINT ("VBAT = %d.%d\n", (int)((vbat/SCALE_V_MADC)/1000),(int)(((vbat/SCALE_V_MADC) - (vbat/SCALE_V_MADC)/1000*1000)));
        }
        #endif      
        return error;  
    }
    else { /* Enable analog converter */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DCDC_STATUS,0L);   
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_DCDC,0x400000L);
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_ENHANCE,reg);   /* Restore ENHANCE */
        Delay(pProTimer,50);
    }

    /*
    ** - monitor vbat vs expected level (VBATH_EXPECT)
    */
    vbath = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATH_EXPECT);
    do
    {
        vbat = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_VBAT);
        if(vbat & 0x10000000L)
            vbat |= 0xF0000000L;
        Delay(pProTimer,10);
    }while((vbat < (vbath - COMP_5V))&&(timer++ < SI3218X_TIMEOUT_DCDC_UP));  /* 2 sec timeout */

#ifdef ENABLE_DEBUG
    if(pProslic->debugMode)
    {
        LOGPRINT ("%sChannel %d : VBAT Up = %d.%d v\n",
            LOGPRINT_PREFIX,
            pProslic->channel,(int)((vbat/SCALE_V_MADC)/1000), 
            (int)(((vbat/SCALE_V_MADC) - (vbat/SCALE_V_MADC)/1000*1000)));
    }
#endif
    if(timer > SI3218X_TIMEOUT_DCDC_UP)
    {
        /* Error handling - shutdown converter, disable channel, set error tag */
        pProslic->channelEnable = 0;
        error = RC_VBAT_UP_TIMEOUT;
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_DCDC, 0x900000L); /* shutdown converter */
#ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            vbat = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_DCDC_STATUS);   
            LOGPRINT ("%sChannel %d : DCDC Power up timeout : Status=0x%08X\n",LOGPRINT_PREFIX, pProslic->channel,vbat);
       }
#endif      
    }

    return error;  
}

/*
** Function: Si3218x_PowerDownConverter
**
** Description: 
** Safely powerdown dcdc converter after ensuring linefeed
** is in the open state.  Test powerdown by setting error
** flag if detected voltage does no fall below 5v.
**
** Returns:
** int (error)
**
*/
int Si3218x_PowerDownConverter(proslicChanType_ptr pProslic)
{
    errorCodeType error = RC_NONE;
    int32 vbat;
    int timer = 0;

    if(pProslic->channelType != PROSLIC) 
        return RC_CHANNEL_TYPE_ERR;
    
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED, LF_FWD_OHT); /* Force out of powersave mode */
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED, LF_OPEN);
    Delay(pProTimer,50);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_DCDC,0x900000L);
    Delay(pProTimer,50);

    /*
    ** Verify VBAT falls below 5v
    */
    do
    {
        vbat = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_VBAT);
        if(vbat & 0x10000000L)
            vbat |= 0xF0000000L;
        Delay(pProTimer,10);
    }while((vbat > COMP_5V)&&(timer++ < SI3218X_TIMEOUT_DCDC_DOWN));  /* 2 sec timeout */
#ifdef ENABLE_DEBUG
    if(pProslic->debugMode)
    {
        LOGPRINT ("%sChannel %d : VBAT Down = %d.%d v\n",
            LOGPRINT_PREFIX,
            pProslic->channel,(int)((vbat/SCALE_V_MADC)/1000), 
            (int)(((vbat/SCALE_V_MADC) - (vbat/SCALE_V_MADC)/1000*1000)));
    }
#endif
    if(timer > SI3218X_TIMEOUT_DCDC_DOWN)
    {
        /* Error handling - shutdown converter, disable channel, set error tag */
        pProslic->channelEnable = 0;
        error = RC_VBAT_DOWN_TIMEOUT;
#ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            LOGPRINT ("%sChannel %d : DCDC Power Down timeout\n", LOGPRINT_PREFIX, pProslic->channel);
        }
#endif      
    }

    return error;  
}

/*
** Function: Si3218x_Calibrate
**
** Description: 
** Performs calibration based on passed ptr to array of
** desired CALRn settings.
**
** Starts calibration on all channels sequentially (not broadcast)
** and continuously polls for completion.  Return error code if
** CAL_EN does not clear for each enabled channel within the passed
** timeout period.
*/
int Si3218x_Calibrate(proslicChanType_ptr *pProslic, int maxChan, uInt8 *calr, int maxTime)
{
  int i;
  int cal_en = 0;
  int cal_en_chan = 0;
  int timer = 0;
  /*
  ** Launch cals sequentially (not serially)
  */
  for(i=0;i<maxChan;i++)
  {
      if((pProslic[i]->channelEnable)&&(pProslic[i]->channelType == PROSLIC))
      {
        pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3218X_REG_CALR0,calr[0]);
        pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3218X_REG_CALR1,calr[1]);
        pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3218X_REG_CALR2,calr[2]);
        pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3218X_REG_CALR3,calr[3]);
      }
  }

  /*
  ** Wait for completion or timeout
  */
  do 
  {
    cal_en = 0;
    pProslic[0]->DelayX(pProslic[0]->pProTimerX,10);
    for(i=0;i<maxChan;i++)
    {
      if(pProslic[i]->channelType != PROSLIC) continue;

      if(pProslic[i]->channelEnable)
      {
        cal_en_chan = pProslic[i]->ReadRegX(pProslic[i]->pProHWX,pProslic[i]->channel,SI3218X_REG_CALR3);

        if((cal_en_chan&0x80)&&(timer == maxTime))
        {
#ifdef ENABLE_DEBUG
          if(pProslic[i]->debugMode)
          {
              LOGPRINT("%sCalibration timout channel %d\n", LOGPRINT_PREFIX, i);
          }
#endif
  pProslic[i]->channelEnable = 0;
          pProslic[i]->error = RC_CAL_TIMEOUT;
        }
        cal_en |= cal_en_chan;
      }
    }         
  }while((timer++ <= maxTime)&&(cal_en&0x80));

	return cal_en;
}

/*
** Functions below are defined in header file and can be called by external files
*/

/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/*
** Function: Si3218x_ShutdownChannel
**
** Description: 
** Safely shutdown channel w/o interruptions to
** other active channels
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0

*/
int Si3218x_ShutdownChannel (proslicChanType_ptr pProslic){
    /*
    ** set linefeed to open state, powerdown dcdc converter
    */
    Si3218x_SetLinefeedStatus(pProslic,LF_OPEN);
    Delay(pProTimer,10);
    Si3218x_PowerDownConverter(pProslic);
    
    return RC_NONE;
}

/*
** Function: Si3218x_VerifyControlInterface
**
** Description: 
** Check control interface readback cababilities
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0
*/
int Si3218x_VerifyControlInterface (proslicChanType_ptr pProslic)
{
    int16 i;
    int16 numOfChan;

    if (isVerifiedProslic(pProslic) == RC_CHANNEL_TYPE_ERR) 
	{
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sSkipping Control Interface Verification on Non-ProSLIC Channel %d\n", 
                    LOGPRINT_PREFIX,
                    pProslic->channel);
#endif

        return RC_CHANNEL_TYPE_ERR;
	}

    /* Broadcast OK */
    numOfChan = (int16)probeDaisyChain(pProslic);
    if (numOfChan == 0)
        return RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT ("%sFound %d channels\n",LOGPRINT_PREFIX, numOfChan);
#endif
    WriteReg(pProHW,BROADCAST,SI3218X_REG_PCMRXLO,0x5a);
    WriteRAM(pProHW,BROADCAST,SI3218X_RAM_UNUSED449,0x12345678L);

    for (i=0;i<numOfChan;i++){
        /*Try to write innocuous register to test SPI is working*/
                
        if (ReadReg(pProHW,(uInt8)i,SI3218X_REG_PCMRXLO) != 0x5A){
            PROSLIC_PRINT_ERROR(pProslic,RC_SPI_FAIL);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sProslic %d not communicating. Register access fail.\n", LOGPRINT_PREFIX,i);
#endif
            return RC_SPI_FAIL;
        }       
        if (ReadRAM(pProHW,(uInt8)i,SI3218X_RAM_UNUSED449) != 0x12345678L){
            PROSLIC_PRINT_ERROR(pProslic,RC_SPI_FAIL);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
                LOGPRINT("%sProslic %d not communicating. RAM access fail.\n", LOGPRINT_PREFIX,i);
#endif
            return RC_SPI_FAIL;
        }

    }
    return 0;
}



/*
** Function: Si3218x_Init_MultiBOM
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate everything except madc & lb
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/
#ifdef SIVOICE_MULTI_BOM_SUPPORT
int Si3218x_Init_MultiBOM (proslicChanType_ptr *pProslic, int size, int preset){

	if(preset < si3218x_genconf_multi_max_preset)
	{
	    /* Copy selected General Configuration parameters to Std structure */
		Si3218x_General_Configuration = Si3218x_General_Configuration_MultiBOM[preset];
	}
	else
	{
		return RC_INVALID_PRESET;
	}
	return Si3218x_Init_with_Options(pProslic,size, INIT_NO_OPT);
}
#endif


/*
** Function: Si3218x_SelectPatch
**
** Select patch based on general parameters
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
** patch:    Pointer to proslicPatch pointer
**
** Return:
** error code
*/
int Si3218x_SelectPatch(proslicChanType_ptr pProslic, const proslicPatch **patch)
{

#ifdef SIVOICE_MULTI_BOM_SUPPORT
	if(Si3218x_General_Configuration.bom_option == BO_DCDC_LCQC_5W || Si3218x_General_Configuration.bom_option == BO_DCDC_LCCB)
	{
		*patch = &(SI3218X_PATCH_A);
	}
	else
	{
        #ifdef ENABLE_DEBUG
		if (pProslic->debugMode)
		{
			LOGPRINT("%sChannel %d : Invalid Patch\n", LOGPRINT_PREFIX, pProslic->channel);
		}
        #endif
		pProslic->channelEnable = 0;
		pProslic->error = RC_INVALID_PATCH;
		return RC_INVALID_PATCH;
	}
#else
	*patch = &(SI3218X_PATCH_A_DEFAULT);
#endif

    return RC_NONE;
}


/*
** Function: Si3218x_GenParamUpdate
**
** Update general parameters
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/
int Si3218x_GenParamUpdate(proslicChanType_ptr pProslic,initSeqType seq)
{
 ramData ram_data;

    switch(seq)
    {
    case INIT_SEQ_PRE_CAL:
        /* 
        ** Force pwrsave off and disable AUTO-tracking - set to user configured state after cal 
        */
        WriteReg(pProHW, pProslic->channel,SI3218X_REG_ENHANCE,0);
        WriteReg(pProHW, pProslic->channel,SI3218X_REG_AUTO,0x2F); 
     
        /* 
        ** General Parameter Updates 
        */

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_P_TH_HVIC, Si3218x_General_Configuration.p_th_hvic);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_COEF_P_HVIC, Si3218x_General_Configuration.coef_p_hvic);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_BAT_HYST, Si3218x_General_Configuration.bat_hyst);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VBATH_EXPECT, Si3218x_General_Configuration.vbath_expect);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VBATR_EXPECT, Si3218x_General_Configuration.vbatr_expect);

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_PWRSAVE_TIMER, Si3218x_General_Configuration.pwrsave_timer);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_OFFHOOK_THRESH, Si3218x_General_Configuration.pwrsave_ofhk_thresh);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VBAT_TRACK_MIN, Si3218x_General_Configuration.vbat_track_min);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VBAT_TRACK_MIN_RNG, Si3218x_General_Configuration.vbat_track_min_rng);

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_THERM_DBI, Si3218x_General_Configuration.therm_dbi);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VOV_DCDC_SLOPE, Si3218x_General_Configuration.vov_dcdc_slope);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VOV_DCDC_OS, Si3218x_General_Configuration.vov_dcdc_os);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VOV_RING_BAT_MAX, Si3218x_General_Configuration.vov_ring_bat_max);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_VERR, Si3218x_General_Configuration.dcdc_verr);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_VERR_HYST, Si3218x_General_Configuration.dcdc_verr_hyst);

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_PD_UVLO, Si3218x_General_Configuration.pd_uvlo);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_PD_OVLO, Si3218x_General_Configuration.pd_ovlo);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_PD_OCLO, Si3218x_General_Configuration.pd_oclo);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_PD_SWDRV, Si3218x_General_Configuration.pd_swdrv);

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_UVPOL, Si3218x_General_Configuration.dcdc_uvpol);

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_RNGTYPE, Si3218x_General_Configuration.dcdc_rngtype);

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_ANA_TOFF, Si3218x_General_Configuration.dcdc_ana_toff);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_ANA_TONMIN, Si3218x_General_Configuration.dcdc_ana_tonmin);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_ANA_TONMAX, Si3218x_General_Configuration.dcdc_ana_tonmax);

  
        /*
        ** Hardcoded RAM
        */

        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_OITHRESH_LO,GCONF.i_oithresh_lo);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_OITHRESH_HI,GCONF.i_oithresh_hi);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_OVTHRESH,GCONF.v_ovthresh);

        ram_data = (GCONF.v_uvthresh > UVTHRESH_OFFS)?(GCONF.v_uvthresh - UVTHRESH_OFFS)/UVTHRESH_SCALE:0L;
        ram_data *= BIT20LSB;
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_UVTHRESH,ram_data);

        ram_data = (GCONF.v_uvhyst > UVHYST_OFFS)?(GCONF.v_uvhyst - UVHYST_OFFS)/UVHYST_SCALE:0L;
        ram_data *= BIT20LSB;
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_UVHYST,ram_data);

        /* Set default audio gain based on PM bom */
        if(Si3218x_General_Configuration.pm_bom == BO_PM_BOM)
        {
            WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_SCALE_KAUDIO,BOM_KAUDIO_PM);
            WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_AC_ADC_GAIN,BOM_AC_ADC_GAIN_PM);
        }
        else
        {
            WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_SCALE_KAUDIO,BOM_KAUDIO_NO_PM);
            WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_AC_ADC_GAIN,BOM_AC_ADC_GAIN_NO_PM);
        }

        /* 
        ** Hardcoded changes to default settings 
        */
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_PDN,0x80);                /* madc powered in open state */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_A1_1,0x71EB851L); /* Fix HPF corner */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PD_REF_OSC,0x200000L);    /* PLL freerun workaround */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ILOOPLPF,0x4EDDB9L);      /* 20pps pulse dialing enhancement */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ILONGLPF,0x806D6L);       /* 20pps pulse dialing enhancement */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VDIFFLPF,0x10038DL);      /* 20pps pulse dialing enhancement */
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_DCDC_VREF_CTRL,0x0L);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VCM_TH,0x106240L);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VCMLPF,0x10059FL);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_CM_SPEEDUP_TIMER,0x0F0000);
        WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_VCM_HYST,0x206280L);
        
        /* Prevent Ref Osc from powering down in PLL Freerun mode (pd_ref_osc) */
        ram_data = ReadRAM(pProHW, pProslic->channel,SI3218X_RAM_PWRSAVE_CTRL_LO);
		WriteRAM(pProHW, pProslic->channel,SI3218X_RAM_PWRSAVE_CTRL_LO,ram_data&0x07FFFFFFL); /* clear b27 */ 
        break;


    case INIT_SEQ_POST_CAL:
        WriteReg(pProHW, pProslic->channel,SI3218X_REG_ENHANCE, Si3218x_General_Configuration.enhance&0x1F);
        WriteReg(pProHW, pProslic->channel,SI3218X_REG_AUTO, Si3218x_General_Configuration.auto_reg);
        if(Si3218x_General_Configuration.zcal_en)
        {
            WriteReg(pProHW,pProslic->channel, SI3218X_REG_ZCAL_EN, 0x04);
        }
        break;

    default:
        break;
    }
    return RC_NONE;
}


/*
** Function: Si3218x_Init_with_Options
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate everything except madc & lb
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/

int Si3218x_Init_with_Options (proslicChanType_ptr *pProslic, int size, initOptionsType init_opt)
{
  /*
   ** This function will initialize the chipRev and chipType members in pProslic
   ** as well as load the initialization structures.
   */
   
  uInt8 data;
  uInt8 calSetup[] = {0x00, 0x00, 0x01, 0x80};  /* CALR0-CALR3 */ 
  int j,k;
  const proslicPatch *patch;
  uInt8 status;

    /*
    **
    ** First qualify general parameters by identifying valid device key.  This
    ** will prevent inadvertent use of other device's preset files, which could
    ** lead to improper initialization and high current states.
    */

    data = Si3218x_General_Configuration.device_key;

    if((data < DEVICE_KEY_MIN)||(data > DEVICE_KEY_MAX)) 
    {
        pProslic[0]->error = RC_INVALID_GEN_PARAM;
        return pProslic[0]->error;
    }

#ifdef ENABLE_DEBUG
  LOGPRINT("%s(%d): size = %d init_opt = %d\n", __FUNCTION__, __LINE__, size, init_opt);
#endif
  if( (init_opt == INIT_REINIT) || (init_opt == INIT_SOFTRESET) )
  {
    if(init_opt == INIT_REINIT)
    {
      j = 1;
    }
    else
    {
      j = size;
    }

    /* Capture the line state and then power down the converter */
    for(k = 0; k < j; k++)
    {
      pProslic[k]->scratch = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_LINEFEED);
      Si3218x_PowerDownConverter(pProslic[k]);
    }

    (*pProslic)->DelayX((*pProslic)->pProTimerX,10);

    if(init_opt == INIT_REINIT)
    {
      for(k= 0; k  < j; k++)
      {
        pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_RESET,0x01); /* device ch 0 */
      }
    }
        (*pProslic)->DelayX((*pProslic)->pProTimerX,100);

  } 

	if( (init_opt != INIT_REINIT)  && (init_opt != INIT_SOFTRESET) )
	{
		/*
		** Identify channel type (ProSLIC or DAA) before initialization.
		** Channels identified as DAA channels will not be modified during
		** the proslic initialization.
        **
        ** Do not return if non-ProSLIC channel is found, simply skip it
		*/
		for (k=0;k<size;k++)
    {
			isVerifiedProslic(pProslic[k]);     /* Populate channelType parameter in channel struct */

#ifdef ENABLE_DEBUG
			if (pProslic[k]->debugMode)
      {
          const char fmt_string[] = "%sChannel %d : Type = %s\n";
          const char *dev_type    = "UNKNOWN";
        if(pProslic[k]->channelType == PROSLIC)
        {
          dev_type = "PROSLIC";	
        }
        else if(pProslic[k]->channelType == DAA)
        {
          dev_type = "DAA";	
        }
        LOGPRINT(fmt_string, LOGPRINT_PREFIX, pProslic[k]->channel, dev_type);
      }
#endif /* ENABLE_DEBUG */
		}
  } /* (init_opt != INIT_REINIT)  && (init_opt != INIT_SOFTRESET) */
  else
  {
    /* We are assuming PROSLIC only - for cases of '178, customers should not use INIT_SOFTRESET */
    if(init_opt == INIT_SOFTRESET)
    {
      for(k = 0; k< size; k++)
      {
        data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_ID);
        if( (data != 0) && (data != 0xFF) )
        {
          /* We'll assume we have a ProSLIC where we expect one... */
          pProslic[k]->channelType = PROSLIC;
          pProslic[k]->error = RC_NONE;
        }
      }
    }
  }

  if((init_opt != INIT_REINIT) )
  {
		/*
		** Read channel id to establish chipRev and chipType
		*/
		for (k=0;k<size;k++)
    {
			if(pProslic[k]->channelType == PROSLIC) 
      {
                pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,1,1);
				data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_ID);
				pProslic[k]->deviceId->chipRev = data&0x7;
				pProslic[k]->deviceId->chipType = getChipType(data);
#ifdef ENABLE_DEBUG
				if (pProslic[k]->debugMode)
        {
          LOGPRINT("%s: Channel %d : Type = %d\n", LOGPRINT_PREFIX, pProslic[k]->channel,pProslic[k]->deviceId->chipType);
          LOGPRINT("%s: Channel %d : Rev  = %d\n", LOGPRINT_PREFIX, pProslic[k]->channel,pProslic[k]->deviceId->chipRev);
        }
#endif /* ENABLE_DEBUG */
			}

      if(init_opt != INIT_SOFTRESET)
      {
	      ProSLIC_SetUserMode(pProslic[k],TRUE, FALSE);
      }
    }   /* for all channels */

		/*
		** Probe each channel and enable all channels that respond 
		*/
		for (k=0;k<size;k++)
    {
      if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
      {
        pProslic[k]->error = RC_NONE;
        pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_PCMRXLO,0x5a);
        if (pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_PCMRXLO) != 0x5A)
        {
          pProslic[k]->channelEnable = 0;
          pProslic[k]->error = RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
          if (pProslic[k]->debugMode)
          {
            LOGPRINT("%s: Channel %d : Communications fault\n", LOGPRINT_PREFIX, 	pProslic[k]->channel);
          }
#endif
          return RC_SPI_FAIL;
        }
      }
    }
  } /* init_opt != INIT_REINIT */

   
  if( (init_opt != INIT_NO_PATCH_LOAD) && (init_opt != INIT_SOFTRESET) )
  {
    /*
    ** Load patch (load on every channel since single channel device)
    */
    for (k=0;k<size;k++)
    {
      if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
      {

        /* Select Patch*/
        if (pProslic[k]->deviceId->chipRev == SI3218X_REVA ) 
        {
          status = (uInt8) Si3218x_SelectPatch(pProslic[k],&patch);
        }
        else
        {
#ifdef ENABLE_DEBUG
          if (pProslic[k]->debugMode)
          {
            LOGPRINT("%sChannel %d : Unsupported Device Revision (%d)\n",
                    LOGPRINT_PREFIX,
                    pProslic[k]->channel,pProslic[k]->deviceId->chipRev );
          }
#endif
          pProslic[k]->channelEnable = 0;
          pProslic[k]->error = RC_UNSUPPORTED_DEVICE_REV;
          return RC_UNSUPPORTED_DEVICE_REV;
        }

        data = 1; /* Use this as a flag to see if we need to load the patch */
        /* Check if the patch is different than we expect */
        if(init_opt == INIT_SOFTRESET)
        {
          uInt32 patchData;
          patchData = pProslic[k]->ReadRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,SI3218X_RAM_PATCHID);

          if( patchData ==  patch->patchSerial)
          {
            data = 0; 
          }
        }

        /* Load Patch */
        if(status == RC_NONE)
        {
          if(data == 1)
          {
            Si3218x_LoadPatch(pProslic[k],patch);
          }
        }
        else
        {
          return status;
        }

        /* Optional Patch Verification */
#ifndef DISABLE_VERIFY_PATCH
        if(data == 1)
        {
          data = (uInt8)Si3218x_VerifyPatch(pProslic[k],patch);
          if (data)
          {
            pProslic[k]->channelEnable=0;
            pProslic[k]->error = RC_PATCH_ERR;  
          } 
        }
#endif                          
      } /* channel == PROSLIC */
    } /* for all channles */
  }/* init_opt  - need to reload patch */
    
  /*
  ** Load general parameters - includes all BOM dependencies
  */
  for (k=0;k<size;k++)
  { 
    if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
    {
        Si3218x_GenParamUpdate(pProslic[k],INIT_SEQ_PRE_CAL);
    }

    pProslic[k]->WriteRAMX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_RAM_IRING_LIM,SI3218X_IRING_LIM_MAX);
  }


	if((init_opt != INIT_NO_CAL) && (init_opt != INIT_SOFTRESET)) /* Must recal on single channel devices */
	{
		/*
		** Calibrate (madc offset)
		*/
		Si3218x_Calibrate(pProslic,size,calSetup,TIMEOUT_MADC_CAL);
	}/* init_opt */

  /*
  ** Bring up DC/DC converters sequentially to minimize
  ** peak power demand on VDC
  */
  for (k=0;k<size;k++)
  { 
    if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
    {
      pProslic[k]->error = Si3218x_PowerUpConverter(pProslic[k]);
    }
  }

	if((init_opt != INIT_NO_CAL) && (init_opt != INIT_SOFTRESET))
  {
    /*
    ** Calibrate remaining cals (except madc, lb)
    */
    calSetup[1] = SI3218X_CAL_STD_CALR1;
    calSetup[2] = SI3218X_CAL_STD_CALR2;

    Si3218x_Calibrate(pProslic,size,calSetup,TIMEOUT_GEN_CAL);
  }

  /*
  ** Apply post calibration general parameters 
  */
  for (k=0;k<size;k++)
  { 

    if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
    {
        Si3218x_GenParamUpdate(pProslic[k],INIT_SEQ_POST_CAL);
    }

    /* Restore linefeed state after initialization for REINIT or SOFTRESET */
    if((init_opt == INIT_REINIT) || (init_opt == INIT_SOFTRESET))
    {
      pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_LINEFEED,pProslic[k]->scratch);
    }
  }

  /* 
  ** If any channel incurred a non-fatal error, return
  ** RC_NON_FATAL_INIT_ERR to trigger user to read each channel's
  ** error status
  */
  for (k=0;k<size;k++)
  {
    if(pProslic[k]->error != RC_NONE)
    {
      return RC_NON_FATAL_INIT_ERR;
    }
  }

  return RC_NONE;
}

/*
** Function: Si3218x_LBCal
**
** Description: 
** Run canned longitudinal balance calibration.  Each channel
** may be calibrated in parallel since there are no shared
** resources between si3218x devices.
**
** Input Parameters: 
** pProslic: pointer to array of PROSLIC channel objects
** size:     number of PROSLIC channel objects   
**
** Return:
** 0
*/
int Si3218x_LBCal(proslicChanType_ptr *pProslic, int size)
{
    int k;
    int i;
    uInt8 data;
    int timeout = 0;

#ifdef DISABLE_MALLOC
    uInt8 lf[64]; 
	uInt8 enhance[64];

    if (size > 64) {
        LOGPRINT("%sToo many channels - wanted %d, max of %d\n",
                LOGPRINT_PREFIX,
                 size, 64);
        return RC_NO_MEM;
    }
#else
    uInt8 *lf;
	uInt8 *enhance;

    lf = SIVOICE_MALLOC(size * sizeof(uInt8));
    if (lf == 0) {
        return RC_NO_MEM;
    }
    enhance = SIVOICE_MALLOC(size * sizeof(uInt8));
    if (enhance == 0) {
        SIVOICE_FREE(lf);
        return RC_NO_MEM;
    }
#endif


    /* Start Cal on each channel first */
    for (k=0;k<size;k++)
    {
        if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
        {
#ifdef ENABLE_DEBUG
            if(pProslic[k]->debugMode)
            {
                LOGPRINT("%sStarting LB Cal on channel %d\n",LOGPRINT_PREFIX,pProslic[k]->channel);
            }
#endif
            enhance[k] = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_ENHANCE); 
			lf[k] = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_LINEFEED); 

			/* Disable powersave mode and set linefeed to fwd active */
			Si3218x_SetPowersaveMode(pProslic[k],PWRSAVE_DISABLE);
			Si3218x_SetLinefeedStatus(pProslic[k],LF_FWD_ACTIVE);
                        
            pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_CALR0,CAL_LB_ALL); /* enable LB cal */
            pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_CALR3,0x80); /* start cal */

        }
    }

    /* Poll for completion sequentially */
    for (k=0; k<size; k++)
    {
        if ((pProslic[k]->channelEnable)&&(pProslic[k]->channelType == PROSLIC))
        {
            i=0;
            do {
                data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,
                                             pProslic[k]->channel,
                                             SI3218X_REG_CALR3);
                pProslic[k]->DelayX(pProslic[k]->pProTimerX, 10);

            } while (data&0x80 && ++i<=TIMEOUT_LB_CAL);

            if (i >= TIMEOUT_LB_CAL) 
			{
#ifdef ENABLE_DEBUG
                if (pProslic[k]->debugMode)
                    LOGPRINT("%sCalibration timeout channel %d\n",
                            LOGPRINT_PREFIX,
                             pProslic[k]->channel);
#endif
                pProslic[k]->error = RC_CAL_TIMEOUT;
                pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_LINEFEED,LF_OPEN); 
                timeout = 1;
            } 
			else 
			{
                pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_LINEFEED,lf[k]); 
            }
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,SI3218X_REG_ENHANCE,enhance[k]); 
        }
    }

#ifndef DISABLE_MALLOC
    SIVOICE_FREE(lf);
	SIVOICE_FREE(enhance);
#endif
    if (timeout != 0) {
        return RC_CAL_TIMEOUT;
    } else {
        return RC_NONE;
    }
}

/*
** Function: Si3218x_EnableInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
**
** Returns:
** 0
*/
int Si3218x_EnableInterrupts (proslicChanType_ptr pProslic){
    uInt8 i;
#ifdef GCI_MODE
    uInt8 data;
#endif
    /* Clear pending interrupts first */
    for(i = SI3218X_REG_IRQ1; i < SI3218X_REG_IRQ4; i++)
    {
#ifdef GCI_MODE
      data = ReadReg(pProHW,pProslic->channel, i);
      WriteReg(pProHW,pProslic->channel,i,data); /*clear interrupts (gci only)*/
#else
      (void)ReadReg(pProHW,pProslic->channel, i);
#endif

    }

    WriteReg (pProHW,pProslic->channel,SI3218X_REG_IRQEN1,Si3218x_General_Configuration.irqen1);
    WriteReg (pProHW,pProslic->channel,SI3218X_REG_IRQEN2,Si3218x_General_Configuration.irqen2);
    WriteReg (pProHW,pProslic->channel,SI3218X_REG_IRQEN3,Si3218x_General_Configuration.irqen3);
    WriteReg (pProHW,pProslic->channel,SI3218X_REG_IRQEN4,Si3218x_General_Configuration.irqen4);

    return RC_NONE;
}

/*
**
** PROSLIC CONFIGURATION FUNCTIONS
**
*/

/*
** Function: Si3218x_RingSetup
**
** Description: 
** configure ringing
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     ring preset
**
** Returns:
** 0
*/
#ifndef DISABLE_RING_SETUP
int Si3218x_RingSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RTPER,Si3218x_Ring_Presets[preset].rtper);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RINGFR,Si3218x_Ring_Presets[preset].freq);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RINGAMP,Si3218x_Ring_Presets[preset].amp);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RINGPHAS,Si3218x_Ring_Presets[preset].phas);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RINGOF,Si3218x_Ring_Presets[preset].offset);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_RING,Si3218x_Ring_Presets[preset].slope_ring);

    if(Si3218x_Ring_Presets[preset].iring_lim > SI3218X_IRING_LIM_MAX)
    {
      Si3218x_Ring_Presets[preset].iring_lim = SI3218X_IRING_LIM_MAX;  
    }

    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_IRING_LIM,Si3218x_Ring_Presets[preset].iring_lim);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RTACTH,Si3218x_Ring_Presets[preset].rtacth);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RTDCTH,Si3218x_Ring_Presets[preset].rtdcth);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RTACDB,Si3218x_Ring_Presets[preset].rtacdb);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RTDCDB,Si3218x_Ring_Presets[preset].rtdcdb);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VOV_RING_BAT,Si3218x_Ring_Presets[preset].vov_ring_bat);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VOV_RING_GND,Si3218x_Ring_Presets[preset].vov_ring_gnd);

#ifndef NOCLAMP_VBATR
	/* Always limit VBATR_EXPECT to the general configuration maximum */
	if(Si3218x_Ring_Presets[preset].vbatr_expect > Si3218x_General_Configuration.vbatr_expect)
	{
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATR_EXPECT,Si3218x_General_Configuration.vbatr_expect);
    #ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sRingSetup : VBATR_EXPECT : Clamped to Gen Conf Limit\n",LOGPRINT_PREFIX);
		}
    #endif
	}
	else
	{
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATR_EXPECT,Si3218x_Ring_Presets[preset].vbatr_expect);
	}

#else
	WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATR_EXPECT,Si3218x_Ring_Presets[preset].vbatr_expect);
#endif


    WriteReg(pProHW,pProslic->channel,SI3218X_REG_RINGTALO,Si3218x_Ring_Presets[preset].talo);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_RINGTAHI,Si3218x_Ring_Presets[preset].tahi);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_RINGTILO,Si3218x_Ring_Presets[preset].tilo);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_RINGTIHI,Si3218x_Ring_Presets[preset].tihi);
  
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VBAT_TRACK_MIN_RNG,Si3218x_Ring_Presets[preset].dcdc_vref_min_rng);

    /* 
    ** LPR Handler
    **
    ** If USERSTAT == 0x01, adjust RINGCON and clear USERSTAT
    */
    if (Si3218x_Ring_Presets[preset].userstat == 0x01)
    {
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_RINGCON,(0x80|Si3218x_Ring_Presets[preset].ringcon) & ~(0x40));
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT,0x00);
    }
    else
    {
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_RINGCON,Si3218x_Ring_Presets[preset].ringcon);
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT,Si3218x_Ring_Presets[preset].userstat);
    }


    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VCM_RING,Si3218x_Ring_Presets[preset].vcm_ring);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VCM_RING_FIXED,Si3218x_Ring_Presets[preset].vcm_ring_fixed);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DELTA_VCM,Si3218x_Ring_Presets[preset].delta_vcm);


    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DCDC_RNGTYPE,Si3218x_Ring_Presets[preset].dcdc_rngtype);


    /* 
    ** If multi bom supported **AND** a buck boost converter
    ** is being used, force dcdc_rngtype to be fixed.
    */
#ifdef SIVOICE_MULTI_BOM_SUPPORT
#define DCDC_RNGTYPE_BKBT     0L
    /* Automatically adjust DCDC_RNGTYPE */
    if(Si3218x_General_Configuration.bom_option == BO_DCDC_BUCK_BOOST) 
    {
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DCDC_RNGTYPE,DCDC_RNGTYPE_BKBT);
    }
#endif


    return RC_NONE;
}
#endif
/*
** Function: Si3218x_ToneGenSetup
**
** Description: 
** configure tone generators
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     tone generator preset
**
** Returns:
** 0
*/
#ifndef DISABLE_TONE_SETUP
int Si3218x_ToneGenSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_OSC1FREQ,Si3218x_Tone_Presets[preset].osc1.freq);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_OSC1AMP,Si3218x_Tone_Presets[preset].osc1.amp);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_OSC1PHAS,Si3218x_Tone_Presets[preset].osc1.phas);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TAHI,(Si3218x_Tone_Presets[preset].osc1.tahi));
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TALO,(Si3218x_Tone_Presets[preset].osc1.talo));
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TIHI,(Si3218x_Tone_Presets[preset].osc1.tihi));
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TILO,(Si3218x_Tone_Presets[preset].osc1.tilo));
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_OSC2FREQ,Si3218x_Tone_Presets[preset].osc2.freq);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_OSC2AMP,Si3218x_Tone_Presets[preset].osc2.amp);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_OSC2PHAS,Si3218x_Tone_Presets[preset].osc2.phas);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O2TAHI,(Si3218x_Tone_Presets[preset].osc2.tahi));
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O2TALO,(Si3218x_Tone_Presets[preset].osc2.talo));
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O2TIHI,(Si3218x_Tone_Presets[preset].osc2.tihi));
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O2TILO,(Si3218x_Tone_Presets[preset].osc2.tilo));
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_OMODE,(Si3218x_Tone_Presets[preset].omode));
    return RC_NONE;
}
#endif
/*
** Function: Si3218x_FSKSetup
**
** Description: 
** configure fsk
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     fsk preset
**
** Returns:
** 0
*/
#ifndef DISABLE_FSK_SETUP
int Si3218x_FSKSetup (proslicChanType *pProslic, int preset){
    uInt8 data; 

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteReg(pProHW,pProslic->channel,SI3218X_REG_FSKDEPTH,0x08); /* Clear Buffer */
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TAHI,0);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TIHI,0);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TILO,0);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_O1TALO,0x13);

    data = ReadReg(pProHW,pProslic->channel,SI3218X_REG_OMODE);
    if (Si3218x_FSK_Presets[preset].eightBit)
        data |= 0x80;
    else 
        data &= ~(0x80);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_FSKDEPTH,Si3218x_FSK_Presets[preset].fskdepth);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_OMODE,data);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_FSK01,Si3218x_FSK_Presets[preset].fsk01);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_FSK10,Si3218x_FSK_Presets[preset].fsk10);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_FSKAMP0,Si3218x_FSK_Presets[preset].fskamp0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_FSKAMP1,Si3218x_FSK_Presets[preset].fskamp1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_FSKFREQ0,Si3218x_FSK_Presets[preset].fskfreq0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_FSKFREQ1,Si3218x_FSK_Presets[preset].fskfreq1);
    return RC_NONE;
}
#endif

/*
** Function: Si3218x_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
**
** Input Parameters:
** pProslic:   pointer to PROSLIC channel obj
** preset:     dtmf preset
**
** Returns:
** 0
*/
#ifndef DISABLE_DTMF_SETUP
int Si3218x_DTMFDecodeSetup (proslicChanType *pProslic, int preset){
     
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
   
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B0_1,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b0_1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B1_1,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b1_1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B2_1,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b2_1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_A1_1,Si3218x_DTMFDec_Presets[preset].dtmfdtf_a1_1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_A2_1,Si3218x_DTMFDec_Presets[preset].dtmfdtf_a2_1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B0_2,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b0_2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B1_2,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b1_2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B2_2,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b2_2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_A1_2,Si3218x_DTMFDec_Presets[preset].dtmfdtf_a1_2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_A2_2,Si3218x_DTMFDec_Presets[preset].dtmfdtf_a2_2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B0_3,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b0_3);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B1_3,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b1_3);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_B2_3,Si3218x_DTMFDec_Presets[preset].dtmfdtf_b2_3);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_A1_3,Si3218x_DTMFDec_Presets[preset].dtmfdtf_a1_3);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_DTMFDTF_A2_3,Si3218x_DTMFDec_Presets[preset].dtmfdtf_a2_3);
    return RC_NONE;
}
#endif

/*
** Function: PROSLIC_ZsynthSetup
**
** Description: 
** configure impedance synthesis
*/
#ifndef DISABLE_ZSYNTH_SETUP
int Si3218x_ZsynthSetup (proslicChanType *pProslic, int preset){
    uInt8 lf;
    uInt8 cal_en = 0;
    uInt16 timer = 500;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    lf = ReadReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED,0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C0,Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C1,Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C2,Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C3,Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c3);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C0,Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C1,Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C2,Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C3,Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c3);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C2,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C3,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c3);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C4,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c4);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C5,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c5);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C6,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c6);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C7,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c7);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C8,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c8);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECFIR_C9,Si3218x_Impedance_Presets[preset].hybrid.ecfir_c9);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECIIR_B0,Si3218x_Impedance_Presets[preset].hybrid.ecfir_b0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECIIR_B1,Si3218x_Impedance_Presets[preset].hybrid.ecfir_b1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECIIR_A1,Si3218x_Impedance_Presets[preset].hybrid.ecfir_a1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ECIIR_A2,Si3218x_Impedance_Presets[preset].hybrid.ecfir_a2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ZSYNTH_A1,Si3218x_Impedance_Presets[preset].zsynth.zsynth_a1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ZSYNTH_A2,Si3218x_Impedance_Presets[preset].zsynth.zsynth_a2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ZSYNTH_B1,Si3218x_Impedance_Presets[preset].zsynth.zsynth_b1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ZSYNTH_B0,Si3218x_Impedance_Presets[preset].zsynth.zsynth_b0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_ZSYNTH_B2,Si3218x_Impedance_Presets[preset].zsynth.zsynth_b2);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_RA,Si3218x_Impedance_Presets[preset].zsynth.ra);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACGAIN,Si3218x_Impedance_Presets[preset].txgain);
	WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACGAIN_SAVE,Si3218x_Impedance_Presets[preset].rxgain);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACGAIN,Si3218x_Impedance_Presets[preset].rxgain);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_B0_1,Si3218x_Impedance_Presets[preset].rxachpf_b0_1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_B1_1,Si3218x_Impedance_Presets[preset].rxachpf_b1_1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_A1_1,Si3218x_Impedance_Presets[preset].rxachpf_a1_1);

    /*
    ** Scale based on desired gain plan
    */
    Si3218x_dbgSetTXGain(pProslic,Si3218x_Impedance_Presets[preset].txgain_db,preset,TXACGAIN_SEL);
    Si3218x_dbgSetRXGain(pProslic,Si3218x_Impedance_Presets[preset].rxgain_db,preset,RXACGAIN_SEL);
    Si3218x_TXAudioGainSetup(pProslic,TXACGAIN_SEL);
    Si3218x_RXAudioGainSetup(pProslic,RXACGAIN_SEL);

    /* 
    ** Perform Zcal in case OHT used (eg. no offhook event to trigger auto Zcal) 
    */
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_CALR0,0x00);   
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_CALR1,0x40);   
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_CALR2,0x00); 
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_CALR3,0x80);  /* start cal */

    /* Wait for zcal to finish */
    do {
        cal_en = ReadReg(pProHW,pProslic->channel,SI3218X_REG_CALR3);
        Delay(pProTimer,1);
        timer--;
    }while((cal_en&0x80)&&(timer>0));  
     
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED,lf);

    if(timer > 0) return RC_NONE;
    else          return RC_CAL_TIMEOUT;
}
#endif

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si3218x_TXAudioGainSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACGAIN,Si3218x_audioGain_Presets[preset].acgain);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C0,Si3218x_audioGain_Presets[preset].aceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C1,Si3218x_audioGain_Presets[preset].aceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C2,Si3218x_audioGain_Presets[preset].aceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C3,Si3218x_audioGain_Presets[preset].aceq_c3);

    return RC_NONE;
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si3218x_RXAudioGainSetup (proslicChanType *pProslic, int preset){

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
	WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACGAIN_SAVE,Si3218x_audioGain_Presets[preset].acgain);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACGAIN,Si3218x_audioGain_Presets[preset].acgain);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C0,Si3218x_audioGain_Presets[preset].aceq_c0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C1,Si3218x_audioGain_Presets[preset].aceq_c1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C2,Si3218x_audioGain_Presets[preset].aceq_c2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C3,Si3218x_audioGain_Presets[preset].aceq_c3);

    return RC_NONE;
}


/*
** Function: PROSLIC_AudioGainScale
**
** Description: 
** Multiply path gain by passed value for PGA and EQ scale (no reference to dB, 
** multiply by a scale factor)
*/
int Si3218x_AudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale,int rx_tx_sel){
 
	if(rx_tx_sel == TXACGAIN_SEL)
	{
		Si3218x_audioGain_Presets[TXACGAIN_SEL].acgain = (Si3218x_Impedance_Presets[preset].txgain/1000)*pga_scale;
		if (Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c0 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c0 |= 0xf0000000L;
		if (Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c1 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c1 |= 0xf0000000L;
		if (Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c2 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c2 |= 0xf0000000L;
		if (Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c3 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c3 |= 0xf0000000L;
		Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c0 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c0/1000)*eq_scale;
		Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c1 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c1/1000)*eq_scale;
		Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c2 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c2/1000)*eq_scale;
		Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c3 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.txaceq_c3/1000)*eq_scale;

		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACGAIN,Si3218x_audioGain_Presets[TXACGAIN_SEL].acgain);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C0,Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c0);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C1,Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c1);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C2,Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c2);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACEQ_C3,Si3218x_audioGain_Presets[TXACGAIN_SEL].aceq_c3);
	}
	else
	{
		Si3218x_audioGain_Presets[RXACGAIN_SEL].acgain = (Si3218x_Impedance_Presets[preset].rxgain/1000)*pga_scale;
		if (Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c0 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
		if (Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c1 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
		if (Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c2 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
		if (Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c3 & 0x10000000L)
			Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c3 |= 0xf0000000L;
		Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c0 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c0/1000)*eq_scale;
		Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c1 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c1/1000)*eq_scale;
		Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c2 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c2/1000)*eq_scale;
		Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c3 = ((int32)Si3218x_Impedance_Presets[preset].audioEQ.rxaceq_c3/1000)*eq_scale;
	
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACGAIN_SAVE,Si3218x_audioGain_Presets[RXACGAIN_SEL].acgain);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACGAIN,Si3218x_audioGain_Presets[RXACGAIN_SEL].acgain);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C0,Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c0);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C1,Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c1);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C2,Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c2);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACEQ_C3,Si3218x_audioGain_Presets[RXACGAIN_SEL].aceq_c3);
	}
    return 0;
}
int Si3218x_TXAudioGainScale (proslicChanType *pProslic, int preset,uInt32 pga_scale, uInt32 eq_scale)
{
	return Si3218x_AudioGainScale(pProslic,preset,pga_scale,eq_scale,TXACGAIN_SEL);
}
int Si3218x_RXAudioGainScale (proslicChanType *pProslic, int preset,uInt32 pga_scale, uInt32 eq_scale)
{
	return Si3218x_AudioGainScale(pProslic,preset,pga_scale,eq_scale,RXACGAIN_SEL);
}


/*
** Function: PROSLIC_DCFeedSetup
**
** Description: 
** configure dc feed
*/
#ifndef DISABLE_DCFEED_SETUP
int Si3218x_DCFeedSetupCfg (proslicChanType *pProslic, Si3218x_DCfeed_Cfg *cfg, int preset){
    uInt8 lf;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    lf = ReadReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED,0);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_VLIM,cfg[preset].slope_vlim);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_RFEED,cfg[preset].slope_rfeed);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_ILIM,cfg[preset].slope_ilim);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_DELTA1,cfg[preset].delta1);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_DELTA2,cfg[preset].delta2);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_V_VLIM,cfg[preset].v_vlim);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_V_RFEED,cfg[preset].v_rfeed);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_V_ILIM,cfg[preset].v_ilim);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_CONST_RFEED,cfg[preset].const_rfeed);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_CONST_ILIM,cfg[preset].const_ilim);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_I_VLIM,cfg[preset].i_vlim);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LCRONHK,cfg[preset].lcronhk);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LCROFFHK,cfg[preset].lcroffhk);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LCRDBI,cfg[preset].lcrdbi);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LONGHITH,cfg[preset].longhith);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LONGLOTH,cfg[preset].longloth);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LONGDBI,cfg[preset].longdbi);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LCRMASK,cfg[preset].lcrmask);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LCRMASK_POLREV,cfg[preset].lcrmask_polrev);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LCRMASK_STATE,cfg[preset].lcrmask_state);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_LCRMASK_LINECAP,cfg[preset].lcrmask_linecap);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VCM_OH,cfg[preset].vcm_oh);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VOV_BAT,cfg[preset].vov_bat);
    WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VOV_GND,cfg[preset].vov_gnd);
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_LINEFEED,lf);

    return RC_NONE;
}

#endif

/*
** Function: PROSLIC_PulseMeterSetup
**
** Description: 
** configure pulse metering
*/
#ifndef DISABLE_PULSE_SETUP
int Si3218x_PulseMeterSetup (proslicChanType *pProslic, int preset){
    uInt8 reg;
    
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    else
    {
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PM_AMP_THRESH,Si3218x_PulseMeter_Presets[preset].pm_amp_thresh);
        reg = (Si3218x_PulseMeter_Presets[preset].pmFreq<<1)|(Si3218x_PulseMeter_Presets[preset].pmAuto<<3);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PM_ACTIVE,Si3218x_PulseMeter_Presets[preset].pmActive);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_PM_INACTIVE,Si3218x_PulseMeter_Presets[preset].pmInactive);
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_PMCON,reg);
        return RC_NONE;
    }
    
}
#endif

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
*/
#ifndef DISABLE_PCM_SETUP
int Si3218x_PCMSetup(proslicChanType *pProslic, int preset){
    uInt8 regTemp;
    uInt8 pmEn;
        
    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    pmEn = ReadReg(pProHW,pProslic->channel,SI3218X_REG_PMCON) & 0x01; /* PM/wideband lockout */
    if (Si3218x_PCM_Presets[preset].widebandEn && pmEn)
    {
        LOGPRINT ("%s Wideband Mode is not supported while Pulse Metering is enabled.\n", LOGPRINT_PREFIX);
    }
    else if (Si3218x_PCM_Presets[preset].widebandEn && !pmEn)
	{
		/* TXIIR settings */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B0_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B1_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A2_3,0x19D5F700L);
		/* RXIIR settings */
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B0_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B1_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A2_3,0x19D5F700L);
		/* 
		** RXHPF  
		** Note:  Calling ProSLIC_ZsynthSetup() will overwrite some
		**        of these values.  ProSLIC_PCMSetup() should always
		**        be called after loading coefficients when using
		**        wideband mode
		*/
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_B0_1,0x7CFF900L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_B1_1,0x18300700L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_A1_1,0x79FF201L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_B0_2,0x7CEDA1DL);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_B1_2,0x106320D4L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_B2_2,0x7CEDA1DL);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_A1_2,0xF9A910FL);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_A2_2,0x185FFDA8L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACHPF_GAIN,0x08000000L);
		/* TXHPF */
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_B0_1,0x0C7FF4CEL);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_B1_1,0x13800B32L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_A1_1,0x079FF201L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_B0_2,0x030FDD10L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_B1_2,0x19E0996CL);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_B2_2,0x030FDD10L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_A1_2,0x0F9A910FL);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_A2_2,0x185FFDA8L);
		WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACHPF_GAIN,0x0CD30000L);

        regTemp = ReadReg(pProHW,pProslic->channel,SI3218X_REG_DIGCON);
#ifndef DISABLE_HPF_WIDEBAND
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_DIGCON,regTemp&~(0xC)); /* Enable HPF */
#else
        WriteReg(pProHW,pProslic->channel,DIGCON,regTemp|(0xC)); /* Disable HPF */
#endif
        regTemp = ReadReg(pProHW,pProslic->channel,SI3218X_REG_ENHANCE);
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_ENHANCE,regTemp|1);
    } else {
        regTemp = ReadReg(pProHW,pProslic->channel,SI3218X_REG_DIGCON);
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_DIGCON,regTemp&~(0xC));
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B0_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B1_1,0x3538E80L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_TXACIIR_A2_3,0x19D5F700L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B0_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B1_1,0x6A71D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A1_1,0x1AA9100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B0_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B1_2,0x2505400L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B2_2,0x216D100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A1_2,0x2CB8100L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A2_2,0x1D7FA500L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B0_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B1_3,0x1276D00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_B2_3,0x2CD9B00L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A1_3,0x2335300L);
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_RXACIIR_A2_3,0x19D5F700L);
        regTemp = ReadReg(pProHW,pProslic->channel,SI3218X_REG_ENHANCE);
        WriteReg(pProHW,pProslic->channel,SI3218X_REG_ENHANCE,regTemp&~(1));
    }
    regTemp = Si3218x_PCM_Presets[preset].pcmFormat;
    regTemp |= Si3218x_PCM_Presets[preset].pcm_tri << 5;
    regTemp |= Si3218x_PCM_Presets[preset].alaw_inv << 2;
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_PCMMODE,regTemp);
    regTemp = ReadReg(pProHW,pProslic->channel,SI3218X_REG_PCMTXHI);
    regTemp &= 3;
    regTemp |= Si3218x_PCM_Presets[preset].tx_edge<<4;
    WriteReg(pProHW,pProslic->channel,SI3218X_REG_PCMTXHI,regTemp);

    return RC_NONE;
}
#endif

/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/

/*
** Function: PROSLIC_SetLinefeedStatus
**
** Description: 
** Sets linefeed state
*/
int Si3218x_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed){
    uInt8 regTemp;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    WriteReg (pProHW, pProslic->channel, SI3218X_REG_LINEFEED,newLinefeed);
    if ((newLinefeed&0xf) == LF_RINGING) {
        /*disable vbat interrupt during ringing*/
        regTemp = ReadReg(pProHW,pProslic->channel,SI3218X_REG_IRQEN1);
        WriteReg (pProHW,pProslic->channel,SI3218X_REG_IRQEN1,regTemp&(~0x80));
    }
    else{
      WriteReg (pProHW,pProslic->channel,SI3218X_REG_IRQEN1,Si3218x_General_Configuration.irqen1);
    }
    return RC_NONE;
}

/*
** Function: Si324x_SetLinefeedStatusBroadcast
**
** Description: 
** Sets linefeed state
*/
int Si3218x_SetLinefeedStatusBroadcast (proslicChanType *pProslic, uInt8 newLinefeed){

    WriteReg (pProHW, BROADCAST, SI3218X_REG_LINEFEED,newLinefeed);
    return 0;
}

/*
** Function: PROSLIC_MWISetup
**
** Description: 
** Modify default vpk or lcrmask_mwi.  Passing 0 will result in the parameter
** not being modified.
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3218x_MWISetup (proslicChanType *pProslic,uInt16 vpk_mag,uInt16 lcrmask_mwi)
{
uInt32 ram_val;

	/* Verify MWI not enabled - cannot make changes while enabled */
	if(ReadReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT) & SIL_MWI_USTAT_SET)
	{
		return RC_MWI_IN_USE;
	}

	/* Voltage mod */
	if(vpk_mag > 0)  /* Skip if 0 passed */
	{
		/* Clamp supplied value to allowable range */
		if(vpk_mag > SIL_MWI_VPK_MAX)		vpk_mag = SIL_MWI_VPK_MAX;
		if(vpk_mag < SIL_MWI_VPK_MIN)		vpk_mag = SIL_MWI_VPK_MIN;
		ram_val = vpk_mag * SCALE_V_MADC * 1000L;
		WriteRAM(pProHW,pProslic->channel,SI3218X_PRAM_VBATH_NEON,ram_val);
	}

	/* LCRMASK mod */
	if(lcrmask_mwi > 0)  /* Skip if 0 passed */
	{
		/* Clamp supplied value to allowable range */
		if(lcrmask_mwi > SIL_MWI_LCRMASK_MAX)		lcrmask_mwi = SIL_MWI_LCRMASK_MAX;
		if(lcrmask_mwi < SIL_MWI_LCRMASK_MIN)		lcrmask_mwi = SIL_MWI_LCRMASK_MIN;
		ram_val = lcrmask_mwi * SIL_MWI_LCRMASK_SCALE;
		WriteRAM(pProHW,pProslic->channel,SI3218X_PRAM_LCRMASK_MWI,ram_val);
	}
	
	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_MWIEnable
**
** Description: 
** Enables MWI feature
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3218x_MWIEnable (proslicChanType *pProslic)
{
uInt8 val;
	
	/* 
	** Check for conditions that would prevent enabling MWI
	*/
	ProSLIC_ReadHookStatus(pProslic,&val);
	if(val != PROSLIC_ONHOOK)	
	{
		return RC_MWI_ENABLE_FAIL;
	}
	else
	{	/* Clear DIAG1 and set USERSTAT enable bit */
		ProSLIC_SetPowersaveMode(pProslic,PWRSAVE_DISABLE);
		WriteReg(pProHW,pProslic->channel,SI3218X_REG_DIAG1,SIL_MWI_TOGGLE_LOW);
		val = ReadReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT);
		WriteReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT,val|SIL_MWI_USTAT_SET);
	}
	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_MWIDisable
**
** Description: 
** Disables MWI feature
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3218x_MWIDisable (proslicChanType *pProslic)
{
uInt8 val;

	/* Clear DIAG1 and USERSTAT enable bit */
	WriteReg(pProHW,pProslic->channel,SI3218X_REG_DIAG1,SIL_MWI_TOGGLE_LOW);
	val = ReadReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT);
	WriteReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT,val&SIL_MWI_USTAT_CLEAR);

	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_SetMWIState
**
** Description: 
** Set MWI State.  Must be enabled via ProSLIC_MWIEnable prior
** to calling this function
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3218x_SetMWIState(proslicChanType *pProslic,uInt8 flash_on)
{

	/* Only continue if MWI is enabled */
	if(ReadReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT) & SIL_MWI_USTAT_SET)
	{
		if(flash_on)
		{
			WriteReg(pProHW,pProslic->channel,SI3218X_REG_DIAG1,SIL_MWI_TOGGLE_HIGH);
		}
		else
		{
			WriteReg(pProHW,pProslic->channel,SI3218X_REG_DIAG1,SIL_MWI_TOGGLE_LOW);
		}
	}
	else
	{
		return RC_MWI_NOT_ENABLED;
	}

	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_GetMWIState
**
** Description: 
** Read MWI State.  Must be enabled via ProSLIC_MWIEnable prior
** to calling this function
**
** Note:  This feature is implemented in patch
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3218x_GetMWIState(proslicChanType *pProslic)
{

	/* Only continue if MWI is enabled */
	if(ReadReg(pProHW,pProslic->channel,SI3218X_REG_USERSTAT) & SIL_MWI_USTAT_SET)
	{
		if(ReadReg(pProHW,pProslic->channel,SI3218X_REG_DIAG1) & SIL_MWI_TOGGLE_HIGH)
			return SIL_MWI_FLASH_ON;
		else
			return SIL_MWI_FLASH_OFF;
	}
	else
	{
		return RC_MWI_NOT_ENABLED;
	}

	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_MWI
**
** Description: 
** implements message waiting indicator
**
** Message waiting (neon flashing) requires modifications to vbath_expect 
** and slope_vlim. The old values are restored to turn off the lamp. We 
** assume all channels set up the same. During off-hook event lamp must 
** be disabled manually. 
**
** Deprecated.   Use Si3218x_SetMWIState()
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int Si3218x_MWI (proslicChanType *pProslic,uInt8 lampOn){
static int32 vbath_save = 0;
static int32 slope_vlim_save = 0;
uInt8 hkStat; 
int32 slope_vlim_tmp;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    slope_vlim_tmp = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_VLIM);
    Si3218x_ReadHookStatus(pProslic,&hkStat);

    if (lampOn && (hkStat == PROSLIC_OFFHOOK) ) {/*cant neon flash during offhook*/
#ifdef ENABLE_DEBUG
        if (pProslic->debugMode)        
            LOGPRINT ("Si3218x MWI cannot operate offhook\n");
#endif
        return RC_LINE_IN_USE;
    }

    if (lampOn) {
        if (slope_vlim_tmp != 0x8000000L) { /*check we're not already on*/
            vbath_save = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATH_EXPECT);
            slope_vlim_save = slope_vlim_tmp;
        }
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATH_EXPECT,0x7AE147AL);/*120V*/
        WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_VLIM,0x8000000L);
    } else {
        if (vbath_save != 0) { /*check we saved some valid value first*/
            WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_VBATH_EXPECT,vbath_save);
            WriteRAM(pProHW,pProslic->channel,SI3218X_RAM_SLOPE_VLIM,slope_vlim_save);
        }
    }

    return RC_NONE;
}
#endif

/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** provisionally function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3218x_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset){
/* Note:  * needs more descriptive return codes in the event of an out of range argument */
    uInt16 vslope = 160;
    uInt16 rslope = 720;
    uInt32 vscale1 = 1386; 
    uInt32 vscale2 = 1422;   /* 1386x1422 = 1970892 broken down to minimize trunc err */
    uInt32 iscale1 = 913;
    uInt32 iscale2 = 334;    /* 913x334 = 304942 */
    uInt32 i_rfeed_val, v_rfeed_val, const_rfeed_val, i_vlim_val, const_ilim_val, v_ilim_val;
    int32 signedVal;
    /* Set Linefeed to open state before modifying DC Feed */

    /* Assumptions must be made to minimize computations.  This limits the
    ** range of available settings, but should be more than adequate for
    ** short loop applications.
    **
    ** Assumtions:
    ** 
    ** SLOPE_VLIM      =>  160ohms
    ** SLOPE_RFEED     =>  720ohms
    ** I_RFEED         =>  3*I_ILIM/4
    ** 
    ** With these assumptions, the DC Feed parameters now become 
    **
    ** Inputs:      V_VLIM, I_ILIM
    ** Constants:   SLOPE_VLIM, SLOPE_ILIM, SLOPE_RFEED, SLOPE_DELTA1, SLOPE_DELTA2
    ** Outputs:     V_RFEED, V_ILIM, I_VLIM, CONST_RFEED, CONST_ILIM
    **
    */

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    /* Validate arguments */
    if((i_ilim_val < 15)||(i_ilim_val > 45)) return 1;   /* need error code */
    if((v_vlim_val < 30)||(v_vlim_val > 52)) return 1;   /* need error code */

    /* Calculate voltages in mV and currents in uA */
    v_vlim_val *= 1000;
    i_ilim_val *= 1000;

    /* I_RFEED */
    i_rfeed_val = (3*i_ilim_val)/4;

    /* V_RFEED */
    v_rfeed_val = v_vlim_val - (i_rfeed_val*vslope)/1000;

    /* V_ILIM */ 
    v_ilim_val = v_rfeed_val - (rslope*(i_ilim_val - i_rfeed_val))/1000;

    /* I_VLIM */
    i_vlim_val = (v_vlim_val*1000)/4903;

    /* CONST_RFEED */
    signedVal = v_rfeed_val * (i_ilim_val - i_rfeed_val);
    signedVal /= (v_rfeed_val - v_ilim_val);
    signedVal = i_rfeed_val + signedVal;

    /* signedVal in uA here */
    signedVal *= iscale1;
    signedVal /= 100;
    signedVal *= iscale2;
    signedVal /= 10;

    if(signedVal < 0)
    {
        const_rfeed_val = (signedVal)+ (1L<<29);
    }
    else
    {
        const_rfeed_val = signedVal & 0x1FFFFFFF;
    }

    /* CONST_ILIM */
    const_ilim_val = i_ilim_val;

    /* compute RAM values */
    v_vlim_val *= vscale1;
    v_vlim_val /= 100;
    v_vlim_val *= vscale2;
    v_vlim_val /= 10;

    v_rfeed_val *= vscale1;
    v_rfeed_val /= 100;
    v_rfeed_val *= vscale2;
    v_rfeed_val /= 10;

    v_ilim_val *= vscale1;
    v_ilim_val /= 100;
    v_ilim_val *= vscale2;
    v_ilim_val /= 10;

    const_ilim_val *= iscale1;
    const_ilim_val /= 100;
    const_ilim_val *= iscale2;
    const_ilim_val /= 10;

    i_vlim_val *= iscale1;
    i_vlim_val /= 100;
    i_vlim_val *= iscale2;
    i_vlim_val /= 10;

    Si3218x_DCfeed_Presets[preset].slope_vlim = 0x18842BD7L;
    Si3218x_DCfeed_Presets[preset].slope_rfeed = 0x1E8886DEL;
    Si3218x_DCfeed_Presets[preset].slope_ilim = 0x40A0E0L;
    Si3218x_DCfeed_Presets[preset].delta1 = 0x1EABA1BFL;
    Si3218x_DCfeed_Presets[preset].delta2 = 0x1EF744EAL;
    Si3218x_DCfeed_Presets[preset].v_vlim = v_vlim_val;
    Si3218x_DCfeed_Presets[preset].v_rfeed = v_rfeed_val;
    Si3218x_DCfeed_Presets[preset].v_ilim = v_ilim_val;
    Si3218x_DCfeed_Presets[preset].const_rfeed = const_rfeed_val;
    Si3218x_DCfeed_Presets[preset].const_ilim = const_ilim_val;
    Si3218x_DCfeed_Presets[preset].i_vlim = i_vlim_val;
        
    return RC_NONE;
}

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** provisionally function for setting up
** dcfeed given desired open circuit voltage.
** Entry I_ILIM value will be used.
*/
int Si3218x_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset)
{
    uInt32 i_ilim_val;
    uInt32 iscale1 = 913;
    uInt32 iscale2 = 334;    /* 913x334 = 304942 */


    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    /* Read present CONST_ILIM value */
    i_ilim_val = Si3218x_DCfeed_Presets[preset].const_ilim;


    i_ilim_val /= iscale2;
    i_ilim_val /= iscale1;

    return Si3218x_dbgSetDCFeed(pProslic,v_vlim_val,i_ilim_val,preset);
}

/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** provisionally function for setting up
** dcfeed given desired loop current.
** Entry V_VLIM value will be used.
*/
int Si3218x_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset)
{
    uInt32 v_vlim_val;
    uInt32 vscale1 = 1386; 
    uInt32 vscale2 = 1422;   /* 1386x1422 = 1970892 broken down to minimize trunc err */

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    /* Read present V_VLIM value */
    v_vlim_val = Si3218x_DCfeed_Presets[preset].v_vlim;

    v_vlim_val /= vscale2;
    v_vlim_val /= vscale1;

    return Si3218x_dbgSetDCFeed(pProslic,v_vlim_val,i_ilim_val, preset);
}

typedef struct
{
    uInt8   freq;
    ramData ringfr;      /* trise scale for trap */
    uInt32  ampScale;
} ProSLIC_SineRingFreqLookup;

typedef struct
{
    uInt8    freq;
    ramData  rtacth;
    ramData rtper;
    ramData rtdb;
} ProSLIC_SineRingtripLookup;

typedef struct
{
    uInt8   freq;
    uInt16  cfVal[6];
} ProSLIC_TrapRingFreqLookup;

typedef struct
{
    uInt8   freq;
    ramData rtper;
    ramData rtdb;
    uInt32  rtacth[6];
} ProSLIC_TrapRingtripLookup;

/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provision function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/
int Si3218x_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset){
    int errVal,i=0;
    uInt32 vScale = 1608872L;   /* (2^28/170.25)*((100+4903)/4903) */
    ramData dcdcVminTmp;

    const ProSLIC_SineRingFreqLookup sineRingFreqTable[] =
/*  Freq RINGFR, vScale */
        {{15, 0x7F6E930L, 18968L},
         {16, 0x7F5A8E0L, 20234L},
         {20, 0x7EFD9D5L, 25301L},
         {22, 0x7EC770AL, 27843L},
         {23, 0x7EAA6E2L, 29113L},
         {25, 0x7E6C925L, 31649L},
         {30, 0x7DBB96BL, 38014L},
         {34, 0x7D34155L, 42270L}, /* Actually 33.33Hz */
         {35, 0x7CEAD72L, 44397L},
         {40, 0x7BFA887L, 50802L},
         {45, 0x7AEAE74L, 57233L},
         {50, 0x79BC384L, 63693L},
         {0,0,0}}; /* terminator */

    const ProSLIC_SineRingtripLookup sineRingtripTable[] =
/*  Freq rtacth */
        { {15, 11440000L, 0x6A000L, 0x4000L },
          {16, 10810000L, 0x64000L, 0x4000L },
          {20, 8690000L,  0x50000L, 0x8000L }, 
          {22, 7835000L,  0x48000L, 0x8000L },
          {23, 7622000L,  0x46000L, 0x8000L }, 
          {25, 6980000L,  0x40000L, 0xA000L }, 
          {30, 5900000L,  0x36000L, 0xA000L }, 
          {34, 10490000L, 0x60000L, 0x6000L }, /* Actually 33.33 */
          {35, 10060000L, 0x5C000L, 0x6000L }, 
          {40, 8750000L,  0x50000L, 0x8000L }, 
          {45, 7880000L,  0x48000L, 0x8000L }, 
          {50, 7010000L,  0x40000L, 0xA000L }, 
          {0,0L}}; /* terminator */

    const ProSLIC_TrapRingFreqLookup trapRingFreqTable[] =
/*  Freq multCF11 multCF12 multCF13 multCF14 multCF15 multCF16*/
    {
        {15, {69,122, 163, 196, 222,244}},
        {16, {65,115, 153, 184, 208,229}},
        {20, {52,92, 122, 147, 167,183}},
        {22, {47,83, 111, 134, 152,166}},
        {23, {45,80, 107, 128, 145,159}},
        {25, {42,73, 98, 118, 133,146}},
        {30, {35,61, 82, 98, 111,122}},
        {34, {31,55, 73, 88, 100,110}},
        {35, {30,52, 70, 84, 95,104}},
        {40, {26,46, 61, 73, 83,91}},
        {45, {23,41, 54, 65, 74,81}},
        {50, {21,37, 49, 59, 67,73}},
        {0,{0L,0L,0L,0L}} /* terminator */
    }; 


    const ProSLIC_TrapRingtripLookup trapRingtripTable[] =
/*  Freq rtper rtdb rtacthCR11 rtacthCR12 rtacthCR13 rtacthCR14 rtacthCR15 rtacthCR16*/
    {
        {15, 0x6A000L,  0x4000L, {16214894L, 14369375L, 12933127L, 11793508L, 10874121L, 10121671L}},
        {16, 0x64000L,  0x4000L, {15201463L, 13471289L, 12124806L, 11056414L, 10194489L, 9489067L}},
        {20, 0x50000L,  0x6000L, {12161171L, 10777031L, 9699845L, 8845131L, 8155591L, 7591253L}},
        {22, 0x48000L,  0x6000L, {11055610L, 9797301L, 8818041L, 8041028L, 7414174L, 6901139L}},
        {23, 0x46000L,  0x6000L, {10574931L, 9371331L, 8434648L, 7691418L, 7091818L, 6601090L}},
        {25, 0x40000L,  0x8000L, {9728937L, 8621625L, 7759876L, 7076105L, 6524473L, 6073003L}},
        {30, 0x36000L,  0x8000L, {8107447L, 7184687L, 6466563L, 5896754L, 5437061L, 5060836L}},
        {34, 0x60000L,  0x6000L, {7297432L, 6466865L, 5820489L, 5307609L, 4893844L, 4555208L}},
        {35, 0x5C000L,  0x6000L, {6949240L, 6158303L, 5542769L, 5054361L, 4660338L, 4337859L}},
        {40, 0x50000L,  0x6000L, {6080585L, 5388516L, 4849923L, 4422565L, 4077796L, 3795627L}},
        {45, 0x48000L,  0x6000L, {5404965L, 4789792L, 4311042L, 3931169L, 3624707L, 3373890L}},
        {50, 0x40000L,  0x8000L, {4864468L, 4310812L, 3879938L, 3538052L, 3262236L, 3036501L}},
        {0,0x0L, 0x0L, {0L,0L,0L,0L}} /* terminator */
    }; 

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    errVal = RC_NONE;

    switch(ringCfg->ringtype)
    {
    case ProSLIC_RING_SINE:
        i=0;
        do
        {
            if(sineRingFreqTable[i].freq >= ringCfg->freq) 
            {
                break;
            }
            i++;
        } while (sineRingFreqTable[i].freq);

        /* Set to maximum value if exceeding maximum value from table */
        if(sineRingFreqTable[i].freq == 0)
        {
            i--;
            errVal = RC_RING_V_LIMITED;
        }

        /* Update RINGFR RINGAMP, RINGOFFSET, and RINGCON */
        Si3218x_Ring_Presets[preset].freq = sineRingFreqTable[i].ringfr;
        Si3218x_Ring_Presets[preset].amp = ringCfg->amp * sineRingFreqTable[i].ampScale;
        Si3218x_Ring_Presets[preset].offset = ringCfg->offset * vScale;
        Si3218x_Ring_Presets[preset].phas = 0L;

        /* Don't alter anything in RINGCON other than clearing the TRAP bit */
        Si3218x_Ring_Presets[preset].ringcon &= 0xFE;

        Si3218x_Ring_Presets[preset].rtper = sineRingtripTable[i].rtper;
        Si3218x_Ring_Presets[preset].rtacdb = sineRingtripTable[i].rtdb;
        Si3218x_Ring_Presets[preset].rtdcdb = sineRingtripTable[i].rtdb;
        Si3218x_Ring_Presets[preset].rtdcth = 0xFFFFFFFL;
        Si3218x_Ring_Presets[preset].rtacth = sineRingtripTable[i].rtacth;
        break;

    case ProSLIC_RING_TRAP_CF11:  
    case ProSLIC_RING_TRAP_CF12:     
    case ProSLIC_RING_TRAP_CF13: 
    case ProSLIC_RING_TRAP_CF14: 
    case ProSLIC_RING_TRAP_CF15:  
    case ProSLIC_RING_TRAP_CF16:  
        i=0;
        do
        {
            if(trapRingFreqTable[i].freq >= ringCfg->freq) 
            {
                break;
            }
            i++;
        } while (trapRingFreqTable[i].freq);

        /* Set to maximum value if exceeding maximum value from table */
        if(trapRingFreqTable[i].freq == 0)
        {
            i--;
            errVal = RC_RING_V_LIMITED;
        }

        /* Update RINGFR RINGAMP, RINGOFFSET, and RINGCON */
        Si3218x_Ring_Presets[preset].amp = ringCfg->amp * vScale;
        Si3218x_Ring_Presets[preset].freq = Si3218x_Ring_Presets[preset].amp/trapRingFreqTable[i].cfVal[ringCfg->ringtype];
        Si3218x_Ring_Presets[preset].offset = ringCfg->offset * vScale;
        Si3218x_Ring_Presets[preset].phas = 262144000L/trapRingFreqTable[i].freq;

        /* Don't alter anything in RINGCON other than setting the TRAP bit */
        Si3218x_Ring_Presets[preset].ringcon |= 0x01; 

        /* RTPER and debouce timers  */
        Si3218x_Ring_Presets[preset].rtper = trapRingtripTable[i].rtper;
        Si3218x_Ring_Presets[preset].rtacdb = trapRingtripTable[i].rtdb;
        Si3218x_Ring_Presets[preset].rtdcdb = trapRingtripTable[i].rtdb;  


        Si3218x_Ring_Presets[preset].rtdcth = 0xFFFFFFFL;
        Si3218x_Ring_Presets[preset].rtacth = trapRingtripTable[i].rtacth[ringCfg->ringtype];


        break;
    }

    /* 
    ** DCDC tracking sluggish under light load at higher ring freq.
    ** Reduce tracking depth above 40Hz.  This should have no effect
    ** if using the Buck-Boost architecture.
    */
    if((sineRingFreqTable[i].freq >= 40)||(Si3218x_General_Configuration.bom_option == BO_DCDC_BUCK_BOOST))
    {
        dcdcVminTmp = ringCfg->amp + ringCfg->offset;
        dcdcVminTmp *= 1000;
        dcdcVminTmp *= SCALE_V_MADC;
        Si3218x_Ring_Presets[preset].dcdc_vref_min_rng = dcdcVminTmp;
    }
    else
    {
        Si3218x_Ring_Presets[preset].dcdc_vref_min_rng = 0x1800000L;
    }

    return errVal;
}


typedef struct
{
    int32   gain;
    uInt32 scale;
} ProSLIC_GainScaleLookup;

#define EXTENDED_GAIN_MAX 9
#define GAIN_MAX 6
#define GAIN_MIN -30
#ifndef ENABLE_HIRES_GAIN
static int Si3218x_dbgSetGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int tx_rx_sel){
    int errVal = 0;
    int32 i;
    int32 gain_pga, gain_eq;
    const ProSLIC_GainScaleLookup gainScaleTable[] =  /*  gain, scale=10^(gain/20) */
    {   
		{-30, 32},
		{-29, 35},
		{-28, 40},
		{-27, 45},
		{-26, 50},
		{-25, 56},
		{-24, 63},
		{-23, 71},
		{-22, 79},
		{-21, 89},
		{-20, 100},
		{-19, 112},
		{-18, 126},
		{-17, 141},
        {-16, 158}, 
        {-15, 178}, 
        {-14, 200}, 
        {-13, 224}, 
        {-12, 251}, 
        {-11, 282}, 
        {-10, 316}, 
        {-9, 355}, 
        {-8, 398}, 
        {-7, 447}, 
        {-6, 501},
        {-5, 562},
        {-4, 631},
        {-3, 708},
        {-2, 794},
        {-1, 891},
        {0, 1000},
        {1, 1122},
        {2, 1259},
        {3, 1413},
        {4, 1585},
        {5, 1778},
        {6, 1995},
        {0xff,0}  /* terminator */
    }; 
 
/* 
** 5.4.0 - Removed relative gain scaling. to support automatic adjustment based on
**         gain plan provided in txgain_db and rxgain_db.  It is presumed that all
**         coefficients were generated for 0dB/0dB gain and the txgain_db and rxgain_db
**         parameters will be used to scale the gain using the existing gain provisioning
**         infrastructure when the zsynth preset is loaded.  This function will ignore 
**         the txgain_db and rxgain_db parameters and scale absolute gain presuming a
**         0dB/0dB coefficient set.
*/

/*
** 6.0.0 - Modifying where gain/attenuation is placed to minimize clipping.
**
**         RX Path:   -30dB < gain <  0dB -   All in RXACGAIN 
**                      0dB < gain <  6dB -   All in RXACEQ
**
**         TX Path:   -30dB < gain <  0dB -   All in TXACEQ
**                      0dB < gain <  6dB -   All in TXACGAIN
*/

    /* Test against max gain */
    if (gain > EXTENDED_GAIN_MAX)
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d out of range\n", LOGPRINT_PREFIX, (int)gain);
		}
#endif
		gain = EXTENDED_GAIN_MAX; /* Clamp to maximum */
	}

	/* Test against min gain */
    if (gain < GAIN_MIN)
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d out of range\n", LOGPRINT_PREFIX, (int)gain);
		}
#endif
		gain = GAIN_MIN; /* Clamp to minimum */
	}

	/* Distribute gain */
	if(gain == 0)
	{
		gain_pga = 0;
		gain_eq = 0;
	}
	else if(gain > GAIN_MAX)
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = GAIN_MAX;
			gain_eq = gain - GAIN_MAX;
		}
		else
		{
			gain_pga = gain - GAIN_MAX;
			gain_eq = GAIN_MAX;
		}
	}	
	else if(gain > 0)
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = gain;
			gain_eq  = 0;
		}
		else
		{
			gain_pga = 0;
			gain_eq = gain;
		}
	}
	else
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = 0;
			gain_eq  = gain;
		}
		else
		{
			gain_pga = gain;
			gain_eq = 0;
		}

	}
   

    /* 
	** Lookup PGA Appropriate PGA Gain
	*/
    i=0;
    do
    {
        if(gainScaleTable[i].gain >= gain_pga)   /* was gain_1 */
        {             
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        Si3218x_audioGain_Presets[0].acgain = (Si3218x_Impedance_Presets[impedance_preset].txgain/1000)*gainScaleTable[i].scale;
    }
    else
    {
        Si3218x_audioGain_Presets[1].acgain = (Si3218x_Impedance_Presets[impedance_preset].rxgain/1000)*gainScaleTable[i].scale;
    }


    /* 
	** Lookup EQ Gain
	*/
    i=0;
    do
    {
        if(gainScaleTable[i].gain >= gain_eq) /* was gain_2 */
        {
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        /*sign extend negative numbers*/
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 |= 0xf0000000L;

        Si3218x_audioGain_Presets[0].aceq_c0 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0/1000)*gainScaleTable[i].scale;
        Si3218x_audioGain_Presets[0].aceq_c1 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1/1000)*gainScaleTable[i].scale;
        Si3218x_audioGain_Presets[0].aceq_c2 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2/1000)*gainScaleTable[i].scale;
        Si3218x_audioGain_Presets[0].aceq_c3 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3/1000)*gainScaleTable[i].scale;
    }
    else
    {
        /*sign extend negative numbers*/
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 |= 0xf0000000L;

        Si3218x_audioGain_Presets[1].aceq_c0 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0/1000)*gainScaleTable[i].scale;
        Si3218x_audioGain_Presets[1].aceq_c1 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1/1000)*gainScaleTable[i].scale;
        Si3218x_audioGain_Presets[1].aceq_c2 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2/1000)*gainScaleTable[i].scale;
        Si3218x_audioGain_Presets[1].aceq_c3 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3/1000)*gainScaleTable[i].scale;
    }


    return errVal;
}
#else /* ENABLE_HIRES_GAIN */
/*
** Function: Si3218x_dbgSetGainHiRes()
**
** Description: 
** Provision function for setting up
** TX and RX gain with 0.1dB resolution instead
** of 1.0dB resolution
*/
static int Si3218x_dbgSetGainHiRes (proslicChanType *pProslic, int32 gain, int impedance_preset, int tx_rx_sel){
    int errVal = 0;
    int32 i;
	int32 coarseGainIndex, fineGainIndex;
    int32 gain_pga, gain_eq;
	int32 coarseGain, fineGain;
	int32 tmp;
    const ProSLIC_GainScaleLookup coarseGainScaleTable[] =  /*  gain, scale=10^(gain/20) */
    { 
		{-30, 32},
		{-29, 35},
		{-28, 40},
		{-27, 45},
		{-26, 50},
		{-25, 56},
		{-24, 63},
		{-23, 71},
		{-22, 79},
		{-21, 89},
		{-20, 100},
		{-19, 112},
		{-18, 126},
		{-17, 141},
        {-16, 158}, 
        {-15, 178}, 
        {-14, 200}, 
        {-13, 224}, 
        {-12, 251}, 
        {-11, 282}, 
        {-10, 316}, 
        {-9, 355}, 
        {-8, 398}, 
        {-7, 447}, 
        {-6, 501},
        {-5, 562},
        {-4, 631},
        {-3, 708},
        {-2, 794},
        {-1, 891},
        {0, 1000},
        {1, 1122},
        {2, 1259},
        {3, 1413},
        {4, 1585},
        {5, 1778},
        {6, 1995},
        {0xff,0}  /* terminator */
    }; 

	const ProSLIC_GainScaleLookup fineGainScaleTable[] =  /*  gain, scale=10^(gain/20) */
    { 
        {-9, 902}, 
        {-8, 912}, 
        {-7, 923}, 
        {-6, 933},
        {-5, 944},
        {-4, 955},
        {-3, 966},
        {-2, 977},
        {-1, 989},
        {0, 1000},
        {1, 1012},
        {2, 1023},
        {3, 1035},
        {4, 1047},
        {5, 1059},
        {6, 1072},
        {7, 1084},
        {8, 1096},
        {9, 1109},
		{0xff,0}  /* terminator */
	};

/*
** 6.0.0 - Modifying where gain/attenuation is placed to minimize clipping.
**
**         RX Path:   -30dB < gain <  0dB -   All in RXACGAIN
**                      0dB < gain <  6dB -   All in RXACEQ
**
**         TX Path:   -30dB < gain <  0dB -   All in TXACEQ
**                      0dB < gain <  6dB -   All in TXACGAIN
**
** 6.2.1 - Added option for fine gain adjust.  All fine adjustment done
**         in RXACGAIN and TXACEQ
*/

    /* Test against max gain */
    if (gain > (GAIN_MAX*10L))
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d dB*10 out of range\n", LOGPRINT_PREFIX, gain);
		}
#endif
		gain = (GAIN_MAX*10L); /* Clamp to maximum */
	}

	/* Test against min gain */
    if (gain < (GAIN_MIN*10L))
	{
		errVal = RC_GAIN_OUT_OF_RANGE;
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("%sdbgSetGain: Gain %d dB*10 out of range\n", LOGPRINT_PREFIX, gain);
		}
#endif
		gain = (GAIN_MIN*10); /* Clamp to minimum */
	}

    /* Distribute gain */
	coarseGain = gain/10L;
	fineGain = gain - (coarseGain*10L);

	/* Distribute coarseGain */
	if(coarseGain == 0)
	{
		gain_pga = 0;
		gain_eq = 0;
	}
	else if(coarseGain > 0)
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = coarseGain;
			gain_eq  = 0;
		}
		else
		{
			gain_pga = 0;
			gain_eq = coarseGain;
		}
	}
	else
	{
		if(tx_rx_sel == TXACGAIN_SEL)
		{
			gain_pga = 0;
			gain_eq  = coarseGain;
		}
		else
		{
			gain_pga = coarseGain;
			gain_eq = 0;
		}
	}

    /* 
	** Lookup PGA Appopriate PGA Gain
	*/
    i=0;
    do
    {
        if(coarseGainScaleTable[i].gain >= gain_pga) 
        {       
            break;
        }
        i++;
    } while (coarseGainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(coarseGainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

	coarseGainIndex = i;  /* Store coarse index */

	/* Find fineGain */
	i = 0;
	do
	{
		if(fineGainScaleTable[i].gain >= fineGain)
		{
			break;
		}
		i++;
	} while (fineGainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(fineGainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

	fineGainIndex = i;

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        Si3218x_audioGain_Presets[0].acgain = ((Si3218x_Impedance_Presets[impedance_preset].txgain/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
    }
    else
    {
        Si3218x_audioGain_Presets[1].acgain = ((Si3218x_Impedance_Presets[impedance_preset].rxgain/1000L)*coarseGainScaleTable[coarseGainIndex].scale)/1000L * fineGainScaleTable[fineGainIndex].scale;
    }

    /* 
	** Lookup EQ Gain
	*/
    i=0;
    do
    {
        if(coarseGainScaleTable[i].gain >= gain_eq) 
        {
            break;
        }
        i++;
    } while (coarseGainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(coarseGainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

	coarseGainIndex = i;  /* Store coarse index */

    if(tx_rx_sel == TXACGAIN_SEL)
    {
        /*sign extend negative numbers*/
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 |= 0xf0000000L;

		tmp = (((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3218x_audioGain_Presets[0].aceq_c0 = tmp;

		tmp = (((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3218x_audioGain_Presets[0].aceq_c1 = tmp;

		tmp = (((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3218x_audioGain_Presets[0].aceq_c2 = tmp;

		tmp = (((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3/1000L)*coarseGainScaleTable[coarseGainIndex].scale);
		tmp = tmp / (int32)1000L;
		tmp = tmp * (int32)fineGainScaleTable[fineGainIndex].scale;
		Si3218x_audioGain_Presets[0].aceq_c3 = tmp;
	}
    else
    {
        /*sign extend negative numbers*/
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
        if (Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 & 0x10000000L)
            Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 |= 0xf0000000L;

        Si3218x_audioGain_Presets[1].aceq_c0 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0/1000)*coarseGainScaleTable[i].scale;
        Si3218x_audioGain_Presets[1].aceq_c1 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1/1000)*coarseGainScaleTable[i].scale;
        Si3218x_audioGain_Presets[1].aceq_c2 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2/1000)*coarseGainScaleTable[i].scale;
        Si3218x_audioGain_Presets[1].aceq_c3 = ((int32)Si3218x_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3/1000)*coarseGainScaleTable[i].scale;
    }


    return errVal;
}
#endif /* ENABLE_HIRES_GAIN */

/*
** Function: PROSLIC_dbgSetTXGain
**
** Description: 
** Provision function for setting up
** TX gain
*/
int Si3218x_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
    SILABS_UNREFERENCED_PARAMETER(audio_gain_preset);
#ifdef ENABLE_HIRES_GAIN
	return Si3218x_dbgSetGainHiRes(pProslic,gain,impedance_preset,TXACGAIN_SEL);
#else
	return Si3218x_dbgSetGain(pProslic,gain,impedance_preset,TXACGAIN_SEL);
#endif
}

/*
** Function: PROSLIC_dbgSetRXGain
**
** Description: 
** Provision function for setting up
** RX gain
*/
int Si3218x_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
    SILABS_UNREFERENCED_PARAMETER(audio_gain_preset);
#ifdef ENABLE_HIRES_GAIN
	return Si3218x_dbgSetGainHiRes(pProslic,gain,impedance_preset,RXACGAIN_SEL);
#else
	return Si3218x_dbgSetGain(pProslic,gain,impedance_preset,RXACGAIN_SEL);
#endif
}

/*
** Function: Si3218x_GetRAMScale
**
** Description: 
** Read scale factor for passed RAM location
**
** Return Value:
** int32 scale
*/
static int32 Si3218x_GetRAMScale(uInt16 addr)
{
int32 scale;

    switch(addr)
    {
        case SI3218X_RAM_MADC_ILOOP:
        case SI3218X_RAM_MADC_ITIP:
        case SI3218X_RAM_MADC_IRING:
        case SI3218X_RAM_MADC_ILONG:
            scale = SCALE_I_MADC;
        break;

        case SI3218X_RAM_MADC_VTIPC:
        case SI3218X_RAM_MADC_VRINGC:
        case SI3218X_RAM_MADC_VBAT:
		case SI3218X_RAM_MADC_VDC:
		case SI3218X_RAM_MADC_VDC_OS:
        case SI3218X_RAM_MADC_VLONG:
        case SI3218X_RAM_VDIFF_SENSE:
        case SI3218X_RAM_VDIFF_FILT:
        case SI3218X_RAM_VDIFF_COARSE:
        case SI3218X_RAM_VTIP:
        case SI3218X_RAM_VRING:
            scale = SCALE_V_MADC;
        break;

        default:
            scale = 1;
        break;
    }

    return scale;
}

/*
** Function: Si3218x_ReadMADCScaled
**
** Description: 
** Read MADC (or other sensed voltages/currents) and
** return scaled value in int32 format.
**
** Return Value:
** int32 voltage in mV or
** int32 current in uA
*/
int32 Si3218x_ReadMADCScaled(proslicChanType_ptr pProslic,uInt16 addr, int32 scale)
{
    int32 data;

    /* 
    ** Read 29-bit RAM and sign extend to 32-bits
    */
    data = ReadRAM(pProHW,pProslic->channel,addr);
    if(data & 0x10000000L)
        data |= 0xF0000000L;

    /*
    ** Scale to provided value, or use defaults if scale = 0
    */
    if(scale == 0)
        scale = Si3218x_GetRAMScale(addr);

    data /= scale;

    return data;
}

/*
** Function: Si3218x_LineMonitor
**
** Description: 
** Monitor line voltages and currents
*/
int Si3218x_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor)
{

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }

    if(pProslic->channelEnable)
    {
        monitor->vtr    = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_VDIFF_FILT);
        if(monitor->vtr & 0x10000000L)
            monitor->vtr |= 0xf0000000L;
        monitor->vtr /= SCALE_V_MADC;

        monitor->vtip    = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_VTIP);
        if(monitor->vtip & 0x10000000L)
            monitor->vtip |= 0xf0000000L;
        monitor->vtip /= SCALE_V_MADC;

        monitor->vring    = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_VRING);
        if(monitor->vring & 0x10000000L)
            monitor->vring |= 0xf0000000L;
        monitor->vring /= SCALE_V_MADC;

		monitor->vlong    = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_VLONG);
        if(monitor->vlong & 0x10000000L)
            monitor->vlong |= 0xf0000000L;
        monitor->vlong /= SCALE_V_MADC;

        monitor->vbat    = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_VBAT);
        if(monitor->vbat & 0x10000000L)
            monitor->vbat |= 0xf0000000L;
        monitor->vbat /= SCALE_V_MADC;

		monitor->vdc    = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_VDC);
        if(monitor->vdc & 0x10000000L)
            monitor->vdc |= 0xf0000000L;
		monitor->vdc *= -1L;  /* Invert since sensor inverted */
        monitor->vdc /= SCALE_V_MADC;

        monitor->itr  = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_ILOOP);
        if(monitor->itr & 0x10000000L)
            monitor->itr |= 0xf0000000L;
        monitor->itr /= SCALE_I_MADC;

        monitor->itip  = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_ITIP);
        if(monitor->itip & 0x10000000L)
            monitor->itip |= 0xf0000000L;
        monitor->itip /= SCALE_I_MADC;

        monitor->iring  = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_IRING);
        if(monitor->iring & 0x10000000L)
            monitor->iring |= 0xf0000000L;
        monitor->iring /= SCALE_I_MADC;

        monitor->ilong  = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_ILONG);
        if(monitor->ilong & 0x10000000L)
            monitor->ilong |= 0xf0000000L;
        monitor->ilong /= SCALE_I_MADC;

        monitor->p_hvic  = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_P_Q1_D); /* P_HVIC_LPF */
        if(monitor->p_hvic & 0x10000000L)
            monitor->p_hvic |= 0xf0000000L;
        monitor->p_hvic /= SCALE_P_MADC;
    }

    return RC_NONE;
}

/*
** Function: Si3218x_PSTNCheck
**
** Description: 
** Continuous monitoring of longitudinal current.
** If an average of N samples exceed avgThresh or a
** single sample exceeds singleThresh, the linefeed 
** is forced into the open state.
**
** This protects the port from connecting to a live
** pstn line (faster than power alarm).
**
*/
int Si3218x_PSTNCheck (proslicChanType *pProslic,proslicPSTNCheckObjType *pPSTNCheck)
{
    uInt8 i;

    if( (pProslic->channelType != PROSLIC) 
        || (pPSTNCheck->samples == 0) )
    {
        return RC_NONE;    /* Ignore DAA channels */
    }

    /* Adjust buffer index */
    if(pPSTNCheck->count >= pPSTNCheck->samples)
    {
        pPSTNCheck->buffFull = TRUE;
        pPSTNCheck->count = 0;   /* reset buffer ptr */
    }

    /* Read next sample */
    pPSTNCheck->ilong[pPSTNCheck->count]  = ReadRAM(pProHW,pProslic->channel,SI3218X_RAM_MADC_ILONG);
    if(pPSTNCheck->ilong[pPSTNCheck->count] & 0x10000000L)
        pPSTNCheck->ilong[pPSTNCheck->count] |= 0xf0000000L;
    pPSTNCheck->ilong[pPSTNCheck->count] /= SCALE_I_MADC;

    /* Monitor magnitude only */
    if(pPSTNCheck->ilong[pPSTNCheck->count] < 0)
        pPSTNCheck->ilong[pPSTNCheck->count] = -pPSTNCheck->ilong[pPSTNCheck->count];

    /* Quickly test for single measurement violation */
    if(pPSTNCheck->ilong[pPSTNCheck->count] > pPSTNCheck->singleThresh)
        return RC_PSTN_CHECK_SINGLE_FAIL;  /* fail */

    /* Average once buffer is full */
    if(pPSTNCheck->buffFull == TRUE)  
    {
        pPSTNCheck->avgIlong = 0;
        for(i=0;i<pPSTNCheck->samples; i++)
        {
            pPSTNCheck->avgIlong += pPSTNCheck->ilong[i];
        }
        pPSTNCheck->avgIlong /= pPSTNCheck->samples;

        if(pPSTNCheck->avgIlong > pPSTNCheck->avgThresh)    
        {
            /* reinit obj and return fail */
            pPSTNCheck->count = 0;
            pPSTNCheck->buffFull = FALSE;
            return RC_PSTN_CHECK_AVG_FAIL;
        }
        else
        {
            pPSTNCheck->count++;
            return RC_NONE;
        }   
    }
    else
    {
        pPSTNCheck->count++;
        return RC_NONE;
    }
}




#ifdef PSTN_DET_ENABLE
/*
** Function: abs_int32
**
** Description: 
** abs implementation for int32 type
*/
static int32 abs_int32(int32 a)
{
    if(a < 0)
        return -1*a;
    return a;
}

/*
** Function: Si3218x_DiffPSTNCheck
**
** Description: 
** Monitor for excessive longitudinal current, which
** would be present if a live pstn line was connected
** to the port.
**
** Returns:
** RC_NONE             - test in progress
** RC_COMPLETE_NO_ERR  - test complete, no alarms or errors
** RC_POWER_ALARM_HVIC - test interrupted by HVIC power alarm
** RC_
** 
*/

int Si3218x_DiffPSTNCheck (proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pPSTNCheck){
int32 ramData;
uInt8 loop_status;
int i;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;    /* Ignore DAA channels */
    }


    switch(pPSTNCheck->pState.stage) 
    {
        case 0: 
            /* Optional OPEN foreign voltage measurement - only execute if LCS = 0 */
            /* Disable low power mode */
            pPSTNCheck->enhanceRegSave = ReadReg(pProHW,pProslic->channel,ENHANCE);
            WriteReg(pProHW,pProslic->channel, ENHANCE, pPSTNCheck->enhanceRegSave&0x07); /* Disable powersave */
            pPSTNCheck->vdiff1_avg = 0;
            pPSTNCheck->vdiff2_avg = 0;
            pPSTNCheck->iloop1_avg = 0;
            pPSTNCheck->iloop2_avg = 0;
            pPSTNCheck->return_status = RC_COMPLETE_NO_ERR;
            /* Do OPEN state hazardous voltage measurement if enabled and ONHOOK */
            Si3218x_ReadHookStatus(pProslic,&loop_status);
            if((loop_status == ONHOOK)&&(pPSTNCheck->femf_enable == 1)) 
                pPSTNCheck->pState.stage++;
            else
                pPSTNCheck->pState.stage = 10;

            return RC_NONE;

        case 1:
            /* Change linefeed to OPEN state for HAZV measurement, setup coarse sensors */
            pPSTNCheck->lfstate_entry = ReadReg(pProHW,pProslic->channel, LINEFEED);
            ProSLIC_SetLinefeedStatus(pProslic,LF_OPEN);
            pPSTNCheck->pState.stage++;
            return RC_NONE;

        case 2:
            /* Settle */
            ProSLIC_PSTN_delay_poll(&(pPSTNCheck->pState), PSTN_DET_OPEN_FEMF_SETTLE);
            return RC_NONE;

        case 3: 
            /* Measure HAZV */
            pPSTNCheck->vdiff_open = Si3218x_ReadMADCScaled(pProslic,VDIFF_COARSE,0);
            /* Stop PSTN check if differential voltage > max_femf_vopen present */
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                LOGPRINT("%sDiff PSTN : Vopen = %d mV\n", LOGPRINT_PREFIX, pPSTNCheck->vdiff_open);
            }
#endif
            if(abs_int32(pPSTNCheck->vdiff_open) > pPSTNCheck->max_femf_vopen)
            {
                pPSTNCheck->pState.stage = 70;
                pPSTNCheck->return_status = RC_PSTN_OPEN_FEMF;
            }
            else
            {
                pPSTNCheck->pState.stage = 10;
            }
            return 0;

        case 10:  
            /* Load first DC feed preset */
            ProSLIC_DCFeedSetup(pProslic,pPSTNCheck->dcfPreset1);
            ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
            pPSTNCheck->pState.stage++;
            return RC_NONE; 

        case 11:
            /* Settle */
            ProSLIC_PSTN_delay_poll(&(pPSTNCheck->pState), PSTN_DET_DIFF_IV1_SETTLE);
            return RC_NONE;

        case 12:
            /* Measure VDIFF and ILOOP, switch to 2nd DCFEED setup */
            pPSTNCheck->vdiff1[pPSTNCheck->pState.sampleIterations] = Si3218x_ReadMADCScaled(pProslic,VDIFF_FILT,0);
            pPSTNCheck->iloop1[pPSTNCheck->pState.sampleIterations] = Si3218x_ReadMADCScaled(pProslic,MADC_ILOOP,0);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                LOGPRINT("%sDiff PSTN: Vdiff1[%d] = %d mV\n", LOGPRINT_PREFIX,
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->vdiff1[pPSTNCheck->pState.sampleIterations]);
                LOGPRINT("%sDiff PSTN: Iloop1[%d] = %d uA\n", LOGPRINT_PREFIX, 
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->iloop1[pPSTNCheck->pState.sampleIterations]);
            }
#endif
            pPSTNCheck->pState.sampleIterations++;
            if(pPSTNCheck->pState.sampleIterations >= pPSTNCheck->samples)
            {
                ProSLIC_DCFeedSetup(pProslic,pPSTNCheck->dcfPreset2);
                pPSTNCheck->pState.stage++;
                pPSTNCheck->pState.sampleIterations = 0;
            }
            return RC_NONE;

        case 13:
            /* Settle feed 500ms */
            ProSLIC_PSTN_delay_poll(&(pPSTNCheck->pState), PSTN_DET_DIFF_IV2_SETTLE);
            return RC_NONE;

        case 14:
            /* Measure VDIFF and ILOOP*/
            pPSTNCheck->vdiff2[pPSTNCheck->pState.sampleIterations] = Si3218x_ReadMADCScaled(pProslic,VDIFF_FILT,0);
            pPSTNCheck->iloop2[pPSTNCheck->pState.sampleIterations] = Si3218x_ReadMADCScaled(pProslic,MADC_ILOOP,0);
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                LOGPRINT("%sDiff PSTN: Vdiff2[%d] = %d mV\n", LOGPRINT_PREFIX,
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->vdiff2[pPSTNCheck->pState.sampleIterations]);
                LOGPRINT("%sDiff PSTN: Iloop2[%d] = %d uA\n", LOGPRINT_PREFIX,
                    pPSTNCheck->pState.sampleIterations,pPSTNCheck->iloop2[pPSTNCheck->pState.sampleIterations]);
            }
#endif
            pPSTNCheck->pState.sampleIterations++;
            if(pPSTNCheck->pState.sampleIterations >= pPSTNCheck->samples)
            {
                /* Compute averages */
                for (i=0; i<pPSTNCheck->samples; i++)
                {
                    pPSTNCheck->vdiff1_avg += pPSTNCheck->vdiff1[i];
                    pPSTNCheck->iloop1_avg += pPSTNCheck->iloop1[i];
                    pPSTNCheck->vdiff2_avg += pPSTNCheck->vdiff2[i];
                    pPSTNCheck->iloop2_avg += pPSTNCheck->iloop2[i];
                }
                pPSTNCheck->vdiff1_avg /= pPSTNCheck->samples;
                pPSTNCheck->iloop1_avg /= pPSTNCheck->samples;
                pPSTNCheck->vdiff2_avg /= pPSTNCheck->samples;
                pPSTNCheck->iloop2_avg /= pPSTNCheck->samples;               
                
                /* Force small (probably offset) currents to minimum value */
                if(abs_int32(pPSTNCheck->iloop1_avg) < PSTN_DET_MIN_ILOOP) pPSTNCheck->iloop1_avg = PSTN_DET_MIN_ILOOP;
                if(abs_int32(pPSTNCheck->iloop2_avg) < PSTN_DET_MIN_ILOOP) pPSTNCheck->iloop2_avg = PSTN_DET_MIN_ILOOP;

                /* Calculate measured loop impedance */          
                pPSTNCheck->rl1 = abs_int32((pPSTNCheck->vdiff1_avg*1000L)/pPSTNCheck->iloop1_avg);
                pPSTNCheck->rl2 = abs_int32((pPSTNCheck->vdiff2_avg*1000L)/pPSTNCheck->iloop2_avg);
                
                /* Force non-zero loop resistance */
                if(pPSTNCheck->rl1 == 0) pPSTNCheck->rl1 = 1;
                if(pPSTNCheck->rl2 == 0) pPSTNCheck->rl2 = 1;

                /* Qualify loop impedances */
                pPSTNCheck->rl_ratio = (pPSTNCheck->rl1*1000L)/pPSTNCheck->rl2;
#ifdef ENABLE_DEBUG
            if (pProslic->debugMode)
            {
                const char fmt_string[] = "%sDiffPSTN: %s = %d %s\n";
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "VDIFF1", pPSTNCheck->vdiff1_avg, "mV");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "ILOOP1",pPSTNCheck->iloop1_avg, "uA");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "VDIFF2",pPSTNCheck->vdiff2_avg, "mV");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "ILOOP2",pPSTNCheck->iloop2_avg, "uA");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "RL1",pPSTNCheck->rl1, "ohm");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "RL2",pPSTNCheck->rl2, "ohm");
                LOGPRINT(fmt_string, LOGPRINT_PREFIX, "RL_Ratio",pPSTNCheck->rl_ratio, " ");            
            }
#endif
           
                /* Restore */
                pPSTNCheck->pState.sampleIterations = 0; 
                pPSTNCheck->pState.stage = 70;
            }
            return RC_NONE;
    
        case 70:  /* Reset test state, restore entry conditions */
            ProSLIC_DCFeedSetup(pProslic,pPSTNCheck->entryDCFeedPreset);
            ProSLIC_SetLinefeedStatus(pProslic,pPSTNCheck->lfstate_entry);
            WriteReg(pProHW,pProslic->channel,ENHANCE, pPSTNCheck->enhanceRegSave);
            pPSTNCheck->pState.stage = 0;
            pPSTNCheck->pState.waitIterations = 0;
            pPSTNCheck->pState.sampleIterations = 0;
            return pPSTNCheck->return_status;
            
    }
return RC_NONE;
}

#endif
