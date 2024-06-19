/*
*Short Description
-------------------------------------------
*Images.h
*
* This is the header file for
* It includes image arrays for various screens and elements like quiz questions and numeric digits,
* facilitating centralised management of graphic resources.
*
*/

#ifndef IMAGES_H_
#define IMAGES_H_
// Image data for the start screen, dimension: 320 x 240 (width x height)
extern const unsigned short StartScreenImg[76800];

// Image data for the level selection screen, dimension: 320 x 240
extern const unsigned short SelectLevelImg[76800];

// Easy level question images, each with dimensions corresponding to the specific content layout
extern const unsigned short EasyQues_1[43888];
extern const unsigned short EasyQues_2[43888];
extern const unsigned short EasyQues_3[43888];

// Medium level question images
extern const unsigned short MedQues_1[25800];
extern const unsigned short MedQues_2[25800];
extern const unsigned short MedQues_3[26015];

// Hard level question images
extern const unsigned short HardQues_1[26015];
extern const unsigned short HardQues_2[26015];
extern const unsigned short HardQues_3[26015];

// Numeric images for displaying digits, each 40 x 40 pixels
extern const unsigned short zero[1600]; // Image data for digit '0'
extern const unsigned short one[1600];  // Image data for digit '1'
extern const unsigned short two[1600];  // Image data for digit '2'
extern const unsigned short three[1600];// Image data for digit '3'
extern const unsigned short four[1600]; // Image data for digit '4'
extern const unsigned short five[1600]; // Image data for digit '5'
extern const unsigned short six[1600];  // Image data for digit '6'
extern const unsigned short seven[1600]; // Image data for digit '7'
extern const unsigned short eight[1600]; // Image data for digit '8'
extern const unsigned short nine[1600]; // Image data for digit '9'

// Images for correct and incorrect answer indicators, each 15 x 15 pixels
extern const unsigned short right[225]; // Image data for the 'correct' indicator
extern const unsigned short wrong[225]; // Image data for the 'incorrect' indicator

// Image data for the "Continue Playing" screen, dimensions 214 x 120
extern const unsigned short Contplaying[25680];

// Image data for the end screen, dimension: 170 x 150
extern const unsigned short EndScreenImg[25500];

// Array of pointers to the digit images, facilitating easy access to any digit image
extern const unsigned short *const Num[10];

#endif
