#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "spidev.h"

typedef  unsigned char uInt8;
typedef  unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uInt16;
#define ramData uint32_t
#define SPI_TRC printf

#define SILABS_MAX_SPI_SPEED 9000000
#define SILABS_BITS_PER_WORD 8
#define SILABS_SPI_RATE      1000000
#define SILABS_SPIDEV "/dev/spidev0.1"
#define PROSLIC_MAX_RAM_WAIT  100


#define SILABS_UNREFERENCED_PARAMETER(param) (void)param
//#define SPI_TRC(...)

typedef struct{
    int fd;
    //struct termios old_termios;
} ctrl_S;

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

#ifndef SILABS_USE_IOC_XFER
#define SILABS_USE_IOC_XFER
#define SILABS_BYTE_LEN 1
#endif

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
      return 0;
    }
  }
  return 1;
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
    return 0;
  }
  else
  {
    return 1;
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
    return 0;
  }
  else
  {
    return 1;
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
    return 0;
  }
  else
  {
    perror("SPI WR Failed");
    return 1;
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
	 printf("DBG: %s: cw = 0x%02x ra = %02x\n", __FUNCTION__, controlWord, regAddr);

#ifdef SILABS_USE_IOC_XFER
  xfer.rx_buf = NULL;
#if (SILABS_BYTE_LEN == 1)
  xfer.tx_buf = &wrbuf;
  rdbuf = 0xFF;
  wrbuf = controlWord;
#else
  xfer.tx_buf = wrbuf;
  *rdbuf = 0xFF;
  wrbuf[1] =
    controlWord; /* On the tested system, we had to send the bytes in reverse order */
  wrbuf[0] = regAddr;
  xfer.len = 2;
  xfer.bits_per_word = 16;
#endif
  rc = ioctl(spi_fd, SPI_IOC_MESSAGE(1),&xfer);
  if(rc < 0)
  {
    perror("SPI RD");
    return 1;
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
	 printf("DBG: %s: cw = 0x%02x ra = %02d data = 0x%02X\n", __FUNCTION__,
            controlWord, regAddr,rdbuf);
    return rdbuf;
#else
    SPI_TRC("DBG: %s: cw = 0x%02x ra = %02d data = 0x%02X 0x%02X\n", __FUNCTION__,
            controlWord, regAddr,*rdbuf, rdbuf[1]);
	printf("DBG: %s: cw = 0x%02x ra = %02d data = 0x%02X 0x%02X\n", __FUNCTION__,
            controlWord, regAddr,*rdbuf, rdbuf[1]);
    return *rdbuf;
#endif
  }
  else
  {
    perror("SPI RD");
    return 1;
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
	printf("DBG: %s: cw = %02x ra = %02d data = 0x%02X\n", __FUNCTION__,
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
    return 0;
  }
  else
  {
    return 1;
  }
}

/******************************************************************************
 * Write RAM
 */

int ctrl_WriteRAMWrapper(ctrl_S *interfacePtr, uInt8 channel,
                         uInt16 ramAddr, ramData data)
{
  ramData myData = data;
  if(wait_ram(interfacePtr,channel) != 0)
  {
    return 1;
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
      return 0;
    }
    else
    {
      perror("SPI WR Failed");
      return 1;
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

  if(wait_ram(interfacePtr,channel) != 0)
  {
    return 1;
  }

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_ADDR_HI,
                            RAM_ADR_HIGH_MASK(ramAddr));

  ctrl_WriteRegisterWrapper(interfacePtr, channel, RAM_ADDR_LO,
                            (unsigned char)(ramAddr&0xFF));

  if(wait_ram(interfacePtr,channel) != 0)
  {
    return 1;
  }

  data = ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B3);
  data = data << 8;
  data |= ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B2);
  data = data << 8;
  data |= ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B1);
  data = data << 8;
  data |= ctrl_ReadRegisterWrapper(interfacePtr, channel, RAM_DATA_B0);

  data = data >>3;

  SPI_TRC("DBG: %s: ramloc = %0d data = 0x%04X\n", __FUNCTION__,
          ramAddr, data);
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
#if 0
  if( (reset_fd = open(LINUX_GPIO, O_WRONLY) ) < 0 )
  {
    perror("Failed to open GPIO");
    close(spi_fd);
    abort();
  }
#endif
  /* TODO: hook up IOC_XFER to menu */
#ifdef SILABS_USE_IOC_XFER
  SPI_TRC("DBG: %s: using IOC transfers with bytelen = %d\n", __FUNCTION__, SILABS_BYTE_LEN);
  xfer.len = SILABS_BYTE_LEN;
  xfer.speed_hz = speed;
  xfer.delay_usecs = 0;
  xfer.bits_per_word = SILABS_BITS_PER_WORD;
  xfer.cs_change = 1; /* Must deassert CS for next transfer */
#else
  SPI_TRC("DBG: %s: using read/write transfers with bytelen = %d\n", __FUNCTION__, SILABS_BYTE_LEN);
  ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
#endif
  ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);

 //interfacePtr->spi_fd = spi_fd;
 //interfacePtr->reset_fd = reset_fd;
  interfacePtr->fd = spi_fd;
  return 0;
}

/******************************************************************************
 * Shutdown the spidev interface
 *
 */

int SPI_Close(ctrl_S *interfacePtr)
{
  SILABS_UNREFERENCED_PARAMETER(interfacePtr);

  if(reset_fd >= 0)
  {
    close(reset_fd);
    reset_fd = -1;
  }

  if(spi_fd >= 0)
  {
    close(spi_fd);
    spi_fd = -1;
  }

  return 0;
}

#endif

int main(int argc, char *argv[])
{
	ctrl_S interfacePtr;
	int fd, fd3;
	unsigned char temp;
	SPI_Init(&interfacePtr);
	fd=open("/dev/spi", O_RDWR);
	if(fd<0)
	{
		perror("open dev/spi fail\n");
	}
	fd3=open("/dev/pcm",O_RDWR);
	if(fd3<0)
	{
		perror("open /dev/pcm fail\n");
	}
	usleep(30);
	ioctl(fd,1);// lagao
#if 0	// sim mode contrl
	ioctl(fd, 5);
	sleep(5);
	ioctl(fd, 4);
#endif
	temp = ctrl_ReadRegisterWrapper(&interfacePtr, 0, 0);
	printf("temp = %x\n", temp);
	//si_write_reg(&spi_ctr,0,11,6);
	//si_read_reg(&spi_ctr,0,11);
	ctrl_WriteRegisterWrapper(&interfacePtr, 0, 11, 6);
	temp = ctrl_ReadRegisterWrapper(&interfacePtr, 0, 11);
	printf("temp = %x\n", temp);

	int data=0x34;
	int data2=0;
	//si_writeRam(&spi_ctr,0,53,data);
	//data2=si_readRam(&spi_ctr,0,53);
	//printf("data2=%x\n",data2);
	ctrl_WriteRAMWrapper(&interfacePtr, 0, 53, data);
	data2 = ctrl_ReadRAMWrapper(&interfacePtr, 0, 53);
	printf("data2=%x\n",data2);
	ioctl(fd,0);// lagao

	return 0;
}


#if 0
int main(int argc, char *argv[])
{
	si_hctrl_t spi_ctr;
	int fd,fd2,fd3, ret = 0;
	char buf[2];
	buf[0] = 0xaa;

	parse_opts(argc, argv);

	fd2=open(device, O_RDWR);
	if(fd2<0)
	{
		perror("open dev/spidev0.1 fail\n");
	}

	fd=open("/dev/spi", O_RDWR);
	if(fd<0)
	{
		perror("open dev/spi fail\n");
	}
#ifdef SPI_CTRL
	/*
	 * spi mode
	 */
	ret = ioctl(fd2, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		perror("can't set spi mode");

	ret = ioctl(fd2, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1)
		perror("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd2, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		perror("can't set bits per word");

	ret = ioctl(fd2, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		perror("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd2, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		perror("can't set max speed hz");

	ret = ioctl(fd2, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		perror("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	
	//transfer(fd2);
	//close(fd2);

	//return ret;

#endif
#if 0
	//ioctl(fd, atoi(argc[1]));
	//usleep(100000);
	//ioctl(fd, 3);
	//usleep(100000);
	while(1)
	{
		write(fd2, buf, 1);
	}
#else

#ifdef SPI_CTRL
	spi_ctr.spi_fd[0]=fd2;
#else
	spi_ctr.spi_fd[0]=fd;
#endif
	fd3=open("/dev/pcm",O_RDWR);
	if(fd3<0)
	{
		perror("open /dev/pcm fail\n");
	}
	usleep(30);
	ioctl(fd,1);// lagao
	si_read_reg(&spi_ctr,0,0);
	//ioctl(fd, SPI_MODE_CONFIG_READ);
	si_read_reg(&spi_ctr,0,0);
	//ioctl(fd, SPI_MODE_CONFIG);
#if 1
	//while(1)
	{
		si_read_reg(&spi_ctr,0,0);
		//ioctl(fd, SPI_MODE_CONFIG_READ);
	}
	si_read_reg(&spi_ctr,0,43);
	si_write_reg(&spi_ctr,0,11,6);
	si_read_reg(&spi_ctr,0,11);
#endif
#if 1
	//while (1)
	{
		int data=0x34;
		int data2=0;
		si_writeRam(&spi_ctr,0,53,data);
		data2=si_readRam(&spi_ctr,0,53);
		printf("data2=%x\n",data2);
	}
	//while(1);
#endif
	ioctl(fd,0);//fuwei
	
	close(fd3);
#endif
	close(fd);
	return 0;
}
#endif

