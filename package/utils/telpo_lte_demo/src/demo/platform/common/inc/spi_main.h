/*
** Copyright (c) 2012 by-2015 Silicon Laboratories
**
** $Id: spi_main.h 4650 2015-01-09 22:03:04Z nizajerk $
**
** ProSLIC API Demonstration Code
**
** Author(s): 
** cdp
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This file provides provides access to SPI driver for
** all proslic motherboards (VMB, VMB2)
**
**
*/
#include "proslic.h"

#ifndef __SPI_MAIN_HDR__
#define __SPI_MAIN_HDR__ 1

#ifdef VMB2
#include "spi_pcm_vcp.h"
#define VMB_INFO "VMB2"
#else
#include "proslic_spiGci_usb.h"
#define VMB_INFO "VMB"
#endif

/*
 * Purpose: interactively ask user for any parameter changes from the default.  Can be empty
 * for customer platforms, but for the VMB1/VMB2, we can change the SCLK & PCLK settings.
 */
void vmbSetup(controlInterfaceType *);


/*
 * Purpose: Allocate/open any system resources needed to run SPI and timers.  Associates
 * the customer/platform implemented SPI, Timer and Semaphore functions with the API.
 */
void initControlInterfaces(controlInterfaceType *ProHWIntf, void *spiIf, void *timerObj);

/* 
 *  Purpose: free any system resources used in the application (timers, SPI)  - should be called prior to
 *  SiVoice_destroyControlInterface()
 */
void destroyControlInterfaces(controlInterfaceType *ProHWIntf);

#endif
