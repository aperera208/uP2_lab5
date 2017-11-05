/**
 *
 * main.c
 *
 * Lab 3
 */


#include "G8RTOS.h"







void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	G8RTOS_Init();

	G8RTOS_Launch();

	while(1)
	{

	}

}
