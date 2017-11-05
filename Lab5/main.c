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
	G8RTOS_Init();


	LED_clear(0xFFFF);



	if(GetPlayerRole() == Host)
	{
	    LED_write(blue, 0x00F0);
	    G8RTOS_AddThread(CreateGame, "Host Create", 100);
	}
	else
	{
	    LED_write(red, 0x0F00);
	    G8RTOS_AddThread(JoinGame, "Client Join", 100);
	}
	G8RTOS_Launch();


	while(1)
	{
	}

}
