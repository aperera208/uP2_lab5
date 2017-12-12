/*
 * FinalProj.h
 *
 *  Created on: Dec 7, 2017
 *      Author: Alex Perera
 */

#ifndef FINALPROJ_H_
#define FINALPROJ_H_

#include <stdbool.h>
#include <stdint.h>
#include "cc3100_usage.h"
#include "LCD.h"
#include "G8RTOS.h"
#include "G8RTOS_Semaphores.h"

#define MAX_NUM_OF_ASTEROIDS        10
#define MAX_NUM_OF_BULLETS          5

#define BULLETSIZE                  4
#define BULLETD2                    (BULLETSIZE>>1)
#define BULLETSPEED                 2

/* Size of game arena */
#define ARENA_MIN_X                  40
#define ARENA_MAX_X                  280
#define ARENA_MIN_Y                  0
#define ARENA_MAX_Y                  240

/* Ship attributes */
#define MAX_HP              16
#define shipSize                16
#define sizediv2            (shipSize>>1)
#define ALIVE               1
#define DEAD                0
#define INVIS               2
#define VIS                 0
#define SHIELD              4
#define NO_SHIELD           0
#define SPEED               4000  //8000/SPEED = How far the ship moves
#define SPEED2              2500
#define SHIP_COLL           20

/* Enum for ship orientation    */
typedef enum
{
    up = (uint8_t)0, down = (uint8_t)1, left = (uint8_t)2, right = (uint8_t)3, up_right = (uint8_t)4, up_left = (uint8_t)5, down_right = (uint8_t)6, down_left = (uint8_t)7
}shipOrientation;

/* Enums for player colors */
typedef enum
{
    PLAYER_RED = LCD_RED,
    PLAYER_BLUE = LCD_BLUE
}playerColor;


/* Enums for bullet type */
typedef enum
{
    no_bullet = (uint8_t)0, normal_shot = (uint8_t)1, spread_shot = (uint8_t)2, charged0 = (uint8_t)3, charged1 = (uint8_t)4, charged2 = (uint8_t)5, charged3 = (uint8_t)6
}bullets;


/* Enums for Asteroid type  */
typedef enum
{
    dead = (uint8_t)0, large = (uint8_t)1, medium = (uint8_t)2, small = (uint8_t)3
}asteroid_status;
/*********************************************** Data Structures ********************************************************************/
/*********************************************** Data Structures ********************************************************************/


#pragma pack ( push, 1)
/*
 * Struct to be sent from the client to the host
 */
typedef struct
{
    uint32_t IP_address;
    uint8_t playerNumber;
    bool ready;
    bool joined;
    bool acknowledge;
} Wifi_Info_t;

/*
 *  Struct for the bullets
 */
typedef struct
{
    int16_t x_center;
    int16_t y_center;
    int16_t x_vel;
    int16_t y_vel;
    bullets bullet_type;
    bool alive;
}Bullets_t;
/*
 * General player info to be used by both host and client
 * Client responsible for translation
 */
typedef struct
{
    int16_t y_center;
    int16_t x_center;
    shipOrientation rotation;
    uint16_t color;                 // Might be able to delete this
    bullets bullet_request;
    uint8_t HP;
    uint8_t state;                  // Bits contain alive, invisible, and shield info
    //bool alive;
    //bool invisible;
    //bool shield;
} GeneralPlayerInfo_t;

/*
 * Struct of all the asteroids, only changed by the host
 */
typedef struct
{
    int16_t currentCenterX;
    int16_t currentCenterY;
    asteroid_status asteroid;
    bool alive;
} Asteroid_t;



/*
 * Struct of all the previous bullet locations, only changed by self for drawing!
 */
typedef struct
{
    int16_t CenterX;
    int16_t CenterY;
}PrevBullet_t;

/*
 * Struct to be sent from the host to the client
 */
typedef struct
{
    GeneralPlayerInfo_t players[2];
    Asteroid_t asteroids[MAX_NUM_OF_ASTEROIDS];
    Bullets_t bullets[MAX_NUM_OF_BULLETS];
    uint8_t numberOfbullets;
    uint8_t numberOfasteroids;
    bool gameDone;
    uint16_t Score;
    PrevBullet_t prevbullets[MAX_NUM_OF_BULLETS];
} GameState_t;
#pragma pack ( pop )

/*
 * Struct of all the previous asteroid locations, only changed by self for drawing!
 */
typedef struct
{
    int16_t CenterX;
    int16_t CenterY;
    asteroid_status asteroid;
}PrevAsteroid_t;




/*
 * Struct of all the previous players locations, only changed by self for drawing
 */
typedef struct
{
    int16_t CenterX;
    int16_t CenterY;
    shipOrientation rotation;
}PrevPlayer_t;


/*********************************************** Data Structures ********************************************************************/


/*********************************************** Client Threads *********************************************************************/


/*
 * Thread for client to join game
 */
void JoinGame();

/*
 * Thread that receives game state packets from host
 */
void ReceiveDataFromHost();

/*
 * Thread that sends UDP packets to host
 */
void SendDataToHost();

/*
 * Thread to read client's joystick
 */
void Read_Joystick_Button_Client();

/*
 * End of game for the client
 */
void EndOfGameClient();


void periodic_button_client();

/*
 * Generate Bullet based on button press flag
 */
void GenerateBulletClient();

/*********************************************** Client Threads *********************************************************************/

/*********************************************** Host Threads *********************************************************************/
/*
 * Thread for the host to create a game
 */
void CreateGame();

/*
 * Thread that sends game state to client
 */
void SendDataToClient();

/*
 * Thread that receives UDP packets from client
 */
void ReceiveDataFromClient();

/*
 * Thread to move a single asteroid
 */
void MoveAsteroid();

/*
 * Thread to move all the bullets
 */
void MoveBullets();

/*
 * Generate Asteroid thread
 */
void GenerateAsteroid();

/*
 * Generate Bullet based on button press flag
 */
void GenerateBulletHost();

/*
 * Thread to read host's joystick
 */
void Read_Joystick_Button_Host();

/*
 * End of game for the host
 */
void EndOfGameHost();

void periodic_button_host();


/*********************************************** Host Threads *********************************************************************/


/*********************************************** Common Threads *********************************************************************/
/*
 * Idle thread
 */
void IdleThread();

/*
 * Thread to draw all the objects in the game
 */
void DrawObjects();

/*
 * Thread to update LEDs based on score
 */
void MoveLEDs();

/*
 * Thread to make spaceship invisible after an asteroid collision
 */
void MakeInvisible();

/*********************************************** Common Threads *********************************************************************/


/*********************************************** Public Functions *********************************************************************/
/*
 * Returns either Host or Client depending on button press
 */
playerType GetPlayerRole();

/*
 * Draw players given center X center coordinate
 */
void DrawPlayer(volatile GeneralPlayerInfo_t * player);


void ErasePlayer( PrevPlayer_t * player);


/*
 * Updates player's paddle based on current and new center
 */
void UpdatePlayerOnScreen(PrevPlayer_t * prevPlayerIn, volatile GeneralPlayerInfo_t * outPlayer);

/*
 * Function updates ball position on screen
 * TODO: change parameters
 */
//void UpdateBulletOnScreen(PrevBall_t * previousBall, Ball_t * currentBall, uint16_t outColor);


/*
 * Function updates asteroid position
 */
// TODO: change parameters
void UpdateAsteroidOnScreen();

/*
 * Initializes and prints initial game state
 */
void InitBoardState();

/*
 * Start Menu
 */
void StartMenu();

/*********************************************** Public Functions *********************************************************************/





#endif /* FINALPROJ_H_ */
