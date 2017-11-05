/*
 * LED.h
 *
 *  Created on: Sep 1, 2017
 *      Author: Alex Perera
 */

#ifndef LED_H_
#define LED_H_

//                      Declared GLOBAL Variables                   //

typedef enum colors{red = 0, green = 1, blue = 2}colors_t;

//          Function Prototypes         //

/*
 *  Turns on RGB LEDs
 *  Param "color": color to be outputted (green, blue, or red)
 *  Param "leds": number of LEDs to be turned on, 16 bit value, one bit for each LED
 */
void LED_write(colors_t color, uint16_t leds);                  // Turn LEDs on



/*
 * Turns off RGB LEDs
 * Param "clear_LEDs": number of LEDs to be turned off, 16 bit value, one bit for each LED
 */
void LED_clear(uint16_t clear_LEDs);                            // Turn LEDs off, set bits to turn off



/*
 * Initialize EUSCI_B2 for I2C communication with LED drivers
 * No parameters
 */
void I2C_init(void);                        // Initialize I2C module



#endif /* LED_H_ */
