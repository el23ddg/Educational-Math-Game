/*
* GameLib.h
*
* Game library
*/

#ifndef GAMELIB_H_
#define GAMELIB_H_
// Include standard integer, boolean, mathematical definitions
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
// Include HPS Watchdog for functionality
#include "HPS_Watchdog/HPS_Watchdog.h"
//Include LT24 display controller
#include "DE1SoC_LT24/DE1SoC_LT24.h"

#include "Images.h"
// Define the dimensions of the LCD display
#define LCD_WIDTH 240
#define LCD_HEIGHT 320

// Define the base address for the key inputs
#define KEY_BASE 0xFF200050

// Define screen identifiers for different game states
#define START_SCREEN 1
#define LEVEL_SCREEN 2
#define CONTPLAY 3
#define END_SCREEN 4

// Define identifiers for different easy-level questions
#define EASY_1 10
#define EASY_2 11
#define EASY_3 12

// Define identifiers for different medium-level questions
#define MED_1 20
#define MED_2 21
#define MED_3 22

// Define identifiers for different hard-level questions
#define HARD_1 30
#define HARD_2 31
#define HARD_3 32

// Function prototype to check for a specific key press
void CheckKeyPress(uint8_t KeyNum);

// Function prototype to get the key that has been pressed
uint8_t GetKeyPressed();

// Function prototype to handle the game logic for easy level
uint8_t EasyLevel(PLT24Ctx_t lt24);

// Function prototype to handle the game logic for medium level
uint8_t MediumLevel(PLT24Ctx_t lt24);

// Function prototype to handle the game logic for hard level
uint8_t HardLevel(PLT24Ctx_t lt24);

// Function prototype to display different screens based on the screen identifier
void ShowScreen(uint8_t ScreenNum, PLT24Ctx_t lt24);

// Function prototype to show the answer feedback based on the user's answer
void ShowAnswer(int difficulty, int current_question, int user_answer, int correct_answer, PLT24Ctx_t lt24);


#endif
