/**
 *
 * main.c
 *
 * Lab 3
 */


#include "G8RTOS.h"




// TEST COMMENT #2

// ALEX COMMENT TO MERGE

// Nick's comment to merge

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	BSP_InitBoard();
	//G8RTOS_Init();

	//G8RTOS_Launch();

	LED_clear(0xFFFF);



	if(GetPlayerRole() == Host)
	{
	    LED_write(blue, 0x00F0);
	}
	else
	{
	    LED_write(red, 0x0F00);
	}


	while(1)
	{
	}

}
