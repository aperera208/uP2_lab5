/*
 * G8RTOS_FIFO.c
 *
 *  Created on: Oct 1, 2017
 *      Author: Alex Perera
 */

#include "G8RTOS.h"

/*********************************************** Sizes and Limits *********************************************************************/
#define MAX_FIFOS 4
#define FIFOSIZE 16

//uint32_t FIFO_number = 0;


/*********************************************** Data Structures Used *****************************************************************/
static FIFO_t fifo[MAX_FIFOS];                              // Array of fifos
/*********************************************** Data Structures Used *****************************************************************/

/*
 * Initialize a FIFO
 * Param FIFO_number: Takes in a index to initialize the FIFO within
 * Return: Error codes
 *
 */
G8RTOS_codes_t FIFO_Init(uint32_t FIFO_number)
{
    int32_t IBit_State = StartCriticalSection();                                // Start critical section, save the I bit in local variable

    if(FIFO_number > MAX_FIFOS-1)                                                // if trying to add more FIFOs than allowed
    {
        return max_fifos_added_error;                                           // return error
    }

    fifo[FIFO_number].head = &fifo[FIFO_number].buffer_array[0];
    fifo[FIFO_number].tail = &fifo[FIFO_number].buffer_array[0];                // Point head and tail for index 0 of FIFO
    G8RTOS_InitSemaphore(&fifo[FIFO_number].Current_Size, 0);                   // Initialize Current Size sempaphore to 0
    G8RTOS_InitSemaphore(&fifo[FIFO_number].FIFO_mutex, 1);                     // Initialize mutex sempaphore to 1, available
    fifo[FIFO_number].lost_data_count = 0;                                      // Initialize lost data count to 0

    EndCriticalSection(IBit_State);                                             // End critical section, restore the I bit


    return fifo_added_success;                                                  // return success

}


/*
 * Write data to a FIFO
 * Param FIFO_number: Index of FIFO to write to
 * Param data: Data to be written to FIFO, uint32_t
 *
 */
G8RTOS_codes_t FIFO_write(uint32_t FIFO_number, int32_t data)
{
    if(fifo[FIFO_number].Current_Size == FIFOSIZE-1 )                             // if there is no more room in the FIFO
    {
        fifo[FIFO_number].lost_data_count++;                                    // increment count of lost data
        return full_fifo_error;                                                 // return error
    }

    *(fifo[FIFO_number].tail) = data;                                           // Put data in index of FIFO that tail points to
    fifo[FIFO_number].tail++;                                                   // Increment tail pointer

    if(fifo[FIFO_number].tail == &fifo[FIFO_number].buffer_array[FIFOSIZE])     // if tail points to the end of buffer for this fifo
    {
        fifo[FIFO_number].tail = &fifo[FIFO_number].buffer_array[0];            // Wrap around and point tail to index 0 of this fifo
    }

    G8RTOS_SignalSemaphore(&fifo[FIFO_number].Current_Size);                    // Increment current size of this fifo, signal semaphore

    return fifo_write_success;
}


/*
 * Read data from FIFO
 * Param FIFO_number: Index of FIFO to write to
 * Return: Data read from FIFO
 */
int32_t FIFO_read(uint32_t FIFO_number)
{
    G8RTOS_WaitSemaphore(&fifo[FIFO_number].FIFO_mutex);                        // Wait for the mutex semaphore to be available
    G8RTOS_WaitSemaphore(&fifo[FIFO_number].Current_Size);                      // Wait for the current size semaphore to be available

    int32_t data = *(fifo[FIFO_number].head);                                  // data pointed to by head (oldest data)
    fifo[FIFO_number].head++;                                                   // Increment head pointer

    if(fifo[FIFO_number].head == &fifo[FIFO_number].buffer_array[FIFOSIZE])     // if head points to the end of buffer for this fifo
    {
        fifo[FIFO_number].head = &fifo[FIFO_number].buffer_array[0];            // Wrap around and point head to index 0 of this fifo
    }


    G8RTOS_SignalSemaphore(&fifo[FIFO_number].FIFO_mutex);                      // Release mutex semaphore

    return data;                                                                // return the data read from fifo
}
