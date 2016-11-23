/******************************************************************************
 * Copyright (c) 2014-2015 by Silicon Laboratories
 *
 * $Id: spi_pcm_vcp.h 4646 2015-01-08 01:22:08Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 */
#ifndef SPI_TYPE_H
#define SPI_TYPE_H

#ifdef __linux__
#include <termios.h>
#endif

#if 0 /* Legacy */
/*
** SCLK Frequency Selects 
*/
#ifndef SCLK_SEL_12000
#define SCLK_SEL_12000   1
#define SCLK_SEL_8000    2
#define SCLK_SEL_4000    5 
#define SCLK_SEL_2000    11
#define SCLK_SEL_1000    23
#endif
/*
** PCLK Frequency Selects 
*/
#ifndef PCLK_SEL_8192
#define PCLK_SEL_8192    0
#define PCLK_SEL_4096    1
#define PCLK_SEL_2048    2
#define PCLK_SEL_1024    3
#define PCLK_SEL_512     4
#define PCLK_SEL_1536    5
#define PCLK_SEL_768     6
#define PCLK_SEL_1544    7
#endif

/*
** PCM Source
*/
#define EXT_PCM_SRC      0
#define INT_PCM_SRC      1

#endif

/* General */
#define EXT_PCM_SRC       0
#define INT_PCM_SRC       1
#define FSYNC_SAME_AS_PCM 0
#define FSYNC_EXT_ONLY    1

/* VMB PCLK */
#define VMB_PCLK_8192     128
#define VMB_PCLK_4096     64
#define VMB_PCLK_2048     32
#define VMB_PCLK_1024     16
#define VMB_PCLK_512      8
#define VMB_PCLK_256      4
#define VMB_PCLK_768      12   
#define VMB_PCLK_1536     24

/* VMB2 PCLK */
#define VMB2_PCLK_8192    0
#define VMB2_PCLK_4096    1
#define VMB2_PCLK_2048    2
#define VMB2_PCLK_1024    3
#define VMB2_PCLK_512     4
#define VMB2_PCLK_1536    5
#define VMB2_PCLK_768     6
#define VMB2_PCLK_1544    7

/* VMB2 SCLK */
#define VMB2_SCLK_12000   1
#define VMB2_SCLK_8000    2
#define VMB2_SCLK_4000    5
#define VMB2_SCLK_2000    11
#define VMB2_SCLK_1000    23

/* VMB2 FSYNC */
#define VMB2_FSYNC_SHORT  0
#define VMB2_FSYNC_LONG   1


/*
** EEPROM Read
*/
unsigned char  ReadEEPROMByte(unsigned short eAddr);

/*
** EEPROM Write
*/
void WriteEEPROMByte(unsigned short eAddr, unsigned char eData);

/*
** Firmware ID
*/
unsigned int GetFirmwareID();

/*
** SPI/GCI structure
*/
typedef struct{
#ifdef __linux__
    int fd;
    struct termios old_termios;
#else /* cygwin */
    int portNum;
    unsigned long handle;
#endif
} ctrl_S;

/*
** Function: SPI_Init
**
** Description: 
** Initializes the SPI interface
*/
int SPI_Init (ctrl_S *hSpi);


/*
** Function: SPI_Close
**
** Description: 
** Shuts down the SPI interface
*/
int SPI_Close (ctrl_S *hSpi);

char *SPI_GetPortNum (ctrl_S *hSpi);

unsigned long SPI_GetVmbHandle(ctrl_S *hSpi);

/*
** Function: ctrl_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (ctrl_S *hCtrl, int status);

/*
** register read 
**
** Description: 
** Reads ProSLIC registers
*/
unsigned char ctrl_ReadRegisterWrapper (ctrl_S *hCtrl, unsigned char channel, unsigned char regAddr);

/*
** Function: ctrl_WriteRegisterWrapper
**
** Description: 
** Writes ProSLIC registers
*/
int ctrl_WriteRegisterWrapper (ctrl_S *hSpiGci, unsigned char channel, unsigned char regAddr, unsigned char data);
//int ctrl_Write2RegisterWrapper (void *hSpiGci, unsigned char channel, unsigned char regAddr0, unsigned char data0, unsigned char regAddr1, unsigned char data1);
/*
** Function: ctrl_WriteRAMWrapper
**
** Description: 
** Writes ProSLIC RAM
*/
int ctrl_WriteRAMWrapper (ctrl_S *hSpiGci, unsigned char channel, unsigned short ramAddr, ramData data);

/*
** Function: ctrl_ReadRAMWrapper
**
** Description: 
** Reads ProSLIC RAM
*/
ramData ctrl_ReadRAMWrapper  (ctrl_S *hSpiGci, unsigned char channel, unsigned short ramAddr);

int SPI_setSCLKFreq(unsigned char sclk_freq_select);

int PCM_setPCLKFreq(unsigned char pclk_freq_select);

int PCM_setFsyncType(unsigned char fsync_select);

int PCM_setSource(unsigned char pcm_internal_select);

unsigned char PCM_readSource();

void SPI_SelectCS(unsigned char cs);

unsigned short setPcmSourceExp(unsigned short internal,int freq,int extFsync);

void SPI_SelectFormat(unsigned char fmt);

#endif
