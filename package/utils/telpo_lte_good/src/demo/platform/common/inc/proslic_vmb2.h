/******************************************************************************
 * Copyright (c) 2014 by Silicon Laboratories
 *
 * $Id: proslic_vmb2.h 4638 2014-12-18 20:14:44Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 * File Description:
 *
 * This header file defines the command structure for the VMB2 evaluation platform.
 * This file is subject to change.
 *
 */


#ifndef __SI_VOICE_VMB2_CMD_H__
#define __SI_VOICE_VMB2_CMD_H__ 1
#include "spi_pcm_vcp.h"

/******************************************************************************/
/* Firmware Debug Commands */
#define SIVOICE_VMB2_CMD_FIRMWARE_TEST_EN        0x10
#define SIVOICE_VMB2_CMD_FIRMWARE_ID             0x11

/******************************************************************************/
/* Generic SPI commands */
#define SIVOICE_VMB2_CMD_SPI_WRITE               0x20
#define SIVOICE_VMB2_CMD_SPI_READ                0x21
#define SIVOICE_VMB2_CMD_SPI_CS_SELECT           0x22
#define SIVOICE_VMB2_CMD_SPI_SCLK_SELECT         0x23
#define SIVOICE_VMB2_CMD_SPI_SCLK_FMT            0x24

/******************************************************************************/
/* Debug Commands */
#define SIVOICE_VMB2_CMD_DBG_SET_CS              0x40
#define SIVOICE_VMB2_CMD_DBG_GET_SELECTED_CS     0x41

/******************************************************************************/
/* Eval board control related commands */
#define SIVOICE_VMB2_CMD_RESET                   0x30
#define SIVOICE_VMB2_CMD_REG_READ                0x31
#define SIVOICE_VMB2_CMD_REG_WRITE               0x32
#define SIVOICE_VMB2_CMD_RAM_DIRECT_READ         0x34 // Unused
#define SIVOICE_VMB2_CMD_RAM_DIRECT_WRITE        0x35 // Unused
#define SIVOICE_VMB2_CMD_RAM_INDIRECT_READ       0x36
#define SIVOICE_VMB2_CMD_RAM_INDIRECT_WRITE      0x37
#define SIVOICE_VMB2_CMD_EEPROM_READ             0x38
#define SIVOICE_VMB2_CMD_EEPROM_WRITE            0x39
#define SIVOICE_VMB2_CMD_RAM_READ                SIVOICE_VMB2_CMD_RAM_INDIRECT_READ
#define SIVOICE_VMB2_CMD_RAM_WRITE               SIVOICE_VMB2_CMD_RAM_INDIRECT_WRITE

/******************************************************************************/
/* Si321x - specific Commands */
#define SIVOICE_VMB2_CMD_DREG_READ_SI321X        0x50
#define SIVOICE_VMB2_CMD_DREG_WRITE_SI321X       0x51
#define SIVOICE_VMB2_CMD_IREG_READ_SI321X        0x52
#define SIVOICE_VMB2_CMD_IREG_WRITE_SI321X       0x53  

/******************************************************************************/
/* PCM configuration related commands */
#define SIVOICE_VMB2_CMD_PCM_RESET               0x60
#define SIVOICE_VMB2_CMD_PCM_OUTPUT_ENABLE       0x61
#define SIVOICE_VMB2_CMD_PCM_PCLK_SELECT         0x62
#define SIVOICE_VMB2_CMD_PCM_FSYNC_SELECT        0x63
#define SIVOICE_VMB2_CMD_PCM_TX_CHANNEL_SELECT   0x64
#define SIVOICE_VMB2_CMD_PCM_RX_CHANNEL_SELECT   0x65
#define SIVOICE_VMB2_CMD_PCM_READ_OE             0x66

/******************************************************************************/
/* PCM Audio PLD access related commands */
#define SIVOICE_VMB2_CMD_PCM_TX_FREE_BYTES       0x70 // Number of bytes free for TX
#define SIVOICE_VMB2_CMD_PCM_RX_AVAILABLE_BYTES  0x71 // Number of bytes available for RX
#define SIVOICE_VMB2_CMD_PCM_STATUS              0x72 // Overrun, underrun status of PCM data
#define SIVOICE_VMB2_CMD_PCM_SET_ENABLE          0x73 // Enable PCM data clocking
#define SIVOICE_VMB2_CMD_PCM_GET_ENABLE          0x74 // Get the status of PCM data clocking (enabled or not)
#define SIVOICE_VMB2_CMD_PCM_BUFFER_CLEAR        0x75 // Clear the PCM circular buffer

/******************************************************************************/
/* Common termination values */
#define SIVOICE_VMB2_CMD_TERM_BITMASK            0xC0     
#define SIVOICE_VMB2_CMD_TERMINATOR              0x40    // Bits 7:6 = 01 - OR'd with CID
#define SIVOICE_VMB2_CMD_PROCESSED               0x80    // Bits 7:6 = 10 
#define SIVOICE_VMB2_CMD_TERM_ERROR              0x33    
#define SIVOICE_VMB2_CMD_EEPROM_WRITE_ERR        0x07

/* PCM clock rate parameter */
typedef enum
{
	SIVOICE_VMB2_PCM_8192,
	SIVOICE_VMB2_PCM_4096,
	SIVOICE_VMB2_PCM_2048,
	SIVOICE_VMB2_PCM_1024,
	SIVOICE_VMB2_PCM_512,
	SIVOICE_VMB2_PCM_1536,
	SIVOICE_VMB2_PCM_768,
	SIVOICE_VMB2_PCM_1544
} SIVOICE_VMB2_PCM_t;


/* SCLK Freq Selects */
#define SIVOICE_VMB2_SCLK_12000                 1
#define SIVOICE_VMB2_SCLK_8000                  2
#define SIVOICE_VMB2_SCLK_4000                  5
#define SIVOICE_VMB2_SCLK_2000                  11
#define SIVOICE_VMB2_SCLK_1000                  23

/* SCLK format settings */
#define     SIVOICE_VMB2_CLK_PHA_0_POL_0        0x00
#define     SIVOICE_VMB2_CLK_PHA_0_POL_1        0x01
#define     SIVOICE_VMB2_CLK_PHA_1_POL_0        0x02
#define     SIVOICE_VMB2_CLK_PHA_1_POL_1        0x03


/* FSYNC Type Selects */
#define SIVOICE_VMB2_FSYNC_SHORT                0x00
#define SIVOICE_VMB2_FSYNC_LONG                 0x01

/* Reset selects */
#define SIVOICE_VMB2_RESET_ASSERT               0x00
#define SIVOICE_VMB2_RESET_DEASSERT             0x01

/* PCM Settings */
#define SIVOICE_VMB2_PCM_SRC_EXTERNAL           0x00
#define SIVOICE_VMB2_PCM_SRC_INTERNAL           0x01

enum {
    SIVOICE_VMB2_COMMAND_BYTE,
    SIVOICE_VMB2_ADDR_HIGH_BYTE,
    SIVOICE_VMB2_ADDR_LOW_BYTE,
    SIVOICE_VMB2_DATA3_BYTE,
    SIVOICE_VMB2_DATA2_BYTE,
    SIVOICE_VMB2_DATA1_BYTE,
    SIVOICE_VMB2_DATA0_BYTE,
    SIVOICE_VMB2_TERM_CID_BYTE
};

/******************************************************************************/
/** @defgroup SIVOICE_VMB2
 *
 * Voice Motherboard 2 (VMB2) functions
 * 
 * @{
 */

/** @defgroup SIVOICE_VMB2_SPI
 *
 * VMB2 SPI Low level functions.
 *
 * @{
 */

#define SIVOICE_VMB2_BUFFER_SZ  8

/**
* @brief 
* Send a SPI write command to the VMB2 
*
* @param[in] *interfacePtr - VMB2 data interface structure.
* @param[in] buffer - a properly formated command byte array for the VMB2. 
* @retval - int error from @ref int @ref SIVOICE_RC_OK indicates no error.

* @note  The byte array is specfic the to VMB2 and customers should not call this
*        function directly.
*/

int SiVoiceVMB2_SendCmd(ctrl_S *interfacePtr, uInt8 *buffer);

/**
* @brief 
* Read the contents of some data, dependent on the byte array 
*
* @param[in] *interfacePtr - VMB2 data interface structure.
* @param[in] buffer - a properly formated command byte array for the VMB2. 
* @retval - int error from @ref int @ref SIVOICE_RC_OK indicates no error.

* @note  The byte array is specific the to VMB2 and customers should not call this
*        function directly.
*/

int SiVoiceVMB2_ReadData(ctrl_S *interfacePtr, uInt8 *buffer);

/* @} SIVOICE_VMB2_SPI */

/** @defgroup SIVOICE_VMB2_UTIL VMB2 utility functions
 *
 * Utility functions usually not needed for "core" SiVoice code
 *
 * @{
 */

int SiVoiceVMB2_Init( ctrl_S *interfacePtr );

/**
* @brief
* Shutdown the VMB2 communications and if requested, reset the EVMB.
*
* @param[in] *interfacePtr - VMB2 data interface structure.
* @retval - int error from @ref int @ref SIVOICE_RC_OK indicates no error.
*
*/

int SiVoiceVMB2_Shutdown(ctrl_S *interfacePtr, BOOLEAN resetEVB);

/** @} SIVOICE_VMB2_UTIL */

/* @} SIVOICE_VMB2 */
#endif /* __SI_VOICE_VMB2_CMD_H__ */
