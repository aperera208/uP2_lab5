/*
 * G8RTOS_Scheduler.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include "G8RTOS.h"

/*
 * G8RTOS_Start exists in asm
 */
extern void G8RTOS_Start();

/* System Core Clock From system_msp432p401r.c */
extern uint32_t SystemCoreClock;

/*
 * Pointer to the currently running Thread Control Block
 */
extern tcb_t * CurrentlyRunningThread;

/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Defines ******************************************************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT 0x01000000
#define StartContextSwitch() SCB->ICSR = SCB_ICSR_PENDSVSET_Msk

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static tcb_t threadControlBlocks[MAX_THREADS];

/* Periodic Thread Control Blocks
 *  - An array of periodic thread control blocks to hold pertinent information for each periodic thread
 */
static periodic_event_t PeriodicThreadControl[MAX_THREADS];

/* Thread Stacks
 *	- An array of arrays that will act as invdividual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];


/*********************************************** Data Structures Used *****************************************************************/


/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads;
static uint32_t NumberOfPEvents;

/*********************************************** Private Variables ********************************************************************/


/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(uint32_t numCycles)
{
	/* Implement this */

    NVIC_DisableIRQ(SysTick_IRQn);                                  // Disable Interrupt while configuring
    SysTick -> VAL = 0;                                             // Clear current value register STCVR
    NVIC_SetPriority(SysTick_IRQn, OSINT_PRIORITY);                              // Set Priority 7 on the interrupt
    SysTick -> LOAD = numCycles-1;                                    // Reload Value Register with # of clock cycles to count down from
                                                                    // 48000 for 1ms
    SysTick -> CTRL = SysTick_CTRL_ENABLE_Msk                       // Core clock, Enable interrupt on count to 0, Enable SysTic counter
                    | SysTick_CTRL_TICKINT_Msk
                    | SysTick_CTRL_ENABLE_Msk;

    NVIC_EnableIRQ(SysTick_IRQn);                                   // Enable SysTick interrupt
}

/*
 * Chooses the next thread to run.
 * Lab4 Priority Scheduler
 * Will schedule the highest priority thread to run
 * Highest Priority = 0, Lowest Priority = 254
 *
 */
void G8RTOS_Scheduler()
{
    uint32_t currentMaxPriority = 256;
    tcb_t * tempNextThread;
    tempNextThread = CurrentlyRunningThread->next;

    for(int i = 0; i < NumberOfThreads; i++)
    {
        // Check if thread is neither sleeping nor blocked and if the priority is higher (lower value)
        if((tempNextThread->Asleep == 0) && (tempNextThread->blocked == 0) && (tempNextThread->priority) < currentMaxPriority)
        {
            CurrentlyRunningThread = tempNextThread;                                                // set this thread as the current thread to run, highest priority so far
            currentMaxPriority = tempNextThread->priority;                                          // update the current max priority value for checking rest of threads
        }
        tempNextThread = tempNextThread->next;                                                      // Next thread to keep checking
    }




    /*
    tcb_t * tempNextThread = CurrentlyRunningThread;
    tcb_t * threadToRun;
    do
    {
        tempNextThread = tempNextThread->next;                                           // Set temp Next Thread to be the next thread in the linked list

        // Check if thread is neither sleeping nor blocked and if the priority is higher (lower value)
        if((tempNextThread->Asleep == 0) && (tempNextThread->blocked == 0) && (tempNextThread->priority) < currentMaxPriority)
        {
            threadToRun = tempNextThread;                                               // set this thread as the current thread to run, highest priority so far
            currentMaxPriority = tempNextThread->priority;                              // update the current max priority value for checking rest of threads
        }

    } while(CurrentlyRunningThread != tempNextThread);                                  // iterate through all threads up to currently running thread

    CurrentlyRunningThread = threadToRun;                                               // Set the currently running thread to the thread with highest priority
      */

}

/*
 * SysTick Handler
 * Currently the Systick Handler will only increment the system time
 * and set the PendSV flag to start the scheduler
 *
 * In the future, this function will also be responsible for sleeping threads and periodic threads
 */
void SysTick_Handler()
{
	/* Implement this */
    SystemTime++;                                                                                                   // Increment the system time

    if(NumberOfPEvents > 0)                                                                                         // Only check if there are periodic threads to run if there are periodic threads
    {
        periodic_event_t *periodic_ptr = &PeriodicThreadControl[0];                                                 // Pointer to first thread

        for (int i = 0; i < NumberOfPEvents; i++)                                                                   // Cycle through all periodic threads
        {
            if (periodic_ptr->execute_time == SystemTime)                                                           // If the execute time is equal to the system time, this thread is ready to run
            {
                periodic_ptr->execute_time = (periodic_ptr->period+SystemTime);                                     // Reset the execute time to the period+SystemTime, this is the next time this thread will run
                periodic_ptr->Handler();                                                                            // Run the periodic event thread
            }
            periodic_ptr = periodic_ptr->next_p_event;
        }
    }

    if(NumberOfThreads > 0)
    {
        tcb_t *ptr_t = CurrentlyRunningThread;                                                                     // Pointer to the currently running thread

        for (int i = 0; i < NumberOfThreads; i++)                                                                   // Go through all the threads added
        {
            if((ptr_t->Asleep == true) && (ptr_t->sleep_count == SystemTime))                                           // if a thread is alseep AND it is ready to be woken up
            {
                ptr_t->Asleep = false;                                                                              // wake up by changing boolean value of Asleep
            }
            ptr_t = ptr_t->next;                                                                                    // Point to next thread
        }
    }

    StartContextSwitch();                                                                                           // Set pending pendSV bit, change threads

}


/*********************************************** Private Functions ********************************************************************/


/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t SystemTime;

/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/


void G8RTOS_Sleep(uint32_t duration)
{
    CurrentlyRunningThread->sleep_count = (duration + SystemTime);                        // Initialize sleep count to (duration + SystemTime), will sleep until that value
    CurrentlyRunningThread->Asleep = true;                                                 // Set Asleep boolean to True, sleeping
    StartContextSwitch();
}



/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void G8RTOS_Init()
{
    BSP_InitBoard();                            // Initialize all hardware on the board

    uint32_t newVTORTable = 0x20000000;
    memcpy((uint32_t *)newVTORTable, (uint32_t *)SCB->VTOR, 57*4);  // 57 interrupt vectors to copy
    SCB->VTOR = newVTORTable;

    for(int i = 0; i < MAX_THREADS; i++)
    {
        threadControlBlocks[i].Alive = false;                       // Initialize all threads to dead
    }

    IDcounter = 0;                              // Initialize ID counter to 0
    SystemTime = 0;                             // Initialize system time to 0
    NumberOfThreads = 0;                        // Set the number of threads to 0
    NumberOfPEvents = 0;                        // Set the number of periodic event threads to 0



}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to thread with highest priority
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
G8RTOS_codes_t G8RTOS_Launch()
{

    /*
    uint32_t currentMaxPriority = 256;
    CurrentlyRunningThread = &threadControlBlocks[0];                                   // Initialize Thread0 the currently running thread
    tcb_t * tempNextThread = CurrentlyRunningThread;                                    // Temporary for iteration
    tcb_t * threadToRun;                                                                // Temporary to hold currently highest priority thread
    do
    {
        tempNextThread = tempNextThread->next;                                           // Set temp Next Thread to be the next thread in the linked list

        if((tempNextThread->priority) < currentMaxPriority)                             // Check if thread priority is higher (lower value)
        {
            threadToRun = tempNextThread;                                               // set this thread as the current thread to run, highest priority so far
            currentMaxPriority = tempNextThread->priority;                              // update the current max priority value for checking rest of threads
        }

    } while(CurrentlyRunningThread != tempNextThread);                                  // iterate through all threads up to first thread

    CurrentlyRunningThread = threadToRun;
    */

    uint32_t currentMaxPriority = 256;
    CurrentlyRunningThread = &threadControlBlocks[0];                                               // Initialize Thread0 the currently running thread
    tcb_t * tempNextThread;
    tempNextThread = CurrentlyRunningThread->next;

    for(int i = 0; i < NumberOfThreads; i++)
    {

        if((tempNextThread->priority) < currentMaxPriority)                                         // Check if the priority is higher (lower value)
        {
            CurrentlyRunningThread = tempNextThread;                                                // set this thread as the current thread to run, highest priority so far
            currentMaxPriority = tempNextThread->priority;                                          // update the current max priority value for checking rest of threads
        }
        tempNextThread = tempNextThread->next;                                                      // Next thread to keep checking
    }

    NVIC_SetPriority(PendSV_IRQn, OSINT_PRIORITY);          // Set Priority 7 on the interrupt
    InitSysTick(48000);                                      // Initialize Systick to 1ms
    G8RTOS_Start();

    return launch_error;
}


/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are still available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers with priority
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Param "name": name of thread
 * Param "priority": priority of thread, 0-highest, 254-lowest
 * Returns: Error code for adding threads
 */
G8RTOS_codes_t G8RTOS_AddThread(void (*threadToAdd)(void), char *name, uint32_t priority)
{
    int32_t IBit_State = StartCriticalSection();                        // Start critical section, save the I bit in local variable

    if(NumberOfThreads > MAX_THREADS)                                   // If Number of Threads is greater than MAX_THREADs
    {
        return max_thread_error;                                        // Return error code
    }
    int i = 0;
    if(NumberOfThreads == 0)                                            // If this is the first thread added
    {
        i = 0;                                                          // Set index i to be 0
    }
    else                                                                // if not the first thread to be added
    {
        for( i = 0 ; i < (MAX_THREADS); i++)                            // iterate through all threads in array
        {
            if(threadControlBlocks[i].Alive == false)                   // if a thread is dead
            {
                break;                                                  // break from for loop and use i as the index to add thread
            }
        }
    }

    threadControlBlocks[i].Asleep = false;                              // Set thread to awake
    threadControlBlocks[i].Alive = true;                                // Set thread to Alive
    threadControlBlocks[i].priority = priority;                         // Set priority
    for( int j = 0; j < 16; j++)                                        // Initialize name
    {
        if(*name == 0)                                                  // if at the end of the name, given by 0
        {
            threadControlBlocks[i].ThreadName[j] = 0;                   // Append 0 to end of thread name
            break;                                                      // break from for loop
        }
        else                                                            // if not at end of name
        {
            threadControlBlocks[i].ThreadName[j] = *name++;             // Append char pointed to by name to the thread name char array, post increment name pointer
        }
    }

    // Unique thread ID
    threadControlBlocks[i].threadID = (IDcounter++ << 16)|i;



    threadControlBlocks[i].sp = &threadStacks[i][STACKSIZE-16];         // Stack Pointer for this thread
    threadStacks[i][STACKSIZE - 16] = 0x04040404;                      // Initialize R4
    threadStacks[i][STACKSIZE - 15] = 0x05050505;                      // Initialize R5
    threadStacks[i][STACKSIZE - 14] = 0x06060606;                      // Initialize R6
    threadStacks[i][STACKSIZE - 13] = 0x07070707;                      // Initialize R7
    threadStacks[i][STACKSIZE - 12] = 0x08080808;                      // Initialize R8
    threadStacks[i][STACKSIZE - 11] = 0x09090909;                      // Initialize R9
    threadStacks[i][STACKSIZE - 10] = 0x10101010;                      // Initialize R10
    threadStacks[i][STACKSIZE - 9]  = 0x11111111;                      // Initialize R11
    threadStacks[i][STACKSIZE - 8]  = 0x00000000;                      // Initialize R0
    threadStacks[i][STACKSIZE - 7]  = 0x01010101;                      // Initialize R1
    threadStacks[i][STACKSIZE - 6]  = 0x02020202;                      // Initialize R2
    threadStacks[i][STACKSIZE - 5]  = 0x03030303;                      // Initialize R3
    threadStacks[i][STACKSIZE - 4]  = 0x12121212;                      // Initialize R12
    threadStacks[i][STACKSIZE - 3]  = 0x14141414;                      // Initialize R14
    threadStacks[i][STACKSIZE - 2]  = (int32_t)threadToAdd;                     // Function Pointer for PC
    threadStacks[i][STACKSIZE - 1]  = THUMBBIT;                        // Status Register initialized to Thumb 2 Mode

    /*
     *  You basically need to put the new thread in front of the CRT and
     *  adjust the pointers for the CRT thread and initialize the new thread's pointers accordingly.
     */


    if(NumberOfThreads  == 0)                                                     // If this is the only thread
    {
        threadControlBlocks[i].prev = &threadControlBlocks[i];                    // Previous pointer points to itself
        threadControlBlocks[i].next = &threadControlBlocks[i];                    // Next pointer points to itself
    }
    else
    {
        CurrentlyRunningThread->next->prev = &threadControlBlocks[i];             // CRT's next thread's prev pointer points to new thread
        threadControlBlocks[i].next = CurrentlyRunningThread->next;               // new thread's next pointer points to CRT's next
        CurrentlyRunningThread->next = &threadControlBlocks[i];                   // CRT's next pointer points to new thread
        threadControlBlocks[i].prev = CurrentlyRunningThread;                     // new thread's prev pointer points to CRT


        /*
        threadControlBlocks[i].prev = &threadControlBlocks[NumberOfThreads - 1];                              // New thread previous pointer points to the last thread
        threadControlBlocks[NumberOfThreads-1].next = &threadControlBlocks[NumberOfThreads];                  // Last thread next pointer points to new thread
        threadControlBlocks[0].prev = &threadControlBlocks[NumberOfThreads];                                  // First thread previous pointer points to new thread
        threadControlBlocks[NumberOfThreads].next = &threadControlBlocks[0];                                  // New thread next pointer points to the the first thread
        */
    }



    NumberOfThreads++;                                                                                  // Increment Number of Threads
    EndCriticalSection(IBit_State);                                                                     // End critical section, restore the I bit
    return thread_added_success;                                                                        // Return 0 for success
}


/*
 * Add Periodic Thread to G8RTOS Scheduler
 * - Checks if the max number of periodic threads have been added
 * - Initializes period, execute time, and current time
 * - Handles previous and next pointers
 * Param "periodicthreadtoAdd" : pointer to the thread to add
 * Param "period" : how often the periodic thread should run
 *
 */
G8RTOS_codes_t G8RTOS_AddPeriodicEvent(void (*periodicthreadtoAdd)(void), uint32_t period)
{
    int32_t IBit_State = StartCriticalSection();                                                 // Start critical section, save the I bit in local variable


    if (NumberOfPEvents > MAX_THREADS )                                                          // If Number of Periodic Events Threads is greater than MAX_THREADS
    {
        return max_periodic_events_error;                                                        // return error code
    }

    PeriodicThreadControl[NumberOfPEvents].Handler = periodicthreadtoAdd;                        // point the periodic thread event to the event handler
    PeriodicThreadControl[NumberOfPEvents].period = period;                                      // Set the period of this periodic event thread
    PeriodicThreadControl[NumberOfPEvents].execute_time = (period+SystemTime);                     // Execute time is period + system time
    PeriodicThreadControl[NumberOfPEvents].current_time = 0;                                     // Initialize the current time value to 0, start counting at 0


    if(NumberOfPEvents == 0)
    {
        PeriodicThreadControl[NumberOfPEvents].prev_p_event = &PeriodicThreadControl[NumberOfPEvents];          // Previous pointer points to itself
        PeriodicThreadControl[NumberOfPEvents].next_p_event = &PeriodicThreadControl[NumberOfPEvents];          // Next pointer points to itself
    }
    else
    {
        PeriodicThreadControl[NumberOfPEvents].prev_p_event = &PeriodicThreadControl[NumberOfPEvents - 1];      // New periodic event thread's previous pointer points to last periodic event thread
        PeriodicThreadControl[NumberOfPEvents-1].next_p_event = &PeriodicThreadControl[NumberOfPEvents];        // Last event thread's next pointer points to new periodic event thread
        PeriodicThreadControl[0].prev_p_event = &PeriodicThreadControl[NumberOfPEvents];                        // First periodic event thread's previous pointer points to new periodic event thread
        PeriodicThreadControl[NumberOfPEvents].next_p_event = &PeriodicThreadControl[0];                        // New periodic event thread's next pointer points to first periodic event thread
    }

    NumberOfPEvents++;                                                                                          // Increment number of periodic event threads

    EndCriticalSection(IBit_State);                                                                             // End critical section, restore the I bit


    return periodic_event_thread_added_success;

}

/*
 *  Gets Thread ID of the Currently Running Thread
 *  Return : Thread ID of type threadID_T (uint32_t)
 *
 */
threadID_t G8RTOS_GetThreadID(void)
{
    return CurrentlyRunningThread->threadID;
}



/*
 * Kill Thread
 * Kills the thread with the threadID specified
 * Param "threadID" : threadID of thread to be killed
 */
G8RTOS_codes_t G8RTOS_KillThread(threadID_t threadID)
{
    int32_t IBit_State = StartCriticalSection();                        // Start critical section, save the I bit in local variable

    if(NumberOfThreads == 1)                                            // If this is the only thread running
    {
        return cannot_kill_last_thread;                                 // return error
    }

    int i;
    for( i = 0; i <= MAX_THREADS; i++)                                  // iterate through all the threads
    {
        if(i == MAX_THREADS)                                            // if gone through all threads and cannot find threadID
        {
            return thread_does_not_exist;                               // return error
        }
        else if(threadControlBlocks[i].threadID == threadID)            // if threadID matches the threadID of thread to be killed
        {
            break;                                                      // break out of for loop
        }

    }

    threadControlBlocks[i].Alive = false;                               // kill the thread

    threadControlBlocks[i].prev->next = threadControlBlocks[i].next;    // Set killed threads previous thread's next pointer to killed threads next
    threadControlBlocks[i].next->prev = threadControlBlocks[i].prev;    // Set killed threads next thread's previous pointer to killed threads previous

    if(threadControlBlocks[i].threadID == CurrentlyRunningThread->threadID) // if the thread to be killed is the same as the currently running thread
    {
        StartContextSwitch();                                               // Start a context switch, this won't happen until interrupts are re-enabled, end critical section
    }

    NumberOfThreads--;                                                  // Decrement number of threads

    EndCriticalSection(IBit_State);                                     // End critical section, restore the I bit
    return no_error;
}


/*
 * Kill Self
 * Kills the Currently Running Thread
 */
G8RTOS_codes_t G8RTOS_KillSelf()
{
    int32_t IBit_State = StartCriticalSection();                        // Start critical section, save the I bit in local variable

    if(NumberOfThreads == 1)                                            // If this is the only thread running
    {
        return cannot_kill_last_thread;                                 // return error
    }

    CurrentlyRunningThread->Alive = false;                              // Kill Currently Running Thread

    CurrentlyRunningThread->prev->next = CurrentlyRunningThread->next;  // Set currently running thread's previous thread's next pointer to currently running thread's next
    CurrentlyRunningThread->next->prev = CurrentlyRunningThread->prev;  // Set currently running thread's next thread's previous pointer to currently running thread's previous
    StartContextSwitch();                                               // start context switch, this won't happen until critical section ends


    NumberOfThreads--;                                                  // Decrement Number of Threads
    EndCriticalSection(IBit_State);                                     // End critical section, restore the I bit


    return no_error;
}


/*
 * Add Aperiodic Event
 * Adds an aperiodic event to the G8RTOS Scheduler
 * Param "AthreadToAdd" : pointer to void void function to add
 * Param "priority" : priority of thread
 * Param "IRQn" :
 */
G8RTOS_codes_t G8RTOS_AddAperiodicEvent(void (*aperiodicthreadToAdd)(void), uint8_t priority, IRQn_Type IRQn)
{
    int32_t IBit_State = StartCriticalSection();                        // Start critical section, save the I bit in local variable


    if((IRQn > PORT6_IRQn) || (IRQn < PSS_IRQn))                        // Make sure the IRQ is a valid peripheral exception
    {
        return IRQn_invalid;
    }

    if(priority > 6)                                                    // Greatest user priority is 6
    {
        return HWI_priority_invalid;
    }

    __NVIC_SetVector(IRQn, (uint32_t)aperiodicthreadToAdd);
    NVIC_SetPriority(IRQn, priority);                                   // Set priority of interrupt
    NVIC_EnableIRQ(IRQn);                                               // Enable interrupt

    EndCriticalSection(IBit_State);                                     // End critical section, restore the I bit


    return no_error;
}

/*
* Currently running thread kills all other threads
* Returns: Error Code for Removing Threads
*/
G8RTOS_codes_t G8RTOS_KillAllOtherThreads()
{
    /* Critical Section */
    int32_t PrimaskState = StartCriticalSection();

    /* Check if this is last thread (we cannot have all threads killed in this scheduler */
    if (NumberOfThreads == 1)
    {
        EndCriticalSection(PrimaskState);
        return cannot_kill_last_thread;
    }

    tcb_t * threadPtr = CurrentlyRunningThread;
    for(int i = 1; i < NumberOfThreads; i++)
    {
        /* Set alive bit of next TCB to false*/
        threadPtr->next->Alive= false;

        threadPtr = threadPtr->next;
    }

    /* Update Number of Threads */
    NumberOfThreads = 1;

    /* Set currently running thread's next TCB to itself */
    CurrentlyRunningThread->next = CurrentlyRunningThread;

    /* Set currently running thread's previous TCB to itself */
    CurrentlyRunningThread->prev = CurrentlyRunningThread;

    EndCriticalSection(PrimaskState);
    return no_error;
}
/*********************************************** Public Functions *********************************************************************/
