/*
 * Short Description
 * ----------------------------------
 * This file manages the visual elements displayed on an LCD screen using a LT24 display controller.
 */

#include "GameLib.h"

// Copy frame buffer to display
// - returns 0 if successful
HpsErr_t LT24_copyFrameColour(PLT24Ctx_t ctx, unsigned short Colour, unsigned int xleft, unsigned int ytop, unsigned int width, unsigned int height ) {

	//Define Window (setWindow validates context for us)
	HpsErr_t status = LT24_setWindow(ctx, xleft, ytop, width, height);
    if (IS_ERROR(status)) return status;

    //And copy the required number of pixels
    unsigned int cnt = (height * width);
    while (cnt--) {
        // Write the specified color to each pixel in the window
        LT24_write(ctx, true, Colour);
    }
    return ERR_SUCCESS;
}

//Copy frame buffer to display
// - returns 0 if successful
HpsErr_t LT24_copyFrameBufferAnswer(int result, PLT24Ctx_t ctx, const unsigned short* framebuffer, unsigned int xleft, unsigned int ytop, unsigned int width, unsigned int height ) {
    //Define Window (setWindow validates context for us)
    HpsErr_t status = LT24_setWindow(ctx, xleft, ytop, width, height);
    if (IS_ERROR(status)) return status;
    //And copy the required number of pixels
    unsigned int cnt = (height * width);
    unsigned short colour;
    while (cnt--) {
        // Get the color from the framebuffer
    	colour = *framebuffer++;
        // Replace black pixels with green for correct answers, or red for incorrect
    	if(colour == 0x0000) {
    		if(result == 1) { // Green for correct
    			colour = 0x4E4E;
    		} else { // red for wrong
    			colour = 0xEA64;
    		}
    	}
        // Write the color to the display
        LT24_write(ctx, true, colour);
    }
    return ERR_SUCCESS;
}

// Display the answer feedback based on difficulty and correctness
void ShowAnswer(int difficulty, int current_question, int user_answer, int correct_answer, PLT24Ctx_t lt24) {

    // Get the image data for the correct answer and user's answer
	const unsigned short *correct_answer_img = &Num[correct_answer][0];
	const unsigned short *user_answer_img = &Num[user_answer][0];

	// If difficulty level is not easy
	if(difficulty != 0) {
		if(user_answer != correct_answer) {
            // Display the correct answer in green
			LT24_copyFrameBufferAnswer(1, lt24, correct_answer_img, 12, 250, 40, 40); // 1- Green
            // Display the user's incorrect answer in red
			LT24_copyFrameBufferAnswer(0, lt24, user_answer_img, 188, 250, 40, 40); // 0- Red
		} else {
            // Display both the correct and user's correct answer in green
			LT24_copyFrameBufferAnswer(1, lt24, correct_answer_img, 12, 250, 40, 40); // 1- Green
			LT24_copyFrameBufferAnswer(1, lt24, user_answer_img, 188, 250, 40, 40); // 1- Green
		}
	}
	// for easy
	else {
        // Display a small green tick for the correct answer
		LT24_copyFrameBuffer(lt24, right, 176, 147 + correct_answer*30, 15, 15);
		if(user_answer != correct_answer) {
            // Display a small red x for the incorrect user's answer
			LT24_copyFrameBuffer(lt24, wrong, 176, 147 + user_answer*30, 15, 15);
		}
	}
}
// Display different screens based on the provided screen identifier
void ShowScreen(uint8_t ScreenNum, PLT24Ctx_t lt24) {

	//Switch case to display different images on the LCD
	switch(ScreenNum) {
		case START_SCREEN:  	// Display the start screen image
			LT24_copyFrameBuffer(lt24, StartScreenImg, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			break;
		case LEVEL_SCREEN:       // Display the level selection screen image
			LT24_copyFrameBuffer(lt24, SelectLevelImg, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			break;
		// Clear the screen with a background colour and display the easy questions
		case EASY_1:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, EasyQues_1, 16, 59, 208, 211);
			break;
		case EASY_2:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, EasyQues_2, 16, 59, 208, 211);
			break;
		case EASY_3:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, EasyQues_3, 16, 59, 208, 211);
			break;
        // Clear the screen with a background colour and display the medium questions
		case MED_1:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, MedQues_1, 12, 100, 215, 120);
			break;
		case MED_2:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, MedQues_2, 12, 100, 215, 120);
			break;
		case MED_3:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, MedQues_3, 12, 100, 215, 121);
			break;
	    // Clear the screen with a background colour and display the hard questions
		case HARD_1:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, HardQues_1, 12, 100, 215, 121);
			break;
		case HARD_2:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, HardQues_2, 12, 100, 215, 121);
			break;
		case HARD_3:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, HardQues_3, 12, 100, 215, 121);
			break;
        // Clear the screen with a background colour and display the "continue playing" screen
		case CONTPLAY:
			LT24_copyFrameColour(lt24, 0xFE2E, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, Contplaying, 12, 100, 214, 120);
			break;
		case END_SCREEN:
        // Clear the screen with a background colour and display the end screen
			LT24_copyFrameColour(lt24, 0xFE2D, 0, 0, LCD_WIDTH, LCD_HEIGHT);
			LT24_copyFrameBuffer(lt24, EndScreenImg, 35, 85, 170, 150);
			break;
		default: break;
	}

	return;
}
