/******************************************************************************
 * Copyright (c) 2015 by Silicon Laboratories
 *
 * $Id: pform_test.c 4902 2015-04-23 15:10:59Z nizajerk $
 *
 * This file contains proprietary information.	 
 * No dissemination allowed without prior written permission from
 * Silicon Laboratories, Inc.
 *
 * File Description:
 *
 * This file implements a simple platform validation tool.
 *
 */

#include <stdio.h>
#include "proslic.h"
#include "proslic_timer.h" 

#include "pform_test.h"
#include "spi_main.h"
#include "spi_tests.h"
#include "timer_tests.h"

/**********************************************************
 * main entry point. 0 = OK, < 0 = error.
 */
int main(int argc, char **argv)
{
	ctrl_S spiGciObj;              /* Link to host spi obj (os specific) */
  systemTimer_S timerObj;        /* Link to host timer obj (os specific)*/
  controlInterfaceType	hwIf;    /* No dynamic memory allocation */
  int rc = -2;

  printf("\nBasic platform validation tool version 0.0.2\n");
  printf("Copyright 2015, Silicon Laboratories\n--------------------------------------------------------\n");

  /* Initialize platform */
	if(SPI_Init(&spiGciObj) == FALSE)
  {
    printf("%s: Cannot connect to %s\n", *argv, VMB_INFO);
    return -1;
  }

  printf("Demo: Initializing system timer...\n");
  TimerInit(&timerObj);

  /*
   ** Establish linkage between host objects/functions
   ** and ProSLIC API (required)
   */
  printf("Demo: Linking function pointers...\n");
  initControlInterfaces(&hwIf, &spiGciObj, &timerObj);

  /* Now start testing */
  rc = spiTests(&hwIf,0);

  /* TimerInit isn't part of the API specification, but may be used for our demo
     code.  This function may be removed for your specific target/OS.
   */
  TimerInit(hwIf.hTimer);
  rc += timerTests(&hwIf,0);

  /* TODO: if you did implement a TimerInit and did allocate resources, you should
    free them here.
   */

  /* Close any system resources */
  destroyControlInterfaces(&hwIf);

  return rc;
}


