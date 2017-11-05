/*
 * Game.c
 *
 *  Created on: Oct 30, 2017
 *      Author: Alex Perera
 */

#include "Game.h"
#include "G8RTOS.h"



/*********************************************** Client Threads *********************************************************************/
/*
 * Thread for client to join game
 *
•   Only thread to run after launching the OS
•   Set initial SpecificPlayerInfo_t strict attributes (you can get the IP address by calling getLocalIP()
•   Send player into to the host
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
void JoinGame();

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
void ReceiveDataFromHost();


/*
 * Thread that sends UDP packets to host
 *
•   Send player info
•   Sleep for 2ms

 */
void SendDataToHost();

/*
 * Thread to read client's joystick
 *
•   You can read the joystick ADC values by calling GetJoystickCoordinates
•   You’ll need to add a bias to the values (found experimentally) since every joystick is offset by some small amount displacement and noise
•   Change Self.displacement accordingly (you can experiment with how much you want to scale the ADC value)
•   Sleep for 10ms
•   Then add the displacement to the bottom player in the list of players (general list that’s sent to the client and used for drawing)
•   By sleeping before updating the bottom player’s position, it makes the game more fair between client and host

 */
void ReadJoystickClient();

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
void EndOfGameClient();

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
    /*
     * Add this thread before G8RTOS_Launch
     *
     *
     *
     */


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
void SendDataToClient();

/*
 * Thread that receives UDP packets from client
•   Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
    o   Note: Remember to release and take the semaphore again so you’re still able to send data
•   Update the player’s current center with the displacement received from the client
•   Sleep for 2ms (again found experimentally)
 */
void ReceiveDataFromClient();

/*
 * Generate Ball thread
•   Adds another MoveBall thread if the number of balls is less than the max
•   Sleeps proportional to the number of balls currently in play
 */
void GenerateBall();

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
void ReadJoystickHost();

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
void MoveBall();

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
void IdleThread();

/*
 * Thread to draw all the objects in the game
 *
•   Should hold arrays of previous players and ball positions
•   Draw and/or update balls (you’ll need a way to tell whether to draw a new ball, or update its position (i.e. if a new ball has just been created – hence the alive attribute in the Ball_t struct.
•   Update players
•   Sleep for 20ms (reasonable refresh rate)
 */
void DrawObjects();

/*
 * Thread to update LEDs based on score
 *
•   Responsible for updating the LED array with current scores
 */
void MoveLEDs();

/*********************************************** Common Threads *********************************************************************/


/*********************************************** Public Functions *********************************************************************/
/*
 * Returns either Host or Client depending on button press
 */
playerType GetPlayerRole();

/*
 * Draw players given center X center coordinate
 */
void DrawPlayer(GeneralPlayerInfo_t * player);

/*
 * Updates player's paddle based on current and new center
 */
void UpdatePlayerOnScreen(PrevPlayer_t * prevPlayerIn, GeneralPlayerInfo_t * outPlayer);

/*
 * Function updates ball position on screen
 */
void UpdateBallOnScreen(PrevBall_t * previousBall, Ball_t * currentBall, uint16_t outColor);

/*
 * Initializes and prints initial game state
 */
void InitBoardState();
