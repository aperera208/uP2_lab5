/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */


#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include "G8RTOS.h"


#define FIFOSIZE 16
typedef uint32_t threadID_t;



/*********************************************** Data Structure Definitions ***********************************************************/

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level, and Blocked Status
 */

/* Create tcb struct here */

/*********************************************** Data Structure Definitions ***********************************************************/

struct tcb{                                     // Struct for tcb
    int32_t *sp;                                // tcb pointer to stack
    struct tcb *next;                           // pointer to next tcb
    struct tcb *prev;                           // pointer to previous tcb
    semaphore_t *blocked;                       // pointer to a blocked semaphore
    uint32_t sleep_count;                       // Count for the duration of time of sleep (system + duration) in ms
    bool Asleep;                                // Boolean for Sleeping threads
    uint32_t priority;                          // Priority for thread to run, 0 is highest, 254 is lowest
    bool Alive;                                 // Boolean for Alive threads
    threadID_t threadID;                        // threadID_t (uint32_t) for the unique threadID for each
    char ThreadName[16];                        // 16 char array for unique thread name

};

typedef struct tcb tcb_t;                       // Typedef struct alias


struct FIFO{
    int32_t buffer_array[FIFOSIZE];                   // FIFO buffer, getting error FIFOSIZE is undefined even though it is defined in "G8RTOS_FIFO.h" which is included
    int32_t  *head;                          // Pointer to head of FIFO
    int32_t  *tail;                          // Pointer to tail of FIFO
    uint32_t lost_data_count;                   // Count of data lost due to full FIFO when data tries to write to FIFO
    semaphore_t Current_Size;                       // Current Size of FIFO, for semaphore
    semaphore_t FIFO_mutex;                         // Mutex for writing and reading semaphore to FIFO
};

typedef struct FIFO FIFO_t;




struct periodic_event{
    void (*Handler)(void);                      // Function pointer pointing to the event handler
    uint32_t period;                            // value in ms
    uint32_t execute_time;                      // period + system time
    uint32_t current_time;                      // time to initialize to
    struct periodic_event *prev_p_event;        // pointer to previous periodic event
    struct periodic_event *next_p_event;        // pointer to next periodic event
};

typedef struct periodic_event periodic_event_t; // typedef struct alias

/*********************************************** Public Variables *********************************************************************/

tcb_t * CurrentlyRunningThread;
typedef enum {full_fifo_error = -10, max_fifos_added_error = -9, max_periodic_events_error= -8 , HWI_priority_invalid= -7, IRQn_invalid = -6, cannot_kill_last_thread = -5, thread_does_not_exist = -4,
            threads_incorrectly_alive= -3, launch_error = -2, max_thread_error = -1,
            no_error = 0, thread_added_success = 1, periodic_event_thread_added_success = 2, fifo_added_success = 3, fifo_write_success = 4}G8RTOS_codes_t;

/*********************************************** Public Variables *********************************************************************/




#endif /* G8RTOS_STRUCTURES_H_ */
