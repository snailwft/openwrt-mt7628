/*
** Copyright (c) 2007-2015 by Silicon Laboratories
**
** $Id: proslic.c 4981 2015-04-28 16:18:41Z nizajerk $
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the generic interface file for the ProSLIC drivers. 
**
** Customers should be calling this level for ProSLIC specific
** functions (vs. chipset specific versions of the code)
**
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "proslic_api_config.h"

#ifdef ENABLE_DEBUG
#define LOGPRINT_PREFIX "ProSLIC:"
#endif

#ifdef SI321X
#include "si321x.h"
#include "si321x_intf.h"
#endif
#ifdef SI324X
#include "si324x.h"
#include "si324x_intf.h"
#endif
#ifdef SI322X
#include "si3226.h"
#include "si3226_intf.h"
#endif
#ifdef SI3217X
#include "si3217x.h"
#include "si3217x_intf.h"
#endif
#ifdef SI3218X
#include "si3218x.h"
#include "si3218x_intf.h"
#endif
#ifdef SI3226X
#include "si3226x.h"
#include "si3226x_intf.h"
#endif
#ifdef SI3228X
#include "si3228x.h"
#include "si3228x_intf.h"
#endif

#define pCtrl(X)           (X)->deviceId->ctrlInterface
#define pProHW(X)          pCtrl((X))->hCtrl
#define WriteRAM(PCHAN, CHANNEL, RAMADDR, RAMDATA)        (PCHAN)->deviceId->ctrlInterface->WriteRAM_fptr(pProHW(PCHAN), (CHANNEL), (RAMADDR), (RAMDATA))
#define ReadRAM(PCHAN, CHANNEL, RAMADDR)         (PCHAN)->deviceId->ctrlInterface->ReadRAM_fptr(pProHW(PCHAN), (CHANNEL), (RAMADDR))
#define SetSemaphore(X)    (X)->deviceId->ctrlInterface->Semaphore_fptr
#define ReadReg(PCHAN, CHANNEL, REGADDR) (PCHAN)->deviceId->ctrlInterface->ReadRegister_fptr(pProHW(PCHAN), (CHANNEL), (REGADDR))
#define WriteReg(PCHAN, CHANNEL, REGADDR, REGDATA) (PCHAN)->deviceId->ctrlInterface->WriteRegister_fptr(pProHW(PCHAN), (CHANNEL), (REGADDR), (REGDATA))

/*
** Timers
*/
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define getTime pProslic->deviceId->ctrlInterface->getTime_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer
#define Delay pProslic->deviceId->ctrlInterface->Delay_fptr


/*
** ProSLIC device driver adapters
*/

int ProSLIC_ShutdownChannel (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ShutdownChannel(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ShutdownChannel(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_ShutdownChannel(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ShutdownChannel(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_ShutdownChannel(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}


int32 ProSLIC_ReadMADCScaled(proslicChanType_ptr hProslic,uInt16 addr,int32 scale){
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ReadMADCScaled(hProslic,addr,scale);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_ReadMADCScaled(hProslic,addr,scale);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ReadMADCScaled(hProslic,addr,scale);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_ReadMADCScaled(hProslic,addr,scale);
#endif
return -1;
}

uInt8 ProSLIC_ReadReg(proslicChanType_ptr hProslic,uInt8 addr){
  return ReadReg(hProslic, hProslic->channel, addr);
}

int ProSLIC_WriteReg(proslicChanType_ptr hProslic,uInt8 addr,uInt8 data){
  return WriteReg(hProslic, hProslic->channel, addr, data);
}

ramData ProSLIC_ReadRAM(proslicChanType_ptr hProslic,uInt16 addr)
{
  if(hProslic->channelType != PROSLIC) return RC_IGNORE;
	return (ReadRAM(hProslic, hProslic->channel, addr));
}

int ProSLIC_WriteRAM(proslicChanType_ptr hProslic,uInt16 addr, ramData data)
{
  if(hProslic->channelType != PROSLIC) return RC_IGNORE;
	return (WriteRAM(hProslic, hProslic->channel, addr,data));
}


int ProSLIC_PrintDebugData(proslicChanType_ptr hProslic)
{
#ifdef ENABLE_DEBUG
  ProSLIC_PrintDebugReg(hProslic);
  return ProSLIC_PrintDebugRAM(hProslic);
#else
  return RC_NONE;
#endif
}

int ProSLIC_PrintDebugReg(proslicChanType_ptr hProslic)
{
#ifdef ENABLE_DEBUG
  uInt8 regAddr;
 
  /* 
     NOTE: Not all ProSLICs have defined values after location 99 
     (and have 1 location after that), but for simplicity, we print them anyway... 
  */
  for(regAddr = 0; regAddr < 127; regAddr++)
  {
    LOGPRINT("%sRegister %003u = 0x%02X\n", LOGPRINT_PREFIX, regAddr, ReadReg(hProslic, hProslic->channel, regAddr));
  }
#endif
  return RC_NONE;
}

/* NOTE: locations above 1024 are protected, the API disables protection during initialization, but if this
   function is called prior to initialization, then UAM is not set and will impact the ReadRAM call...
   Upper limit is based upon chipset type...
*/
int ProSLIC_PrintDebugRAM(proslicChanType_ptr hProslic)
{
#ifdef ENABLE_DEBUG
  uInt16 ramAddr;
  uInt16 maxAddr= 0;
  if(hProslic->channelType != PROSLIC) return RC_IGNORE;

#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
    maxAddr = 42;
#endif
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		maxAddr = 906;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		maxAddr = 1578;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		maxAddr = 1596;
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		maxAddr = 1644;
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		maxAddr = 1646;
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		maxAddr = 1646;
#endif

  for(ramAddr = 0; ramAddr < maxAddr; ramAddr++)
  {
    LOGPRINT("%sRAM %04u = 0x%08X\n", LOGPRINT_PREFIX, ramAddr, (unsigned int)(ReadRAM(hProslic, hProslic->channel, ramAddr)));
  }

#endif /* ENABLE_DEBUG */
  return RC_NONE;
}

int ProSLIC_VerifyControlInterface(proslicChanType_ptr hProslic){
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_VerifyControlInterface(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_VerifyControlInterface(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_VerifyControlInterface(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_VerifyControlInterface(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_VerifyControlInterface(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_VerifyControlInterface(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_VerifyControlInterface(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}

#ifdef SIVOICE_MULTI_BOM_SUPPORT
int ProSLIC_Init_MultiBOM (proslicChanType_ptr *hProslic,int size, int preset){
#ifdef SI324X
	if ((*hProslic)->deviceId->chipType >= SI3240 && (*hProslic)->deviceId->chipType <= SI3247)
		return Si324x_Init(hProslic,size);
#endif
#ifdef SI322X
	if ((*hProslic)->deviceId->chipType >= SI3226 && (*hProslic)->deviceId->chipType <= SI3227)
		return Si3226_Init_MultiBOM(hProslic,size,preset);
#endif
#ifdef SI321X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_Init(hProslic,size);
#endif
#ifdef SI3217X
	if ((*hProslic)->deviceId->chipType >= SI32171 && (*hProslic)->deviceId->chipType <= SI32179)
		return Si3217x_Init_MultiBOM(hProslic,size,preset);
#endif
#ifdef SI3218X
	if ((*hProslic)->deviceId->chipType >= SI32180 && (*hProslic)->deviceId->chipType <= SI32189)
		return Si3218x_Init_MultiBOM(hProslic,size,preset);
#endif
#ifdef SI3226X
	if ((*hProslic)->deviceId->chipType >= SI32260 && (*hProslic)->deviceId->chipType <= SI32269)
		return Si3226x_Init_MultiBOM(hProslic,size,preset);
#endif
#ifdef SI3228X
	if ((*hProslic)->deviceId->chipType >= SI32280 && (*hProslic)->deviceId->chipType <= SI32289)
		return Si3228x_Init_MultiBOM(hProslic,size,preset);
#endif
	return RC_COMPLETE_NO_ERR;
}
#endif

int ProSLIC_Init (proslicChanType_ptr *hProslic,int size){
#ifdef SI324X
	if ((*hProslic)->deviceId->chipType >= SI3240 && (*hProslic)->deviceId->chipType <= SI3247)
		return Si324x_Init(hProslic,size);
#endif
#ifdef SI322X
	if ((*hProslic)->deviceId->chipType >= SI3226 && (*hProslic)->deviceId->chipType <= SI3227)
		return Si3226_Init(hProslic,size);
#endif
#ifdef SI321X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_Init(hProslic,size);
#endif
#ifdef SI3217X
	if ((*hProslic)->deviceId->chipType >= SI32171 && (*hProslic)->deviceId->chipType <= SI32179)
		return Si3217x_Init(hProslic,size);
#endif
#ifdef SI3226X
	if ((*hProslic)->deviceId->chipType >= SI32260 && (*hProslic)->deviceId->chipType <= SI32269)
		return Si3226x_Init(hProslic,size);
#endif
#ifdef SI3218X
	if ((*hProslic)->deviceId->chipType >= SI32180 && (*hProslic)->deviceId->chipType <= SI32189)
		return Si3218x_Init(hProslic,size);
#endif
#ifdef SI3228X
	if ((*hProslic)->deviceId->chipType >= SI32280 && (*hProslic)->deviceId->chipType <= SI32289)
		return Si3228x_Init(hProslic,size);
#endif
	return RC_COMPLETE_NO_ERR;
}

/* Deprecated */
int ProSLIC_InitBroadcast (proslicChanType_ptr *hProslic){
#ifdef SI324X
	if ((*hProslic)->deviceId->chipType >= SI3240 && (*hProslic)->deviceId->chipType <= SI3247)
		return Si324x_InitBroadcast(hProslic);
#endif
#ifdef SI321X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_InitBroadcast(hProslic);
#endif
    SILABS_UNREFERENCED_PARAMETER(hProslic);
	return RC_UNSUPPORTED_FEATURE;
}


int ProSLIC_Reinit (proslicChanType_ptr hProslic,int size){
#ifdef SI324X
	if ((hProslic)->deviceId->chipType >= SI3240 && (hProslic)->deviceId->chipType <= SI3247)
		return Si324x_Init(&hProslic,size);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_Reinit(hProslic,size);
#endif
#ifdef SI321X
	if ((hProslic)->deviceId->chipType >= SI3210 && (hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_Init(&hProslic,size);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_Init_with_Options(&hProslic, size, INIT_REINIT);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_Init_with_Options(&hProslic, size, INIT_REINIT);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_Init_with_Options(&hProslic, size, INIT_REINIT);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_Init_with_Options(&hProslic, size, INIT_REINIT);
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_Init_with_Options (proslicChanType_ptr *hProslic,int size, int option){
#ifdef SI3226X
	if ((*hProslic)->deviceId->chipType >= SI32260 && (*hProslic)->deviceId->chipType <= SI32269)
		    return Si3226x_Init_with_Options(hProslic,size, option);
#endif
#ifdef SI3228X
	if ((*hProslic)->deviceId->chipType >= SI32280 && (*hProslic)->deviceId->chipType <= SI32289)
		    return Si3228x_Init_with_Options(hProslic,size, option);
#endif
#ifdef SI3217X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		    return Si3217x_Init_with_Options(hProslic,size, option);
#endif
#ifdef SI3218X
	if ((*hProslic)->deviceId->chipType >= SI32180 && (*hProslic)->deviceId->chipType <= SI32189)
		    return Si3218x_Init_with_Options(hProslic,size, option);
#endif
	return RC_IGNORE;
}

#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined SI3226X || defined SI3228X
/* Check patch data - returns TRUE if no error.*/
static BOOLEAN ProSLIC_VerifyPatchData(proslicChanType *pProslic, const ramData *data, uInt16 maxCount )
{
    int loop;
    ramData read_data;

    for(loop = 0; loop < maxCount; loop++)
    {
       if(*data)
       {
        read_data = ReadRAM(pProslic, pProslic->channel, PROSLIC_RAM_PRAM_DATA);
        if( ((*data) << 9) != read_data)
        {
            return FALSE;
        }
       }
       else
       {
        break;
       }
       data++;
    }
    return TRUE;
}

/* Check if the jump table is written correctly.. */
static BOOLEAN ProSLIC_VerifyPatchJMPLow(proslicChanType *pProslic, const uInt16 *data)
{
    uInt8 address = PATCH_JMPTBL_START_ADDR;
    int regData;

    for(address = PATCH_JMPTBL_START_ADDR; address < (PATCH_JMPTBL_START_ADDR+(2*PATCH_NUM_LOW_ENTRIES)); address++)
    {
        if(*data)
        {
       regData = ReadReg(pProslic, pProslic->channel, address);
       if(regData != ((*data) & 0xFF))
       {
        return FALSE;
       } 

       address++;

       regData = ReadReg(pProslic, pProslic->channel, address);
       if(regData != (((*data)>>8) & 0xFF))
       {
        return FALSE;
       } 

       data++;
       }
       else
       {
        break;
        }
    }
    return TRUE;
}

#if defined SI3226X || defined SI3228X || defined SI3218X
/* For chipsets supporting more than 8 jump entries, verify them */
static BOOLEAN ProSLIC_VerifyPatchJMPHigh(proslicChanType *pProslic, const uInt16 *data)
{
    uInt16 address = PATCH_JMPTBL_HIGH_ADDR;
    ramData read_data;

    for(address = PATCH_JMPTBL_HIGH_ADDR; address < (PATCH_JMPTBL_HIGH_ADDR+PATCH_NUM_HIGH_ENTRIES); address++)
    {
        read_data = (ReadRAM(pProslic, pProslic->channel, address) & 0x1FFFL);
        if(*data != read_data)
        {
            return FALSE;
        
        }
        data++;
    }
    return TRUE;
}
#endif /* SI3226X, SI3228X, SI3218X */

/* Load the first 8 jump table entries */
static void ProSLIC_LoadPatchJMPLow(proslicChanType *pProslic, uInt8 channel, const uInt16 *data)
{
    uInt8 address;
    for(address = PATCH_JMPTBL_START_ADDR; address < (PATCH_JMPTBL_START_ADDR+(2*PATCH_NUM_LOW_ENTRIES)); address++)
    {
        WriteReg(pProslic, channel, address,((*data) & 0xFF));
        address++;
        WriteReg(pProslic, channel, address,(((*data)>>8) & 0xFF));
        data++;
    }
}

/* Load Patch data */
static void ProSLIC_LoadPatchData(proslicChanType *pProslic, uInt8 channel, const ramData *data)
{
    int loop;

    WriteRAM(pProslic, channel, PROSLIC_RAM_PRAM_ADDR, 0);

    for(loop = 0; loop < PATCH_MAX_SIZE; loop++)
    {
        if(*data)
        {
            /* Can we take advantage of auto increment, if not, set the address */
            if( (pProslic->deviceId->chipRev < 3)
                && (channel == PROSLIC_CHAN_BROADCAST))
            {
                WriteRAM(pProslic, channel, PROSLIC_RAM_PRAM_ADDR, loop << 19);
            }

            WriteRAM(pProslic, channel, PROSLIC_RAM_PRAM_DATA, (*data) << 9);
        }
        else
        {
            return;
        }
        data++;
    }
}

/* Load Support RAM - basically RAM address/data pairs to be written as part of the Patch process */
static void ProSLIC_LoadSupportRAM(proslicChanType *pProslic, uInt8 channel, const uInt16 *address, const ramData *data)
{
    while( *address )
    {
        WriteRAM(pProslic, channel, *address, *data);
        address++;
        data++;
    } 
}

#if defined SI3226X || defined SI3228X || defined SI3218X
/* Load Jump table high for chipsets supporting more than 8 jump entries */
static void ProSLIC_LoadJMPHigh(proslicChanType *pProslic, uInt8 channel, const uInt16 *data)
{
    uInt16 loop;
    for(loop = PATCH_JMPTBL_HIGH_ADDR; loop < (PATCH_JMPTBL_HIGH_ADDR+PATCH_NUM_HIGH_ENTRIES); loop++)
    {
        WriteRAM(pProslic, channel, loop, ((*data) & 0x1FFFL) );
        data++;
    }
}
#endif /* SI3226X, SI3228X, SI3218X */

/* Code assumes ProSLIC_LoadPatch has verified chip type */
BOOLEAN ProSLIC_LoadPatch_extended(proslicChanType *pProslic, const proslicPatch *pPatch, BOOLEAN is_broadcast)
{
    uInt8 channel;
    const uInt16 jmp_disable[PATCH_NUM_LOW_ENTRIES] = {0,0,0,0,0,0,0,0};
#if defined SI3226X || defined SI3228X || defined SI3218X
    BOOLEAN hasJmpTableHigh = FALSE;
    const uInt16 jmphigh_disable[PATCH_NUM_HIGH_ENTRIES] = {0, 0, 0, 0, 0, 0, 0, 0};
#endif

    if(pPatch == NULL)
    {
        return RC_NONE;
    }
    
    if(is_broadcast == TRUE)
    {
        channel = PROSLIC_CHAN_BROADCAST;
    }
    else
    {
        channel = pProslic->channel;
    }


#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
    {
        LOGPRINT("%sloading patch: %08lx\n", LOGPRINT_PREFIX, pPatch->patchSerial);
    }
#endif
    /* TODO: see if we can skip patch loads for dual channel chips */

    ProSLIC_SetUserMode(pProslic,TRUE, is_broadcast);

    /* Disable Patch */
    WriteReg(pProslic, channel, PROSLIC_REG_JMPEN, 0); 
    ProSLIC_LoadPatchJMPLow(pProslic, channel, jmp_disable);

#if defined SI3226X || defined SI3228X || defined SI3218X
	if( 0 
#ifdef SI3226X
            || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269) 
#endif
#ifdef SI3228X
            || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289) 
#endif
#ifdef SI3218X
            || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
            )
    {
        hasJmpTableHigh = TRUE;
        ProSLIC_LoadJMPHigh(pProslic, channel, jmphigh_disable);
    }
#endif

    ProSLIC_LoadPatchData(pProslic, channel, pPatch->patchData);

    /* Make sure last RAM write completed */
    Delay(pProTimer, 1);

    WriteReg(pProslic, channel, PROSLIC_REG_RAM_ADDR_HI, 0);

    ProSLIC_LoadPatchJMPLow(pProslic, channel, pPatch->patchEntries);

#if defined SI3226X || defined SI3228X || defined SI3218X
    if(hasJmpTableHigh == TRUE)
    {
        ProSLIC_LoadJMPHigh(pProslic, channel, &(pPatch->patchEntries[PATCH_NUM_LOW_ENTRIES]));
    }
#endif        
#if defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined(SI3226X) || defined(SI3228X)
    WriteRAM(pProslic, channel, PROSLIC_RAM_PATCHID, pPatch->patchSerial); /* write the patch ID */

    ProSLIC_LoadSupportRAM(pProslic, channel, pPatch->psRamAddr, pPatch->psRamData);
#endif
#ifdef DISABLE_VERIFY_PATCH
    WriteReg(pProslic, channel, PROSLIC_REG_JMPEN, 1);
#endif
    return RC_NONE;

}

#endif /* patch helper functions */


int ProSLIC_LoadPatch (proslicChanType_ptr pProslic,const proslicPatch *pPatch)
{
    if(pProslic->channelType != PROSLIC) return RC_IGNORE;

#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_LoadPatch(pProslic,pPatch);
#endif

#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined(SI3226X) || defined(SI3228X)
    if(0
#ifdef SI324X
	|| (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
        || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
        || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
        || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	    || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
        || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
    )
    {
        return ProSLIC_LoadPatch_extended(pProslic, pPatch, FALSE);
    }
#endif /* Si3226, Si3217x, Si3218x, Si3226x, Si3228x, Si324X */
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_VerifyPatch (proslicChanType_ptr hProslic,proslicPatch *pPatch){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI3217X) || defined(SI3218X) || defined(SI3226X) || defined(SI3228X) || defined(SI324X) || defined (SI322X)
    if (0 
#ifdef SI324X
	   || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	    || (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	    || (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	    || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	    || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
    )
    {
        if(pPatch == NULL)
            return RC_NONE;

        WriteReg(hProslic, hProslic->channel, PROSLIC_REG_JMPEN, 0);
        WriteRAM(hProslic, hProslic->channel, PROSLIC_RAM_PRAM_ADDR, 0);

        if(ProSLIC_VerifyPatchData(hProslic, pPatch->patchData, PATCH_MAX_SIZE) == FALSE)
        {
#ifdef ENABLE_DEBUG
            if (hProslic->debugMode)
                LOGPRINT("%sPatch data corrupted: channel %d\n",LOGPRINT_PREFIX,hProslic->channel);
#endif
            return RC_PATCH_RAM_VERIFY_FAIL;
        }

        /*zero out RAM_ADDR_HI*/
        WriteReg (hProslic, hProslic->channel, PROSLIC_REG_RAM_ADDR_HI,0);

        if( ProSLIC_VerifyPatchJMPLow(hProslic, pPatch->patchEntries) == FALSE)
        {
#ifdef ENABLE_DEBUG
            if (hProslic->debugMode)
                LOGPRINT("%sPatch jumptable corrupted: channel %d\n",LOGPRINT_PREFIX,hProslic->channel);
#endif
            return RC_PATCH_RAM_VERIFY_FAIL;
        }

#if defined SI3226X || defined SI3228X || defined SI3218X
	    if( 0
#ifdef SI3226X
               || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269) 
#endif
#ifdef SI3228X
               || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289) 
#endif
#ifdef SI3218X
               || (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
            )

        {
            if( ProSLIC_VerifyPatchJMPHigh(hProslic, &(pPatch->patchEntries[PATCH_NUM_HIGH_ENTRIES])) == FALSE)
            {
#ifdef ENABLE_DEBUG
            if (hProslic->debugMode)
                LOGPRINT("%sPatch jumptable high corrupted: channel %d\n",LOGPRINT_PREFIX,hProslic->channel);
#endif
                return RC_PATCH_ENTRY_VERIFY_FAIL;
            }           
        }

#endif

        WriteReg (hProslic, hProslic->channel, PROSLIC_REG_JMPEN,1); /*enable the patch*/

        return RC_NONE;
    }

#endif /* Si3226, Si3217x, Si3218x, Si3226x, Si3228x, Si324X */
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_SetMuteStatus (proslicChanType_ptr pProslic, ProslicMuteModes muteEn){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetMuteStatus(pProslic,muteEn);
#endif

#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
	  uInt8 regTemp;
    uInt8 newRegValue;

	  regTemp = ReadReg (pProslic,pProslic->channel,PROSLIC_REG_DIGCON);
	
	  WriteReg (pProslic,pProslic->channel,PROSLIC_REG_DIGCON,regTemp&~(0x3));
    newRegValue = regTemp &~(0x3);
	
	  if (muteEn & PROSLIC_MUTE_RX)
    {
          newRegValue |= 1;
    }

	  if (muteEn & PROSLIC_MUTE_TX)
    {
          newRegValue |= 2;
    }

    if(newRegValue != regTemp)
    {
		  return WriteReg (pProslic,pProslic->channel,PROSLIC_REG_DIGCON,newRegValue);
    }

  }
#endif /* Non-321x */
	return RC_COMPLETE_NO_ERR;
}
int ProSLIC_SetLoopbackMode (proslicChanType_ptr pProslic, ProslicLoopbackModes newMode){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetLoopbackMode(pProslic,newMode);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
  	uInt8 regTemp, newValue;
	  newValue = regTemp = ReadReg (pProslic,pProslic->channel,PROSLIC_REG_LOOPBACK);
	  switch (newMode)
    {
      case PROSLIC_LOOPBACK_NONE:
        newValue &= ~(0x11);
        break;

      case PROSLIC_LOOPBACK_DIG:
        newValue |= 1;
        break;

      case PROSLIC_LOOPBACK_ANA:
        newValue |= 0x10;
        break;
    }

    if(newValue != regTemp)
    {
      return WriteReg (pProslic,pProslic->channel,PROSLIC_REG_LOOPBACK, newValue);
    } 
  }
#endif /* non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_EnableInterrupts (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_EnableInterrupts(hProslic);
#endif
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_EnableInterrupts(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_EnableInterrupts(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_EnableInterrupts(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_EnableInterrupts(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_EnableInterrupts(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_EnableInterrupts(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_DisableInterrupts (proslicChanType_ptr hProslic)
{
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DisableInterrupts(hProslic);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
	if ( 0 
#ifdef SI324X
    || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
    )
  {
#ifdef GCI_MODE
    uInt8 data;
#endif
    uInt8 i;

    for(i = PROSLIC_REG_IRQEN1; i < PROSLIC_REG_IRQEN4; i++)
    {
      /* Disable the interupts */
      WriteReg(hProslic, hProslic->channel, i, 0);
    }      

    /* Clear the pending interrupts */
    for(i = PROSLIC_REG_IRQ1; i < PROSLIC_REG_IRQ4; i++)
    {
#ifdef GCI_MODE
      data = ReadReg(hProslic, hProslic->channel, i);
      WriteReg( hProslic, hProslic->channel, i, data);
#else
      (void)ReadReg(hProslic, hProslic->channel, i);
#endif
    }
    return RC_NONE;
  }
#endif /* non-Si321x */
	return RC_IGNORE;
}

int ProSLIC_RingSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_RING_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_RingSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RingSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_RingSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RingSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_RingSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RingSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_RingSetup(hProslic,preset);
#endif
#endif /*DISABLE_RING_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_ToneGenSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_TONE_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_ToneGenSetup(hProslic,preset);
#endif
#endif /*DISABLE_TONE_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_FSKSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_FSK_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_FSKSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_FSKSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_FSKSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_FSKSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_FSKSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_FSKSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_FSKSetup(hProslic,preset);
#endif
#endif /*DISABLE_FSK_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_DTMFDecodeSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_DTMF_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_DTMFDecodeSetup(hProslic,preset);
#endif
#endif /*DISABLE_DTMF_SETUP*/
    SILABS_UNREFERENCED_PARAMETER(hProslic);
    SILABS_UNREFERENCED_PARAMETER(preset);
    return RC_COMPLETE_NO_ERR;
}

int ProSLIC_ZsynthSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_ZSYNTH_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_ZsynthSetup(hProslic,preset);
#endif
#endif /*DISABLE_ZSYNTH_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_GciCISetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_CI_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_GciCISetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_GciCISetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_GciCISetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GciCISetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GciCISetup(hProslic,preset);
#endif
#endif /*DISABLE_CI_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_ModemDetSetup (proslicChanType_ptr hProslic,int preset)
{
	SILABS_UNREFERENCED_PARAMETER(preset);
	ProSLIC_UnsupportedFeatureNoArg(hProslic, "ProSLIC_ModemDetSetup");
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_TXAudioGainSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_TXAudioGainSetup(hProslic,preset);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_RXAudioGainSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_RXAudioGainSetup(hProslic,preset);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_TXAudioGainScale (proslicChanType_ptr hProslic,int preset, uInt32 pga_scale, uInt32 eq_scale){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_RXAudioGainScale (proslicChanType_ptr hProslic,int preset, uInt32 pga_scale, uInt32 eq_scale){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_AudioGainSetup (proslicChanType_ptr pProslic,int32 rxgain, int32 txgain,int preset){
    int rc;
#ifndef DISABLE_AUDIOGAIN_SETUP
    int atx_preset = TXACGAIN_SEL;
    int arx_preset = RXACGAIN_SEL;

#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		Si321x_CalcAG(pProslic,rxgain, txgain, &arx_preset, &atx_preset);
#endif
   
    rc = ProSLIC_dbgSetTXGain(pProslic,txgain,preset,atx_preset);

    if( rc  == RC_NONE) {
        rc = ProSLIC_TXAudioGainSetup(pProslic,TXACGAIN_SEL);
    }

    if( rc  == RC_NONE) {
        rc = ProSLIC_dbgSetRXGain(pProslic,rxgain,preset,arx_preset);
    }

    if( rc  == RC_NONE) {
        rc = ProSLIC_RXAudioGainSetup(pProslic,RXACGAIN_SEL);
    }

#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return rc;
}

int ProSLIC_DCFeedSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_DCFEED_SETUP
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_DCFeedSetup(hProslic,preset);
#endif
#endif /*DISABLE_DCFEED_SETUP*/
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_DCFeedSetupCfg (proslicChanType_ptr hProslic, ProSLIC_DCfeed_Cfg *cfg, int preset){
#ifndef DISABLE_DCFEED_SETUP
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DCFeedSetupCfg(hProslic,cfg,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_DCFeedSetupCfg(hProslic,cfg,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DCFeedSetupCfg(hProslic,cfg,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_DCFeedSetupCfg(hProslic,cfg,preset);
#endif
#endif /*DISABLE_DCFEED_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_GPIOSetup (proslicChanType_ptr hProslic){
#ifndef DISABLE_GPIO_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_GPIOSetup(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_GPIOSetup(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GPIOSetup(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GPIOSetup(hProslic);
#endif
#endif /*DISABLE_GPIO_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PulseMeterSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_PULSE_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_PulseMeterSetup(hProslic,preset);
#endif
#endif /*DISABLE_PULSE_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PCMSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_PCM_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PCMSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PCMSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PCMSetup(hProslic,preset);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_PCMSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PCMSetup(hProslic,preset);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_PCMSetup(hProslic,preset);
#endif
#endif /*DISABLE_PCM_SETUP*/
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PCMTimeSlotSetup (proslicChanType_ptr pProslic, uInt16 rxcount, uInt16 txcount){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMTimeSlotSetup(pProslic,rxcount,txcount);
#endif

#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 data;
    data = txcount & 0xff;
    WriteReg(pProslic,pProslic->channel,PROSLIC_REG_PCMTXLO,data);
    data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_PCMTXHI);
    data &= 0x10;  /* keep TX_EDGE bit */
    data |= ((txcount >> 8)&0x03) ;
    WriteReg(pProslic,pProslic->channel,PROSLIC_REG_PCMTXHI,data);
    data = rxcount & 0xff;
    WriteReg(pProslic,pProslic->channel,PROSLIC_REG_PCMRXLO,data);
    data = rxcount >> 8 ;
    WriteReg(pProslic,pProslic->channel,PROSLIC_REG_PCMRXHI,data);
    return RC_NONE;
  }
#endif /* Non-Si321x */
	return RC_COMPLETE_NO_ERR;
}

typedef ProslicInt proslicIntTypeMap[SI_MAX_INTERRUPTS][8];

static int ProSLIC_ReadInterruptsHelper(proslicChanType_ptr pProslic, uInt8 *regData, uInt8 numChannels)
{
    uInt8 i;
    uInt8 intsActive;
    uInt8 *currentData = regData;
#if defined(PROSLIC_OPTIMIZE_INTERRUPTS) && defined(SI324X)
    intsActive = ReadReg(pProslic, pProslic->channel, PROSLIC_REG_IRQ);
    if( (numChannels == 4) && ((1<<(pProslic->channel &0x3)) & intsActive ) == 0)
    {
        return RC_IGNORE;
    }
#else
	SILABS_UNREFERENCED_PARAMETER(numChannels);
#endif

    intsActive = ReadReg(pProslic, pProslic->channel, PROSLIC_REG_IRQ0);

#ifdef PROSLIC_OPTIMIZE_INTERRUPTS
    /* For dual channel devices, we need to shift the upper nibble for the second channel
       if the caller requested the second channel.  We determine this by channel ID being
       even or odd.  If this is NOT true - for example a Si3217x and then a Si3226x chipset
       configuration on the same daisychain, then this optimization logic will not work */
#if defined(SI322X) || defined(SI3226X) || defined(SI3228X) || defined (SI324X)
       if( (numChannels != 0) && ((pProslic->channel) & 0x1))
       {
         intsActive = intsActive >> 4;
       }        
#endif /* Multichannel devices */
#endif

    /* If there are no interrupts, stop... return back to calling function */
    if((intsActive &0xF) == 0)
    {
        return RC_IGNORE;
    }

    for(i = PROSLIC_REG_IRQ1; i < PROSLIC_REG_IRQ4; i++)
    {
#ifdef PROSLIC_OPTIMIZE_INTERRUPTS
        /* Read IRQn Register only if IRQ0 states there was an interrupt pending, otherwise
           skip it. This eliminates unneeded SPI transactions.
        */
        if( (intsActive & (1<<(i-PROSLIC_REG_IRQ1))) == 0)
        {
           *currentData++ = 0;
           continue;
        }
#endif
        *currentData = (uInt8)ReadReg(pProslic, pProslic->channel, i);
#ifdef GCI_MODDE
        WriteReg(pProslic, pProslic->channel, i, *current_data);
#endif
        currentData++;
    } /* for loop */
    return RC_COMPLETE_NO_ERR;
}

/* 
 Reads IRQ0 to determine if an interrupt has occurred for the particular device, 
 if so, reads the interrupt register(s) that fired the interrupt and then
 maps the interrupt(s) to the generic interrupt value to return to the caller.

 Code assumes normal chipset/compatibility testing has already been done.  
*/
static int ProSLIC_GetInterruptHelper(proslicChanType_ptr pProslic, const proslicIntTypeMap intMap, proslicIntType *pIntData, uInt8 numChannels)
{
    int i,j;
    uInt8 intData[SI_MAX_INTERRUPTS];
    uInt8 *currentData;
    uInt8 map, intCount;
    BOOLEAN safetyInt = FALSE;

    /* Initialize interrupt count to 0 */
    pIntData-> number = 0;

    if( ProSLIC_ReadInterruptsHelper(pProslic, intData, numChannels) == RC_IGNORE) 
    {
        /* No interrupts for the given channel. */
        return RC_NONE;
    }
   
    /* At this point we've collected all the registers, now decode the data */
    currentData = intData;
    intCount = 0;
    for(i = 0; i < SI_MAX_INTERRUPTS; i++)
    {
        if(*currentData)
        {
            for(j = 0; j < 8; j++)
            {
                if( *currentData & (1<<j) )
                {
                    map = intMap[i][j];
                    pIntData->irqs[intCount] = map;

                    if( (map == IRQ_P_HVIC) 
                        || (map == IRQ_P_THERM) )
                    {
                        safetyInt = TRUE;
                    }

                    intCount++;
                }
            }
        }
        currentData++;
    }
    pIntData->number = intCount;

    if( safetyInt == TRUE)
    {
        if(ProSLIC_isReinitRequired(pProslic))
        {
            return RC_REINIT_REQUIRED;
        }
    }

    return pIntData->number;
}


int ProSLIC_GetInterrupts (proslicChanType_ptr hProslic,proslicIntType *pIntData){
	pIntData->number=0;
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_GetInterrupts(hProslic,pIntData);
#endif
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
    {
        const proslicIntTypeMap si324xInterrupts = { 
            {IRQ_OSC1_T1, IRQ_OSC1_T2, IRQ_OSC2_T1, IRQ_OSC2_T2, IRQ_RING_T1, IRQ_RING_T2, IRQ_FSKBUF_AVAIL, IRQ_VBAT},
            {IRQ_RING_TRIP, IRQ_LOOP_STATUS, IRQ_LONG_STAT, IRQ_VOC_TRACK, IRQ_DTMF, IRQ_INDIRECT, IRQ_TXMDM, IRQ_RXMDM},
            {IRQ_PQ1, IRQ_PQ2, IRQ_PQ3, IRQ_PQ4, IRQ_PQ5, IRQ_PQ6, IRQ_RING_FAIL, IRQ_CM_BAL},
            {IRQ_USER_0, IRQ_USER_1, IRQ_USER_2, IRQ_USER_3, IRQ_USER_4, IRQ_USER_5, IRQ_USER_6, IRQ_USER_7}};

        return ProSLIC_GetInterruptHelper(hProslic, si324xInterrupts, pIntData, 4);

    }
		
#endif

#if defined (SI3217X) || defined (SI3218X)
	if (0
#ifdef SI3217X    
        || (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
        || (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
    )
    {
        const proslicIntTypeMap si3217xInterrupts = { 
            {IRQ_OSC1_T1, IRQ_OSC1_T2, IRQ_OSC2_T1, IRQ_OSC2_T2, IRQ_RING_T1, IRQ_RING_T2, IRQ_FSKBUF_AVAIL, IRQ_VBAT},
            {IRQ_RING_TRIP, IRQ_LOOP_STATUS, IRQ_LONG_STAT, IRQ_VOC_TRACK, IRQ_DTMF, IRQ_INDIRECT, IRQ_TXMDM, IRQ_RXMDM},
            {IRQ_P_HVIC, IRQ_P_THERM, IRQ_PQ3, IRQ_PQ4, IRQ_PQ5, IRQ_PQ6, IRQ_DSP, IRQ_MADC_FS},
            {IRQ_USER_0, IRQ_USER_1, IRQ_USER_2, IRQ_USER_3, IRQ_USER_4, IRQ_USER_5, IRQ_USER_6, IRQ_USER_7}};

        return ProSLIC_GetInterruptHelper(hProslic, si3217xInterrupts, pIntData, 1);

    }
#endif
#if defined (SI322X) || defined (SI3226X) || defined (SI3228X)
    if( 0
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3226X
	    || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	    || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
    )
    {
        const proslicIntTypeMap si3226xInterrupts = { 
            {IRQ_OSC1_T1, IRQ_OSC1_T2, IRQ_OSC2_T1, IRQ_OSC2_T2, IRQ_RING_T1, IRQ_RING_T2, IRQ_FSKBUF_AVAIL, IRQ_VBAT},
            {IRQ_RING_TRIP, IRQ_LOOP_STATUS, IRQ_LONG_STAT, IRQ_VOC_TRACK, IRQ_DTMF, IRQ_INDIRECT, IRQ_TXMDM, IRQ_RXMDM},
            {IRQ_P_HVIC, IRQ_P_THERM, IRQ_PQ3, IRQ_PQ4, IRQ_PQ5, IRQ_PQ6, IRQ_DSP, IRQ_MADC_FS},
            {IRQ_USER_0, IRQ_USER_1, IRQ_USER_2, IRQ_USER_3, IRQ_USER_4, IRQ_USER_5, IRQ_USER_6, IRQ_USER_7}};

        return ProSLIC_GetInterruptHelper(hProslic, si3226xInterrupts, pIntData, 2);
    }
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_ReadHookStatus (proslicChanType_ptr pProslic,uInt8 *pHookStat){
    *pHookStat = PROSLIC_ONHOOK;
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_ReadHookStatus(pProslic,pHookStat);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
	if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    if (ReadReg(pProslic,pProslic->channel,PROSLIC_REG_LCRRTP) & 2)
        *pHookStat=PROSLIC_OFFHOOK;
    return RC_NONE;
  }
#endif /* Non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_SetLinefeedStatus (proslicChanType_ptr hProslic, uInt8 newLinefeed){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_SetLinefeedStatusBroadcast (proslicChanType_ptr hProslic, uInt8 newLinefeed){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
	return RC_COMPLETE_NO_ERR;
}
int ProSLIC_PolRev (proslicChanType_ptr pProslic,uInt8 abrupt,uInt8 newPolRevState)
{
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_PolRev(pProslic,abrupt,newPolRevState);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 data = 0;
    switch (newPolRevState)
    {
      case POLREV_STOP:
        data = 0;
        break;
      case POLREV_START:
        data = 2;
        break;
      case WINK_START:
        data = 6;
        break;
      case WINK_STOP:
        data = 4;
        break;
    }	
    
    /* Cannot polrev/wink while low power mode is active */
	  ProSLIC_SetPowersaveMode(pProslic,PWRSAVE_DISABLE);

	  if (abrupt)
  		data |= 1;

	  WriteReg(pProslic, pProslic->channel, PROSLIC_REG_POLREV,data);

    return RC_NONE;

  }
#endif /* Non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_GPIOControl (proslicChanType_ptr pProslic,uInt8 *pGpioData, uInt8 read){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_GPIOControl(pProslic,pGpioData,read);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined (SI3226X) 
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
  )
  {
  	if (read)
		  *pGpioData = 0xf & ReadReg(pProslic,pProslic->channel,PROSLIC_REG_GPIO);
	  else
		  WriteReg(pProslic,pProslic->channel,PROSLIC_REG_GPIO,(*pGpioData)|(ReadReg(pProslic,pProslic->channel,PROSLIC_REG_GPIO)&0xf0));
    return RC_NONE;
  }
#endif /* non-si321x case */
	return RC_COMPLETE_NO_ERR;
}

/*
** Optional Neon Message Waiting Support
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWISetup (proslicChanType_ptr hProslic, uInt16 vpk_mag, uInt16 lcrmask_mwi){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined SI324X || defined SI322X || defined SI321X
	if ( 0
#ifdef SI324X
        || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI321X
	    || (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
#endif
        )
		    return RC_UNSUPPORTED_FEATURE;
#endif /* SI324X || SI322X || SI321X */

#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWISetup(hProslic,vpk_mag,lcrmask_mwi);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_MWISetup(hProslic,vpk_mag,lcrmask_mwi);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWISetup(hProslic,vpk_mag,lcrmask_mwi);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_MWISetup(hProslic,vpk_mag,lcrmask_mwi);
#endif
	return RC_COMPLETE_NO_ERR;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWIEnable (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined SI324X || defined SI322X || defined SI321X
	if ( 0
#ifdef SI324X
        || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI321X
	    || (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
#endif
        )
		    return RC_UNSUPPORTED_FEATURE;
#endif /* SI324X || SI322X || SI321X */

#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWIEnable(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_MWIEnable(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWIEnable(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_MWIEnable(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWIDisable (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined SI324X || defined SI322X || defined SI321X
	if ( 0
#ifdef SI324X
        || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI321X
	    || (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
#endif
        )
		    return RC_UNSUPPORTED_FEATURE;
#endif /* SI324X || SI322X || SI321X */

#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWIDisable(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_MWIDisable(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWIDisable(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_MWIDisable(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_SetMWIState (proslicChanType_ptr hProslic,uInt8 flash_on){
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined SI324X || defined SI322X || defined SI321X
	if ( 0
#ifdef SI324X
        || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI321X
	    || (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
#endif
        )
		    return RC_UNSUPPORTED_FEATURE;
#endif /* SI324X || SI322X || SI321X */


#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetMWIState(hProslic,flash_on);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_SetMWIState(hProslic,flash_on);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetMWIState(hProslic,flash_on);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_SetMWIState(hProslic,flash_on);
#endif
	return RC_COMPLETE_NO_ERR;
}
#endif


#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_GetMWIState (proslicChanType_ptr hProslic){
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined SI324X || defined SI322X || defined SI321X
	if ( 0
#ifdef SI324X
        || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI321X
	    || (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
#endif
        )
		    return RC_UNSUPPORTED_FEATURE;
#endif /* SI324X || SI322X || SI321X */

#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GetMWIState(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_GetMWIState(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GetMWIState(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_GetMWIState(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWI (proslicChanType_ptr hProslic,uInt8 lampOn){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_MWI(hProslic,lampOn);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_MWI(hProslic,lampOn);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_MWI(hProslic,lampOn);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWI(hProslic,lampOn);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_MWI(hProslic,lampOn);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWI(hProslic,lampOn);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_MWI(hProslic,lampOn);
#endif
	return RC_COMPLETE_NO_ERR;
}
#endif


int ProSLIC_ToneGenStart (proslicChanType_ptr pProslic,uInt8 timerEn){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_ToneGenStart(pProslic,timerEn);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("%s%s on channel %d\n",LOGPRINT_PREFIX, __FUNCTION__, pProslic->channel);
#endif

    data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_OCON);
    data |= 0x11 + (timerEn ? 0x66 : 0);
    return WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OCON,data);
  }
#endif /* Non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_ToneGenStop (proslicChanType_ptr pProslic){
			if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_ToneGenStop(pProslic);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("%s%s on channel %d\n",LOGPRINT_PREFIX, __FUNCTION__, pProslic->channel);
#endif

    data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_OCON);
	  data &= ~(0x77);
    return WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OCON,data);

  }
#endif /* Non-Si321x case */
	return RC_COMPLETE_NO_ERR;

}

int ProSLIC_RingStart (proslicChanType_ptr pProslic)
{
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sRingStart\n", LOGPRINT_PREFIX);
#endif

    return(ProSLIC_SetLinefeedStatus(pProslic, LF_RINGING));
}

int ProSLIC_RingStop (proslicChanType_ptr pProslic){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sRingStart\n", LOGPRINT_PREFIX);
#endif

    return(ProSLIC_SetLinefeedStatus(pProslic, LF_FWD_ACTIVE));
}

int ProSLIC_CheckCIDBuffer (proslicChanType_ptr pProslic, uInt8 *fsk_buf_avail){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_CheckCIDBuffer(pProslic,fsk_buf_avail);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
	  uInt8 data;
	  data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_IRQ1);
#ifdef GCI_MODE
	  WriteReg(pProslic,pProslic->channel,PROSLIC_REG_IRQ1,data); /*clear (for GCI)*/
#endif
	  *fsk_buf_avail = (data&0x40) ? 1 : 0;
      return RC_NONE;
  }
#endif /* Non-Si321x */
	return RC_COMPLETE_NO_ERR;

}

int ProSLIC_EnableCID (proslicChanType_ptr pProslic){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_EnableCID(pProslic);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
	  uInt8 data;
#ifdef ENABLE_DEBUG
	  if (pProslic->debugMode)
		  LOGPRINT ("%sEnableCID on channel %d\n",LOGPRINT_PREFIX, pProslic->channel);
#endif	
	  WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OCON,0);

	  data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_OMODE);
	  data |= 0xA;
	  WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OMODE,data);

	  return(WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OCON,0x5));
  }
#endif /* Non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_DisableCID (proslicChanType_ptr pProslic){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_DisableCID(pProslic);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
 	  uInt8 data;
#ifdef ENABLE_DEBUG
	  if (pProslic->debugMode)
		  LOGPRINT ("%sDisableCID on channel %d\n",LOGPRINT_PREFIX, pProslic->channel);
#endif
	  WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OCON,0);
	  data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_OMODE);
	  data &= ~(0x8);
	  return(WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OMODE,data));
  }
#endif /* non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_SendCID (proslicChanType_ptr pProslic, uInt8 *buffer, uInt8 numBytes){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_SendCID(pProslic,buffer,numBytes);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
#ifndef ENABLE_DEBUG
	  if (pProslic->debugMode)
		  LOGPRINT ("%sSendCID on channel %d\n",LOGPRINT_PREFIX, pProslic->channel);
#endif
    while (numBytes-- > 0)
    {
        WriteReg(pProslic,pProslic->channel,PROSLIC_REG_FSKDAT,*(buffer++));
    }
    return RC_NONE;
  }
#endif /* Non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_ModifyCIDStartBits(proslicChanType_ptr pProslic, uInt8 enable_startStop)
{
    if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 data;

    if(pProslic->channelType != PROSLIC) 
    {
        return RC_CHANNEL_TYPE_ERR;
    }

    data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_OMODE);

    if(enable_startStop == FALSE)
    {
      data &= ~0x80;
    }
    else
    {
      data |= 0x80;
    }

    WriteReg(pProslic,pProslic->channel,PROSLIC_REG_OMODE,data);

    return RC_NONE;

  }
#endif /* non-1x case */

	return RC_UNSUPPORTED_FEATURE;

}

int ProSLIC_PCMStart (proslicChanType_ptr pProslic)
{
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMStart(pProslic);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 data;
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sPCMStart\n", LOGPRINT_PREFIX);
#endif
    data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_PCMMODE);
    data |= 0x10;
    return WriteReg(pProslic,pProslic->channel,PROSLIC_REG_PCMMODE,data);
	}	
#endif /* non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PCMStop (proslicChanType_ptr pProslic)
{
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMStop(pProslic);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 data;
#ifdef ENABLE_DEBUG
    if (pProslic->debugMode)
        LOGPRINT("%sPCMStart\n", LOGPRINT_PREFIX);
#endif
    data = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_PCMMODE);
    data &= ~0x10;
    return WriteReg(pProslic,pProslic->channel,PROSLIC_REG_PCMMODE,data);
	}	
#endif /* non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}

/*
** Function: ProSLIC_ResetDialPulseDetect
**
** Description: 
** reset dial pulse detection state machine (helper function for 
** ProSLIC_InitializeHookChangeDetect.
*/
static void ProSLIC_ResetDialPulseDetect(hookChangeType *pPulse)
{
	pPulse->currentPulseDigit = 0;
	pPulse->lookingForTimeout = 0;
	pPulse->last_hook_state = 5; /* this is invalid */
}

/*
** Function: ProSLIC_InitializeHookChangeDetect
**
** Description: 
** Initialize dial pulse detection parameters
*/
int ProSLIC_InitializeHookChangeDetect(hookChangeType *pPulse,void *hookTime)
{
	pPulse->hookTime = hookTime;
    pPulse->last_state_reported =  SI_HC_NO_ACTIVITY;
	ProSLIC_ResetDialPulseDetect(pPulse);
	return RC_NONE;
}

/*
** Function: ProSLIC_HookChangeDetect
**
** Description: 
** implements pulse dial detection and should be called at every hook transition
*/
uInt8 ProSLIC_HookChangeDetect (proslicChanType *pProslic, hookChange_Cfg *pHookChangeCfg, hookChangeType *pHookChangeData){
	uInt8 hookStat;
	int delta_time;
	
	TimeElapsed(pProTimer,pHookChangeData->hookTime,&delta_time); 
	ProSLIC_ReadHookStatus(pProslic,&hookStat);

	/* Did we have a hook state change? */
	if(hookStat !=  pHookChangeData->last_hook_state)
	{
		pHookChangeData->last_hook_state = hookStat;
		getTime(pProTimer,pHookChangeData->hookTime); 
	    pHookChangeData->lookingForTimeout = 1;

		if (hookStat == PROSLIC_OFFHOOK)
		{
			if ((delta_time >= (pHookChangeCfg->minOnHook)) && (delta_time <= (pHookChangeCfg->maxOnHook)))
			{
				pHookChangeData->currentPulseDigit++;
			}
            else
            {
                /* Did we see a hook flash? */
                if( (delta_time >= pHookChangeCfg->minHookFlash) && (delta_time <= pHookChangeCfg->maxHookFlash) )
                {
                    pHookChangeData->last_state_reported = SI_HC_HOOKFLASH;
                    ProSLIC_ResetDialPulseDetect(pHookChangeData);
                    return SI_HC_HOOKFLASH;
                }
            }
        }	

		return SI_HC_NEED_MORE_POLLS; 
	}

    if( (pHookChangeData->lookingForTimeout == 1) 
                && (delta_time >=  pHookChangeCfg->minInterDigit) )
    {

            if(delta_time  > pHookChangeCfg->minHook)
            {
                if(pHookChangeData->last_hook_state == PROSLIC_ONHOOK)
                {
	                ProSLIC_ResetDialPulseDetect(pHookChangeData);
                    pHookChangeData->last_state_reported = SI_HC_ONHOOK_TIMEOUT;
                    return SI_HC_ONHOOK_TIMEOUT;
                }

                if(pHookChangeData->last_hook_state == PROSLIC_OFFHOOK)
                {
	                ProSLIC_ResetDialPulseDetect(pHookChangeData);

                    /* Check if we saw either a pulse digit or hook flash prior to this,
                     * if so, we're already offhook, so do not report a offhook event,
                     * just stop polling.
                     */
                    if((pHookChangeData->last_state_reported == SI_HC_ONHOOK_TIMEOUT)
                            || (pHookChangeData->last_state_reported == SI_HC_NO_ACTIVITY) )
                    {
                        pHookChangeData->last_state_reported = SI_HC_OFFHOOK_TIMEOUT;
                        return SI_HC_OFFHOOK_TIMEOUT;
                    }
                    else
                    {
                        return SI_HC_NO_ACTIVITY;
                    }
                }
            }
            else
            {
                uInt8 last_digit = pHookChangeData->currentPulseDigit;

                if(last_digit)
                {
                    pHookChangeData->last_state_reported = last_digit;
	                ProSLIC_ResetDialPulseDetect(pHookChangeData);
				    return last_digit;
                }
            }
		    return SI_HC_NEED_MORE_POLLS; 
    }
 	
    return SI_HC_NEED_MORE_POLLS; 
}

int ProSLIC_DTMFReadDigit (proslicChanType_ptr pProslic,uInt8 *pDigit)
{
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_DTMFReadDigit (pProslic,pDigit);
#endif
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
     if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
#ifdef ENABLE_DEBUG
	  if (pProslic->debugMode)
		  LOGPRINT ("%sDTMFReadDigit on channel %d\n",LOGPRINT_PREFIX, pProslic->channel);
#endif
	  *pDigit = ReadReg(pProslic,pProslic->channel,PROSLIC_REG_TONDTMF) & 0xf;

    return RC_NONE;
  }
#endif /* non-Si321x case */
	return RC_COMPLETE_NO_ERR;
}
int ProSLIC_PLLFreeRunStart (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
	if (0 
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227) 
#endif
#ifdef SI3217X
      ||(hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
      ||(hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
      || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
      || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 tmp;

    WriteReg(hProslic,hProslic->channel,PROSLIC_REG_ZCAL_EN,0x00);
    tmp = ReadReg(hProslic,hProslic->channel,PROSLIC_REG_ENHANCE);
    return(WriteReg(hProslic,hProslic->channel,PROSLIC_REG_ENHANCE,tmp|0x4));
  }
#endif /* 17x, 18x, 26x or 28x case */

	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PLLFreeRunStop (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined (SI3217X) || defined (SI3218X) || defined (SI3226X) || defined (SI3228X)
	if (0 
#ifdef SI322X
	    || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
      ||(hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
      ||(hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
      || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
      || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 tmp;

    tmp = ReadReg(hProslic,hProslic->channel,PROSLIC_REG_ENHANCE);
    WriteReg(hProslic,hProslic->channel,PROSLIC_REG_ENHANCE,tmp&~(0x4));
    return WriteReg(hProslic,hProslic->channel,PROSLIC_REG_ZCAL_EN,0x04);
  }
#endif /* 17x, 18x, 26x, 28x or 2x case */
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_GetPLLFreeRunStatus (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined (SI322X) || defined (SI3217X) || defined (SI3218X) || defined (SI3226X) || defined (SI3228X)
  if (0 
#ifdef SI322X
	  || (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
      ||(hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
      ||(hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
      || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
      || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    uInt8 tmp;
    tmp = ReadReg(hProslic,hProslic->channel,PROSLIC_REG_ENHANCE);
    if(tmp & 0x02)
    {
        return RC_PLL_FREERUN_ACTIVE;
    }
    return RC_NONE;
  }
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PulseMeterEnable (proslicChanType_ptr hProslic)
{
    uInt8 widebandEn;

    if(hProslic->channelType != PROSLIC) return RC_IGNORE;

    widebandEn = ReadReg(hProslic,hProslic->channel,PROSLIC_REG_ENHANCE) & 0x01;

if (widebandEn)
    LOGPRINT ("%s Pulse Metering is not supported while Wideband Mode is enabled.\n", LOGPRINT_PREFIX);
    
#if defined (SI3217X) || defined (SI3218X) || defined (SI3226X) || defined (SI3228X)
  if(0 
#ifdef SI3217X
	  || (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
      ||(hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189 && !widebandEn)
#endif
#ifdef SI3226X
	  || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289 && !widebandEn)
#endif
  )
  {
    return WriteReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON,ReadReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON) | (0x01));
  }
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PulseMeterDisable (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined (SI3217X) || defined (SI3218X) || defined (SI3226X) || defined (SI3228X)
  if(0 
#ifdef SI3217X
	  || (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
      ||(hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    return(WriteReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON,ReadReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON) & ~(0x05)));
  }
#endif

	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PulseMeterStart (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterStart (hProslic);
#endif
#if defined (SI3217X) || defined (SI3218X) || defined (SI3226X) || defined (SI3228X) || defined (SI324X)
  if(0
#ifdef SI3226X
	  || (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
#endif
#ifdef SI3217X
	 ||(hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
      ||(hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI324X
	  || (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
#endif
    )
    {
      return WriteReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON,ReadReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON) | (0x5));
    }
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PulseMeterStop (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return ProSLIC_PulseMeterDisable(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterStop (hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
    return(WriteReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON,ReadReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON) & ~(0x04)));
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
    return(WriteReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON,ReadReg(hProslic,hProslic->channel,PROSLIC_REG_PMCON) & ~(0x04)));
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return ProSLIC_PulseMeterDisable(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return ProSLIC_PulseMeterDisable(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_SetDCDCInversionFlag (proslicChanType_ptr hProslic, uInt8 flag){
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
    hProslic->dcdc_polarity_invert = flag;
    return RC_COMPLETE_NO_ERR;
}


int ProSLIC_PowerUpConverter (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PowerUpConverter(hProslic);;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PowerUpConverter(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_PowerUpConverter(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PowerUpConverter(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_PowerUpConverter(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_PowerDownConverter (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PowerDownConverter(hProslic);;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PowerDownConverter(hProslic);
#endif
#ifdef SI3218X
	if (hProslic->deviceId->chipType >= SI32180 && hProslic->deviceId->chipType <= SI32189)
		return Si3218x_PowerDownConverter(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PowerDownConverter(hProslic);
#endif
#ifdef SI3228X
	if (hProslic->deviceId->chipType >= SI32280 && hProslic->deviceId->chipType <= SI32289)
		return Si3228x_PowerDownConverter(hProslic);
#endif
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_LBCal (proslicChanType_ptr *pProslic, int size){
		if((*pProslic)->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (pProslic[0]->deviceId->chipType >= SI3226 && pProslic[0]->deviceId->chipType <= SI3227)
		return Si3226_LBCal (pProslic,size);
#endif
#ifdef SI3217X
	if (pProslic[0]->deviceId->chipType >= SI32171 && pProslic[0]->deviceId->chipType <= SI32179)
		return Si3217x_LBCal(pProslic,size);
#endif
#ifdef SI3218X
	if (pProslic[0]->deviceId->chipType >= SI32180 && pProslic[0]->deviceId->chipType <= SI32189)
		return Si3218x_LBCal(pProslic,size);
#endif
#ifdef SI3226X
	if (pProslic[0]->deviceId->chipType >= SI32260 && pProslic[0]->deviceId->chipType <= SI32269)
		return Si3226x_LBCal(pProslic,size);
#endif
#ifdef SI3228X
	if (pProslic[0]->deviceId->chipType >= SI32280 && pProslic[0]->deviceId->chipType <= SI32289)
		return Si3228x_LBCal(pProslic,size);
#endif
#ifdef SI324X
	  if (pProslic[0]->deviceId->chipType >= SI3240 && pProslic[0]->deviceId->chipType <= SI3247)
		return Si324x_LBCal(pProslic,size);
#endif
	return RC_COMPLETE_NO_ERR;
}

#define PROSLIC_RAM_CMDAC_FWD 1476
#define PROSLIC_RAM_CMDAC_REV 1477
#define PROSLIC_RAM_CAL_TRNRD_DACT 1458
#define PROSLIC_RAM_CAL_TRNRD_DACR 1459

int ProSLIC_GetLBCalResult (proslicChanType *pProslic,int32 *result1,int32 *result2, int32 *result3, int32 *result4){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
    if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    *result1 = ReadRAM(pProslic, pProslic->channel, PROSLIC_RAM_CMDAC_FWD);
    *result2 = ReadRAM(pProslic, pProslic->channel, PROSLIC_RAM_CMDAC_REV);
    *result3 = ReadRAM(pProslic, pProslic->channel, PROSLIC_RAM_CAL_TRNRD_DACT);
    *result4 = ReadRAM(pProslic, pProslic->channel, PROSLIC_RAM_CAL_TRNRD_DACR);
    return RC_NONE;
  }
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result){
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
    int32 results[4];
#endif
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)

    if( ProSLIC_GetLBCalResult(pProslic, &(results[0]), &(results[1]), &(results[2]), &(results[3])) == 0 )
    {
      /* 
        Bits 31:24   CMDAC_FWD[25:18]
        Bits 23:16   CMDAC_REV[25:18]
        Bits 15:8    CAL_TRNRD_DACT[20:13]
        Bits 7:0     CAL_TRNRD_DACR[20:13]
      */
      *result = (results[0]<<6) & 0xff000000L;
      *result |=(results[1]>>1) & 0x00ff0000L;
      *result |=(results[2]>>5) & 0x0000ff00L;
      *result |=(results[3]>>13)& 0x000000ffL;
      return RC_NONE;
    }
#endif
	  return RC_COMPLETE_NO_ERR;
}

int ProSLIC_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2,int32 result3,int32 result4){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
    if( 0 
#ifdef SI324X
	  || (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
#endif
#ifdef SI322X
	  || (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
#endif
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
  )
  {
    WriteRAM(pProslic,pProslic->channel,PROSLIC_RAM_CMDAC_FWD,result1);
	WriteRAM(pProslic,pProslic->channel,PROSLIC_RAM_CMDAC_REV,result2);
	WriteRAM(pProslic,pProslic->channel,PROSLIC_RAM_CAL_TRNRD_DACT,result3);
	WriteRAM(pProslic,pProslic->channel,PROSLIC_RAM_CAL_TRNRD_DACR,result4);
#ifdef API_TEST
    ramVal = ReadRAM(pProslic,pProslic->channel,PROSLIC_RAM_CMDAC_FWD);
    LOGPRINT ("%s UNPACKED CMDAC_FWD = %08x\n", LOGPRINT_PREFIX, ramVal);
    ramVal = ReadRAM(pProslic, pProslic->channel,PROSLIC_RAM_CMDAC_REV);
    LOGPRINT ("%s UNPACKED CMDAC_REF = %08x\n", LOGPRINT_PREFIX, ramVal);
    ramVal = ReadRAM(pProslic,pProslic->channel,PROSLIC_RAM_CAL_TRNRD_DACT);
    LOGPRINT ("%s UNPACKED CAL_TRNRD_DACT = %08x\n", LOGPRINT_PREFIX, ramVal);
    ramVal = ReadRAM(pProslic,pProslic->channel,PROSLIC_RAM_CAL_TRNRD_DACR);
    LOGPRINT ("%s UNPACKED CAL_TRNRD_DACR = %08x\n", LOGPRINT_PREFIX, ramVal);
#endif
   return RC_NONE;
  }
#endif
  return RC_COMPLETE_NO_ERR;
}

int ProSLIC_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI324X) || defined(SI322X) || defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
  {
    ramData ramVal[4];
    ramVal[0] = (*result&0xff000000L)>>6;
    ramVal[1] = (*result&0x00ff0000L)<<1;
    ramVal[2] = (*result&0x0000ff00L)<<5;
    ramVal[3] = (*result&0x000000ffL)<<13;
    
    return ProSLIC_LoadPreviousLBCal(pProslic, ramVal[0], ramVal[1], ramVal[2], ramVal[3]);
  }
#else
	return RC_COMPLETE_NO_ERR;
#endif
}

int ProSLIC_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val,int32 preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetDCFeedVopen(pProslic,v_vlim_val,preset);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
		return Si3218x_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
		return Si3228x_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
		return Si3218x_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
		return Si3228x_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetDCFeedIloop(pProslic,i_ilim_val,preset);
#endif
	return RC_COMPLETE_NO_ERR;
}


int ProSLIC_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetRinging (pProslic,ringCfg,preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetRinging (pProslic,ringCfg,preset);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
		return Si3218x_dbgSetRinging (pProslic,ringCfg,preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetRinging (pProslic,ringCfg,preset);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
		return Si3228x_dbgSetRinging (pProslic,ringCfg,preset);
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
		return Si3218x_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
		return Si3228x_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_dbgSetRXGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
	return RC_COMPLETE_NO_ERR;
}

int ProSLIC_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;

#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
		return Si3218x_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
		return Si3228x_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_dbgSetTXGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
	return RC_COMPLETE_NO_ERR;
}


/*
** Function: ProSLIC_LineMonitor
**
** Description: 
** Generic monitoring function
**
** Returns:
** 0
*/
int ProSLIC_LineMonitor (proslicChanType *pProslic, proslicMonitorType *monitor){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_LineMonitor (pProslic,monitor);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_LineMonitor (pProslic,monitor);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_LineMonitor (pProslic, monitor);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
		return Si3218x_LineMonitor (pProslic, monitor);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_LineMonitor (pProslic, monitor);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
		return Si3228x_LineMonitor (pProslic, monitor);
#endif
	return RC_COMPLETE_NO_ERR;
}

/*
** Function: ProSLIC_PSTNCheck
**
** Description: 
** Monitor for excessive longitudinal current, which
** would be present if a live pstn line was connected
** to the port.
**
** Returns:
** 0 - no pstn detected
** 1 - pstn detected
*/
int ProSLIC_PSTNCheck (proslicChanType *pProslic, proslicPSTNCheckObjType *pPSTNCheck){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
	    return Si3226_PSTNCheck (pProslic,pPSTNCheck);
#endif

#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_PSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
		return Si3218x_PSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_PSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
		return Si3228x_PSTNCheck (pProslic,pPSTNCheck);
#endif
    return RC_COMPLETE_NO_ERR;
}


/*
** Function: ProSLIC_DiffPSTNCheck
**
** Description: 
** Monitor for excessive longitudinal current, which
** would be present if a live pstn line was connected
** to the port.
**
** Returns:
** RC_NONE - test in progress
** RC_IGNORE - non-ProSLIC channel
** RC_COMPLETE_NO_ERR - ProSLIC does not support feature.
** RC_PSTN_OPEN_FEMF | RC_COMPLETE_NO_ERR - test completed.
** 
*/
#ifdef PSTN_DET_ENABLE
int ProSLIC_DiffPSTNCheck (proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pPSTNCheck){
	if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
	    return Si3217x_DiffPSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3218X
	if (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
	    return Si3218x_DiffPSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
	    return Si3226x_DiffPSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3228X
	if (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
	    return Si3228x_DiffPSTNCheck (pProslic,pPSTNCheck);
#endif
    return RC_COMPLETE_NO_ERR;
}
#endif

/*
** Function: ProSLIC_CreatePSTNCheckObj
**
** Description: 
** Allocate memory for pstnCheckObj
**
** Returns:
** RC_NONE
** RC_UNSUPPORTED_FEATER if malloc disabled
*/
int ProSLIC_CreatePSTNCheckObj(proslicPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
  	*pstnCheckObj = SIVOICE_CALLOC(sizeof(proslicPSTNCheckObjType),1);
    return RC_NONE;
#else
	return RC_UNSUPPORTED_FEATURE;
#endif
}

/*
** Function: ProSLIC_CreateDiffPSTNCheckObj
**
** Description: 
** Allocate memory for proslicDiffPstnCheckObj
**
** Returns:
** RC_NONE
** RC_UNSUPPORTED_FEATER if malloc disabled
*/

#ifdef PSTN_DET_ENABLE
int ProSLIC_CreateDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
  	*pstnCheckObj = SIVOICE_CALLOC(sizeof(proslicDiffPSTNCheckObjType),1);
    return RC_NONE;
#else
	return RC_UNSUPPORTED_FEATURE;
#endif
}
#endif
/*
** Function: ProSLIC_DestroyPSTNCheckObj
**
** Description: 
** Free memory for pstnCheckObj
**
** Returns:
** RC_NONE
** RC_UNSUPPORTED_FEATER if malloc disabled
*/
int ProSLIC_DestroyPSTNCheckObj(proslicPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
  if(pstnCheckObj)
	  SIVOICE_FREE((proslicPSTNCheckObjType_ptr)*pstnCheckObj);
  return RC_NONE;
#else
	return RC_UNSUPPORTED_FEATURE;
#endif
}


/*
** Function: ProSLIC_DestroyDiffPSTNCheckObj
**
** Description: 
** Free memory for pstnDiffCheckObj
**
** Returns:
** RC_NONE
** RC_UNSUPPORTED_FEATER if malloc disabled
*/
#ifdef PSTN_DET_ENABLE
int ProSLIC_DestroyDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
    if(pstnCheckObj)
	    SIVOICE_FREE((proslicDiffPSTNCheckObjType_ptr)*pstnCheckObj);
    return RC_NONE;
#else
	return RC_UNSUPPORTED_FEATURE;
#endif
}
#endif

/*
** Function: ProSLIC_InitPSTNCheckObj
**
** Description: 
** Initialize pstnCheckObj structure members
**
** Returns:
** RC_NONE
*/
int ProSLIC_InitPSTNCheckObj(proslicPSTNCheckObjType_ptr pstnCheckObj, int32 avgThresh, int32 singleThresh, uInt8 samples)
{
    pstnCheckObj->avgThresh = avgThresh;
    pstnCheckObj->singleThresh = singleThresh;
    pstnCheckObj->samples = samples;
    pstnCheckObj->avgIlong = 0;
    pstnCheckObj->count = 0;
    pstnCheckObj->buffFull = 0;
    
    return RC_NONE;
}

/*
** Function: ProSLIC_InitDiffPSTNCheckObj
**
** Description: 
** Initialize pstnCheckObj structure members
**
** Returns:
** RC_NONE
*/
#ifdef PSTN_DET_ENABLE
int ProSLIC_InitDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr pstnDiffCheckObj, 
                                int preset1,
                                int preset2,
                                int entry_preset,
                                int femf_enable)
{
    pstnDiffCheckObj->samples = PSTN_DET_DIFF_SAMPLES;
    pstnDiffCheckObj->max_femf_vopen = PSTN_DET_MAX_FEMF;
    pstnDiffCheckObj->entryDCFeedPreset = entry_preset;
    pstnDiffCheckObj->dcfPreset1 = preset1;
    pstnDiffCheckObj->dcfPreset2 = preset2;
    pstnDiffCheckObj->femf_enable = femf_enable;
    pstnDiffCheckObj->pState.stage = 0;
    pstnDiffCheckObj->pState.sampleIterations = 0;
    pstnDiffCheckObj->pState.waitIterations = 0;
    
    return RC_NONE;
}
#endif


/*
** Function: ProSLIC_SetPwrsaveMode
**
** Description: 
** Enable or disable powersave mode
**
** Returns:
** RC_NONE
*/
#define PROSLIC_REG_ENHANCE 47

int ProSLIC_SetPowersaveMode (proslicChanType *pProslic, int pwrsave){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#if defined(SI3217X) || defined(SI3218X) || defined (SI3226X) || defined (SI3228X)
    if(0
#ifdef SI3217X
	  || (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#endif
#ifdef SI3218X
	  || (pProslic->deviceId->chipType >= SI32180 && pProslic->deviceId->chipType <= SI32189)
#endif
#ifdef SI3226X
	  || (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#endif
#ifdef SI3228X
	  || (pProslic->deviceId->chipType >= SI32280 && pProslic->deviceId->chipType <= SI32289)
#endif
    )
    {
      uInt8 regData;
      regData = ReadReg(pProslic,pProslic->channel, PROSLIC_REG_ENHANCE);

      if(pwrsave == PWRSAVE_DISABLE)  
      {
          regData &= 0x07;
      }
      else
      {
          regData |= 0x10;
      }

      return WriteReg(pProslic,pProslic->channel, PROSLIC_REG_ENHANCE, regData);
    }
#endif
    return RC_COMPLETE_NO_ERR;
}

/*
** Function: ProSLIC_SetDAAEnable
**
** Description: 
** Enable or disable adjacent FXO (Si32178 only)
**
** Returns:
** RC_NONE
*/
int ProSLIC_SetDAAEnable (proslicChanType *pProslic, int enable){
    if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
    return Si3217x_SetDAAEnable (pProslic,enable);
#else
    SILABS_UNREFERENCED_PARAMETER(enable);
    return RC_NONE;
#endif
}

/*
** Function: ProSLIC_SetUserMode
**
** Decription: Enables User Access Mode (UAM) if the part supports it.
**
** Returns:
** RC_NONE if successful.
*/
#define PROSLIC_REG_USERMODE_ENABLE 126

int ProSLIC_SetUserMode(proslicChanType *pProslic,BOOLEAN isEnabled, BOOLEAN isBcast)
{
    uInt8 data;
    uInt8 channel;


	if(isEnabled == FALSE)
		return RC_NONE;

  if( (pProslic->channelType != PROSLIC) ||
	  (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M) )
    {
      return RC_UNSUPPORTED_FEATURE;
    }

    if(isBcast == TRUE)
    {
      channel = PROSLIC_CHAN_BROADCAST;
    }
    else
    {
      channel = pProslic->channel;
    }

    data = ReadReg(pProslic,channel,PROSLIC_REG_USERMODE_ENABLE); /*we check first channel. we assume all channels same user mode state for broadcast */

    if (((data&1) != 0) == isEnabled)
        return RC_NONE;

    WriteReg(pProslic,channel,PROSLIC_REG_USERMODE_ENABLE,2);
    WriteReg(pProslic,channel,PROSLIC_REG_USERMODE_ENABLE,8);
    WriteReg(pProslic,channel,PROSLIC_REG_USERMODE_ENABLE,0xe);
    return WriteReg(pProslic,channel,PROSLIC_REG_USERMODE_ENABLE,0);

}
/*
** Function: ProSLIC_isReinitRequired
**
** Description: 
** Checks for improper ring exit
**
** Returns:
** RC_NONE                -  Reinit not required
** RC_REINIT_REQUIRED     -  Corrupted state machine - reinit required
**
*/

int ProSLIC_isReinitRequired(proslicChanType *pProslic)
{
	uInt8 lf;
	ramData rkdc_sum;

 if( (pProslic->channelType != PROSLIC) ||
	  (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M) )
    {
      return RC_IGNORE;
    }

	/* Check for improper ring exit which may cause dcfeed corruption */
	
	lf = ReadReg(pProslic, pProslic->channel,PROSLIC_REG_LINEFEED);
	rkdc_sum = ReadRAM(pProslic,pProslic->channel,PROSLIC_RAM_RDC_SUM);
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
    {
      LOGPRINT("%sisReinitRequired: Linefeed = %d\n", LOGPRINT_PREFIX, lf);
	    LOGPRINT("%sisReinitRequired: RDC_SUM = %d\n",  LOGPRINT_PREFIX, (int)rkdc_sum);
	}
#endif
	if((rkdc_sum & 0x400000)&&(!(lf & 0x44)))
	{
		return RC_REINIT_REQUIRED;
	}
	else
	{
		return RC_NONE;
	}
}

/*
** Function: LoadRegTable
**
** Description: 
** Generic function to load register/RAM with predefined addr/value 
*/
int ProSLIC_LoadRegTable (proslicChanType *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int broadcast){
    uInt16 i;
    uInt8 channel;

    /* DAA doesn't have a RAM table.. skip it... */
    if( (pRamTable != 0) 
      && (pProslic->channelType != PROSLIC) )
    {
      return RC_IGNORE;
    }

    if (broadcast){
        channel = PROSLIC_CHAN_BROADCAST;
    }
    else {
        channel = pProslic->channel;
    }

    i=0; 
    if (pRamTable != 0){
        while (pRamTable[i].address != 0xffff){
            WriteRAM(pProslic,channel,pRamTable[i].address,pRamTable[i].initValue); 
            i++;
        }
    }
    i=0;
    if (pRegTable != 0){
        while (pRegTable[i].address != 0xff){
            WriteReg(pProslic,channel,pRegTable[i].address,pRegTable[i].initValue);
            i++;
        }
    }

    return RC_NONE;
}

/*
** Function: LoadRegTables
**
** Description: 
** Generic function to load register/RAM with predefined addr/value 
*/

int ProSLIC_LoadRegTables(proslicChanType_ptr *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int size)
{
    int i;
    for (i=0;i<size;i++)
    {
        if (pProslic[i]->channelEnable)
            ProSLIC_LoadRegTable(pProslic[i],pRamTable,pRegTable,FALSE);
    }
  return RC_NONE;
}

int ProSLIC_UnsupportedFeatureNoArg(proslicChanType_ptr pProslic, const char *function_name)
{
#ifdef ENABLE_DEBUG
  LOGPRINT("%s: unsupported %s was called on channel %d\n", LOGPRINT_PREFIX, function_name, pProslic->channel);
#else
  SILABS_UNREFERENCED_PARAMETER(pProslic);
  SILABS_UNREFERENCED_PARAMETER(function_name);
#endif
  return RC_UNSUPPORTED_FEATURE;
}


/*
** Function: ProSLIC_PSTN_delay_poll
**
** Description: 
** Delay function called within PSTN detection functions
**
** Return Value:
** none
*/
#ifdef PSTN_DET_ENABLE
void ProSLIC_PSTN_delay_poll(proslicTestStateType *pState, uInt16 delay)
{
uInt16 delayCount;

    if((delay/PSTN_DET_POLL_RATE) < 2)
        delayCount = 0;
    else
        delayCount = (delay/PSTN_DET_POLL_RATE) - 2;

    pState->waitIterations++;
    if((pState->waitIterations == delayCount) || (delayCount == 0))
    {
        pState->waitIterations = 0;
        pState->stage++;
    }
}
#endif
