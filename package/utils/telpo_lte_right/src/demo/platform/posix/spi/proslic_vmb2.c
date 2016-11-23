/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories
 *
 * $Id: proslic_vmb2.c 4904 2015-04-23 15:41:51Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 * File Description:
 *
 * This file implements the VMB2 common functions.
 *
 */

#include "si_voice_datatypes.h"
#include "si_voice.h"
#include "si_voice_ctrl.h"
#include "proslic_vmb2.h"
#include "sivoice_example_os.h"
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "proslic.h"

#define  SiVoiceByteMask(X) ((X) & 0xFF)
static ctrl_S *Global_interfacePtr;

#if 1
#define SILABS_SPIDEV "/dev/spidev0.1"
#define LINUX_GPIO   "/sys/class/gpio/gpio50/value" /* Which GPIO pin to use for reset? */
#define SILABS_MAX_SPI_SPEED 9000000 /* This is actually not the maximum physical speed, just a "safe" max. */
#define SILABS_SPI_RATE      40000000 /* For IOC_XFER */
#define PROSLIC_MAX_RAM_WAIT  100


/* #define SPI_TRC printf */ /* Uncomment this line and comment the one below to get SPI debug */
#define SPI_TRC(...)
#define SILABS_USE_IOC_XFER 1  /* Set this if your SPIDev implementation does not support read/write and just ioctl transfers */

#define SILABS_BITS_PER_WORD 8 /* MUST be either 8, 16 or 32 */
#define SILABS_RAMWRITE_BLOCK_MODE 1 /* If enabled, will send 24 bytes down vs. register access mode, in some systems this is more efficient */

#define SILABS_BYTE_LEN (SILABS_BITS_PER_WORD/8) /* Should be able to set this independent of BITS_PER_WORD */
#endif

#if 0

/******************************************************************************
 *
 * Reset the device
 *
 */

int ctrl_ResetWrapper (ctrl_S *interfacePtr, int inReset)
{
    uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

    SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
    s_buf[ SIVOICE_VMB2_COMMAND_BYTE ] = SIVOICE_VMB2_CMD_RESET;

    if(inReset != 0)
    {
        s_buf[ SIVOICE_VMB2_DATA0_BYTE ] = SIVOICE_VMB2_RESET_ASSERT;
    }
    else
    {
        s_buf[ SIVOICE_VMB2_DATA0_BYTE ] = SIVOICE_VMB2_RESET_DEASSERT;
    }

    return SiVoiceVMB2_SendCmd(interfacePtr, s_buf);
}

/******************************************************************************
 * Write to a direct register (8 bits)
 */

int ctrl_WriteRegisterWrapper(ctrl_S *interfacePtr, unsigned char channel,
  unsigned char regAddr, unsigned char regData)
{
    uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

    SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
    s_buf[  SIVOICE_VMB2_COMMAND_BYTE  ] = SIVOICE_VMB2_CMD_REG_WRITE;
    s_buf[  SIVOICE_VMB2_TERM_CID_BYTE ] = SiVoiceByteMask(channel); 

    s_buf[ SIVOICE_VMB2_ADDR_LOW_BYTE ] = SiVoiceByteMask(regAddr);
    s_buf[ SIVOICE_VMB2_DATA0_BYTE ]    = SiVoiceByteMask(regData);

    if ( SiVoiceVMB2_SendCmd(interfacePtr, s_buf) != RC_NONE )
    {
        return RC_SPI_FAIL;
    }

    return RC_NONE;
}

/******************************************************************************
 * Read the contents of a direct register.
 */

unsigned char ctrl_ReadRegisterWrapper(ctrl_S *interfacePtr, 
  unsigned char channel, unsigned char regAddr)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE  ] = SIVOICE_VMB2_CMD_REG_READ;
  s_buf[ SIVOICE_VMB2_TERM_CID_BYTE ] = SiVoiceByteMask(channel); 
  s_buf[ SIVOICE_VMB2_ADDR_LOW_BYTE ] = SiVoiceByteMask(regAddr);

  if ( SiVoiceVMB2_ReadData(interfacePtr, s_buf) == RC_NONE )
  {
    return s_buf[ SIVOICE_VMB2_DATA0_BYTE ];
  }
  else
  {
    return 0xFF; /* We didn't read it correctly... */
  }
}

/******************************************************************************
 * Write RAM 
 */

int ctrl_WriteRAMWrapper(ctrl_S *interfacePtr, unsigned char channel, 
  unsigned short address, ramData data)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]  = SIVOICE_VMB2_CMD_RAM_WRITE;
  s_buf[ SIVOICE_VMB2_TERM_CID_BYTE ] = SiVoiceByteMask(channel); 

  s_buf[ SIVOICE_VMB2_ADDR_HIGH_BYTE ] = SiVoiceByteMask((address)>>8);
  s_buf[ SIVOICE_VMB2_ADDR_LOW_BYTE ]  = SiVoiceByteMask((address));
  s_buf[ SIVOICE_VMB2_DATA3_BYTE ]     = SiVoiceByteMask((data)>>24);
  s_buf[ SIVOICE_VMB2_DATA2_BYTE ]     = SiVoiceByteMask((data)>>16);
  s_buf[ SIVOICE_VMB2_DATA1_BYTE ]     = SiVoiceByteMask((data)>>8);
  s_buf[ SIVOICE_VMB2_DATA0_BYTE ]     = SiVoiceByteMask(data);

  if( SiVoiceVMB2_SendCmd(interfacePtr, s_buf) != RC_NONE )
  {
    return RC_SPI_FAIL;
  }

  return RC_NONE;
}

/******************************************************************************
 * Read from RAM (indirect registers)
 */

ramData ctrl_ReadRAMWrapper(ctrl_S *interfacePtr, 
  unsigned char channel, 
  unsigned short ramAddr)

{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ] = SIVOICE_VMB2_CMD_RAM_READ;
  s_buf[ SIVOICE_VMB2_ADDR_HIGH_BYTE ] = SiVoiceByteMask( (ramAddr>>8) );
  s_buf[ SIVOICE_VMB2_ADDR_LOW_BYTE ] = SiVoiceByteMask( ramAddr );

  if( SiVoiceVMB2_ReadData(interfacePtr, s_buf) != RC_NONE )
  {
    return RC_SPI_FAIL;
  }

  return ( (s_buf[ SIVOICE_VMB2_DATA3_BYTE ] << 24)
          | (s_buf[ SIVOICE_VMB2_DATA2_BYTE ] << 16) 
          | (s_buf[ SIVOICE_VMB2_DATA1_BYTE ] << 8) 
          | (s_buf[ SIVOICE_VMB2_DATA0_BYTE ] ) );

}
#endif

/******************************************************************************
 *
 */

void SPI_SelectCS(unsigned char chipSelect)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]     = SIVOICE_VMB2_CMD_SPI_CS_SELECT;
  s_buf[ SIVOICE_VMB2_DATA0_BYTE ]  = chipSelect & 0x1;

  return SiVoiceVMB2_SendCmd( Global_interfacePtr, s_buf);
}

/******************************************************************************
 * Change the SPI clock frequency setting .
 *
 */

int SPI_setSCLKFreq(unsigned char spiMode)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]     = SIVOICE_VMB2_CMD_SPI_SCLK_SELECT;
  s_buf[ SIVOICE_VMB2_DATA0_BYTE ]  = spiMode;

  return SiVoiceVMB2_SendCmd( Global_interfacePtr, s_buf);
}

/******************************************************************************
 * Read the PCM source setting.
 *
 * Returns the PCM source setting.
 */

unsigned char PCM_readSource()
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ] = SIVOICE_VMB2_CMD_PCM_OUTPUT_ENABLE;

  SiVoiceVMB2_ReadData( Global_interfacePtr, s_buf);

  return s_buf[ SIVOICE_VMB2_DATA0_BYTE ];
}

/******************************************************************************
 * Change the PCM source setting.
 *
 */

int PCM_setSource(unsigned char pcmSelect)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]     = SIVOICE_VMB2_CMD_PCM_OUTPUT_ENABLE;
  s_buf[ SIVOICE_VMB2_DATA0_BYTE ]  = pcmSelect;

  return SiVoiceVMB2_SendCmd( Global_interfacePtr, s_buf);
}

/******************************************************************************
 * Change the PCM clock setting.
 *
 */

int PCM_setPCLKFreq(unsigned char pcmFrequency)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]    = SIVOICE_VMB2_CMD_PCM_PCLK_SELECT;
  s_buf[ SIVOICE_VMB2_DATA0_BYTE ] = pcmFrequency;

  return SiVoiceVMB2_SendCmd( Global_interfacePtr, s_buf);
}

/******************************************************************************
 * Change the PCM Framesync
 *
 */

int PCM_setFsyncType(unsigned char pcmFramesync)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]    = SIVOICE_VMB2_CMD_PCM_FSYNC_SELECT;
  s_buf[ SIVOICE_VMB2_DATA0_BYTE ] = pcmFramesync;

  return SiVoiceVMB2_SendCmd( Global_interfacePtr, s_buf);
}

/******************************************************************************
 * Retrieve the firmware ID as an int.
 *
 */

unsigned int GetFirmwareID()
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]    = SIVOICE_VMB2_CMD_FIRMWARE_ID;

  SiVoiceVMB2_ReadData( Global_interfacePtr, s_buf);

  return ((s_buf[ SIVOICE_VMB2_DATA1_BYTE ] <<8)
        | (s_buf[ SIVOICE_VMB2_DATA0_BYTE ]) );
}

/******************************************************************************
 * Configure the PCM source in 1 call.. NOT implemented in VMB2.
 *
 */

unsigned short setPcmSourceExp(unsigned short internal,int freq,int extFsync)
{
  return 0;
}

/******************************************************************************
 * Configure the SPI Clock source
 *
 */

void SPI_SelectFormat(unsigned char fmt)
{
  uInt8 s_buf[ SIVOICE_VMB2_BUFFER_SZ ];

  SIVOICE_MEMSET(s_buf, 0, SIVOICE_VMB2_BUFFER_SZ);
  s_buf[ SIVOICE_VMB2_COMMAND_BYTE ]    = SIVOICE_VMB2_CMD_SPI_SCLK_FMT;
  s_buf[ SIVOICE_VMB2_DATA0_BYTE ] = fmt;

  return SiVoiceVMB2_SendCmd( Global_interfacePtr, s_buf);

}

/******************************************************************************
 * Initialize the VMB2
 *
 */
#if 0
int SPI_Init(ctrl_S *interfacePtr)
{
  if (SiVoiceOpenIF(interfacePtr) != RC_NONE )
  {
    return FALSE;
  }
  Global_interfacePtr = interfacePtr;

  /* Place the daisychain in reset while programming the clocks */
  ctrl_ResetWrapper( interfacePtr, FALSE);
  SiVoiceMsSleep(250);

  /* Now configure the clock and sources */
  PCM_setSource( SIVOICE_VMB2_PCM_SRC_INTERNAL);
  SPI_setSCLKFreq( SIVOICE_VMB2_SCLK_2000 );
  ctrl_ResetWrapper( interfacePtr, TRUE );
  PCM_setPCLKFreq( SIVOICE_VMB2_PCM_2048 );
  SiVoiceMsSleep(500);

  /* take the daisychain out of reset */
  ctrl_ResetWrapper( interfacePtr, FALSE);
  SiVoiceMsSleep(250);

  return TRUE;

}

/******************************************************************************
 * Shutdown the VMB2
 *
 */

int SPI_Close(ctrl_S *interfacePtr)
{
  ctrl_ResetWrapper( interfacePtr, TRUE );

  return SiVoiceCloseIF( interfacePtr );

}
#endif

#if 1
/* Basic register definitions, regardless of chipset ('4x, '2x, '17x, '26x compatible */
#define PROSLIC_CW_RD    0x60
#define PROSLIC_CW_WR    0x20
#define PROSLIC_CW_BCAST 0x80
#define PROSLIC_BCAST    0xFF

/* chanNumtoCID() implemented as a macro instead of a function */
#define CNUM_TO_CID(CHAN) ( (( (CHAN)<<4)&0x10) \
                            | (( (CHAN)<<2) & 0x8) \
                            | (( (CHAN)>>2) & 0x2) \
                            | (( (CHAN)>>4) & 0x1) \
                            | ( (CHAN) & 0x4) )
#define RAM_STAT_REG          4
#define RAM_ADDR_HI           5
#define RAM_DATA_B0           6
#define RAM_DATA_B1           7
#define RAM_DATA_B2           8
#define RAM_DATA_B3           9
#define RAM_ADDR_LO           10
#define RAM_ADR_HIGH_MASK(ADDR) (((ADDR)>>3)&0xE0)

static int spi_fd = -1;
static int reset_fd = -1;

#ifdef SILABS_USE_IOC_XFER

static struct spi_ioc_transfer xfer;
#if (SILABS_BYTE_LEN == 1)
static uint8_t wrbuf, rdbuf;
#else
static uint8_t wrbuf[SILABS_BYTE_LEN], rdbuf[SILABS_BYTE_LEN];
#endif

#else
#if (SILABS_BYTE_LEN != 1)
static uint8_t xbuf[SILABS_BYTE_LEN];
#endif

#endif /* IOC_XFER */

/******************************************************************************
 *
 * Reset the device
 *
 */
#if 1
int ctrl_ResetWrapper (ctrl_S *interfacePtr, int inReset)
{
  char buf;
  SILABS_UNREFERENCED_PARAMETER(interfacePtr);

  SPI_TRC("DBG: %s: inReset = %d\n", __FUNCTION__, inReset);
  if(reset_fd >= 0)
  {
    if(inReset)
    {
      buf = '0';
    }
    else
    {
      buf = '1';
    }

    if(write(reset_fd,  &buf, 1) == 1)
    {
      return RC_NONE;
    }
  }
  return RC_SPI_FAIL;
}
#endif
/******************************************************************************
 * Figure out how fast we can go.
 */

uint32_t get_max_spi_speed()
{
  static uint32_t max_speed = 0;

  if( max_speed == 0)
  {
   if (ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &max_speed) == 0 )
    {
      if(max_speed >= SILABS_MAX_SPI_SPEED)
      {
        max_speed = SILABS_MAX_SPI_SPEED; /* Clamp it to a "safe" limit, your particular chipset may actually run faster than this... */
      }
    }
    else
    {
      max_speed = SILABS_MAX_SPI_SPEED;
    }
  }

  return max_speed;
}


/******************************************************************************
 * Configure the SPI bus speed...
 */

void set_spi_speed(uint32_t speed)
{
  SPI_TRC("DBG: %s SPICLK = %d Hz\n", __FUNCTION__, speed);
#ifdef SILABS_USE_IOC_XFER
  xfer.speed_hz = speed;
#else
  ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
#endif
}

/******************************************************************************
 * Write to a direct register (8 bits)
 */

int ctrl_WriteRegisterWrapper(ctrl_S *interfacePtr, uInt8 channel,
                              uInt8 regAddr, uInt8 regData)
{
  uint8_t controlWord;
  SILABS_UNREFERENCED_PARAMETER(interfacePtr);

  if( channel == PROSLIC_BCAST )
  {
    controlWord = PROSLIC_CW_BCAST;
  }
  else
  {
    controlWord = CNUM_TO_CID(channel);
  }

  controlWord |= PROSLIC_CW_WR;
  SPI_TRC("DBG: %s: cw = 0x%02X ra = 0x%02X data = 0x%02X\n", __FUNCTION__,
          controlWord, regAddr, regData);

#ifdef SILABS_USE_IOC_XFER
  xfer.rx_buf = NULL;
  xfer.tx_buf = &wrbuf;
#if (SILABS_BYTE_LEN == 1)
  wrbuf = controlWord;
  /* NOTE: in theory, we could of done 3 SPI_IOC_MESSAGES as 1 call... */
  ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer);
  wrbuf = regAddr;
  ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer);
  wrbuf = regData;
  if(ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer)>=0)
  {
    return RC_NONE;
  }
  else
  {
    return RC_SPI_FAIL;
  }
#else /* 2 or 4 byte transfer */
  xfer.len = SILABS_BYTE_LEN;
  /* On system tested, we had to do a byte swap */
  wrbuf[0] = regAddr;
  wrbuf[1] = controlWord;

#if (SILABS_BYTE_LEN == 2)
  ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer);
  wrbuf[0] = wrbuf[1] = regData;
#else /* 4 byte  */
  wrbuf[2] = wrbuf[3] = regData;
#endif /* 4 byte */

#endif /*  2 or 4*/
  if(ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer) >= 0)
  {
    return RC_NONE;
  }
  else
  {
    return RC_SPI_FAIL;
  }

#else /* Non-IOCTL transfer */

#if (SILABS_BYTE_LEN == 1)
  /* Send 1 byte at a time since we need a CS deassert between bytes */
  write(spi_fd, &controlWord, 1);
  write(spi_fd, &regAddr, 1);

  /* In theory, we should check all access... */
  if(write(spi_fd, &regData, 1) == 1)
#else
#if (SILABS_BYTE_LEN == 4)
  xbuf[3] = controlWord;
  xbuf[2] = regAddr;
  xbuf[0] = xbuf[1] = regData;

  /* Need to adjust bits per word if we do a write vs. a read */
  controlWord = SILABS_BITS_PER_WORD;
  ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &controlWord);
#else
  xbuf[0] = regAddr;
  xbuf[1] = controlWord;

  write(spi_fd, xbuf, 2);
  xbuf[0] = xbuf[1] = regData;
#endif

  if(write(spi_fd, xbuf, SILABS_BYTE_LEN) == SILABS_BYTE_LEN)
#endif /* multibyte transfer */
  {
    return RC_NONE;
  }
  else
  {
    perror("SPI WR Failed");
    return RC_SPI_FAIL;
  }
#endif /* read/write transfer vs. IOC. */
}

/******************************************************************************
 * Read the contents of a direct register.
 */

unsigned char ctrl_ReadRegisterWrapper(ctrl_S *interfacePtr,
                                       uInt8 channel, uInt8 regAddr)
{
  uint8_t controlWord;
  uint8_t data;
#ifdef SILABS_USE_IOC_XFER
  int rc;
#endif

  SILABS_UNREFERENCED_PARAMETER(interfacePtr);

  controlWord = CNUM_TO_CID(channel);

  controlWord |= PROSLIC_CW_RD;
  SPI_TRC("DBG: %s: cw = 0x%02x ra = %02x\n", __FUNCTION__, controlWord, regAddr);

#ifdef SILABS_USE_IOC_XFER
  xfer.rx_buf = NULL;
#if (SILABS_BYTE_LEN == 1)
  xfer.tx_buf = &wrbuf;
  rdbuf = 0xFF;
  wrbuf = controlWord;
#else
  xfer.tx_buf = wrbuf;
  *rdbuf = 0xFF;
  wrbuf[1] = controlWord; /* On the tested system, we had to send the bytes in reverse order */
  wrbuf[0] = regAddr;
  xfer.len = 2;
  xfer.bits_per_word = 16;
#endif
  rc = ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer);
  if(rc < 0)
  {
    perror("SPI RD");
    return RC_SPI_FAIL;
  }

#if (SILABS_BYTE_LEN == 1)
  wrbuf = regAddr;
  ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer);
  xfer.rx_buf = &rdbuf;
#else
  xfer.rx_buf = rdbuf;
#endif
  xfer.tx_buf = NULL;
  if(ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer) >= 0)
  {
#if (SILABS_BYTE_LEN == 1)
    SPI_TRC("DBG: %s: cw = 0x%02x ra = %02d data = 0x%02X\n", __FUNCTION__,
            controlWord, regAddr,rdbuf);
    return rdbuf;
#else
    SPI_TRC("DBG: %s: cw = 0x%02x ra = %02d data = 0x%02X 0x%02X\n", __FUNCTION__,
            controlWord, regAddr,*rdbuf, rdbuf[1]);
    return *rdbuf;
#endif
  }
  else
  {
    perror("SPI RD");
    return RC_SPI_FAIL;
  }
#else /* Regular read/write */
#if (SILABS_BYTE_LEN == 1)
  /* Send 1 byte at a time since we need a CS deassert between bytes */

  write(spi_fd, &controlWord, 1);
  write(spi_fd, &regAddr, 1);
  data = 0xFF;

  /* In theory, we should check all access... */
  if(read(spi_fd, &data, 1) == 1)
  {
    SPI_TRC("DBG: %s: cw = %02x ra = %02d data = 0x%02X\n", __FUNCTION__,
            controlWord, regAddr,data);
    return data;
  }
#else /* Multibyte */
  xbuf[0] = regAddr;
  xbuf[1] = controlWord;

  /* Need to adjust bits per word if we do a write vs. a read */
  controlWord = 16;
  ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &controlWord);

  write(spi_fd, xbuf, 2);
  *xbuf = 0xFF;
  if(read(spi_fd, xbuf, 2) == 2)
  {
    SPI_TRC("DBG: %s: cw = %02x ra = %02d data = 0x%02X\n", __FUNCTION__,
            controlWord, regAddr,*xbuf);
    return *xbuf;
  }
#endif
  else
  {
    perror("SPI RD1 Failed");
    return 0xFF; /* RC_SPI_FAIL is the alternative return code */
  }
#endif
}

/******************************************************************************
 * Wait for RAM access.
 */

static int wait_ram(ctrl_S *interfacePtr, unsigned char channel)
{
  uint32_t timeout = PROSLIC_MAX_RAM_WAIT;

  while( (ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_STAT_REG) & 0x01)
         && timeout)
  {
    timeout--;
  }

  if(timeout)
  {
    return RC_NONE;
  }
  else
  {
    return RC_SPI_FAIL;
  }
}

/******************************************************************************
 * Write RAM
 */

int ctrl_WriteRAMWrapper(ctrl_S *interfacePtr, uInt8 channel,
                         uInt16 ramAddr, ramData data)
{
  ramData myData = data;
  if(wait_ram(interfacePtr,channel) != RC_NONE)
  {
    return RC_SPI_FAIL;
  }

  SPI_TRC("DBG: %s: ramloc = %0d data = 0x%04X\n", __FUNCTION__,
          ramAddr, myData);
#ifdef SILABS_RAM_BLOCK_ACCESS
  {
    uInt8 ramWriteData[6*4]; /* This encapsulates the 6 reg writes into 1 block */
    const uInt8 regAddr[6] = {RAM_ADDR_HI, RAM_DATA_B0, RAM_DATA_B1, RAM_DATA_B2, RAM_DATA_B3, RAM_ADDR_LO};
    int i;
    uInt8 scratch;

    /* Setup control word & registers for ALL the reg access */
    scratch = CNUM_TO_CID(channel) | PROSLIC_CW_WR;

    for(i = 0; i < 6; i++)
    {
      ramWriteData[i<<2]     = regAddr[i];
      ramWriteData[(i<<2)+1] = scratch
                               ; /* On system tested, we had to do a swap of CW + Reg addr */
    }

    ramWriteData[2] = ramWriteData[3] = RAM_ADR_HIGH_MASK(ramAddr);

    ramWriteData[6] = ramWriteData[7] = (uInt8)(myData<<3);
    myData = myData >> 5;

    ramWriteData[10] = ramWriteData[11] = (uInt8)(myData & 0xFF);
    myData = myData >> 8;

    ramWriteData[14] = ramWriteData[15] = (uInt8)(myData & 0xFF);
    myData = myData >> 8;

    ramWriteData[18] = ramWriteData[19] = (uInt8)(myData & 0xFF);

    ramWriteData[22] = ramWriteData[23] = (uInt8)(ramAddr& 0xFF);

    if( write(spi_fd, ramWriteData, 24) == 24)
    {
      return RC_NONE;
    }
    else
    {
      perror("SPI WR Failed");
      return RC_SPI_FAIL;
    }
  }
#else
  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_ADDR_HI,
                            RAM_ADR_HIGH_MASK(ramAddr));

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_DATA_B0,
                            ((unsigned char)(myData<<3)));

  myData = myData >> 5;

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_DATA_B1,
                            ((unsigned char)(myData & 0xFF)));

  myData = myData >> 8;

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_DATA_B2,
                            ((unsigned char)(myData & 0xFF)));

  myData = myData >> 8;

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_DATA_B3,
                            ((unsigned char)(myData & 0xFF)));

  return(ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_ADDR_LO,
                                   (unsigned char)(ramAddr & 0xFF)));
#endif
}

/******************************************************************************
 * Read from RAM (indirect registers)
 */

ramData ctrl_ReadRAMWrapper(ctrl_S *interfacePtr,
                            uInt8 channel,
                            uInt16 ramAddr)

{
  ramData data;

  if(wait_ram(interfacePtr,channel) != RC_NONE)
  {
    return RC_SPI_FAIL;
  }

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_ADDR_HI,
                            RAM_ADR_HIGH_MASK(ramAddr));

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_ADDR_LO,
                            (unsigned char)(ramAddr&0xFF));

  if(wait_ram(interfacePtr,channel) != RC_NONE)
  {
    return RC_SPI_FAIL;
  }

  data = ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B3);
  data = data << 8;
  data |= ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B2);
  data = data << 8;
  data |= ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B1);
  data = data << 8;
  data |= ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B0);

  data = data >>3;

 // SPI_TRC("DBG: %s: ramloc = %0d data = 0x%04X\n", __FUNCTION__,  ramAddr, data);
  return data;
}

/******************************************************************************
 * Initialize the spidev interface
 *
 */

int SPI_Init(ctrl_S *interfacePtr)
{

  uint8_t bits = SILABS_BITS_PER_WORD;
  uint32_t mode =  SPI_MODE_3;
  uint32_t speed = SILABS_SPI_RATE;
  reset_fd = spi_fd = -1;

  if( (spi_fd =  open(SILABS_SPIDEV, O_RDWR)) < 0 )
  {
    perror("Failed to open SPI device");
    abort();
  }
  /* TODO: hook up IOC_XFER to menu */
#ifdef SILABS_USE_IOC_XFER
//  SPI_TRC("DBG: %s: using IOC transfers with bytelen = %d\n", __FUNCTION__, SILABS_BYTE_LEN);
  xfer.len = SILABS_BYTE_LEN;
  xfer.speed_hz = speed;
  xfer.delay_usecs = 0;
  xfer.bits_per_word = SILABS_BITS_PER_WORD;
  xfer.cs_change = 1; /* Must deassert CS for next transfer */
#else
  //SPI_TRC("DBG: %s: using read/write transfers with bytelen = %d\n", __FUNCTION__, SILABS_BYTE_LEN);
  ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
#endif
  ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);

 // interfacePtr->spi_fd = spi_fd;
 // interfacePtr->reset_fd = reset_fd;
  interfacePtr->fd = spi_fd;
  return TRUE;
}

/******************************************************************************
 * Shutdown the spidev interface
 *
 */

int SPI_Close(ctrl_S *interfacePtr)
{
  SILABS_UNREFERENCED_PARAMETER(interfacePtr);

  if(spi_fd >= 0)
  {
    close(spi_fd);
    spi_fd = -1;
  }

  return TRUE;
}

#endif


