/*
**
** vmb2_vcp_wrapper.h
** VMB2 VCP driver header file
**
** Author(s): 
** cdp
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the header file for VMB2 driver 
**
** Dependancies:
** vmb2_dll.lib, vmb2_dll.dll
**
*/

/*
** PCLK Frequency Selects 
*/
#define PCLK_SEL_8192    0x00
#define PCLK_SEL_4096    0x01
#define PCLK_SEL_2048    0x02
#define PCLK_SEL_1024    0x03
#define PCLK_SEL_512     0x04
#define PCLK_SEL_1536    0x05
#define PCLK_SEL_768     0x06
#define PCLK_SEL_1544    0x07

/*
** SCLK Freqency Selects 
*/
#define SCLK_SEL_12000   1
#define SCLK_SEL_8000    2
#define SCLK_SEL_4000    5 
#define SCLK_SEL_2000    11
#define SCLK_SEL_1000    23

/*
** FSYNC Selects
*/
#define FSYNC_SEL_SHORT  0x00
#define FSYNC_SEL_LONG   0x01

/*
** PCM OE Selects
*/
#define PCM_SRC_EXTERNAL  0x00
#define PCM_SRC_INTERNAL  0x01



/*
** DLL Function: initVMB
**
** Description: 
** Initializes motherboard communication
**
** Input Parameters: 
** none
**
** Return:
** 1 - success
** 0 - failed to open port
*/
int initVMB();

/*
** DLL Function: closeVMB
**
** Description: 
** Closes VCP port
**
** Input Parameters: 
** none
**
** Return:
** 1 - success
** 0 - failed to close port
*/
int closeVMB();


/*
** DLL Function: GetVmbHandle
**
** Description: 
** Returns HANDLE to open VCP port
**
** Input Parameters: 
** none
**
** Return:
** handle
*/
unsigned long GetVmbHandle();


/*
** DLL Function: getPortNum
**
** Description: 
** Returns the COM port # assigned to the open VCP 
**
** Input Parameters: 
** none
**
** Return:
** port number
*/
int getPortNum();

/*
** DLL Function: getFirmwareID
**
** Description: 
** Returns the Firware id in the form
**  bits[7:4]   Major Revision #
**  bits[3:0]   Minor Revision #
**
** Input Parameters: 
** none
**
** Return:
** firmware revision ID
*/
unsigned int getFirmwareID();


/*
** DLL Function: spiReadReg
**
** Description: 
** Reads a single ProSLIC register
**
** Input Parameters: 
** regAddr 	Address of register to read
** channel	Channel
**
** Return:
** register contents
*/
unsigned char spiReadReg (unsigned char regAddr, unsigned char channel);

/*
** DLL Function: spiWriteReg
**
** Description: 
** Writes a single ProSLIC register
**
** Input Parameters: 
** regAddr 	Address of register to write
** regData 	Data to write to register
** channel	Channel 
**
** Return:
** none
*/
void spiWriteReg (unsigned char regAddr, unsigned char regData, unsigned char channel);
void spiWrite2Reg (unsigned char regAddr0,unsigned char regData0,unsigned char regAddr1,unsigned char regData1,unsigned char channel);
/*
** DLL Function: spiReadRAM
**
** Description: 
** Reads a single ProSLIC RAM location
**
** Input Parameters: 
** ramAddr	Address of RAM location to read
** channel	Channel
**
** Return:
** data read from RAM location
*/
unsigned long spiReadRAM(unsigned short ramAddr, unsigned char channel);

/*
** DLL Function: spiWriteRAM
**
** Description: 
** Writes a single ProSLIC RAM location
**
** Input Parameters: 
** ramAddr	Address of RAM location to write
** ramData 	Data to write to RAM location
** channel      Channel
**
** Return:
** none
*/
void spiWriteRAM(unsigned short ramAddr, unsigned long ramData, unsigned char channel);

/*
** DLL Function: spiReset0
**
** Description: 
** Sets the reset pin of the ProSLIC low
**
** Input Parameters: 
** none
**
** Return:
** none
*/
void spiReset0();

/*
** DLL Function: spiReset1
**
** Description: 
** Sets the reset pin of the ProSLIC high
**
** Input Parameters: 
** none
**
** Return:
** none
*/
void spiReset1();

/*
** DLL Function: spiSelectCS
**
** Description: 
** Selects which CS to use
**
** Input Parameters: 
** none
**
** Return:
** none
*/
void spiSelectCS(unsigned char cs);

/*
** DLL Function: spiSetSCLKFreq
**
** Description: 
** Sets the reset pin of the ProSLIC high
**
** Input Parameters: 
** none
**
** Return:
** none
*/
void spiSetSCLKFreq(unsigned char sclk_freq_select);

/*
** DLL Function: pcmSetPCLKFreq
**
** Description: 
** Select PCLK Frequency
**
** Input Parameters: 
** none
**
** Return:
** none
*/
void pcmSetPCLKFreq(unsigned char pclk_freq_select);

/*
** DLL Function: pcmSetFsyncType
**
** Description: 
** Select FSYNC pulse type
**
** Input Parameters: 
** none
**
** Return:
** none
*/
void pcmSetFsyncType(unsigned char fsync_select);

/*
** DLL Function: pcmSetSource
**
** Description: 
** Sets the PCM source (internal/external)
**
** Input Parameters: 
** 0 - External PCM
** 1 - Internal PCM
**
** Return:
** none
*/
void pcmSetSource(unsigned char pcm_internal_select);

/*
** DLL Function: pcmReadSource
**
** Description: 
** Reads the PCM source (internal/external)
**
** Input Parameters: 
** none
**
** Return:
** 0 - External PCM
** 1 - Internal PCM
*/
unsigned char pcmReadSource();


/*
** DLL Function: spiReadEEPROM
**
** Description: 
** Reads a byte from EVB's EEPROM
**
** Input Parameters: 
** eAddr 	Address of byte to read
**
** Return:
** memory contents
*/
unsigned char spiReadEEPROM (unsigned short eAddr);

/*
** DLL Function: spiWriteEEPROM
**
** Description: 
** Write a byte to EVB's EEPROM
**
** Input Parameters: 
** eAddr 	Address of byte to write
** eData    Data to write
**
** Return:
** none
*/
void spiWriteEEPROM (unsigned short eAddr,unsigned char eData);