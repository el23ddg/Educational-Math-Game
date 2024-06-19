# Educational Math Game for DE1-SoC

## Introduction
The Educational Math Game is a simple yet engaging game designed to run on the DE1-SoC board. It offers users math challenges across three levels of difficulty: Easy, Medium, and Hard. Each level has a set number of questions, and players must answer within a specified time period per question. The game utilizes various hardware features of the DE1-SoC board, such as slide switches, push buttons, seven-segment display, and audio codec, to provide an interactive  gaming experience.

## Demo Test Video
Watch the demo test video [here](https://www.youtube.com/watch?v=mdEdF6C8wkM).


## Hardware Components
The game leverages the following hardware components of the DE1-SoC board:
- Slide Switches: Used for user input to select answers in the Medium and Hard difficulty levels.
- Push Buttons: Used for user input to navigate the game menu, select difficulty levels, and confirm answers.
- Seven Segment Display: Displays the countdown timer and the player's score.
- Audio Codec: Plays sound files to provide audio feedback and enhance the gaming experience.
- LT24 LCD: Displays text messages and instructions to guide the player through the game.

## Game Flow
The game follows a structured flow to provide a seamless gaming experience:
1. Start Menu: The player is presented with a start menu that displays instructions to be followed during the game.
2. Menu: Upon starting the game, the player can choose between 3 levels.
3. Difficulty Selection: The player can choose from three difficulty levels: Easy, Medium, and Hard.
4. In Progress: Once the difficulty level is selected, the game enters the "In Progress" state. Questions are displayed one by one, and the player must provide their answer within the allocated time.
5. Ask Continue: After completing a set of questions in each level, the player is asked if they want to continue playing or quit the game.
6. End: If the player chooses to end the game or completes all levels, the game displays the final score and returns to the start menu.

## Code Structure
The code for the Educational Math Game is organized into several functions and modules:
- `main()`: The entry point of the program, which initializes the hardware components, generates questions and manages the game flow using a state machine.
- `generate_questions()`: Generates math questions for all difficulty levels.
- `display_question()`: Displays the current question based on the difficulty level and question index.
- `handle_user_input()`: Handles user input for answering questions using slide switches and push buttons.
- `evaluate_answer()`: Evaluates the user's answer and updates the score accordingly.
- `update_game_state()`: Updates the game state to move to the next question or level.
- `ask_continue()`: Prompts the player to continue playing or end the game after completing a level.
- `display_game_over()`: Displays the game over screen and final score.
- `select_difficulty()`: Allows the player to select the difficulty level at the start of the game.
- `start_menu()`: Displays the start menu and handles user input to start or quit the game.
- `initialisetimer_timer()`: Initializes the timer for the game.
- `check_timer()`: Checks the timer to determine if the question period has elapsed.
- `reset_timer()`: Resets the countdown timer.
- `audio_initialise()`: Initializes the audio peripherals.
- `audio_files_init()`: Initializes the audio files by mounting the file system and reading the welcome audio file.
- `play_sound()`: Plays the sound from the audio buffer.

## Getting Started
To run the Educational Math Game on your DE1-SoC board, follow these steps:
1. Connect the necessary hardware components (slide switches, push buttons, seven-segment display, audio codec, LT24 LCD) to the appropriate pins on the DE1-SoC board.
2. Set up the development environment for the DE1-SoC board, including the necessary drivers and libraries.
3. Compile the provided code and load it onto the DE1-SoC board.
4. Power on the DE1-SoC board and follow the on-screen instructions to play the game.

## Conclusion
The Educational Math Game showcases the capabilities of the DE1-SoC board by utilizing various hardware components to create an interactive and educational gaming experience. It provides a fun and challenging way for players to practice their math skills while enjoying the engaging gameplay. The modular code structure allows for easy extensibility and customization, making it a great starting point for further enhancements and additions to the game.
