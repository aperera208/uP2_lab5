/*
 * G8RTOS_Semaphores.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include "G8RTOS.h"

/*********************************************** Dependencies and Externs *************************************************************/
#define StartContextSwitch() SCB->ICSR = SCB_ICSR_PENDSVSET_Msk


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value)
{
	/* Implement this */

    int32_t IBit_State = StartCriticalSection();                  // Start critical section, save the I bit in local variable

    (*s) = value;                                                 // Initialize given semaphore with given value

    EndCriticalSection(IBit_State);                               // End critical section, restore the I bit
}

/*
 * Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_WaitSemaphore(semaphore_t *s)
{
	/* Implement this */

    int32_t IBit_State = StartCriticalSection();                    // Start critical section, save the I bit in local variable

    /*
    while( (*s) == 0)                                               // Spinlock
    {
        EndCriticalSection(IBit_State);
        IBit_State = StartCriticalSection();
    }
    */

    (*s) = (*s) - 1;                                                // Decrement semaphore

    if ((*s) < 0)
    {
        CurrentlyRunningThread->blocked = s;                            // Blocked is pointing to the semaphore to be blocked
        EndCriticalSection(IBit_State);                                 // End critical section, restore the I bit
        StartContextSwitch();
    }

    EndCriticalSection(IBit_State);                                 // End critical section, restore the I bit
}

/*
 * Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_SignalSemaphore(semaphore_t *s)
{
	/* Implement this */

    tcb_t *temp_ptr;                                                // Temp pointer to go through the linked lists of TCBs

    int32_t IBit_State = StartCriticalSection();                    // Start critical section, save the I bit in local variable


    (*s) = (*s) + 1;                                                // Increment semaphore value

    if ( (*s) <= 0)
    {
        temp_ptr = CurrentlyRunningThread->next;                     // Temp pointer points to CRT next
        while(temp_ptr->blocked != s )                               // Go through the linked lists looking for this blocked semaphore
        {
            temp_ptr = temp_ptr->next;                                // If this isnt the thread with the blocked semaphore, go to the next thread
        }
        temp_ptr->blocked = 0;                                       // Unblock this semaphore
    }

    EndCriticalSection(IBit_State);                                 // End critical section, restore the I bit

}

/*********************************************** Public Functions *********************************************************************/


