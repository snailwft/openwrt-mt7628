/*
** Copyright 2014-2015, Silicon Labs
** spi_pcm_vcp.c
** 
** SPI and PCM driver implementation file
**
** Author(s): 
** CDP
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the implementation file for the SPI/PCM driver used 
** in the ProSLIC demonstration code. It calls the library
** that initializes and talks to the voice motherboard.
**
** Dependancies:
** 
**
*/
#include "vmb2_vcp_wrapper.h" // This is the motherboard interface library
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "spi_pcm_vcp.h"


/*
** EEPROM Read
*/
unsigned char  ReadEEPROMByte(unsigned short eAddr)
{
    return spiReadEEPROM(eAddr);
}

/*
** EEPROM Write
*/
void WriteEEPROMByte(unsigned short eAddr, unsigned char eData)
{
    spiWriteEEPROM(eAddr,eData);
}

unsigned int GetFirmwareID()
{
    return getFirmwareID();
}


/*
** Function: SPI_Init
**
** Description: 
** Initializes the SPI interface by opening the USB VCP
**
** Input Parameters: 
** none
**
** Return:
** 1 - Success
** 0 - Failed to open port
**
*/
int SPI_Init (ctrl_S *hSpi)
{
	uInt8 result;
	result = initVMB(); 
	/* Ensure RESET is high */
    spiReset1();
	return result;
}

/*
** Function: SPI_Close
**
** Description: 
** Closes VCP
**
** Input Parameters: 
** none
**
** Return:
** 1 - Success
** 0 - Failed to close port
**
*/
int SPI_Close (ctrl_S *hSpi)
{
	uInt8 result;
	result = closeVMB();  
	return result;
}

/*
** Function: SPI_GetPortNum
**
** Description: 
** Returns VCP port number (for debug purposes)
**
** Input Parameters: 
** none
**
** Return:
** port number
**
*/
char *SPI_GetPortNum (ctrl_S *hSpi)
{
  static char buf[6];
  sprintf(buf, "COM%d",getPortNum());
	return buf;
}

/*
** Function: SPI_GetVmbHandle
**
** Description: 
** Returns VCP port file handle (for debug purposes)
**
** Input Parameters: 
** none
**
** Return:
** handle (void *)
**
*/
unsigned long SPI_GetVmbHandle(ctrl_S *hSpi)
{
    return GetVmbHandle();
}


/*
** Function: ctrl_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (ctrl_S *hctrl, int status)
{
	if (status)
		spiReset0();
	else
		spiReset1();
	return 0;
}

/*
** SPI/GCI register read 
**
** Description: 
** Reads a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** regAddr: Address of register to read
** return data: data to read from register
**
** Return:
** none
*/
uInt8 ctrl_ReadRegisterWrapper (ctrl_S * hctrl, uInt8 channel, uInt8 regAddr)
{
    return spiReadReg(regAddr,channel);
}


/*
** Function: ctrl_WriteRegisterWrapper 
**
** Description: 
** Writes a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of register to write
** data: data to write to register
**
** Return:
** none
*/
int ctrl_WriteRegisterWrapper (ctrl_S * hctrl, uInt8 channel, uInt8 regAddr, uInt8 data){
    spiWriteReg(regAddr,data,channel);
    return 0;
}
/*
** Function: SPI_ReadRAMWrapper
**
** Description: 
** Reads a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** address: Address of RAM location to read
** pData: data to read from RAM location
**
** Return:
** none
*/
ramData ctrl_ReadRAMWrapper (ctrl_S * hctrl, uInt8 channel, uInt16 ramAddr)
{
    return spiReadRAM(ramAddr,channel); 
}


/*
** Function: SPI_WriteRAMWrapper
**
** Description: 
** Writes a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of RAM location to write
** data: data to write to RAM location
**
** Return:
** none
*/
int ctrl_WriteRAMWrapper (ctrl_S * hctrl, uInt8 channel, uInt16 ramAddr, ramData data)
{
    spiWriteRAM(ramAddr,data,channel);
    return 0;
}

/*
** Function: SPI_setSCLKFreq
**
** Description: 
** Select VMB2 SCLK Freq
**
** Input Parameters: 
** freq_select 
**
** Return:
** none
*/
int SPI_setSCLKFreq(unsigned char sclk_freq_select)
{
    switch(sclk_freq_select)
    {
    case SCLK_SEL_12000:
    case SCLK_SEL_8000:
    case SCLK_SEL_4000:
    case SCLK_SEL_2000:
    case SCLK_SEL_1000:
        spiSetSCLKFreq(sclk_freq_select);
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

int PCM_setPCLKFreq(unsigned char pclk_freq_select)
{
    switch(pclk_freq_select)
    {
    case PCLK_SEL_8192:
    case PCLK_SEL_4096:
    case PCLK_SEL_2048:
    case PCLK_SEL_1024:
    case PCLK_SEL_512:
    case PCLK_SEL_1536:
    case PCLK_SEL_768:
    case PCLK_SEL_1544:
        pcmSetPCLKFreq(pclk_freq_select);
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

int PCM_setFsyncType(unsigned char fsync_select)
{
    switch(fsync_select)
    {
    case FSYNC_SEL_SHORT:
    case FSYNC_SEL_LONG:
        pcmSetFsyncType(fsync_select);
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

int PCM_setSource(unsigned char pcm_internal_select)
{
    switch(pcm_internal_select)
    {
    case PCM_SRC_EXTERNAL:
    case PCM_SRC_INTERNAL:
        pcmSetSource(pcm_internal_select);
        return 1;
        break;
    default:
        return 0;
        break;
    }
}


/*
** Wrapper for setPcmSourceExp()
**
*/
unsigned short setPcmSourceExp(unsigned short internal,int freq,int extFsync)
{
	return 0;
}

unsigned char PCM_readSource()
{
    return pcmReadSource();
}

void SPI_SelectCS(unsigned char cs)
{
	spiSelectCS(cs);
}

void SPI_SelectFormat(unsigned char fmt)
{
  spiSelectClockFormat(fmt);
}


