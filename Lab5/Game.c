/*
 * Game.c
 *
 *  Created on: Oct 30, 2017
 *      Author: Alex Perera
 */

#include "Game.h"
#include "G8RTOS.h"
#include "driverlib.h"

GeneralPlayerInfo_t host_p0;
GeneralPlayerInfo_t client_p1;
SpecificPlayerInfo_t client_info;
GameState_t GameZ;


semaphore_t LCDMutex;
semaphore_t CC_3100Mutex;
semaphore_t PlayerMutex;
semaphore_t GSMutex;
/*********************************************** Client Threads *********************************************************************/
/*
 * Thread for client to join game
 *
•   Only thread to run after launching the OS
•   Set initial SpecificPlayerInfo_t struct attributes (you can get the IP address by calling getLocalIP()
•   Send player info to the host
•   Wait for server response
•   If you’ve joined the game, acknowledge you’ve joined to the host and show connection with an LED
•   Initialize the board state, semaphores, and add the following threads
    o   ReadJoystickClient
    o   SendDataToHost
    o   ReceiveDataFromHost
    o   DrawObjects
    o   MoveLEDs
    o   Idle
•   Kill self
 *
 */
void JoinGame()
{
    client_info.acknowledge = false;
    client_info.ready = false;
    client_info.joined = false;
    client_info.playerNumber = Client;
    client_info.displacement = PADDLE_X_CENTER;
    initCC3100(Client);
    client_info.IP_address = getLocalIP();


    SendData((_u8*)&client_info, HOST_IP_ADDR, sizeof(client_info));

    _i32 retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
    }

    if(client_info.acknowledge == true)
    {
        LED_write(blue, 0x8000);
    }

    client_info.ready = true;
    SendData((_u8*)&client_info, HOST_IP_ADDR, sizeof(client_info));

    retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
    }

    if(client_info.joined == true)
    {
        LED_write(green, 0x8000);
    }

    retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&client_p1, sizeof(client_p1));
    }

    retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&host_p0, sizeof(host_p0));
    }


    InitBoardState();



    // TODO initialize semaphores and add more threads
    G8RTOS_InitSemaphore(&LCDMutex, 1);
    G8RTOS_InitSemaphore(&CC_3100Mutex, 1);
    G8RTOS_InitSemaphore(&GSMutex, 1);
    G8RTOS_InitSemaphore(&PlayerMutex, 1);
    G8RTOS_AddThread(IdleThread, "Idle", 255);
    //G8RTOS_Sleep(3000);

    G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
    G8RTOS_AddThread(ReadJoystickClient, "Read JoyClient", 200);
    G8RTOS_AddThread(SendDataToHost, "Send data to host", 200);
    G8RTOS_AddThread(ReceiveDataFromHost, "Receive data host", 200);
    G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);

    G8RTOS_KillSelf();

}


/*
 * Thread that receives game state packets from host
 *
•  Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
        o   Note: Remember to release and take the semaphore again so you’re still able to send data
•   Empty the received packet
•   If the game is done, add EndOfGameClient thread with the highest priority
•   Sleep for 5ms
 *
 */
void ReceiveDataFromHost()
{
    GameState_t temp_gamestate;
    while(1)
    {

        _i32 retval = -1;
        while(retval != 0)
        {
            G8RTOS_WaitSemaphore(&CC_3100Mutex);
            retval = ReceiveData((_u8*)&temp_gamestate, sizeof(temp_gamestate));
            G8RTOS_SignalSemaphore(&CC_3100Mutex);
            G8RTOS_Sleep(1);
        }

        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ = temp_gamestate;
        G8RTOS_SignalSemaphore(&GSMutex);

        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ.player = client_info;
        GameZ.players[Client].currentCenter = client_info.displacement;
        //GameZ.LEDScores[Host]++;
        G8RTOS_SignalSemaphore(&GSMutex);

        if(temp_gamestate.gameDone == true)
        {
            G8RTOS_AddThread(EndOfGameClient, "End Game", 1);
        }

        G8RTOS_Sleep(5);

    }


}


/*
 * Thread that sends UDP packets to host
 *
•   Send player info
•   Sleep for 2ms

 */
void SendDataToHost()
{
    SpecificPlayerInfo_t temp_client;

    while(1)
    {
        G8RTOS_WaitSemaphore(&PlayerMutex);
        temp_client = client_info;
        G8RTOS_SignalSemaphore(&PlayerMutex);


        G8RTOS_WaitSemaphore(&CC_3100Mutex);

        SendData((_u8*)&temp_client, HOST_IP_ADDR, sizeof(temp_client));

        G8RTOS_SignalSemaphore(&CC_3100Mutex);

        G8RTOS_Sleep(2);

    }

}

/*
 * Thread to read client's joystick
 *
 */
void ReadJoystickClient()
{
    int16_t x_coord;
    int16_t y_coord;

    while(1)
    {
        GetJoystickCoordinates(&x_coord, &y_coord);

        G8RTOS_WaitSemaphore(&PlayerMutex);
        client_info.displacement -= x_coord/2000;
        if(client_info.displacement   > ARENA_MAX_X - PADDLE_LEN_D2 )
        {
            client_info.displacement = ARENA_MAX_X - PADDLE_LEN_D2;
        }
        else if(client_info.displacement < ARENA_MIN_X + PADDLE_LEN_D2 + 1)
        {
            client_info.displacement = ARENA_MIN_X + PADDLE_LEN_D2 + 1;
        }
        G8RTOS_SignalSemaphore(&PlayerMutex);

        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ.player = client_info;
        GameZ.players[Client].currentCenter = client_info.displacement;
        //GameZ.LEDScores[Host]++;
        G8RTOS_SignalSemaphore(&GSMutex);

        G8RTOS_Sleep(10);

    }
}

/*
 * End of game for the client
 *
•  Wait for all semaphores to be released
•   Kill all semaphores
•   Re-initialize semaphores
•   Clear screen with winner’s color
•   Wait for host to restart game
•   Add all threads back
•   Kill Self
 */
void EndOfGameClient()
{
    while(1);
}

/*********************************************** Client Threads *********************************************************************/


/*********************************************** Host Threads *********************************************************************/
/*
 * Thread for the host to create a game
 *
•   Only thread created before launching the OS
•   Initializes the players
•   Establish connection with client (use an LED on the Launchpad to indicate Wi-Fi connection)
    o   Should be trying to receive a packet from the client
    o   Should acknowledge client once client has joined
•   Initialize the board (draw arena, players, and scores)
•   Add the following threads:
    o   GenerateBall
    o   DrawObjects
    o   ReadJoystickHost
    o   SendDataToClient
    o   ReceiveDataFromClient
    o   MoveLEDs (lower priority)
    o   Idle
•   Kill self
 */
void CreateGame()
{

    /* Initialize the players  */
    host_p0.color = PLAYER_RED;
    host_p0.currentCenter = PADDLE_X_CENTER;
    host_p0.position = BOTTOM;

    client_p1.color = PLAYER_BLUE;
    client_p1.currentCenter = PADDLE_X_CENTER;
    client_p1.position = TOP;

    initCC3100(Host);

    _i32 retval = -1;
    while(retval != 0)
    {

        retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
    }

    client_info.acknowledge = true;

    SendData((_u8*)&client_info, client_info.IP_address, sizeof(client_info));

    retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
    }


    if(client_info.ready == true)
    {
        LED_write(red, 0x4000);
    }

    client_info.joined = true;

    SendData((_u8*)&client_info, client_info.IP_address, sizeof(client_info));

    SendData((_u8*)&client_p1, client_info.IP_address , sizeof(client_p1));

    SendData((_u8*)&host_p0, client_info.IP_address , sizeof(host_p0));



    InitBoardState();


    // TODO initialize semaphores and add more threads

    G8RTOS_InitSemaphore(&LCDMutex, 1);
    G8RTOS_InitSemaphore(&CC_3100Mutex, 1);
    G8RTOS_InitSemaphore(&GSMutex, 1);
    //G8RTOS_InitSemaphore(&PlayerMutex, 1);

    G8RTOS_AddThread(IdleThread, "Idle", 255);
    //G8RTOS_Sleep(3000);

    G8RTOS_AddThread(GenerateBall, "Gen Ball", 200);
    G8RTOS_AddThread(ReadJoystickHost, "R Joy Host", 200);
    G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
    G8RTOS_AddThread(ReceiveDataFromClient, "Rec from client", 200);
    G8RTOS_AddThread(SendDataToClient, "Send data to client", 200);
    G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);

    G8RTOS_KillSelf();
}

/*
 * Thread that sends game state to client
 *
•   Fill packet for client
•   Send packet
•   Check if game is done
    o   If done, Add EndOfGameHost thread with highest priority
•   Sleep for 5ms (found experimentally to be a good amount of time for synchronization)
 */
void SendDataToClient()
{
    GameState_t tempGamez;
    while(1)
    {
        G8RTOS_WaitSemaphore(&GSMutex);
        tempGamez = GameZ;
        G8RTOS_SignalSemaphore(&GSMutex);

        G8RTOS_WaitSemaphore(&CC_3100Mutex);
        SendData((_u8*)&tempGamez, tempGamez.player.IP_address, sizeof(tempGamez));
        G8RTOS_SignalSemaphore(&CC_3100Mutex);


        if(tempGamez.gameDone == true)
        {
            G8RTOS_AddThread(EndOfGameClient, "End Game", 1);
        }

        G8RTOS_Sleep(5);

    }
}

/*
 * Thread that receives UDP packets from client
•   Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
    o   Note: Remember to release and take the semaphore again so you’re still able to send data
•   Update the player’s current center with the displacement received from the client
•   Sleep for 2ms (again found experimentally)
 */
void ReceiveDataFromClient()
{

    while(1)
    {

        _i32 retval = -1;
        while(retval != 0)
        {
            G8RTOS_WaitSemaphore(&CC_3100Mutex);
            retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
            G8RTOS_SignalSemaphore(&CC_3100Mutex);
            G8RTOS_Sleep(1);
        }

        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ.player = client_info;
        GameZ.players[Client].currentCenter = client_info.displacement;
        //GameZ.LEDScores[Host]++;
        G8RTOS_SignalSemaphore(&GSMutex);


        G8RTOS_Sleep(2);
    }
}
/*
 * Thread to move a single ball
•   Go through array of balls and find one that’s not alive
•   Once found, initialize random position and X and Y velocities, as well as color and alive attributes
•   Checking for collision given the current center and the velocity
•   If collision occurs, adjust velocity and color accordingly
•   If the ball passes a player, adjust score, account for the game possibly ending, and kill self
•   Otherwise, just move the ball in its current direction according to its velocity
•   Sleep for 35ms
 */
void MoveBall()
{
    GameState_t temp_games;
    bool kill = false;

    G8RTOS_WaitSemaphore(&GSMutex);
    temp_games = GameZ;
    G8RTOS_SignalSemaphore(&GSMutex);

    srand(time(NULL));

    int i;
    for(i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        if(temp_games.balls[i].alive == false)
        {
            break;
        }
        else if(i == MAX_NUM_OF_BALLS-1)
        {
            G8RTOS_KillSelf();
        }
    }

    temp_games.balls[i].alive = true;
    temp_games.balls[i].color = LCD_WHITE;

    temp_games.balls[i].currentCenterX = rand() % (HORIZ_CENTER_MAX_BALL - HORIZ_CENTER_MIN_BALL) + HORIZ_CENTER_MIN_BALL;
    temp_games.balls[i].currentCenterY = rand() % (VERT_CENTER_MAX_BALL - VERT_CENTER_MIN_BALL) + VERT_CENTER_MIN_BALL;

    int x_vel = (rand() % MAX_BALL_SPEED*2 + 1) - MAX_BALL_SPEED;
    int y_vel = (rand() % MAX_BALL_SPEED*2 + 1) - MAX_BALL_SPEED;

    G8RTOS_WaitSemaphore(&GSMutex);
    GameZ = temp_games;
    G8RTOS_SignalSemaphore(&GSMutex);


    while(1)
    {
        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ.balls[i].currentCenterX += x_vel;
        GameZ.balls[i].currentCenterY += y_vel;

        if(GameZ.balls[i].currentCenterX > HORIZ_CENTER_MAX_BALL)
        {
            GameZ.balls[i].currentCenterX = HORIZ_CENTER_MAX_BALL;
            x_vel = -1*x_vel;
        }
        if(GameZ.balls[i].currentCenterX < HORIZ_CENTER_MIN_BALL + 1)
        {
            GameZ.balls[i].currentCenterX = HORIZ_CENTER_MIN_BALL + 1;
            x_vel = -1*x_vel;
        }
        if(GameZ.balls[i].currentCenterY > VERT_CENTER_MAX_BALL)
        {
            kill = true;
            GameZ.balls[i].alive = false;

            if(GameZ.balls[i].color == LCD_BLUE)
            {
                GameZ.LEDScores[Client]++;
            }
            GameZ.numberOfBalls--;
        }
        if(GameZ.balls[i].currentCenterY < VERT_CENTER_MIN_BALL)
        {
            kill = true;
            GameZ.balls[i].alive = false;

            if(GameZ.balls[i].color == LCD_RED)
            {
                GameZ.LEDScores[Host]++;
            }
            GameZ.numberOfBalls--;
        }

        G8RTOS_SignalSemaphore(&GSMutex);

        if(kill == true)
        {
            G8RTOS_KillSelf();
        }

        G8RTOS_Sleep(35);
    }
}

/*
 * Generate Ball thread
•   Adds another MoveBall thread if the number of balls is less than the max
•   Sleeps proportional to the number of balls currently in play
 */
void GenerateBall()
{
    int numballs = 0;
    while(1)
    {
        G8RTOS_WaitSemaphore(&GSMutex);

        if(GameZ.numberOfBalls < MAX_NUM_OF_BALLS)
        {
            G8RTOS_AddThread(MoveBall, "MoveBall", 200);
            GameZ.numberOfBalls++;
            numballs = GameZ.numberOfBalls;
        }

        G8RTOS_SignalSemaphore(&GSMutex);

        G8RTOS_Sleep(2000*numballs);
    }
}

/*
 * Thread to read host's joystick
 *
•   You can read the joystick ADC values by calling GetJoystickCoordinates
•   You’ll need to add a bias to the values (found experimentally) since every joystick is offset by some small amount displacement and noise
•   Change Self.displacement accordingly (you can experiment with how much you want to scale the ADC value)
•   Sleep for 10ms
•   Then add the displacement to the bottom player in the list of players (general list that’s sent to the client and used for drawing)
•   By sleeping before updating the bottom player’s position, it makes the game more fair between client and host
 */
void ReadJoystickHost()
{
    int16_t x_coord;
    int16_t y_coord;

    while(1)
    {
        GetJoystickCoordinates(&x_coord, &y_coord);

        G8RTOS_Sleep(10);



        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ.players[Host].currentCenter -= x_coord/2000;
        if(GameZ.players[Host].currentCenter   > ARENA_MAX_X - PADDLE_LEN_D2 )
        {
            GameZ.players[Host].currentCenter = ARENA_MAX_X - PADDLE_LEN_D2;
        }
        else if(GameZ.players[Host].currentCenter < ARENA_MIN_X + PADDLE_LEN_D2 + 1)
        {
            GameZ.players[Host].currentCenter = ARENA_MIN_X + PADDLE_LEN_D2 + 1;
        }
        G8RTOS_SignalSemaphore(&GSMutex);

    }
}



/*
 * End of game for the host
•   Wait for all the semaphores to be released
•   Kill all other threads (you’ll need to make a new function in the scheduler for this)
•   Re-initialize semaphores
•   Clear screen with the winner’s color
•   Print some message that waits for the host’s action to start a new game
•   Create an aperiodic thread that waits for the host’s button press (the client will just be waiting on the host to start a new game
•   Once ready, send notification to client, reinitialize the game (adding back all the threads) and kill self
 */
void EndOfGameHost();

/*********************************************** Host Threads *********************************************************************/


/*********************************************** Common Threads *********************************************************************/
/*
 * Idle thread
 */
void IdleThread()
{
    while(1);
}

/*
 * Thread to draw all the objects in the game
 *
•   Should hold arrays of previous players and ball positions
•   Draw and/or update balls (you’ll need a way to tell whether to draw a new ball, or update its position (i.e. if a new ball has just been created – hence the alive attribute in the Ball_t struct.
•   Update players
•   Sleep for 20ms (reasonable refresh rate)
 */
void DrawObjects()
{
    PrevPlayer_t prevhost_p0;
    PrevPlayer_t prevclient_p1;
    GameState_t temp_gamez;
    PrevBall_t prevballs[MAX_NUM_OF_BALLS];

    prevhost_p0.Center = host_p0.currentCenter;
    prevclient_p1.Center = client_p1.currentCenter;

    for(int i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        prevballs[i].CenterX = ARENA_MIN_X - 4;
        prevballs[i].CenterY = ARENA_MIN_X - 4;
    }


    while(1)
    {
        G8RTOS_WaitSemaphore(&GSMutex);
        temp_gamez = GameZ;
        G8RTOS_SignalSemaphore(&GSMutex);



        //G8RTOS_WaitSemaphore(&LCDMutex);
        for(int i = 0; i < MAX_NUM_OF_BALLS; i++)
        {
            if(temp_gamez.balls[i].alive == true)
            {
                LCD_DrawRectangle(prevballs[i].CenterX - BALL_SIZE_D2, prevballs[i].CenterX + BALL_SIZE_D2, prevballs[i].CenterY - BALL_SIZE_D2, prevballs[i].CenterY + BALL_SIZE_D2, LCD_BLACK);
                prevballs[i].CenterX = temp_gamez.balls[i].currentCenterX;
                prevballs[i].CenterY = temp_gamez.balls[i].currentCenterY;
                LCD_DrawRectangle(temp_gamez.balls[i].currentCenterX - BALL_SIZE_D2, temp_gamez.balls[i].currentCenterX + BALL_SIZE_D2, temp_gamez.balls[i].currentCenterY - BALL_SIZE_D2, temp_gamez.balls[i].currentCenterY + BALL_SIZE_D2, temp_gamez.balls[i].color);
            }
        }
        //G8RTOS_SignalSemaphore(&LCDMutex);

        host_p0 = temp_gamez.players[Host];
        client_p1 = temp_gamez.players[Client];

        UpdatePlayerOnScreen(&prevhost_p0, &host_p0);
        UpdatePlayerOnScreen(&prevclient_p1, &client_p1);

        prevhost_p0.Center = host_p0.currentCenter;
        prevclient_p1.Center = client_p1.currentCenter;

        G8RTOS_Sleep(20);

    }


}

/*
 * Thread to update LEDs based on score
 *
•   Responsible for updating the LED array with current scores
 */
void MoveLEDs()
{
    uint16_t prev_leds[2] = {0,0};
    GameState_t temp_gamez;
    LED_clear(0xFFFF);

    while(1)
    {
        G8RTOS_WaitSemaphore(&GSMutex);
        temp_gamez = GameZ;
        G8RTOS_SignalSemaphore(&GSMutex);

        if(temp_gamez.LEDScores[Host] != prev_leds[Host] )
        {
            prev_leds[Host] = temp_gamez.LEDScores[Host];

            if(prev_leds[Host] > 16) prev_leds[Host] = 16;


            uint16_t led_pattern = 0;

            for(int i = 0; i < prev_leds[Host]; i++)
            {
                led_pattern = led_pattern >> 1;
                led_pattern |= 0x8000;
            }


            LED_write(red, led_pattern);
        }

        if(GameZ.LEDScores[Client] != prev_leds[Client])
        {

            prev_leds[Client] = temp_gamez.LEDScores[Client];

            if(prev_leds[Client] > 16) prev_leds[Client] = 16;

            uint16_t led_pattern = 0;
            for(int i = 0; i < prev_leds[Client]; i++)
            {
                led_pattern = led_pattern >> 1;
                led_pattern |= 0x8000;
            }

            LED_write(blue, led_pattern);
        }

        G8RTOS_Sleep(100);

    }

}

/*********************************************** Common Threads *********************************************************************/


/*********************************************** Public Functions *********************************************************************/
/*
 * Returns either Host or Client depending on button press
 */
playerType GetPlayerRole()
{
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN4|GPIO_PIN5);

    while(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN4) && GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5));

    if(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN4) == GPIO_INPUT_PIN_LOW)
    {
        return Host;
    }
    else
    {
        return Client;
    }

}

/*
 * Draw players given center X center coordinate
 */
void DrawPlayer(GeneralPlayerInfo_t * player)
{


}

/*
 * Updates player's paddle based on current and new center
 */
void UpdatePlayerOnScreen(PrevPlayer_t * prevPlayerIn, GeneralPlayerInfo_t * outPlayer)
{
    int16_t offset = outPlayer->currentCenter - prevPlayerIn->Center;


    if(outPlayer->position == TOP)
    {
        //LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID , LCD_BLACK);
        //LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, outPlayer->currentCenter + PADDLE_LEN_D2, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, client_p1.color);
        if(offset > 0)
        {
            G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 , outPlayer->currentCenter - PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2, outPlayer->currentCenter + PADDLE_LEN_D2, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, client_p1.color);
            G8RTOS_SignalSemaphore(&LCDMutex);
        }
        else if (offset < 0)
        {
            G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, prevPlayerIn->Center - PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, client_p1.color);
            G8RTOS_SignalSemaphore(&LCDMutex);
        }

    }
    else
    {
        //LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y , LCD_BLACK);
        //LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, outPlayer->currentCenter + PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, host_p0.color);
        if(offset > 0)
        {
            G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 , outPlayer->currentCenter - PADDLE_LEN_D2 , ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, LCD_BLACK);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2, outPlayer->currentCenter + PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, host_p0.color);
            G8RTOS_SignalSemaphore(&LCDMutex);
        }
        else if (offset < 0)
        {
            G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 , ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, LCD_BLACK);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, prevPlayerIn->Center - PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, host_p0.color);
            G8RTOS_SignalSemaphore(&LCDMutex);
        }

    }


}

/*
 * Function updates ball position on screen
 */
void UpdateBallOnScreen(PrevBall_t * previousBall, Ball_t * currentBall, uint16_t outColor);

/*
 * Initializes and prints initial game state
 */
void InitBoardState()
{
    GameZ.player = client_info;
    GameZ.players[Host] = host_p0;
    GameZ.players[Client] = client_p1;
    GameZ.numberOfBalls = 0;
    GameZ.winner = false;
    GameZ.gameDone = false;
    GameZ.LEDScores[Host] = 0;
    GameZ.LEDScores[Client] = 0;
    GameZ.overallScores[Host] = 0;
    GameZ.overallScores[Client] = 0;

    LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MIN_Y, ARENA_MAX_Y, LCD_BLACK);           // Draw square black arena
    LCD_DrawRectangle(ARENA_MIN_X, ARENA_MIN_X+1, ARENA_MIN_Y, ARENA_MAX_Y, LCD_WHITE);         // Draw left edge of arena in white
    LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X+1, ARENA_MIN_Y, ARENA_MAX_Y, LCD_WHITE);         // Draw right edge of arena in white
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MIN_Y, ARENA_MIN_Y+1, LCD_WHITE);         // Draw top edge of arena in white
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MAX_Y-1, ARENA_MAX_Y, LCD_WHITE);         // Draw bottom edge of arena in white

    LCD_DrawRectangle(client_p1.currentCenter - PADDLE_LEN_D2 , client_p1.currentCenter + PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, client_p1.color);   // Client player paddle
    LCD_DrawRectangle(host_p0.currentCenter - PADDLE_LEN_D2, host_p0.currentCenter + PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y , host_p0.color);     // Host player paddle

    LCD_Text(MIN_SCREEN_X + 10, MIN_SCREEN_Y + 5, "00", client_p1.color);      // Client score
    LCD_Text(MIN_SCREEN_X + 10, MAX_SCREEN_Y - 20, "00", host_p0.color);        // Host score

}
