/*
 * LED.c
 *
 *  Created on: Sep 1, 2017
 *      Author: Alex Perera
 */

#include "msp.h"
#include "LED.h"


//              Function Implementations            //

//                    I2C Write Function                    //
/*
 * Write data to a slave with I2C
 * Param "slaveaddr": Location of slave address to write to
 * Param "memaddr": location to write to within the slave
 * Param "bytecount": number of bytes of data to send
 * Param "data": pointer to array of 8 bit values to send
 */
static inline void I2C_write(int slaveaddr, unsigned char memaddr, int bytecount, unsigned char* data )
{

    EUSCI_B2->I2CSA = slaveaddr;                                                // Send slave address to I2C Slave Addr Register


    EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_TXSTT;                                     // Generate start condition, bit1 set

    while(EUSCI_B2->CTLW0 & EUSCI_B_CTLW0_TXSTT);                              // Wait for complete address to send


    EUSCI_B2->TXBUF = memaddr;                                                  // Send memory address to slave




    while(!(EUSCI_B2->IFG & EUSCI_B_IFG_TXIFG));                               // EUSCI_B Transmit Interrupt Flag. set when TXBUF is empty

    while(bytecount > 0)                                                          // Keep sending while there's data to send
    {
        EUSCI_B2->TXBUF = *data++;                                              // Send data to slave, increment pointer for next value
        while(!(EUSCI_B2->IFG & EUSCI_B_IFG_TXIFG));                           // EUSCI_B Transmit Interrupt Flag 0. set when TXBUF is empty
        bytecount--;                                                              // Decrement byte counter for while loop
    }

    EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_TXSTP;                                     // Generate stop condition, bit2 set

    while((EUSCI_B2->CTLW0 & EUSCI_B_CTLW0_TXSTP ));                              // Wait for Stop condition to send

    //EUSCI_B2->IFG = 0;                                                           // Clear flags
}

//                    LED Write Function                    //
/*
 *  Turns on RGB LEDs
 *  Param "color": color to be outputted (green, blue, or red)
 *  Param "leds": number of LEDs to be turned on, 16 bit value, one bit for each LED
 */
void LED_write(colors_t color, uint16_t leds)                               // Turn on LEDs, set bit to turn on, pass in color: red, green, or blue
{
    unsigned char data[4] = {};                                             // Data array of 4 char (bytes), 32 bits

    unsigned char temp_array[16] = {};                                      // Temporary array of 16 char (bytes)
    for (int i = 0; i < 16; i++)
    {
        temp_array[i] = leds & (1 << i) ? 1 : 0;                            // Convert uint16_t value into array of binary characters
    }

    int j = 0;                                                              // Counter for nested for loop
    int counter = 4;                                                        // Counter for number of times to do the nested for loop

    for(int i = 0; i < 4; i++)                                              // Split into Data array of 4 char (bytes), 32 bits
    {
        for( j  ; j < counter; j++)                                         // Go through all 16 values in temp array
        {
            if(temp_array[j] == 1)                                          // If the value in the temp array is 1, LED on, corresponding value sent is 01
            {
                data[3-i] = ((data[3-i] << 2) | 1);                         // Fill in MSB of data array first
            }                                                               // Shift data array right twice, or with 1
            else
            {
                data[3-i] = (data[3-i] << 2);                               // If the value in the temp array is 0, LED off, corresponding value sent is 00
            }                                                               // Shift data array right twice
        }
        counter +=4;                                                        // Add 4 to the counter of number of times to do the nested for loop
    }                                                                       // This is to go through all 16 values in temp_array

    I2C_write( 0x60|color , 0x16, 4, data);                                 // Send the data array and color enum to the LEDs via I2C
}


//                    LED Clear Function                    //
/*
 * Turns off RGB LEDs
 * Param "clear_LEDs": number of LEDs to be turned off, 16 bit value, one bit for each LED
 */
void LED_clear(uint16_t clear_LEDs)                                         // Turn off LEDs, set bit to turn off
{
    LED_write(blue, ~clear_LEDs);                                           // Turn off all blue LEDs of bits set
    LED_write(green, ~clear_LEDs);                                          // Turn off all green LEDs of bits set
    LED_write(red, ~clear_LEDs);                                            // Turn of all red LEDs of bits set
}


/*
 * Initialize EUSCI_B2 for I2C communication with LED drivers
 * No parameters
 */
void I2C_init(void)
{
    EUSCI_B2 -> CTLW0 |= EUSCI_B_CTLW0_SWRST;                                     // Set UCSWRST for configuration (avoid unpredictable behavior), Software reset enable
    EUSCI_B2 -> CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK |  EUSCI_B_CTLW0_SYNC |  EUSCI_B_CTLW0_MODE_3 | EUSCI_B_CTLW0_MST|EUSCI_B_CTLW0_TR;      // Standard Mode Clock | Synchronous Mode
                                                                                 // I2C Mode | Master Mode
                                                                                  // 7-bit slave address (UCSLA10,Bit 14 = 0), Single Master Environment (UCMM,Bit 13 = 0)
    EUSCI_B2 -> BRW = 30;                                                         // 3 MHz / 30 = 100 kHz
    P3 -> SEL0 |= 0xC0;                                                           // P3.7 (SCL), P3.6 (SDA) configured for UCB2
    P3 -> SEL1 &= 0xC0;

    EUSCI_B2 -> CTLW0 &= ~1;                                                      // CTLW0 & 0xFFFE, Released for Operation
}






