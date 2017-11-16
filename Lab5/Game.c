/*
 * Game.c
 *
 *  Created on: Oct 30, 2017
 *      Author: Alex Perera
 */

#include "Game.h"
#include "G8RTOS.h"
#include "driverlib.h"


volatile GeneralPlayerInfo_t host_p0;
volatile GeneralPlayerInfo_t client_p1;
volatile SpecificPlayerInfo_t client_info;
volatile GameState_t GameZ;


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
    // Initialize specific player client info //
    client_info.acknowledge = false;
    client_info.ready = false;
    client_info.joined = false;
    client_info.playerNumber = Client;
    client_info.displacement = PADDLE_X_CENTER;

    // Initialize CC3100 //
    initCC3100(Client);

    // Set the IP of the client by calling getLocalIP //
    client_info.IP_address = getLocalIP();

    // Performs handshaking UNTIL connection is made and acknowledge, ready, joined are changed and sent back and forth  //
    while(client_info.acknowledge == false || client_info.ready == false || client_info.joined == false)
        {
        int count = 0;

        SendData((_u8*)&client_info, HOST_IP_ADDR, sizeof(client_info));

        _i32 retval = -1;
        while(retval != 0 && count  < 10)
        {
            retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
            count++;
        }

        if(count >= 10) continue;
        count = 0;

        if(client_info.acknowledge == true)
        {
            LED_write(blue, 0x8000);
        }

        client_info.ready = true;
        SendData((_u8*)&client_info, HOST_IP_ADDR, sizeof(client_info));

        retval = -1;
        while(retval != 0 && count  < 10)
        {
            retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
            count++;
        }

        if(count >= 10) continue;
        count = 0;

        if(client_info.joined == true)
        {
            LED_write(green, 0x8000);
        }
    }

    // Receive client general player info from the host //
    _i32 retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&client_p1, sizeof(client_p1));
    }

    // Send the client general player info to the host for handshaking  //
    SendData((_u8*)&client_p1, HOST_IP_ADDR , sizeof(client_p1));

    // Receive the host specific player info from the host //
    retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&host_p0, sizeof(host_p0));
    }

    // Send the host specific player info to the host for handshaking  //
    SendData((_u8*)&host_p0, HOST_IP_ADDR , sizeof(host_p0));

    // Initialize the LCD with the PONG game board  //
    InitBoardState();

    // Initialize Semaphores //
    G8RTOS_InitSemaphore(&LCDMutex, 1);
    G8RTOS_InitSemaphore(&CC_3100Mutex, 1);
    G8RTOS_InitSemaphore(&GSMutex, 1);
    G8RTOS_InitSemaphore(&PlayerMutex, 1);

    // Add threads for the client to use  //
    G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
    G8RTOS_AddThread(ReadJoystickClient, "Read JoyClient", 200);
    G8RTOS_AddThread(SendDataToHost, "Send data to host", 100);
    G8RTOS_AddThread(ReceiveDataFromHost, "Rec data from host", 100);
    G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);
    G8RTOS_AddThread(IdleThread, "Idle", 255);

    // Kill JoinGame thread  //
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
    // Temporary gamestate to avoid overuse of semaphores  //
    GameState_t temp_gamestate;

    // Sleep at the start to prevent paddle draw error on restart  //
    G8RTOS_Sleep(300);

    while(1)
    {
        // Receive the Gamestate from the Host //
        _i32 retval = -1;
        while(retval != 0)
        {
            G8RTOS_WaitSemaphore(&CC_3100Mutex);
            retval = ReceiveData((_u8*)&temp_gamestate, sizeof(temp_gamestate));
            G8RTOS_SignalSemaphore(&CC_3100Mutex);
            // G8RTOS_Sleep(1);
        }

        // Check to make sure the IP Addresses are equal, this is an error check to make sure no garbage data is received //
        if(temp_gamestate.player.IP_address == GameZ.player.IP_address)
        {
            // Copy local Gamestate into global Gamestate //
            G8RTOS_WaitSemaphore(&GSMutex);
            GameZ = temp_gamestate;
            G8RTOS_SignalSemaphore(&GSMutex);

            // Check if gameDone boolean is true //
            if(temp_gamestate.gameDone == true)
            {
                // Add End of Game Client thread with highest priority  //
                G8RTOS_AddThread(EndOfGameClient, "End Game", 1);
            }
        }

        // Sleep for 3 ms, best synchronization //
        G8RTOS_Sleep(3);
    }
}


/*
 * Thread that sends UDP packets to host
•   Send player info
•   Sleep for 2ms
 */
void SendDataToHost()
{
    // Temporary Specific player info for client  //
    SpecificPlayerInfo_t temp_client;

    while(1)
    {
        // Copy global specific client info into temporary  //
        G8RTOS_WaitSemaphore(&PlayerMutex);
        temp_client = client_info;
        G8RTOS_SignalSemaphore(&PlayerMutex);

        // Send the specific client info to host  //
        G8RTOS_WaitSemaphore(&CC_3100Mutex);
        SendData((_u8*)&temp_client, HOST_IP_ADDR, sizeof(temp_client));
        G8RTOS_SignalSemaphore(&CC_3100Mutex);

        // Sleep for 2 ms, good for synchronization
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
        // Get joystick client coordinates //
        GetJoystickCoordinates(&x_coord, &y_coord);

        // Check to make sure paddle is within game arena //
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

        // Sleep for 10 ms //
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
    // Wait for all semaphores to be released //
    G8RTOS_WaitSemaphore(&GSMutex);
    G8RTOS_WaitSemaphore(&CC_3100Mutex);
    G8RTOS_WaitSemaphore(&PlayerMutex);
    G8RTOS_WaitSemaphore(&LCDMutex);

    // Kill all the other running threads //
    G8RTOS_KillAllOtherThreads();

    // Rewrite the final LED scores //
    uint16_t led_pattern = 0;
    for(int i = 0; i < GameZ.LEDScores[Host]; i++)
    {
        led_pattern = led_pattern >> 1;
        led_pattern |= 0x8000;
    }
    LED_write(red, led_pattern);
    led_pattern = 0;
    for(int i = 0; i < GameZ.LEDScores[Client]; i++)
    {
        led_pattern = led_pattern >> 1;
        led_pattern |= 0x8000;
    }
    LED_write(blue, led_pattern);

    // Reinitialize semaphores //
    G8RTOS_InitSemaphore(&PlayerMutex, 1);
    G8RTOS_InitSemaphore(&CC_3100Mutex, 1);
    G8RTOS_InitSemaphore(&GSMutex, 1);
    G8RTOS_InitSemaphore(&LCDMutex, 1);

    // Clear the LCD with the winner's color //
    if(GameZ.winner == Host)
    {
       LCD_Clear(PLAYER_RED);
    }
    else
    {
        LCD_Clear(PLAYER_BLUE);
    }

    // Print message for client to wait for host to restart //
    LCD_Text(0, (ARENA_MAX_Y>>1), "WAITING FOR HOST TO RESTART", LCD_BLACK);

    // Receive the Gamestate from the Host
    _i32 retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&GameZ, sizeof(GameZ));
    }



    // Redraw the Pong Arena
    InitBoardState();

    // Add all Client threads back //
    G8RTOS_AddThread(SendDataToHost, "Send data to host", 200);
    G8RTOS_AddThread(ReadJoystickClient, "Read JoyClient", 200);
    G8RTOS_AddThread(ReceiveDataFromHost, "Rec data from host", 200);
    G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);
    G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
    G8RTOS_AddThread(IdleThread, "Idle", 255);

    // Kill Self
    G8RTOS_KillSelf();
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

    // Initialize the players  //
    host_p0.color = PLAYER_RED;
    host_p0.currentCenter = PADDLE_X_CENTER;
    host_p0.position = BOTTOM;
    client_p1.color = PLAYER_BLUE;
    client_p1.currentCenter = PADDLE_X_CENTER;
    client_p1.position = TOP;

    // Initialize the CC3100 //
    initCC3100(Host);

    // Handshaking at first connect, send and receive acknowledge and ready //
    while(client_info.acknowledge == false || client_info.ready == false)
    {
        int count = 0;
        _i32 retval = -1;
        while(retval != 0 && count < 10)
        {
            retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
            count++;
        }

        if(count >= 10) continue;
        count = 0;

        client_info.acknowledge = true;

        SendData((_u8*)&client_info, client_info.IP_address, sizeof(client_info));

        retval = -1;
        while(retval != 0 && count < 10)
        {
            retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
            count++;
        }
        if(count >= 10) continue;
        count = 0;

        if(client_info.ready == true)
        {
            LED_write(red, 0x4000);
        }

        client_info.joined = true;

        SendData((_u8*)&client_info, client_info.IP_address, sizeof(client_info));
    }

    // Send the specific client info to the client, for handshaking //
    SendData((_u8*)&client_p1, client_info.IP_address , sizeof(client_p1));

    // Receive general client info from client //
    _i32 retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&client_p1, sizeof(client_p1));
    }

    // Send general host info to client //
    SendData((_u8*)&host_p0, client_info.IP_address , sizeof(host_p0));

    // Receive general host info from client, for handshaking //
    retval = -1;
    while(retval != 0)
    {
        retval = ReceiveData((_u8*)&host_p0, sizeof(host_p0));
    }

    // Initialize Pong arena //
    InitBoardState();

    // Initialize all the semaphores //
    G8RTOS_InitSemaphore(&LCDMutex, 1);
    G8RTOS_InitSemaphore(&CC_3100Mutex, 1);
    G8RTOS_InitSemaphore(&GSMutex, 1);
    G8RTOS_InitSemaphore(&PlayerMutex, 1);

    // Add all host threads //
    G8RTOS_AddThread(GenerateBall, "Gen Ball", 200);
    G8RTOS_AddThread(ReadJoystickHost, "R Joy Host", 200);
    G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
    G8RTOS_AddThread(ReceiveDataFromClient, "Rec from client", 200);
    G8RTOS_AddThread(SendDataToClient, "Send data to client", 200);
    G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);
    G8RTOS_AddThread(IdleThread, "Idle", 255);

    // Kill self //
    G8RTOS_KillSelf();
}

/*
 * Thread that sends game state to client
•   Fill packet for client
•   Send packet
•   Check if game is done
    o   If done, Add EndOfGameHost thread with highest priority
•   Sleep for 5ms (found experimentally to be a good amount of time for synchronization)
 */
void SendDataToClient()
{
    // Temporary gamestate to reduce semaphore use //
    GameState_t tempGamez;

    while(1)
    {
        // Copy global gamestate into local gamestate //
        G8RTOS_WaitSemaphore(&GSMutex);
        tempGamez = GameZ;
        G8RTOS_SignalSemaphore(&GSMutex);

        // Send the gamestate to the client //
        G8RTOS_WaitSemaphore(&CC_3100Mutex);
        SendData((_u8*)&tempGamez, tempGamez.player.IP_address, sizeof(tempGamez));
        G8RTOS_SignalSemaphore(&CC_3100Mutex);

        // Check if the game is done //
        if(tempGamez.gameDone == true)
        {
            // Add End of Game Host with highest priority //
            G8RTOS_AddThread(EndOfGameHost, "End Game", 1);
        }

        // Sleep for 5ms, good for synchronization //
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
    // Sleep at the beginning to prevent paddle error on restart //
    G8RTOS_Sleep(350);

    while(1)
    {
        // Receive the specific client info from client //
        _i32 retval = -1;
        while(retval != 0)
        {
            G8RTOS_WaitSemaphore(&CC_3100Mutex);
            retval = ReceiveData((_u8*)&client_info, sizeof(client_info));
            G8RTOS_SignalSemaphore(&CC_3100Mutex);
        }

        // Update gamestate with specific client info //
        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ.player = client_info;
        GameZ.players[Client].currentCenter = client_info.displacement;
        G8RTOS_SignalSemaphore(&GSMutex);

        // Sleep for 2ms, good for synchronization //
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
    // Temporary gamestate
    GameState_t temp_games;

    // Set bool to decide if to kill a ball //
    bool kill = false;

    // Copy gamestate into local temporary gamestate //
    temp_games = GameZ;

    // Go through balls array, looking for dead balls //
    // if no dead balls, kill self  //
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

    // Initialize ball to be created to alive and white
    temp_games.balls[i].alive = true;
    temp_games.balls[i].color = LCD_WHITE;

    // Randomize location of ball //
    temp_games.balls[i].currentCenterX = rand() % (HORIZ_CENTER_MAX_BALL - HORIZ_CENTER_MIN_BALL) + HORIZ_CENTER_MIN_BALL;
    temp_games.balls[i].currentCenterY = rand() % (VERT_CENTER_MAX_BALL - VERT_CENTER_MIN_BALL) + VERT_CENTER_MIN_BALL;

    // Randomize velocity of ball in x and y //
    int x_vel = (rand() % MAX_BALL_SPEED*2 + 1) - MAX_BALL_SPEED;
    int y_vel = (rand() % MAX_BALL_SPEED*2 + 1) - MAX_BALL_SPEED;

    // Copy local gamestate into global gamestate //
    GameZ = temp_games;

    // Initialize width and height for collision algorithm //
    int32_t w = WIDTH_TOP_OR_BOTTOM;
    int32_t h = HEIGHT_TOP_OR_BOTTOM;


    while(1)
    {
        // Copy global gamestate into local gamestate //
        G8RTOS_WaitSemaphore(&GSMutex);
        temp_games = GameZ;
        G8RTOS_SignalSemaphore(&GSMutex);

        // Calculate values for collision algorithm //
        int32_t dx_host = temp_games.balls[i].currentCenterX - temp_games.players[Host].currentCenter;
        int32_t dy_host = temp_games.balls[i].currentCenterY - (VERT_CENTER_MAX_BALL - BALL_SIZE);

        int32_t dx_client = temp_games.balls[i].currentCenterX - temp_games.players[Client].currentCenter;
        int32_t dy_client = temp_games.balls[i].currentCenterY -  (VERT_CENTER_MIN_BALL);

        // Check for collision on host (bottom) paddle //
        if (abs(dx_host) <= w && abs(dy_host) <= h)
        {
            /* collision! */
            int32_t wy = w * dy_host;
            int32_t hx = h * dx_host;

            temp_games.balls[i].color = PLAYER_RED;

            if (wy > hx)
                if (wy > -hx)
                {
                   /* collision at the top */
                    x_vel = 0;
                    y_vel = -1*y_vel - 1;
                    if(y_vel == 0)
                    {
                        y_vel -= 1;
                    }
                }
                else
                {
                    /* on the left */
                    y_vel = -1*y_vel - 1;
                    x_vel = -1*(abs(x_vel)+1);
                    if(y_vel == 0)
                    {
                        y_vel -= 1;
                    }
                }
            else
                if (wy > -hx)
                {
                    /* on the right */
                    y_vel = -1*y_vel - 1;
                    x_vel = (abs(x_vel)+1);
                    if(y_vel == 0)
                    {
                        y_vel -= 1;
                    }
                }
        }
        // Check for collision on client (top) paddle //
        else if(abs(dx_client) <= w && abs(dy_client) <= h)
        {
            int32_t wy = w * dy_client;
            int32_t hx = h * dx_client;

            temp_games.balls[i].color = PLAYER_BLUE;

            if (wy > hx)
                if (wy > -hx)
                {
                    x_vel = 0;
                    y_vel = -1*y_vel + 1;
                    if(y_vel == 0)
                    {
                        y_vel += 1;
                    }
                }

                else
                {
                    /* on the left */
                    y_vel = -1*y_vel + 1;
                    x_vel = -1*(abs(x_vel)+1);
                    if(y_vel == 0)
                    {
                        y_vel += 1;
                    }
                }
            else
                if (wy > -hx)
                {
                    /* on the right */
                    y_vel = -1*y_vel + 1;
                    x_vel = (abs(x_vel)+1);
                    if(y_vel == 0)
                    {
                        y_vel += 1;
                    }
                }
        }


        // Max and min velocity check //
        if(x_vel > MAX_BALL_SPEED)
        {
            x_vel = MAX_BALL_SPEED;
        }
        else if ( x_vel < -1*MAX_BALL_SPEED)
        {
            x_vel = -1*MAX_BALL_SPEED;
        }

        if(y_vel > MAX_BALL_SPEED)
        {
            y_vel = MAX_BALL_SPEED;
        }
        else if( y_vel < -1*MAX_BALL_SPEED)
        {
            y_vel = -1*MAX_BALL_SPEED;
        }

        // Update velocities in gamestate //
        temp_games.balls[i].currentCenterX += x_vel;
        temp_games.balls[i].currentCenterY += y_vel;

        // Check for collision on side of play arena //
        if(temp_games.balls[i].currentCenterX > HORIZ_CENTER_MAX_BALL)
        {
            temp_games.balls[i].currentCenterX = HORIZ_CENTER_MAX_BALL;
            x_vel = -1*x_vel;
        }
        if(temp_games.balls[i].currentCenterX < HORIZ_CENTER_MIN_BALL + 1)
        {
            temp_games.balls[i].currentCenterX = HORIZ_CENTER_MIN_BALL + 1;
            x_vel = -1*x_vel;
        }

        // Check if ball passed the bottom paddle //
        if(temp_games.balls[i].currentCenterY > VERT_CENTER_MAX_BALL + BALL_SIZE + 6)
        {
            // Set boolean kill to true, set alive to false //
            kill = true;
            temp_games.balls[i].alive = false;

            // Update LED score if the ball is owned by blue player //
            if(temp_games.balls[i].color == PLAYER_BLUE)
            {
                temp_games.LEDScores[Client]++;
            }
            // Decrement number of balls //
            temp_games.numberOfBalls--;
        }

        // Check if ball passed the top paddle //
        if(temp_games.balls[i].currentCenterY < VERT_CENTER_MIN_BALL - BALL_SIZE - 6)
        {
            // Set boolean kill to true, set alive to false //
            kill = true;
            temp_games.balls[i].alive = false;

            // Update LED score if the ball is owned by red player //
            if(temp_games.balls[i].color == PLAYER_RED)
            {
                temp_games.LEDScores[Host]++;
            }
            // Decrement number of balls //
            temp_games.numberOfBalls--;
        }

        // Check if the game is won and the game isn't over yet (bug fix for overall score inc twice) //
        if(temp_games.LEDScores[Host] >= ScoreToWin && temp_games.gameDone == false )
        {
            // Increment overall score, set gameDone to true, set game winner to host //
            temp_games.overallScores[Host]++;
            temp_games.gameDone = true;
            temp_games.winner = Host;
        }

        // Check if the game is won and the game isn't over yet (bug fix for overall score inc twice) //
        if(temp_games.LEDScores[Client] >= ScoreToWin && temp_games.gameDone == false )
        {
            // Increment overall score, set gameDone to true, set game winner to host //
            temp_games.overallScores[Client]++;
            temp_games.gameDone = true;
            temp_games.winner = Client;
        }

        // Store local gamestate back into global gamestate //
        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ = temp_games;
        G8RTOS_SignalSemaphore(&GSMutex);

        // If this ball is to be killed, set by check above //
        if(kill == true)
        {
            // Kill this ball thread //
            G8RTOS_KillSelf();
        }

        // Sleep for 35ms, refresh rate //
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
    // Local numballs variable //
    int numballs = 0;

    while(1)
    {
        // Copy global number of balls into local numballs //
        numballs = GameZ.numberOfBalls;

        // If max number of balls are not on the screen //
        if(numballs < MAX_NUM_OF_BALLS)
        {
            // Add another ball and increment number of balls //
            G8RTOS_AddThread(MoveBall, "MoveBall", 200);
            numballs++;
        }

        // Store local numballs into global gamestate number of balls //
        G8RTOS_WaitSemaphore(&GSMutex);
        GameZ.numberOfBalls = numballs;
        G8RTOS_SignalSemaphore(&GSMutex);

        // Sleep proportional to number of balls on the screen (0.5s) //
        G8RTOS_Sleep(500*numballs);
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
        // Joystick coordinates for the host //
        GetJoystickCoordinates(&x_coord, &y_coord);

        // Sleep before doing anything to make game fairer between host and client //
        G8RTOS_Sleep(10);

        // Update center of paddle based on x coordinate of joystick //
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
•   Once ready, send notification to client, reinitialize the game (adding back all the threads) and kill self
 */
void EndOfGameHost()
{

    // Wait for all the semaphores //
    G8RTOS_WaitSemaphore(&GSMutex);
    G8RTOS_WaitSemaphore(&CC_3100Mutex);
    G8RTOS_WaitSemaphore(&LCDMutex);
    G8RTOS_WaitSemaphore(&PlayerMutex);

    // Kill all the other running threads //
    G8RTOS_KillAllOtherThreads();

    // Rewrite the final LED scores //
    uint16_t led_pattern = 0;
    for(int i = 0; i < GameZ.LEDScores[Host]; i++)
    {
        led_pattern = led_pattern >> 1;
        led_pattern |= 0x8000;
    }
    LED_write(red, led_pattern);
    led_pattern = 0;
    for(int i = 0; i < GameZ.LEDScores[Client]; i++)
    {
        led_pattern = led_pattern >> 1;
        led_pattern |= 0x8000;
    }
    LED_write(blue, led_pattern);

    // Reinitialize all semaphores //
    G8RTOS_InitSemaphore(&GSMutex, 1);
    G8RTOS_InitSemaphore(&CC_3100Mutex, 1);
    G8RTOS_InitSemaphore(&LCDMutex, 1);
    G8RTOS_InitSemaphore(&PlayerMutex, 1);

    // Clear screen with winner color //
    if(GameZ.winner == Host)
    {
       LCD_Clear(PLAYER_RED);
    }
    else
    {
        LCD_Clear(PLAYER_BLUE);
    }

    // Print message for host to hit button to restart //
    LCD_Text(0, (ARENA_MAX_Y>>1), "PRESS B1 TO RESTART", LCD_BLACK);

    // Wait for Host to press button //
    GetPlayerRole();

    // Send the gamestate to the client //
    SendData((_u8*)&GameZ, GameZ.player.IP_address, sizeof(GameZ));



    while(1)
    {
        // Initialize pong arena //
        InitBoardState();

        // Add back all host threads //
        G8RTOS_AddThread(ReadJoystickHost, "R Joy Host", 200);
        G8RTOS_AddThread(SendDataToClient, "Send data to client", 100);
        G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);
        G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
        G8RTOS_AddThread(GenerateBall, "Gen Ball", 200);
        G8RTOS_AddThread(IdleThread, "Idle", 255);
        G8RTOS_AddThread(ReceiveDataFromClient, "Rec from client", 100);

        // Kill Self //
        G8RTOS_KillSelf();
    }
}
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
    // Previous paddle positions, local gamestate, and previous balls array //
    PrevPlayer_t prevhost_p0;
    PrevPlayer_t prevclient_p1;
    GameState_t temp_gamez;
    PrevBall_t prevballs[MAX_NUM_OF_BALLS];

    // Initialize previous paddle positions to the center, for restarting the game //
    prevhost_p0.Center = PADDLE_X_CENTER;
    prevclient_p1.Center = PADDLE_X_CENTER;

    // Initialize previous balls positions to outside the arena //
    for(int i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        prevballs[i].CenterX = ARENA_MIN_X - 4;
        prevballs[i].CenterY = ARENA_MIN_X - 4;
    }


    while(1)
    {
        // Copy global gamestate into local temporary gamestate, reduce semaphore usage //
        G8RTOS_WaitSemaphore(&GSMutex);
        temp_gamez = GameZ;
        G8RTOS_SignalSemaphore(&GSMutex);

        // Draw new ball and erase old ball, iterate through array of balls //
        //G8RTOS_WaitSemaphore(&LCDMutex);
        for(int i = 0; i < MAX_NUM_OF_BALLS; i++)
        {
            // if the ball is alive, erase old ball, draw new ball and update previous position //
            if(temp_gamez.balls[i].alive == true)
            {
                if((prevballs[i].CenterY + BALL_SIZE_D2 < VERT_CENTER_MAX_BALL - BALL_SIZE_D2) && (prevballs[i].CenterY - BALL_SIZE_D2 > VERT_CENTER_MIN_BALL) )
                {
                    LCD_DrawRectangle(prevballs[i].CenterX - BALL_SIZE_D2, prevballs[i].CenterX + BALL_SIZE_D2, prevballs[i].CenterY - BALL_SIZE_D2, prevballs[i].CenterY + BALL_SIZE_D2, LCD_BLACK);

                }
                else
                {
                    if(((prevballs[i].CenterX - BALL_SIZE_D2 < temp_gamez.players[Host].currentCenter - PADDLE_LEN_D2) ||  (prevballs[i].CenterX - BALL_SIZE_D2 > temp_gamez.players[Host].currentCenter + PADDLE_LEN_D2)) && prevballs[i].CenterY > ARENA_MAX_Y >> 1)
                    {
                        LCD_DrawRectangle(prevballs[i].CenterX - BALL_SIZE_D2, prevballs[i].CenterX + BALL_SIZE_D2, prevballs[i].CenterY - BALL_SIZE_D2, prevballs[i].CenterY + BALL_SIZE_D2, LCD_BLACK);
                    }
                    else if(((prevballs[i].CenterX - BALL_SIZE_D2 < temp_gamez.players[Client].currentCenter - PADDLE_LEN_D2) || (prevballs[i].CenterX - BALL_SIZE_D2 > temp_gamez.players[Client].currentCenter + PADDLE_LEN_D2)) && (prevballs[i].CenterY <= ARENA_MAX_Y >> 1))
                    {
                        LCD_DrawRectangle(prevballs[i].CenterX - BALL_SIZE_D2, prevballs[i].CenterX + BALL_SIZE_D2, prevballs[i].CenterY - BALL_SIZE_D2, prevballs[i].CenterY + BALL_SIZE_D2, LCD_BLACK);
                    }
                    if(!(prevballs[i].CenterY + BALL_SIZE_D2 < VERT_CENTER_MAX_BALL - BALL_SIZE_D2))
                    {
                        LCD_DrawRectangle(prevballs[i].CenterX - BALL_SIZE_D2, prevballs[i].CenterX + BALL_SIZE_D2, prevballs[i].CenterY - BALL_SIZE_D2, VERT_CENTER_MAX_BALL - BALL_SIZE_D2, LCD_BLACK);
                    }
                    else if(!(prevballs[i].CenterY - BALL_SIZE_D2 > VERT_CENTER_MIN_BALL))
                    {
                        LCD_DrawRectangle(prevballs[i].CenterX - BALL_SIZE_D2, prevballs[i].CenterX + BALL_SIZE_D2, VERT_CENTER_MIN_BALL, prevballs[i].CenterY + BALL_SIZE_D2, LCD_BLACK);
                    }
                }
                prevballs[i].CenterX = temp_gamez.balls[i].currentCenterX;
                prevballs[i].CenterY = temp_gamez.balls[i].currentCenterY;
                LCD_DrawRectangle(temp_gamez.balls[i].currentCenterX - BALL_SIZE_D2, temp_gamez.balls[i].currentCenterX + BALL_SIZE_D2, temp_gamez.balls[i].currentCenterY - BALL_SIZE_D2, temp_gamez.balls[i].currentCenterY + BALL_SIZE_D2, temp_gamez.balls[i].color);
            }
        }
        //G8RTOS_SignalSemaphore(&LCDMutex);

        // Store temporary general player info in gamestate to specific player info //
        host_p0 = temp_gamez.players[Host];
        client_p1 = temp_gamez.players[Client];

        // Draw head of new paddle, erase tail of old paddle //
        UpdatePlayerOnScreen((PrevPlayer_t *) &prevhost_p0, (GeneralPlayerInfo_t *)&host_p0);
        UpdatePlayerOnScreen(&prevclient_p1, &client_p1);

        // Update previous paddle positions //
        prevhost_p0.Center = host_p0.currentCenter;
        prevclient_p1.Center = client_p1.currentCenter;

        // Sleep for 12 ms //
        G8RTOS_Sleep(10);
    }
}

/*
 * Thread to update LEDs based on score
 *
•   Responsible for updating the LED array with current scores
 */
void MoveLEDs()
{
    // Array for previous LEDs //
    uint16_t prev_leds[2] = {0,0};

    // Local temporary gamestate //
    GameState_t temp_gamez;

    // Clear the LEDs on intialization of this thread //
    LED_clear(0xFFFF);

    while(1)
    {
        // Copy global gamestate into local temporary gamestate //
        G8RTOS_WaitSemaphore(&GSMutex);
        temp_gamez = GameZ;
        G8RTOS_SignalSemaphore(&GSMutex);

        // If the Host LED scores changed //
        if(temp_gamez.LEDScores[Host] != prev_leds[Host] )
        {
            // Update previous LED scores //
            prev_leds[Host] = temp_gamez.LEDScores[Host];

            // If the previous LED scores hit max, set it to max //
            if(prev_leds[Host] > 16) prev_leds[Host] = 16;

            // Display LED scores //
            uint16_t led_pattern = 0;
            for(int i = 0; i < prev_leds[Host]; i++)
            {
                led_pattern = led_pattern >> 1;
                led_pattern |= 0x8000;
            }
            LED_write(red, led_pattern);
        }

        // If the client LED scores changed //
        if(temp_gamez.LEDScores[Client] != prev_leds[Client])
        {
            // Update previous LED scores //
            prev_leds[Client] = temp_gamez.LEDScores[Client];

            // If the previous LED scores hit max, set it to max //
            if(prev_leds[Client] > 16) prev_leds[Client] = 16;

            // Display LED scores //
            uint16_t led_pattern = 0;
            for(int i = 0; i < prev_leds[Client]; i++)
            {
                led_pattern = led_pattern >> 1;
                led_pattern |= 0x8000;
            }
            LED_write(blue, led_pattern);
        }
        // Sleep for 100ms, thread low priority //
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
    // Initialize buttons //
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN4|GPIO_PIN5);

    // Poll until button pressed //
    while(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN4) && GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5));


    // Return Host or Client depending on button press //
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
    // Difference between previous position center and new position center //
    int16_t offset = outPlayer->currentCenter - prevPlayerIn->Center;

    // Draw new head of paddle and erase old tail of paddle, check for direction to erase and draw in proper locations //
    if(outPlayer->position == TOP)
    {
        // RIGHT //
        if(offset > 0)
        {
            //G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 , outPlayer->currentCenter - PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2 , outPlayer->currentCenter + PADDLE_LEN_D2, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, client_p1.color);
            //G8RTOS_SignalSemaphore(&LCDMutex);
        }
        // LEFT //
        else if (offset < 0)
        {
            //G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2 , prevPlayerIn->Center - PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, client_p1.color);
            //G8RTOS_SignalSemaphore(&LCDMutex);
        }
    }
    else
    {
        if(offset > 0)
        {
            //G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 , outPlayer->currentCenter - PADDLE_LEN_D2 , ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, LCD_BLACK);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2, outPlayer->currentCenter + PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, host_p0.color);
            //G8RTOS_SignalSemaphore(&LCDMutex);
        }
        else if (offset < 0)
        {
            //G8RTOS_WaitSemaphore(&LCDMutex);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 , ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, LCD_BLACK);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, prevPlayerIn->Center - PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y, host_p0.color);
            //G8RTOS_SignalSemaphore(&LCDMutex);
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
    // Reintialize all general player info //
    host_p0.currentCenter = PADDLE_X_CENTER;
    client_p1.currentCenter = PADDLE_X_CENTER;
    client_info.displacement = PADDLE_X_CENTER;

    // Reinitialize gamestate //
    GameZ.player = client_info;
    GameZ.players[Host] = host_p0;
    GameZ.players[Client] = client_p1;
    GameZ.numberOfBalls = 0;
    GameZ.winner = false;
    GameZ.gameDone = false;
    GameZ.LEDScores[Host] = 0;
    GameZ.LEDScores[Client] = 0;

    GameZ.player.displacement = PADDLE_X_CENTER;

    // Kill all balls and set color to white //
    for(int i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        GameZ.balls[i].alive = false;
        GameZ.balls[i].color = LCD_WHITE;
    }

    // Clear the LCD //
    LCD_Clear(LCD_BLACK);

    // Draw the pong arena //
    LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MIN_Y, ARENA_MAX_Y, LCD_BLACK);           // Draw square black arena
    LCD_DrawRectangle(ARENA_MIN_X, ARENA_MIN_X+1, ARENA_MIN_Y, ARENA_MAX_Y, LCD_WHITE);         // Draw left edge of arena in white
    LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X+1, ARENA_MIN_Y, ARENA_MAX_Y, LCD_WHITE);         // Draw right edge of arena in white
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MIN_Y, ARENA_MIN_Y+1, LCD_WHITE);         // Draw top edge of arena in white
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MAX_Y-1, ARENA_MAX_Y, LCD_WHITE);         // Draw bottom edge of arena in white

    // Draw initial paddles //
    LCD_DrawRectangle(client_p1.currentCenter - PADDLE_LEN_D2 , client_p1.currentCenter + PADDLE_LEN_D2 , ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, client_p1.color);   // Client player paddle
    LCD_DrawRectangle(host_p0.currentCenter - PADDLE_LEN_D2, host_p0.currentCenter + PADDLE_LEN_D2, ARENA_MAX_Y-PADDLE_WID, ARENA_MAX_Y , host_p0.color);     // Host player paddle

    // Display overall scores on LCD //
    char buffer_c[2];
    char buffer_h[2];
    sprintf(buffer_c, "%02d", GameZ.overallScores[Client] );
    sprintf(buffer_h, "%02d", GameZ.overallScores[Host] );
    LCD_Text(MIN_SCREEN_X + 10, MIN_SCREEN_Y + 5, (uint8_t*)buffer_c, client_p1.color);      // Client score
    LCD_Text(MIN_SCREEN_X + 10, MAX_SCREEN_Y - 20, (uint8_t*)buffer_h, host_p0.color);        // Host score

}


void StartMenu()
{
    // Display start menu with button layout //
    LCD_Text((MAX_SCREEN_X>>1)-64, (MAX_SCREEN_Y>>4), "WELCOME TO PONG!" , LCD_WHITE);

    LCD_DrawRectangle((MIN_SCREEN_X + (MAX_SCREEN_X>>2)-32), (MIN_SCREEN_X +(MAX_SCREEN_X>>2)+32), (MAX_SCREEN_Y>>1)-24, (MAX_SCREEN_Y>>1)+40, LCD_WHITE);
    LCD_DrawRectangle((MAX_SCREEN_X>>1)-32, (MAX_SCREEN_X>>1)+32, (MAX_SCREEN_Y - (MAX_SCREEN_Y>>2)-24), (MAX_SCREEN_Y - (MAX_SCREEN_Y>>2)+40), LCD_WHITE);

    LCD_DrawRectangle((MAX_SCREEN_X>>1)-32, (MAX_SCREEN_X>>1)+32, (MAX_SCREEN_Y>>2)-24, (MAX_SCREEN_Y>>2)+40, LCD_RED);
    LCD_DrawRectangle((MAX_SCREEN_X - (MAX_SCREEN_X>>2)-32), (MAX_SCREEN_X - (MAX_SCREEN_X>>2)+32), (MAX_SCREEN_Y>>1)-24, (MAX_SCREEN_Y>>1)+40, LCD_BLUE);

    LCD_Text((MAX_SCREEN_X>>1)-16, (MAX_SCREEN_Y>>2), "HOST" , LCD_BLACK);
    LCD_Text((MAX_SCREEN_X - (MAX_SCREEN_X>>2)-24), (MAX_SCREEN_Y>>1), "CLIENT" , LCD_BLACK);

    // Initialize overall scores, only happens at beginning of program //
    GameZ.overallScores[Host] = 0;
    GameZ.overallScores[Client] = 0;
}

