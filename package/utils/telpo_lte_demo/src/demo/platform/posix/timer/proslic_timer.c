/*
** $Id: proslic_timer.c 4638 2014-12-18 20:14:44Z nizajerk $
**
** System specific functions implementation file
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the implementation file for the system specific timer functions.
**
**
*/
#include "si_voice_datatypes.h"
#include "proslic.h"
#include "si_voice_timer_intf.h"
#include "proslic_timer.h"

#ifdef WIN32_GET_TIME_NEEDED
/* This implements the minimum gettimeofday needed for our purposes, for a "real" windows system
   running for days, this will not work for us without a fuller implementation since we don't
   calculate the seconds structure completely, so we would have a wrap issue on the month boundaries.

   It does not return back the same values as found in other OS's such as Linux. Linux returns back
   uSec instead of mSec values (which is what is implemented here).
*/
int gettimeofday(struct timeval *tv, void *tz)
{
	SYSTEMTIME sys_time;
    SILABS_UNREFERENCED_PARAMETER(tz);

	if(tv != NULL)
	{
		GetSystemTime(&sys_time);
		tv->tv_sec = sys_time.wSecond;
		tv->tv_sec += (sys_time.wMinute*60);
		tv->tv_sec += (sys_time.wHour*60*60);
		tv->tv_sec += (sys_time.wDay*24*60*60);
		tv->tv_usec= sys_time.wMilliseconds;
		return 0;
	}
	return -1;
}
#endif

/*
** These are the global functions
*/

/*
** Function: SYSTEM_TimerInit
*/
void TimerInit (systemTimer_S *pTimerObj)
{
    SILABS_UNREFERENCED_PARAMETER(pTimerObj);
    
}
/*
** Function: SYSTEM_Delay
*/
int time_DelayWrapper (void *hTimer, int timeInMs)
{
  usleep(timeInMs*1000);
	return RC_NONE;
}

/*
** Function: SYSTEM_TimeElapsed
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs)
{
  if((startTime != NULL) && (timeInMs != NULL))
  {
    struct timeval timeNow; 
		struct timeval result;
    gettimeofday(&timeNow, NULL);
    
		timersub(&(timeNow),&(((timeStamp *)startTime)->timerObj),&result);

		*timeInMs =(result.tv_usec/1000) + (result.tv_sec *1000);

  }

	return RC_NONE;
}

/*
** Function: SYSTEM_GetTime
*/
int time_GetTimeWrapper (void *hTimer, void *time){
  if(time != NULL)
  {
    gettimeofday(&(( (timeStamp *)time)->timerObj), NULL);
  }
	return 0;
}

