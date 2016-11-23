/******************************************************************************
 * Copyright (c) 2014-2015 by Silicon Laboratories
 *
 * $Id: spi_tests.h 4656 2015-01-10 00:23:43Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 * File Description:
 *
 * This file implements some basic SPI communications test to the ProSLIC.
 *
 */

#ifndef __SPI_TESTS_HDR__
#define __SPI_TESTS_HDR__ 1
#include "proslic.h"
#include "pform_test.h"

typedef (*spi_test)(controlInterfaceType *hwIf, uInt8 channel);

/* 0 = OK */
int spiTests(controlInterfaceType *hwIf,  uInt8 channel);

/* Common SPI functions */
int spiReadRevIDTestCommon(controlInterfaceType *hwIf, uInt8 channel, uInt8 regAddr);

int spiBasicWriteTestCommon(controlInterfaceType *hwIf, uInt8 channel, uInt8 regAddr);

int spiResetTestCommon(controlInterfaceType *hwIf, uInt8 channel, uInt8 regAddr);

#endif /* __SPI_TESTS_HDR__ */
