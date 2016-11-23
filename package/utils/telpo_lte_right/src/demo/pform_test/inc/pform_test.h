/******************************************************************************
 * Copyright (c) 2014 by Silicon Laboratories
 *
 * $Id: pform_test.h 4638 2014-12-18 20:14:44Z nizajerk $
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

#ifndef __PFORM_TEST_HDR__
#define __PFORM_TEST_HDR__
#include "proslic_api_config.h"

#define TEST_LOG LOGPRINT ("%-32.32s: ", __FUNCTION__); LOGPRINT 
#define REPORT_LOG(RESULT) LOGPRINT("%-32.32s: %s\n", __FUNCTION__, \
  (RESULT == 0 ? "PASSED" : "FAILED"))
#endif /* __PFORM_TEST_HDR__ */
