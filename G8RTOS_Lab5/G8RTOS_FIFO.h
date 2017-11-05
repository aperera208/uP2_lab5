/*
 * G8RTOS_FIFO.h
 *
 *  Created on: Oct 1, 2017
 *      Author: Alex Perera
 */

#ifndef G8RTOS_FIFO_H_
#define G8RTOS_FIFO_H_



#include "G8RTOS.h"



/*
 * Initialize a FIFO
 * Param FIFO_number: Takes in a index to initialize the FIFO within
 * Return: Error codes
 *
 */
G8RTOS_codes_t FIFO_Init(uint32_t FIFO_index);


/*
 * Write data to a FIFO
 * Param FIFO_number: Index of FIFO to write to
 * Param data: Data to be written to FIFO, uint32_t
 *
 */
G8RTOS_codes_t FIFO_write(uint32_t FIFO_number, int32_t data);


/*
 * Read data from FIFO
 * Param FIFO_number: Index of FIFO to write to
 * Return: Data read from FIFO
 */
int32_t FIFO_read(uint32_t FIFO_number);

#endif /* G8RTOS_FIFO_H_ */
