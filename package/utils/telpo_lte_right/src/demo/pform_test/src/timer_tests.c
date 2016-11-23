/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories
 *
 * $Id: timer_tests.c 4896 2015-04-21 23:33:17Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 * File Description:
 *
 * This file implements some basic timer API tests.
 *
 */

#include "timer_tests.h"
#include <stdlib.h> /* for labs */
#include "proslic_timer.h" /* timeStamp definition */
#define SI_TIMER_SLOP 2 /* How may mSec can we be off by? */
#define SI_ABS        labs

#define SI_TIMER_DELTA_OK(ACTUAL, EXPECTED) ( SI_ABS(((ACTUAL) - (EXPECTED))) <= SI_TIMER_SLOP)

/**********************************************************
 * Check that the elapsed timer/delay time relationship is OK
 */
static int timerElapsed_1Sec(controlInterfaceType *hwIf, uInt8 channel)
{
    timeStamp timeStart;
    int rc;
    int delta_time; /* In terms of mSec */

    hwIf->getTime_fptr(hwIf->hTimer, &timeStart);
    hwIf->Delay_fptr(hwIf->hTimer, 1000);

    hwIf->timeElapsed_fptr(hwIf->hTimer, &timeStart, &delta_time);

    if(SI_TIMER_DELTA_OK(delta_time, 1000))
    {
        rc = 0;
    }
    else
    {
        TEST_LOG("measured: %d mSec, expected: %d +/-%d mSec\n",
            delta_time, 1000, SI_TIMER_SLOP);
        rc = 1;
    }
    REPORT_LOG(rc);

    return rc;
}

/**********************************************************
 * Check that the elapsed timer/delay time relationship is OK
 */
static int timerElapsed_10mSec(controlInterfaceType *hwIf, uInt8 channel)
{
    timeStamp timeStart;
    int rc;
    int delta_time; /* In terms of mSec */

    hwIf->getTime_fptr(hwIf->hTimer, &timeStart);
    hwIf->Delay_fptr(hwIf->hTimer, 10);

    hwIf->timeElapsed_fptr(hwIf->hTimer, &timeStart, &delta_time);

    if(SI_TIMER_DELTA_OK(delta_time, 10))
    {
        rc = 0;
    }
    else
    {
        TEST_LOG("measured: %d mSec, expected: %d +/-%d mSec\n",
            delta_time, 10, SI_TIMER_SLOP);
        rc = 1;
    }
    REPORT_LOG(rc);

    return rc;
}

/**********************************************************
 *  Run through some basic timer tests. 
 */

static timer_test testList[] = {
  timerElapsed_1Sec,
  timerElapsed_10mSec,
  NULL};

int timerTests(controlInterfaceType *hwIf, uInt8 channel)
{
  int rc = 0;
  timer_test *test = testList;

  do
  {
    rc = (*test)(hwIf, channel);
    test++;
  } while((*test != NULL) && (rc == 0));

  return rc;
}

