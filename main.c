/**
 * main.c
 *
 * Educational Math Game for the DE1-SoC Board
 *
 * Description:
 * This is a simple educational math game designed to run on the DE1-SoC board. It offers users
 * math challenges across three levels of difficulty: Easy, Medium, and Hard. Each level has a set
 * number of questions, and players must answer within a set time period per question.
 * The game uses hardware features of the DE1-SoC board, such as slide switches and push buttons,
 * to interact with the user. The game progresses through various states, from start menu to
 * difficulty selection, question answering, and continues until the player chooses to end the game
 * or completes all levels.
 *
 * Hardware Components Used:
 * - DE1-SoC Board
 * - Slide Switches for user input
 * - Push Buttons for user input
 * - Seven Segment Display for showing countdown and score
 * - Audio Codec for playing sound files
 *  LT24 LCD for  display text message
 *
 * Game Flow:
 * 1. Start Menu: User chooses to start or quit the game.
 * 2. Menu: Displays a welcome message and moves to difficulty selection.
 * 3. Difficulty Selection: User selects difficulty level.
 * 4. In Progress: Game displays questions and waits for user input.
 * 5. Ask Continue: After a set of questions, asks if user wants to continue.
 * 6. End: Displays the final score and resets to start menu.
 * 7. Quit: Exits the game.
 */

//including different libraries and drivers
// Include drivers.
#include "HPS_PrivateTimer/HPS_PrivateTimer.h"
#include "DE1SoC_SevenSeg/DE1SoC_SevenSeg.h"
#include "HPS_Watchdog/HPS_Watchdog.h"
#include "DE1SoC_Addresses/DE1SoC_Addresses.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Util/macros.h"

//Audio related headers
#include "DE1SoC_WM8731/DE1SoC_WM8731.h"
#include "HPS_GPIO/HPS_GPIO.h"
#include "HPS_I2C/HPS_I2C.h"
#include "FPGA_PIO/FPGA_PIO.h"
#include "FatFS/ff.h"

//LCD header library
#include "GameLib.h"


// Status function to exit on failure of timer driver
void exitOnFail(signed int status, signed int successStatus) {
    if (status != successStatus) {
        exit((int) status); //Add breakpoint here to catch failure
    }
}

/*
 * Function: check_status_audio_files
 * Description: Checks the status of audio file operations
 * Input(s): FRESULT result - result of file operation
 * Return: void
 */
void check_status_audio_files( FRESULT result ) {
	switch (result) {
		case FR_OK:
		printf("Success \n");
		break;
		default:
		printf("Failure \n");
	}
}

// Select which sevenSegDisplay displays to use.
#define SINGLE_DISPLAY_LOCATION 0
#define DOUBLE_HEX_DISPLAY_LOCATION 2
#define DOUBLE_DEC_DISPLAY_LOCATION 4

#define QUESTION_PERIOD 225000000  // Time period for each question.
#define MAX_QUESTIONS_PER_LEVEL 3  // Maximum questions per level.

unsigned int countdown=20;

// Define memory-mapped I/O addresses. for switch and buttons

#define SW_BASE  0xFF200040
#define KEY_BASE 0xFF200050

// Context structures for various peripherals
PFPGAPIOCtx_t leds;
PWM8731Ctx_t audio;
PHPSGPIOCtx_t gpio;
PHPSI2CCtx_t   i2c;

// File system objects and audio file declarations
FATFS *file_system; // Structure of File system object
FILINFO filinfo; // information about object read
FRESULT fr; // FATFS Return

//correct_answer.wav audio file declarations for running this audio file
FIL *correct_answer_file; // File pointer to point to "correct_answer.wav" file.
int16_t *correct_answer_buffer; //buffer for correct_answer

//wrong_answer.wav audio file declarations for running this audio file
FIL *wrong_answer_file; // File pointer to point to "wrong_answer.wav" file.
int16_t *wrong_answer_buffer; //buffer for wrong_answer

unsigned int correct_answer_size; // Size of  correct_answer buffer
unsigned int wrong_answer_size; // Size of  correct_answer buffer
signed int audio_sample;
unsigned int space;

signed int audio_sample;

volatile unsigned int *KEY_ptr  = (unsigned int *)0xFF200050;

//// ARM A9 Private Timer  related addresses.
volatile unsigned int *private_timer_load = (unsigned int *)(LSC_BASE_PRIV_TIM + 0x0);
volatile unsigned int *private_timer_value = (unsigned int *)(LSC_BASE_PRIV_TIM + 0x4);
volatile unsigned int *private_timer_control = (unsigned int *)(LSC_BASE_PRIV_TIM + 0x8);
volatile unsigned int *private_timer_interrupt = (unsigned int *)(LSC_BASE_PRIV_TIM + 0xC);
const unsigned int CountPeriod = 225000000;



// Define game states and difficulty levels.
typedef enum { START_MENU, MENU, SELECT_DIFFICULTY, IN_PROGRESS, ASK_CONTINUE, END, QUIT } GameState;

typedef enum { EASY, MEDIUM, HARD } Difficulty;
GameState game_state = START_MENU;
Difficulty difficulty = EASY;


// Define structure for math questions.
typedef struct {
    char* question;
    int answer;  // Numeric answers for Medium and Hard.
    char* choices[4];  // Multiple-choice for Easy level.
    int correct_choice;  // Correct choice index for Easy level.
    int user_answer;  // User provided answer.
} MathQuestion;

/*
 * WAV Header Declaration
 * Referenced from online sources
 * Availability: http://soundfile.sapp.org/doc/WaveFormat/
*/
typedef struct {
	uint8_t id[4]; /** should always contain "RIFF" */
	uint32_t totallength; /** total file length minus 8 */
	uint8_t wavefmt[8]; /** should be "WAVEfmt " */
	uint32_t format; /** Sample format. 16 for PCM format. */
	uint16_t pcm; /** 1 for PCM format */
	uint16_t channels; /** Channels */
	uint32_t frequency; /** sampling frequency */
	uint32_t bytes_per_second; /** Bytes per second */
	uint16_t bytes_per_capture; /** Bytes per capture */
	uint16_t bits_per_sample; /** Bits per sample */
	uint8_t data[4]; /** should always contain "data" */
	uint32_t bytes_in_data; /** No. bytes in data */
} WAV_Header_TypeDef;



// Array of questions.
MathQuestion questions[3][MAX_QUESTIONS_PER_LEVEL];
int current_question = 0;
int score = 0;

// Function declarations.
//void delay(int milliseconds);
void generate_questions();
void display_question();
void evaluate_answer();
void update_game_state(unsigned char Timeout);
void ask_continue();
void display_game_over();
unsigned char handle_user_input();
void select_difficulty();
int process_switch_input(int);

void start_menu();
void initialisetimer_timer(PTimerCtx_t* timerCtx);
void check_timer(PTimerCtx_t* timerCtx);
void reset_time();


/*--------------------------------------------------
Function Name: buffer_size
Description: Checks the status as to whether certain commands have run successfully or not
Input(s): FRESULT
Return: NULL
----------------------------------------------------*/
unsigned int buffer_size( FIL *input_file )
{
	WAV_Header_TypeDef TempHeader ; //
	int size_file = f_size ( input_file ); // Get Size of the file
	size_file = size_file - sizeof ( TempHeader ); // To get actual data size
	return size_file;
}

/**
 * Function: fileread
 * Description: Reads the WAV file into a buffer
 * Input(s): FIL *input_file - pointer to the input file, int16_t *buffer - buffer to store the data
 * Return: signed short int* - pointer to the read buffer
 */
signed short int *fileread( FIL *input_file, int16_t *buffer )
{
    WAV_Header_TypeDef wavHeader; // WAV_Header for wav header.
    unsigned int read_size =0;
    int file_size = f_size ( input_file ); // Read the total size of wav file

    check_status_audio_files ( f_read ( input_file, &wavHeader, sizeof(wavHeader), &read_size)); // Read the WAV file header

    file_size = (file_size - sizeof(wavHeader)); //File size

    HPS_ResetWatchdog();

    int16_t *temp_buffer;
    temp_buffer = (int16_t *)malloc(sizeof(int16_t) *file_size);

    //Read file
    check_status_audio_files ( f_read(input_file, temp_buffer, file_size, &read_size) );

    buffer = temp_buffer;

    HPS_ResetWatchdog(); // Reset Watchdog.

    return temp_buffer;
}

/**
 * Function: audio_files_init
 * Description: Initializes the audio files by mounting the file system and reading the correct_answer.wav & wrong_answer.wav file
 * Input(s): None
 * Return: void
 */
void audio_files_init()
{
	file_system = malloc ( sizeof (FATFS) ); // FATFS pointer memory allocation

	printf("Driver mounting");
	check_status_audio_files(f_mount ( file_system , "" , 0)); //mounting the drive to program

	// Allocate memory for the file
	correct_answer_file = malloc ( sizeof (FIL));
	wrong_answer_file = malloc ( sizeof (FIL));

	printf("Opening correct_answer.wav file\n");
	check_status_audio_files( f_open ( correct_answer_file ,"correct_answer.wav", FA_READ));
	HPS_ResetWatchdog(); // reset watchdog

	printf("Opening wrong_answer.wav file\n");
	check_status_audio_files( f_open ( wrong_answer_file ,"wrong_answer.wav", FA_READ));
	HPS_ResetWatchdog(); // reset watchdog

	correct_answer_buffer = fileread( correct_answer_file, correct_answer_buffer );
	correct_answer_size = buffer_size (correct_answer_file);

	wrong_answer_buffer = fileread( wrong_answer_file, wrong_answer_buffer);
	wrong_answer_size = buffer_size (wrong_answer_file);
}

/* Function: audio_initialise
* Description: Initialises the audio peripherals
* Input(s): None
* Return: void
*/
void audio_initialise()
{

	exitOnFail(HPS_GPIO_initialise(LSC_BASE_ARM_GPIO, ARM_GPIO_DIR, ARM_GPIO_I2C_GENERAL_MUX, 0, &gpio), ERR_SUCCESS);
	exitOnFail(HPS_I2C_initialise(LSC_BASE_I2C_GENERAL, I2C_SPEED_STANDARD, &i2c), ERR_SUCCESS);
    exitOnFail(WM8731_initialise(LSC_BASE_AUDIOCODEC, i2c, &audio), ERR_SUCCESS);
	WM8731_clearFIFO(audio, true,true);// clear FIFO space

	WM8731_getFIFOSpace(audio, &space);

}


/**
 * Function: play_sound
 * Description: Plays the sound from the audio buffer
 * Input(s): int16_t *audio_buffer - buffer containing audio data, unsigned int audio_size - size of the audio buffer
 * Return: void
 */
void play_sound( int16_t *audio_buffer, unsigned int audio_size )
{
	int crnt_pointer = 0; // data index
	int volume = 10000; // Volume of audio output

		while ( crnt_pointer < (audio_size/2) )
		{

			WM8731_getFIFOSpace(audio, &space);
			if (space > 0)
			{ // Checks if FIFO pointer is free
				audio_sample = audio_buffer[crnt_pointer] * volume; // Pass data onto buffer
				WM8731_writeSample(audio, audio_sample, audio_sample);
				crnt_pointer = crnt_pointer +1;
			}

			HPS_ResetWatchdog(); // reset watchdog
		}

		HPS_ResetWatchdog(); // reset watchdog


}


/**
 * Function: read_slide_switches
 * Description: Reads the state of slide switches
 * Input(s): None
 * Return: int - state of the slide switches
 */
int read_slide_switches() {
    volatile int *sw_ptr = (int *)SW_BASE;
    return *sw_ptr;
}

/**
 * Function: read_push_buttons
 * Description: Reads the state of push buttons
 * Input(s): None
 * Return: int - state of the push buttons
 */
int read_push_buttons() {
    volatile int *key_ptr = (int *)KEY_BASE;
    return *key_ptr;
}

/**
 * Function: generate_questions
 * Description: Generates math questions for all difficulty levels
 * Input(s): None
 * Return: void
 */
void generate_questions() {

    questions[EASY][0] = (MathQuestion){"What is 6 * 7?", 42, {"42", "36", "48", "30"}, 0, -1};
    questions[EASY][1] = (MathQuestion){"What is 15 / 3?", 5, {"3", "5", "7", "9"}, 1, -1};
    questions[EASY][2] = (MathQuestion){"What is 8 * 9?", 72, {"72", "64", "80", "68"}, 0, -1};

    questions[MEDIUM][0] = (MathQuestion){"Solve for x: 3x - 2 = 7", 3, {0}, 0, -1};
    questions[MEDIUM][1] = (MathQuestion){"What is the value of 4x when x = 2?", 8, {0}, 0, -1};
    questions[MEDIUM][2] = (MathQuestion){"If x - 2 = 5, what is x?", 7, {0}, 0, -1};
    questions[HARD][0] = (MathQuestion){"Find x if 5x = 45", 9, {0}, 0, -1};
    questions[HARD][1] = (MathQuestion){"What is the unit digit of 2^8?", 6, {0}, 0, -1};
    questions[HARD][2] = (MathQuestion){"Calculate the derivative of x^2 at x = 3", 6, {0}, 0, -1};
}



/**
* Function: display_question
* Description: Displays the current question based on difficulty and question index
* Input(s): None
* Return: void
*/
void display_question(PLT24Ctx_t lt24) {

	unsigned short QuesId = (difficulty + 1)*10 + current_question;

	ShowScreen(QuesId, lt24);

	printf("Question: %s\n", questions[difficulty][current_question].question);
    printf("Options: \n");
    if (difficulty == EASY)
    	{
			for (int i = 0; i < 4; i++)
			{
				printf("%s\t", questions[difficulty][current_question].choices[i]);
			}
			printf("Press KEY0 for A, KEY1 for B, KEY2 for C, KEY3 for D to select your answer.\n");
    }
    else {
        printf("Use the switches to set your answer (0-9), and press KEY0 to confirm.\n");
    }
}

/**
 * Function: initialisetimer_timer
 * Description: Initializes the timer for the game
 * Input(s): PTimerCtx_t* timerCtx - pointer to the timer context
 * Return: void
 */
void initialisetimer_timer(PTimerCtx_t* timerCtx) {
	    exitOnFail(Timer_initialise(LSC_BASE_PRIV_TIM, timerCtx), ERR_SUCCESS); // Initialise Timer Controller
	    exitOnFail(Timer_setLoad(*timerCtx, QUESTION_PERIOD), ERR_SUCCESS); // Set Timer Controller Load Value
	    exitOnFail(Timer_setPrescaler(*timerCtx, 0), ERR_SUCCESS);     // Set Timer Controller Prescaler Value
	    exitOnFail(Timer_setAutoReload(*timerCtx, true), ERR_SUCCESS); // Enable Timer AutoReload
	    exitOnFail(Timer_enable(*timerCtx, true), ERR_SUCCESS);        // Enable Timer
	    unsigned int timerCurrentValue;
	    exitOnFail(Timer_currentValue(*timerCtx, &timerCurrentValue), ERR_SUCCESS); // Read Timer Current Value
	    ResetWDT(); // Reset watchdog
}

/**
 * Function: reset_timer
 * Description: Resets the countdown timer
 * Input(s): None
 * Return: void
 **/
void reset_timer() {
    countdown = 9;  // Reset countdown to 10 seconds
    //DE1SoC_SevenSeg_SetSingle(0, countdown);  // Display initial countdown value
}

/**
 * Function: check_timer
 * Description: Checks the timer to determine if the question period has elapsed
 * Input(s): PTimerCtx_t* timerCtx - pointer to the timer context
 * Return: void
 */
void check_timer(PTimerCtx_t* timerCtx) {

	 unsigned int timerCurrentValue;
	    bool irqFlag = false;

	    exitOnFail(Timer_currentValue(*timerCtx, &timerCurrentValue), ERR_SUCCESS); // Read Timer Current Value
	    exitOnFail(Timer_irqFlag(*timerCtx, &irqFlag), ERR_SUCCESS); // Read Timer IRQ Flags

	    if (irqFlag) {
	        countdown--;  // Decrease countdown
	        DE1SoC_SevenSeg_SetDoubleDec(0, countdown);  // Update display
	        if (countdown == 0) {
	            // Time is up, handle timeout (e.g., move to the next question or update game state)
	            update_game_state(0);
	            reset_timer();  // Reset timer for the next question
	        }
	    }

}

/**
 * Function: evaluate_answer
 * Description: Evaluates the user's answer and updates the score
 * Input(s): None
 * Return: void
 */
void evaluate_answer(PLT24Ctx_t lt24) {
    int user_answer = questions[difficulty][current_question].user_answer;
    int correct_answer = (difficulty == EASY) ? questions[difficulty][current_question].correct_choice : questions[difficulty][current_question].answer;
    printf("User answer: %d, Correct answer: %d\n", user_answer, correct_answer);

    if(user_answer == 10) {
    	return;
    }
    ShowAnswer(difficulty, current_question, user_answer, correct_answer, lt24);
    if (user_answer == correct_answer) {
        score++;
        printf("Correct! Score: %d\n", score);
        //printf("Playing audio\n");
        //play_sound (welcome_buffer, welcome_size ); // Say the application
        DE1SoC_SevenSeg_SetSingle(2,score);
        play_sound (correct_answer_buffer, correct_answer_size );
    } else {
    	play_sound (wrong_answer_buffer, wrong_answer_size );
        printf("Incorrect. The correct answer is: %d\n", correct_answer);
    }
}

/**
 * Function: process_switch_input
 * Description: Processes the input from slide switches
 * Input(s): int switch_inp - input from the switches
 * Return: int - processed value
 */
int process_switch_input(int switch_inp)
{
	int val = -1;
	for(int i =0; i<10;i++)
	{
		int ans = pow(2,i);
		if((pow(2,i))==switch_inp)
		{
			val = i;
			break;
		}
	}

	return val;
}

/**
 * Function: ask_continue
 * Description: Asks the user if they want to continue playing after a level is complete
 * Input(s): None
 * Return: void
 */
void ask_continue(PLT24Ctx_t lt24) {

    printf("Continue playing? Press KEY3 to continue or KEY1 to end.\n");
    ShowScreen(CONTPLAY, lt24);
   //score = 0;
    	 DE1SoC_SevenSeg_SetSingle(2,score);
    while (1) {

    	int keys = read_push_buttons();
    	HPS_ResetWatchdog();  // Reset the watchdog timer.

    	// If key3 pressed continue
        if (keys & 0x08) {
        	current_question = 0;
        	game_state = SELECT_DIFFICULTY;
            break;
        }
        // If key0 is pressed end the game
        else if (keys & 0x01) {
            game_state = END;
            break;
        }
    }

}

/**
 * Function: display_game_over
 * Description: Displays the game over screen and final score
 * Input(s): None
 * Return: void
 */
void display_game_over(PLT24Ctx_t lt24) {

	ShowScreen(END_SCREEN, lt24);
	// Continue only if Key0 is pressed
	while(1) {
		int keys = read_push_buttons();
		HPS_ResetWatchdog();  // Reset the watchdog timer.
		if (keys & 0x01) {
			break;
		}
	}

	printf("Game Over\n");
    printf("Final Score: %d\n", score);
}

/**
 * Function: update_game_state
 * Description: Updates the game state to move to the next question or level
 * Input(s): None
 * Return: void
 */
void update_game_state(unsigned char Timeout) {

	current_question++;

	// Continue to next question if key0 is pressed
	if(Timeout != 1) {
		while(1) {

			int keys = read_push_buttons();
			HPS_ResetWatchdog();  // Reset the watchdog timer.

			if (keys & 0x01) {
				break;
			}
		}
	}

    if (current_question >= MAX_QUESTIONS_PER_LEVEL) {
        game_state = ASK_CONTINUE;
    }
}

/**
 * Function: handle_user_input
 * Description: Handles user input for answering questions
 * Input(s): None
 * Return: void
 */
unsigned char handle_user_input() {

	unsigned int CountdownTimer = 20;
	DE1SoC_SevenSeg_SetDoubleDec(DOUBLE_DEC_DISPLAY_LOCATION,CountdownTimer);
	unsigned char Timeout = 0;
	unsigned int StartTimerValue = *private_timer_value;
	unsigned int CurrentTimerValue;

	if (difficulty == EASY) {
        while (1) {
        	CurrentTimerValue = *private_timer_value;
        	if (*private_timer_interrupt & 0x1) {
        		// If the timer interrupt flag is set, clear the flag
        		*private_timer_interrupt = 0x1;
        	}

        	int keys = read_push_buttons();
            HPS_ResetWatchdog();  // Reset the watchdog timer.
            if (keys & 0x01) {
                questions[difficulty][current_question].user_answer = 0;
                break;
            } else if (keys & 0x02) {
                questions[difficulty][current_question].user_answer = 1;
                break;
            } else if (keys & 0x04) {
                questions[difficulty][current_question].user_answer = 2;
                break;
            } else if (keys & 0x08) {
                questions[difficulty][current_question].user_answer = 3;
                break;
            }

            if(StartTimerValue - CurrentTimerValue >= CountPeriod) {
            	StartTimerValue = *private_timer_value;
            	CountdownTimer -= 1;
            	DE1SoC_SevenSeg_SetDoubleDec(DOUBLE_DEC_DISPLAY_LOCATION,CountdownTimer);
            	if(CountdownTimer == 0) {
            		questions[difficulty][current_question].user_answer = 10; // 10 means no answer
            		Timeout = 1;
            		break;
            	}

            }


        }
    } else {
		while(1)
		{
			HPS_ResetWatchdog();  // Reset the watchdog timer.

			if (*private_timer_interrupt & 0x1) {
				// If the timer interrupt flag is set, clear the flag
				*private_timer_interrupt = 0x1;
			}


			 while (!(read_push_buttons() & 0x01)) {
				 CurrentTimerValue = *private_timer_value;
				if (*private_timer_interrupt & 0x1) {
					// If the timer interrupt flag is set, clear the flag
					*private_timer_interrupt = 0x1;
				}


				 HPS_ResetWatchdog();  // Reset the watchdog timer.

				 if(StartTimerValue - CurrentTimerValue >= CountPeriod) {
					StartTimerValue = *private_timer_value;
					CountdownTimer -= 1;

					DE1SoC_SevenSeg_SetDoubleDec(DOUBLE_DEC_DISPLAY_LOCATION,CountdownTimer);

					if(CountdownTimer == 0) {
						Timeout = 1;
						break;
					}
				 }
			 }

			if(CountdownTimer == 0) {
				DE1SoC_SevenSeg_SetDoubleDec(DOUBLE_DEC_DISPLAY_LOCATION,CountdownTimer);
				questions[difficulty][current_question].user_answer = 10; // 10 means no answer
				Timeout = 1;
				break;
			}

			int switch_inp = read_slide_switches();
			int answer = process_switch_input(switch_inp);

			if(!(answer>=0 && answer<10))
			{
				printf("Please enter just a single digit value through switch, Try again !\n");
			}
			else
			{
				printf("Switch input noted : %d",answer);
				questions[difficulty][current_question].user_answer = answer;
				break;
			}
		}
    }

	return Timeout;
}

/**
 * Function: select_difficulty
 * Description: Allows user to select the difficulty level at the start of the game
 * Input(s): None
 * Return: void
 */
void select_difficulty(PLT24Ctx_t lt24) {

    ShowScreen(LEVEL_SCREEN, lt24); // Show select difficulty screen
	while (1) {
        int keys = read_push_buttons();
        HPS_ResetWatchdog();  // Reset the watchdog timer.
        if (keys & 0x01) {
            difficulty = EASY;
            break;
        } else if (keys & 0x02) {
            difficulty = MEDIUM;
            break;
        } else if (keys & 0x04) {
            difficulty = HARD;
            break;
        }

    }
    game_state = IN_PROGRESS;  // Transition to start showing questions.
}

/**
 * Function: start_menu
 * Description: Displays the start menu to allow user to start or quit the game
 * Input(s): None
 * Return: void
 */
void start_menu(PLT24Ctx_t lt24) {

	ShowScreen(START_SCREEN, lt24); // 1 - Show start screen

	DE1SoC_SevenSeg_SetSingle(2,0); // Initialise HEX0 display with 0 for score

	while (1) {
        int keys = read_push_buttons();
        HPS_ResetWatchdog();  // Reset the watchdog timer.
        if (keys) {  // Wait until any key is pressed.
            if (keys & 0x08) {
                game_state = MENU;  // If KEY3, proceed to the menu.
                break;
            } else if (keys & 0x02) {
                game_state = QUIT;  // If KEY1, quit the game.
                break;
            }
        }
    }
}



int main(void) {
	//Initialise Timer Driver and generate questions
    PTimerCtx_t timerCtx;
	initialisetimer_timer(&timerCtx);
	audio_initialise(); // Initialise audio
	HPS_ResetWatchdog(); // Reset watchdog
	audio_files_init();
	HPS_ResetWatchdog();

    generate_questions();  // Pre-generate questions for all levels.

    // Variables
	PLT24Ctx_t lt24;

	*private_timer_load      = 0xFFFFFFFF;
	*private_timer_control   = (0 << 8) | (0 << 2) | (1 << 1) | (1 << 0);

	if (*private_timer_interrupt & 0x1) {
		// If the timer interrupt flag is set, clear the flag
		*private_timer_interrupt = 0x1;
	}

	DE1SoC_SevenSeg_SetDoubleDec(DOUBLE_DEC_DISPLAY_LOCATION,countdown);

	// Initialise the LCD Display.
	exitOnFail(
			LT24_initialise(LSC_BASE_GPIO_JP1,LSC_BASE_LT24HWDATA, &lt24), //Initialise LCD
			ERR_SUCCESS);                                                  //Exit if not successful

	    while (1) {
	        //check_timer(&timerCtx);  // Continuously check the timer in the main loop.

	        switch (game_state) {
	            case START_MENU:
	                start_menu(lt24);  // Handle start menu options.
	                break;
	            case MENU:
	                printf("Welcome to the Math Game!\n");
	                game_state = SELECT_DIFFICULTY;  // Move to select difficulty.
	                break;
	            case SELECT_DIFFICULTY:
	                select_difficulty(lt24);  // Select the game difficulty.
	                reset_timer();  // Reset the timer at the start of each level.
	                break;
	            case IN_PROGRESS:
	                if (current_question < MAX_QUESTIONS_PER_LEVEL) {
	                    display_question(lt24);  // Display the current question.
	                    unsigned char Timeout = handle_user_input();  // Handle user input for the question.
	                    evaluate_answer(lt24);  // Check the answer.
	                    update_game_state(Timeout);  // Decide next step.
	                }
	                break;
	            case ASK_CONTINUE:
	                ask_continue(lt24);  // Ask if player wants to continue.
	                break;
	            case END:
	                display_game_over(lt24);  // Show game over screen.
	                game_state = START_MENU;  // Reset to start menu after game over.
	                score = 0;  // Reset the score.
	                break;
	            case QUIT:
	                printf("Exiting the game.\n");
	                return 0;  // Exit game loop and end program.
	            default:
	                printf("Unhandled game state.\n");
	                break;
	        }
	        HPS_ResetWatchdog();  // Reset the watchdog timer.
	    }
	    return 0;
}
