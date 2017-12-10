/*
 * FinalProj.c
 *
 *  Created on: Dec 7, 2017
 *      Author: Alex Perera
 */

#include "G8RTOS.h"
#include "driverlib.h"
#include "FinalProj.h"


volatile GeneralPlayerInfo_t host_p0;
volatile GeneralPlayerInfo_t client_p1;
volatile Wifi_Info_t client_info;
volatile GameState_t Game;
 PrevBullet_t prevBullets;
//prevBullets.CenterX = 0;
//prevBulletss.CenterY = 0;

semaphore_t LCDMutex;
semaphore_t CC_3100Mutex;
semaphore_t PlayerMutex;
semaphore_t GSMutex;

char buffer_c[2];
uint8_t count = 0;

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

    // Initialize the LCD with the game board  //
    InitBoardState();

    // Initialize Semaphores //
    G8RTOS_InitSemaphore(&LCDMutex, 1);
    G8RTOS_InitSemaphore(&CC_3100Mutex, 1);
    G8RTOS_InitSemaphore(&GSMutex, 1);
    G8RTOS_InitSemaphore(&PlayerMutex, 1);

    // Add threads for the client to use  //
    /*


    G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);
       */
    G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
    G8RTOS_AddThread(Read_Joystick_Button_Client, "Read JoyClient", 200);
    G8RTOS_AddThread(IdleThread, "Idle", 255);
    G8RTOS_AddThread(SendDataToHost, "Send data to host", 100);
    G8RTOS_AddThread(ReceiveDataFromHost, "Rec data from host", 100);

    // Kill JoinGame thread  //
    G8RTOS_KillSelf();
}

/*
 * Thread to read client's joystick
 *
 */
void Read_Joystick_Button_Client()
{

    int16_t x_coord;
    int16_t y_coord;

    int16_t x_temp;
    int16_t y_temp;

    int16_t x_update;
    int16_t y_update;
    shipOrientation orient_temp;

    GeneralPlayerInfo_t temp_client;

    while(1)
    {

        // Copy global gamestate into local gamestate //
        G8RTOS_WaitSemaphore(&GSMutex);
        x_temp = Game.players[Client].x_center;
        y_temp = Game.players[Client].y_center;
        G8RTOS_SignalSemaphore(&GSMutex);


        x_update = 0;
        y_update = 0;

        // Get joystick client coordinates //
        GetJoystickCoordinates(&x_coord, &y_coord);

        if( abs(x_coord) < 3000)
        {
            x_coord = 0;
        }
        if( abs(y_coord) < 3000)
        {
            y_coord = 0;
        }

        // DOWN LEFT
        if(x_coord > 0 && y_coord > 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED2;
            y_update += y_coord/SPEED2;
            orient_temp = down_left;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // UP RIGHT
        else if(x_coord > 0 && y_coord < 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED2;
            y_update += y_coord/SPEED2;
            orient_temp = up_left;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // UP LEFT
        else if( x_coord < 0 && y_coord < 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED2;
            y_update += y_coord/SPEED2;
            orient_temp = up_right;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // DOWN LEFT
        else if(x_coord < 0 && y_coord > 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED2;
            y_update += y_coord/SPEED2;
            orient_temp = down_right;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // LEFT
        else if(x_coord < 0 && y_coord == 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/(SPEED2-500);
            y_update += y_coord/(SPEED2-500);
            orient_temp = right;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // RIGHT
        else if(x_coord > 0 && y_coord == 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/(SPEED2-500);
            y_update += y_coord/(SPEED2-500);
            orient_temp = left;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // DOWN
        else if(x_coord == 0 && y_coord > 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED2;
            y_update += y_coord/SPEED2;
            orient_temp = down;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // UP
        else if(x_coord == 0 && y_coord < 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED2;
            y_update += y_coord/SPEED2;
            orient_temp = up;
            G8RTOS_SignalSemaphore(&GSMutex);
        }

        if(((y_update + y_temp - 10 < MIN_SCREEN_Y) && ( (orient_temp == up) || (orient_temp == up_left)|| (orient_temp == up_right) )))
        {
            y_update = 0;
        }
        if(((y_temp + y_update + 10 > MAX_SCREEN_Y) && ( (orient_temp == down) || (orient_temp == down_left)|| (orient_temp == down_right) )))
        {
            y_update = 0;
        }
        if(((x_temp + x_update - 10 < MIN_SCREEN_X) && ( (orient_temp == left) || (orient_temp == up_left)|| (orient_temp == down_left) )))
        {
            x_update = 0;
        }
        if(((x_temp + x_update + 10 > MAX_SCREEN_X) && ( (orient_temp == right) || (orient_temp == up_right)|| (orient_temp == down_right) )))
        {
            x_update = 0;
        }

        G8RTOS_WaitSemaphore(&GSMutex);
        temp_client = Game.players[Client];
        G8RTOS_SignalSemaphore(&GSMutex);

        temp_client.x_center += x_update;
        temp_client.y_center += y_update;
        temp_client.rotation = orient_temp;

        G8RTOS_WaitSemaphore(&PlayerMutex);
        client_p1 = temp_client;
        G8RTOS_SignalSemaphore(&PlayerMutex);

        /*
         *         G8RTOS_WaitSemaphore(&GSMutex);

        Game.players[Client].x_center += x_update;
        Game.players[Client].y_center += y_update;
        Game.players[Client].rotation = orient_temp;

        G8RTOS_SignalSemaphore(&GSMutex);
 */

        // Sleep for 10 ms //
        G8RTOS_Sleep(10);
    }
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
        //if(temp_gamestate.player.IP_address == GameZ.player.IP_address)
       // {
            // Copy local Gamestate into global Gamestate //
            G8RTOS_WaitSemaphore(&GSMutex);
            Game = temp_gamestate;
            G8RTOS_SignalSemaphore(&GSMutex);

            // Check if gameDone boolean is true //
            if(temp_gamestate.gameDone == true)
            {
                // Add End of Game Client thread with highest priority  //
                G8RTOS_AddThread(EndOfGameClient, "End Game", 1);
            }
        //}

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
   // Temporary General player info for client  //
    GeneralPlayerInfo_t temp_client;

   while(1)
   {
       // Copy global specific client info into temporary  //
       G8RTOS_WaitSemaphore(&PlayerMutex);
       // TODO: Maybe add mutex for general player info
       temp_client = client_p1; //Game.players[Client];
       G8RTOS_SignalSemaphore(&PlayerMutex);

       // Send the specific client info to host  //
       G8RTOS_WaitSemaphore(&CC_3100Mutex);
       SendData((_u8*)&temp_client, HOST_IP_ADDR, sizeof(temp_client));
       G8RTOS_SignalSemaphore(&CC_3100Mutex);

       // Sleep for 2 ms, good for synchronization
       G8RTOS_Sleep(2);
   }
}

void EndOfGameClient()
{
    while(1);
}


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

/*    host_p0.color = PLAYER_RED;
    host_p0.currentCenter = PADDLE_X_CENTER;
    host_p0.position = BOTTOM;
    client_p1.color = PLAYER_BLUE;
    client_p1.currentCenter = PADDLE_X_CENTER;
    client_p1.position = TOP;*/





    client_info.acknowledge = false;
    client_info.ready = false;
    //client_info.joined = false;

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
    /*
    G8RTOS_AddThread(GenerateBall, "Gen Ball", 200);

    G8RTOS_AddThread(MoveLEDs, "LED Thread", 250);
    */


    G8RTOS_AddThread(IdleThread, "Idle", 255);
    G8RTOS_AddThread(DrawObjects, "Draw Objects", 200);
    G8RTOS_AddThread(Read_Joystick_Button_Host, "R Joy Host", 200);
    G8RTOS_AddThread(ReceiveDataFromClient, "Rec from client", 200);
    G8RTOS_AddThread(SendDataToClient, "Send data to client", 200);
    G8RTOS_AddThread(MoveBullets, "Move Bullets", 200);
    G8RTOS_AddPeriodicEvent(periodic_button_host, 250);

    // Kill self //
    G8RTOS_KillSelf();
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
void Read_Joystick_Button_Host()
{
    int16_t x_coord;
    int16_t y_coord;

    int16_t x_temp;
    int16_t y_temp;

    int16_t x_update;
    int16_t y_update;
    shipOrientation orient_temp;

    while(1)
    {
        // Copy global gamestate into local gamestate //
        G8RTOS_WaitSemaphore(&GSMutex);
        x_temp = Game.players[Host].x_center;
        y_temp = Game.players[Host].y_center;
        G8RTOS_SignalSemaphore(&GSMutex);


        x_update = 0;
        y_update = 0;
        // Joystick coordinates for the host //
        GetJoystickCoordinates(&x_coord, &y_coord);


        // Sleep before doing anything to make game fairer between host and client //
        G8RTOS_Sleep(10);


       // x_diff = x_temp - x_coord;
       // y_diff = y_temp - y_coord;

        if( abs(x_coord) < 3000)
        {
            x_coord = 0;
        }
        if( abs(y_coord) < 3000)
        {
            y_coord = 0;
        }

        // DOWN LEFT
        if(x_coord > 0 && y_coord > 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = down_left;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // UP RIGHT
        else if(x_coord > 0 && y_coord < 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = up_left;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // UP LEFT
        else if( x_coord < 0 && y_coord < 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = up_right;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // DOWN LEFT
        else if(x_coord < 0 && y_coord > 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = down_right;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // LEFT
        else if(x_coord < 0 && y_coord == 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = right;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // RIGHT
        else if(x_coord > 0 && y_coord == 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = left;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // DOWN
        else if(x_coord == 0 && y_coord > 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = down;
            G8RTOS_SignalSemaphore(&GSMutex);
        }
        // UP
        else if(x_coord == 0 && y_coord < 0)
        {
            G8RTOS_WaitSemaphore(&GSMutex);
            x_update -= x_coord/SPEED;
            y_update += y_coord/SPEED;
            orient_temp = up;
            G8RTOS_SignalSemaphore(&GSMutex);
        }

        if(((y_update + y_temp - 10 < MIN_SCREEN_Y) && ( (orient_temp == up) || (orient_temp == up_left)|| (orient_temp == up_right) )))
        {
            y_update = 0;
        }
        if(((y_temp + y_update + 10 > MAX_SCREEN_Y) && ( (orient_temp == down) || (orient_temp == down_left)|| (orient_temp == down_right) )))
        {
            y_update = 0;
        }
        if(((x_temp + x_update - 10 < MIN_SCREEN_X) && ( (orient_temp == left) || (orient_temp == up_left)|| (orient_temp == down_left) )))
        {
            x_update = 0;
        }
        if(((x_temp + x_update + 10 > MAX_SCREEN_X) && ( (orient_temp == right) || (orient_temp == up_right)|| (orient_temp == down_right) )))
        {
            x_update = 0;
        }

        G8RTOS_WaitSemaphore(&GSMutex);
        Game.players[Host].x_center += x_update;
        Game.players[Host].y_center += y_update;
        Game.players[Host].rotation = orient_temp;
        G8RTOS_SignalSemaphore(&GSMutex);





        /*
        if(GameZ.players[Host].currentCenter   > ARENA_MAX_X - PADDLE_LEN_D2 )
        {
            GameZ.players[Host].currentCenter = ARENA_MAX_X - PADDLE_LEN_D2;
        }
        else if(GameZ.players[Host].currentCenter < ARENA_MIN_X + PADDLE_LEN_D2 + 1)
        {
            GameZ.players[Host].currentCenter = ARENA_MIN_X + PADDLE_LEN_D2 + 1;
        }
        */




    }
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
        tempGamez = Game;
        G8RTOS_SignalSemaphore(&GSMutex);

        // Send the gamestate to the client //
        G8RTOS_WaitSemaphore(&CC_3100Mutex);
        SendData((_u8*)&tempGamez, client_info.IP_address, sizeof(tempGamez));
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
            retval = ReceiveData((_u8*)&client_p1, sizeof(client_p1));
            G8RTOS_SignalSemaphore(&CC_3100Mutex);
        }

        // Update gamestate with general client info //
        G8RTOS_WaitSemaphore(&GSMutex);
        Game.players[Client] = client_p1;
        G8RTOS_SignalSemaphore(&GSMutex);

        // Sleep for 2ms, good for synchronization //
        G8RTOS_Sleep(2);
    }
}

void EndOfGameHost()
{
    while(1);
}


void periodic_button_host()
{
    if(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN4) == GPIO_INPUT_PIN_LOW)
    {
        LED_write(blue, ++count);
        //G8RTOS_AddThread(GenerateBulletHost, "Bullet Gen", 200);
        Game.players[Host].state |= SHIELD;
    }
    else if(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5) == GPIO_INPUT_PIN_LOW)
    {
        LED_write(green, ++count);
        G8RTOS_AddThread(GenerateBulletHost, "Bullet Gen", 200);
        Game.players[Host].bullet_request = normal_shot;
    }
    else if(GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN5) == GPIO_INPUT_PIN_LOW)
    {
        LED_write(red, ++count);
        //G8RTOS_AddThread(GenerateBulletHost, "Bullet Gen", 200);
        Game.players[Host].bullet_request = spread_shot;
    }
    else if(GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4) == GPIO_INPUT_PIN_LOW)
    {
        LED_write(blue, ++count);
        //G8RTOS_AddThread(GenerateBulletHost, "Bullet Gen", 200);
        Game.players[Host].bullet_request = charged0;
    }
}

void GenerateBulletHost()
{

    G8RTOS_WaitSemaphore(&GSMutex);
    GameState_t temp_game = Game;
    G8RTOS_SignalSemaphore(&GSMutex);

    if (temp_game.numberOfbullets >= MAX_NUM_OF_BULLETS)
    {
        G8RTOS_KillSelf();
    }

    if( temp_game.players[Host].bullet_request == normal_shot)
    {
        for(int i = 0; i < MAX_NUM_OF_BULLETS; i++)
        {
            if(temp_game.bullets[i].alive == false)
            {
                temp_game.bullets[i].alive = true;
                temp_game.bullets[i].bullet_type = normal_shot;
                if(temp_game.players[Host].rotation == up)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center - 7;
                    temp_game.bullets[i].x_vel = 0;
                    temp_game.bullets[i].y_vel = -1;
                    break;

                }
                else if(temp_game.players[Host].rotation == down)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center + 7;
                    temp_game.bullets[i].x_vel = 0;
                    temp_game.bullets[i].y_vel = 1;
                    break;

                }
                else if(temp_game.players[Host].rotation == left)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center - 7;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center;
                    temp_game.bullets[i].x_vel = -1;
                    temp_game.bullets[i].y_vel = 0;
                    break;

                }
                else if(temp_game.players[Host].rotation == right)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center + 7;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center;
                    temp_game.bullets[i].x_vel = 1;
                    temp_game.bullets[i].y_vel = 0;
                    break;

                }
                else if(temp_game.players[Host].rotation == up_left)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center - 4;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center - 4;
                    temp_game.bullets[i].x_vel = -1;
                    temp_game.bullets[i].y_vel = -1;
                    break;

                }
                else if(temp_game.players[Host].rotation == up_right)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center + 4;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center - 4;
                    temp_game.bullets[i].x_vel = 1;
                    temp_game.bullets[i].y_vel = -1;
                    break;

                }
                else if(temp_game.players[Host].rotation == down_left)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center - 4;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center + 4;
                    temp_game.bullets[i].x_vel = -1;
                    temp_game.bullets[i].y_vel = 1;
                    break;
                }
                else if(temp_game.players[Host].rotation == down_right)
                {
                    temp_game.bullets[i].x_center = temp_game.players[Host].x_center + 4;
                    temp_game.bullets[i].y_center = temp_game.players[Host].y_center + 4;
                    temp_game.bullets[i].x_vel = 1;
                    temp_game.bullets[i].y_vel = 1;
                    break;
                }

            }

        }
        temp_game.numberOfbullets++;


    }
    /*else if(request = spread_shot)
    {

    }
    else if(request = charged0)
    {

    }*/


    G8RTOS_WaitSemaphore(&GSMutex);
    Game = temp_game;
    G8RTOS_SignalSemaphore(&GSMutex);


    G8RTOS_KillSelf();
}

void MoveBullets()
{
    GameState_t temp_game;

    while(1)
    {
        G8RTOS_WaitSemaphore(&GSMutex);
        temp_game = Game;
        G8RTOS_SignalSemaphore(&GSMutex);

        for(int i = 0; i < MAX_NUM_OF_BULLETS; i++)
        {
            if(temp_game.bullets[i].alive == true)
            {
                temp_game.bullets[i].x_center += temp_game.bullets[i].x_vel;
                temp_game.bullets[i].y_center += temp_game.bullets[i].y_vel;

                if(((temp_game.bullets[i].x_center > MAX_SCREEN_X) || (temp_game.bullets[i].x_center < MIN_SCREEN_X)) || ((temp_game.bullets[i].y_center > MAX_SCREEN_Y) || (temp_game.bullets[i].y_center < MIN_SCREEN_Y) ))
                {
                    temp_game.bullets[i].alive = false;
                    temp_game.numberOfbullets--;
                }
            }
        }

        G8RTOS_WaitSemaphore(&GSMutex);
        Game = temp_game;
        G8RTOS_SignalSemaphore(&GSMutex);

        G8RTOS_Sleep(20);
    }

}





void DrawObjects()
{
    PrevPlayer_t prevhost_p0;
    PrevPlayer_t prevclient_p1;

    GameState_t temp_game;

    prevhost_p0.CenterX = (ARENA_MAX_X >> 1) + 25;
    prevhost_p0.CenterY = (ARENA_MAX_Y >> 1);
    prevhost_p0.rotation = down_right;

    prevclient_p1.CenterX = (ARENA_MAX_X >> 1) - 25;
    prevclient_p1.CenterY = (ARENA_MAX_Y >> 1);
    prevclient_p1.rotation = up_left;

    GeneralPlayerInfo_t client_temp;

    while(1)
    {
        // Copy global gamestate into local temporary gamestate, reduce semaphore usage //
        G8RTOS_WaitSemaphore(&GSMutex);
        temp_game = Game;
        G8RTOS_SignalSemaphore(&GSMutex);


        for(int i = 0; i < MAX_NUM_OF_BULLETS; i++)
        {
            if(temp_game.bullets[i].alive == true)
            {
                if(temp_game.bullets[i].bullet_type == normal_shot)
                {
                    LCD_DrawRectangle(temp_game.prevbullets[i].CenterX - BULLETD2 , temp_game.prevbullets[i].CenterX + BULLETD2 , temp_game.prevbullets[i].CenterY - BULLETD2, temp_game.prevbullets[i].CenterY + BULLETD2, LCD_BLACK);
                    LCD_DrawRectangle(temp_game.bullets[i].x_center - BULLETD2, temp_game.bullets[i].x_center + BULLETD2 , temp_game.bullets[i].y_center - BULLETD2 , temp_game.bullets[i].y_center + BULLETD2 , LCD_PINK);

                    temp_game.prevbullets[i].CenterX = temp_game.bullets[i].x_center;
                    temp_game.prevbullets[i].CenterY = temp_game.bullets[i].y_center;

                }
            }
        }



        host_p0 = temp_game.players[Host];
        client_temp = temp_game.players[Client];

        if((prevhost_p0.CenterX != host_p0.x_center) || (prevhost_p0.CenterY != host_p0.y_center) || (prevhost_p0.rotation != host_p0.rotation))
        {
            UpdatePlayerOnScreen((PrevPlayer_t *) &prevhost_p0, (GeneralPlayerInfo_t *)&host_p0);
        }

        if((abs(host_p0.x_center - client_temp.x_center) < SHIP_COLL) && (abs(host_p0.y_center - client_temp.y_center) < SHIP_COLL))
        {
            UpdatePlayerOnScreen((PrevPlayer_t *) &prevhost_p0, (GeneralPlayerInfo_t *)&host_p0);
            UpdatePlayerOnScreen(&prevclient_p1, &client_temp);
        }

        if((prevclient_p1.CenterX != client_temp.x_center) || (prevclient_p1.CenterY != client_temp.y_center) || (prevclient_p1.rotation != client_temp.rotation))
        {
            UpdatePlayerOnScreen(&prevclient_p1, &client_temp);
        }

        if(((host_p0.x_center < MIN_SCREEN_X + 50) && (host_p0.y_center < MIN_SCREEN_Y + 50)) || ((client_temp.x_center < MIN_SCREEN_X + 50) && (client_temp.y_center < MIN_SCREEN_Y + 50)))
        {
            LCD_Text(MIN_SCREEN_X + 10, MIN_SCREEN_Y + 5, (uint8_t*)buffer_c, LCD_ORANGE);      // Client score
        }

        // Update previous  positions //
        prevhost_p0.CenterX = host_p0.x_center;
        prevhost_p0.CenterY = host_p0.y_center;
        prevhost_p0.rotation = host_p0.rotation;

        prevclient_p1.CenterX = client_temp.x_center;
        prevclient_p1.CenterY = client_temp.y_center;
        prevclient_p1.rotation = client_temp.rotation;





        // Sleep for 10 ms //
        G8RTOS_Sleep(15);
    }


}



/*
 * Initializes and prints initial game state
*/
void InitBoardState()
{
    // Reintialize all general player info //
    host_p0.x_center = (ARENA_MAX_X >> 1) + 25;
    host_p0.y_center = (ARENA_MAX_Y >> 1);
    host_p0.rotation = down_right;
    host_p0.color = PLAYER_RED;
    host_p0.bullet_request = no_bullet;
    host_p0.HP = MAX_HP;
    host_p0.state = ALIVE | VIS | NO_SHIELD;

    client_p1.x_center = (ARENA_MAX_X >> 1) - 25;
    client_p1.y_center = (ARENA_MAX_Y >> 1);
    client_p1.rotation = up_left;
    client_p1.color = PLAYER_BLUE;
    client_p1.bullet_request = no_bullet;
    client_p1.HP = MAX_HP;
    client_p1.state = ALIVE | VIS | NO_SHIELD;

    // Reinitialize gamestate //
    Game.players[Host] = host_p0;
    Game.players[Client] = client_p1;
    Game.numberOfasteroids = 0;
    Game.numberOfbullets = 0;
    Game.gameDone = false;
    Game.Score = 0;



    // Kill all asteroids and set color to white //
    for(int i = 0; i < MAX_NUM_OF_ASTEROIDS; i++)
    {
        Game.asteroids[i].alive = false;

    }

    // Kill all bullets and set color to white //
    for(int i = 0; i < MAX_NUM_OF_BULLETS; i++)
    {
        Game.bullets[i].bullet_type = no_bullet;
        Game.prevbullets[i].CenterX = 400;//prevBullets.CenterX;
        Game.prevbullets[i].CenterY = 400;//prevBullets.CenterY;
        Game.bullets[i].x_center = 0;
        Game.bullets[i].y_center = 0;
        Game.bullets[i].x_vel = 0;
        Game.bullets[i].y_vel = 0;
        Game.bullets[i].alive = false;
    }
    // Clear the LCD //
    LCD_Clear(LCD_BLACK);

    // Draw the pong arena //
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MIN_Y, ARENA_MAX_Y, LCD_BLACK);           // Draw square black arena
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MIN_X+1, ARENA_MIN_Y, ARENA_MAX_Y, LCD_WHITE);         // Draw left edge of arena in white
    //LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X+1, ARENA_MIN_Y, ARENA_MAX_Y, LCD_WHITE);         // Draw right edge of arena in white
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MIN_Y, ARENA_MIN_Y+1, LCD_WHITE);         // Draw top edge of arena in white
    //LCD_DrawRectangle(ARENA_MIN_X, ARENA_MAX_X, ARENA_MAX_Y-1, ARENA_MAX_Y, LCD_WHITE);         // Draw bottom edge of arena in white

    // Draw initial paddles //
    //LCD_DrawRectangle(client_p1.x_center - sizediv2 , client_p1.x_center + sizediv2 , client_p1.y_center - sizediv2, client_p1.y_center + sizediv2, client_p1.color);   // Client player paddle
    //LCD_DrawRectangle(host_p0.x_center - sizediv2 , host_p0.x_center + sizediv2 , host_p0.y_center - sizediv2, host_p0.y_center + sizediv2 , host_p0.color);     // Host player paddle
    DrawPlayer(&host_p0);
    DrawPlayer(&client_p1);


    // Display overall scores on LCD //
   // char buffer_h[2];
    sprintf(buffer_c, "%02d", Game.Score);
    //sprintf(buffer_h, "%02d", GameZ.overallScores[Host] );
    LCD_Text(MIN_SCREEN_X + 10, MIN_SCREEN_Y + 5, (uint8_t*)buffer_c, LCD_ORANGE);      // Client score
    //LCD_Text(MIN_SCREEN_X + 10, MAX_SCREEN_Y - 20, (uint8_t*)buffer_h, host_p0.color);        // Host score

}


/*
 *
 */
void ErasePlayer( PrevPlayer_t * player)
{
    switch(player->rotation)
    {
        case up:
            for(int i = 1; i < 15; i++)
            {
                LCD_DrawRectangle(player->CenterX - i, player->CenterX + i, player->CenterY - 6 + i,  player->CenterY - 5 + i, LCD_BLACK);
            }
        break;


        case down:
            for(int i = 1; i < 15; i++)
            {
               LCD_DrawRectangle(player->CenterX - i, player->CenterX + i, player->CenterY + 5 - i,  player->CenterY + 6 - i, LCD_BLACK);
            }
        break;

        case right:
            for(int i = 1; i < 15; i++)
            {
               LCD_DrawRectangle( player->CenterX + 5 - i, player->CenterX + 6 - i,player->CenterY - i, player->CenterY + i , LCD_BLACK);
            }
            break;

        case left:
            for(int i = 1; i < 15; i++)
            {
                LCD_DrawRectangle(player->CenterX - 6 + i, player->CenterX - 5 + i,player->CenterY - i ,  player->CenterY + i, LCD_BLACK);
            }
            break;

        case down_right:
            for(int i = 1; i < 20; i++)
            {
                LCD_DrawRectangle(player->CenterX - i + 5, player->CenterX + 5, player->CenterY - 6 + i,  player->CenterY - 5 + i, LCD_BLACK);
            }
            break;

        case down_left:
            for(int i = 1; i < 20; i++)
            {
                LCD_DrawRectangle(player->CenterX - 5, player->CenterX - 5 + i, player->CenterY - 6 + i,  player->CenterY - 5 + i, LCD_BLACK);
            }
            break;

        case up_right:
            for(int i = 0; i < 20; i++)
            {
                LCD_DrawRectangle(player->CenterX - 6 + i, player->CenterX - 5 + i, player->CenterY - 6,  player->CenterY - 5 + i, LCD_BLACK);
            }
            break;


        case up_left:
            for(int i = 0; i < 20; i++)
            {
                LCD_DrawRectangle(player->CenterX - 7 , player->CenterX - 6 + i, player->CenterY - 7,  player->CenterY + 6 - i, LCD_BLACK);
            }
            break;


        default:
            break;

    }


}

/*
 *
 */
void DrawPlayer(volatile GeneralPlayerInfo_t * player)
{
    switch(player->rotation)
    {
        case up:
            for(int i = 1; i < 10; i++)
            {
                LCD_DrawRectangle(player->x_center - i, player->x_center + i, player->y_center - 6 + i,  player->y_center - 5 + i, player->color);
            }
        break;


        case down:
            for(int i = 1; i < 10; i++)
            {
               LCD_DrawRectangle(player->x_center - i, player->x_center + i, player->y_center + 5 - i,  player->y_center + 6 - i, player->color);
            }
        break;

        case right:
            for(int i = 1; i < 10; i++)
            {
               LCD_DrawRectangle( player->x_center + 5 - i, player->x_center + 6 - i,player->y_center - i, player->y_center + i , player->color);
            }
            break;

        case left:
            for(int i = 1; i < 10; i++)
            {
                LCD_DrawRectangle(player->x_center - 6 + i, player->x_center - 5 + i,player->y_center - i ,  player->y_center + i, player->color);
            }
            break;

        case down_right:
            for(int i = 1; i < 14; i++)
            {
                LCD_DrawRectangle(player->x_center - i + 5, player->x_center + 5, player->y_center - 6 + i,  player->y_center - 5 + i, player->color);
            }
            break;

        case down_left:
            for(int i = 1; i < 14; i++)
            {
                LCD_DrawRectangle(player->x_center - 5, player->x_center - 5 + i, player->y_center - 6 + i,  player->y_center - 5 + i, player->color);
            }
            break;

        case up_right:
            for(int i = 0; i < 14; i++)
            {
                LCD_DrawRectangle(player->x_center - 6 + i, player->x_center - 5 + i, player->y_center - 6,  player->y_center - 5 + i, player->color);
            }
            break;


        case up_left:
            for(int i = 0; i < 15; i++)
            {
                LCD_DrawRectangle(player->x_center - 7 , player->x_center - 6 + i, player->y_center - 7,  player->y_center + 6 - i, player->color);
            }
            break;


        default:
            break;

    }


}


void StartMenu()
{
    // Display start menu with button layout //
    LCD_Text((MAX_SCREEN_X>>1)-64, (MAX_SCREEN_Y>>4), "WELCOME TO PONG!", LCD_WHITE);

    LCD_DrawRectangle((MIN_SCREEN_X + (MAX_SCREEN_X>>2)-32), (MIN_SCREEN_X +(MAX_SCREEN_X>>2)+32), (MAX_SCREEN_Y>>1)-24, (MAX_SCREEN_Y>>1)+40, LCD_WHITE);
    LCD_DrawRectangle((MAX_SCREEN_X>>1)-32, (MAX_SCREEN_X>>1)+32, (MAX_SCREEN_Y - (MAX_SCREEN_Y>>2)-24), (MAX_SCREEN_Y - (MAX_SCREEN_Y>>2)+40), LCD_WHITE);

    LCD_DrawRectangle((MAX_SCREEN_X>>1)-32, (MAX_SCREEN_X>>1)+32, (MAX_SCREEN_Y>>2)-24, (MAX_SCREEN_Y>>2)+40, LCD_RED);
    LCD_DrawRectangle((MAX_SCREEN_X - (MAX_SCREEN_X>>2)-32), (MAX_SCREEN_X - (MAX_SCREEN_X>>2)+32), (MAX_SCREEN_Y>>1)-24, (MAX_SCREEN_Y>>1)+40, LCD_BLUE);

    LCD_Text((MAX_SCREEN_X>>1)-16, (MAX_SCREEN_Y>>2), "HOST" , LCD_BLACK);
    LCD_Text((MAX_SCREEN_X - (MAX_SCREEN_X>>2)-24), (MAX_SCREEN_Y>>1), "CLIENT" , LCD_BLACK);

    // Initialize overall scores, only happens at beginning of program //
    Game.Score = 0;
    //Game.overallScores[Client] = 0;
}


void UpdatePlayerOnScreen(PrevPlayer_t * prevPlayerIn, volatile GeneralPlayerInfo_t * outPlayer)
{




    int16_t x_diff = outPlayer->x_center - prevPlayerIn->CenterX;
    int16_t y_diff = outPlayer->y_center - prevPlayerIn->CenterY;


    // DOWN LEFT
    if(outPlayer->rotation == down_right)//x_diff > 0 && y_diff > 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

        //outPlayer->rotation = down_right;

        // Draw New Player
        DrawPlayer(outPlayer);

    }
    // UP RIGHT
    else if(outPlayer->rotation == up_right)//x_diff > 0 && y_diff < 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

        //outPlayer->rotation = up_right;

        // Draw New Player
        DrawPlayer(outPlayer);
    }
    // UP LEFT
    else if(outPlayer->rotation == up_left )// x_diff < 0 && y_diff < 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

        //outPlayer->rotation = up_left;

        // Draw New Player
        DrawPlayer(outPlayer);
    }
    // DOWN LEFT
    else if(outPlayer->rotation == down_left)//x_diff < 0 && y_diff > 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

        //outPlayer->rotation = down_left;

        // Draw New Player
        DrawPlayer(outPlayer);
    }
    // LEFT
    else if(outPlayer->rotation == left)//x_diff < 0 && y_diff == 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

        //outPlayer->rotation = left;

        // Draw New Player
        DrawPlayer(outPlayer);
    }
    // RIGHT
    else if(outPlayer->rotation == right)// x_diff > 0 && y_diff == 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

       // outPlayer->rotation = right;

        // Draw New Player
        DrawPlayer(outPlayer);
    }
    // DOWN
    else if(outPlayer->rotation == down)//x_diff == 0 && y_diff > 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

        //outPlayer->rotation = down;

        // Draw New Player
        DrawPlayer(outPlayer);
    }
    // UP
    else if(outPlayer->rotation == up)//x_diff == 0 && y_diff < 0)
    {
        // Erase previous player
        ErasePlayer(prevPlayerIn);

        //outPlayer->rotation = up;

        // Draw New Player
        DrawPlayer(outPlayer);
    }

}




/*
 * Returns either Host or Client depending on button press
*/
playerType GetPlayerRole()
{
    // Initialize buttons //
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN4|GPIO_PIN5);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN4|GPIO_PIN5);

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
 * Idle thread
*/
void IdleThread()
{
    while(1);
}
