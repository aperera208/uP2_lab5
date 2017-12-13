/*
 * G8RTOS_Scheduler.h
 */



#ifndef G8RTOS_SCHEDULER_H_
#define G8RTOS_SCHEDULER_H_
#include "msp.h"
#include "G8RTOS.h"
#include "G8RTOS_Structures.h"

/*********************************************** Sizes and Limits *********************************************************************/
#define MAX_THREADS 32
#define STACKSIZE 256
#define OSINT_PRIORITY 7
#define MAX_FIFOS 4
#define FIFOSIZE 16
/*********************************************** Sizes and Limits *********************************************************************/

/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
extern uint32_t SystemTime;

static uint16_t IDcounter;

/*
typedef uint32_t threadID_t;
*/
/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/


/*
 * Puts thread to sleep for duration milliseconds
 *
 */
void G8RTOS_Sleep(uint32_t duration);




/*
 * Initializes variables and hardware for G8RTOS usage
 */
void G8RTOS_Init();

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes Systick Timer
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
G8RTOS_codes_t G8RTOS_Launch();

/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
G8RTOS_codes_t G8RTOS_AddThread(void (*threadToAdd)(void), char *name, uint32_t priority);


/*
 * Add Periodic Thread to G8RTOS Scheduler
 * - Checks if the max number of periodic threads have been added
 * - Initializes period, execute time, and current time
 * - Handles previous and next pointers
 * Param "periodicthreadtoAdd" : pointer to the thread to add
 * Param "period" : how often the periodic thread should run
 *
 */
G8RTOS_codes_t G8RTOS_AddPeriodicEvent(void (*periodicthreadtoAdd)(void),  uint32_t period);

/*
 *  Gets Thread ID of the Currently Running Thread
 *  Return : Thread ID of type threadID_T (uint32_t)
 *
 */
threadID_t G8RTOS_GetThreadID(void);


/*
 * Kill Thread
 * Kills the thread with the threadID specified
 * Param "threadID" : threadID of thread to be killed
 */
G8RTOS_codes_t G8RTOS_KillThread(threadID_t threadID);


/*
 * Kill Self
 * Kills the Currently Running Thread
 */
G8RTOS_codes_t G8RTOS_KillSelf();


/*
 * Add Aperiodic Event
 * Adds an aperiodic event to the G8RTOS Scheduler
 * Param "AthreadToAdd" : pointer to void void function to add
 * Param "priority" : priority of thread
 * Param "IRQn" :
 */
G8RTOS_codes_t G8RTOS_AddAperiodicEvent(void (*aperiodicthreadToAdd)(void), uint8_t priority, IRQn_Type IRQn);


G8RTOS_codes_t G8RTOS_KillAllOtherThreads();


/*********************************************** Public Functions *********************************************************************/

#endif /* G8RTOS_SCHEDULER_H_ */
