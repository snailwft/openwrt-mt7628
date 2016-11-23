/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories
 *
 * $Id: timer_tests.h 4896 2015-04-21 23:33:17Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 * File Description:
 *
 * This file implements some basic timer tests
 *
 */

#ifndef __TIMER_TESTS_HDR__
#define __TIMER_TESTS_HDR__ 1
#include "proslic.h"
#include "pform_test.h"

typedef (*timer_test)(controlInterfaceType *hwIf, uInt8 channel);

/* 0 = OK */
int timerTests(controlInterfaceType *hwIf,  uInt8 channel);

#endif /* __TIMER_TESTS_HDR__ */
