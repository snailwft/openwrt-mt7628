/*
** Posix compatible timer
*/
#ifndef MLT_TIMER_H
#define MLT_TIMER_H

#ifdef __GNUC__
#include <unistd.h>
#include <sys/time.h>
#else
#error Unsupported compiler
#endif

typedef struct
{
} systemTimer_S ;

/*
** System time stamp
*/
typedef struct{
	struct timeval timerObj;
} timeStamp;

/*
** Function: SYSTEM_TimerInit
**
** Description: 
** Initializes the timer used in the delay and time measurement functions
** by doing a long inaccurate sleep and counting the ticks
**
** Input Parameters: 
**
** Return:
** none
*/

void TimerInit (systemTimer_S *pTimerObj);

/*
** Function: DelayWrapper
**
** Description: 
** Waits the specified number of ms
**
** Input Parameters: 
** hTimer: timer pointer
** timeInMs: time in ms to wait
**
** Return:
** none
*/
int time_DelayWrapper (void *hTimer, int timeInMs);


/*
** Function: TimeElapsedWrapper
**
** Description: 
** Calculates number of ms that have elapsed
**
** Input Parameters: 
** hTImer: pointer to timer object
** startTime: timer value when function last called
** timeInMs: pointer to time elapsed
**
** Return:
** timeInMs: time elapsed since start time
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs);

int time_GetTimeWrapper (void *hTimer, void *time);
#endif

