# Snake Game

An OpenGL-based Snake game with sound, score tracking, and history functionality.

## Features
- Classic Snake gameplay with arrow keys and WASD controls
- Score tracking and high score history
- Sound effects for eating food and game over
- Modern OpenGL graphics

## Sound Files
The game requires two sound files in the same directory as the executable:
1. `eat.wav` - Played when the snake eats food
2. `gameover.wav` - Played when the game ends

You can use any WAV files with these names or create your own.

## Controls
- Arrow keys or WASD to move the snake
- Space to start the game or restart after game over
- M to return to the menu from the game over screen
- ESC to exit the game

## Compilation
This project uses C++11 features and requires the following libraries:
- OpenGL
- GLUT
- Windows multimedia library (for sound)

The project is configured for Code::Blocks with MinGW.
