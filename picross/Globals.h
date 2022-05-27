#if !defined(GLOBALS_H)
#define GLOBALS_H 1

#include <windows.h>

namespace globals
{
	// Changes as the user resizes the window, used for resizing the picross board
	inline int window_width = 800;
	inline int window_height = 800;

	// Running indicates if the program should still be checking for user inputs, when this is set to false, the program ends
	inline bool running = true;
	// Game over will change the screen to show a message that the puzzle has been completed
	inline bool game_over = false;

	// Last edit refers to the number identifier of what the user last did (adding a space, x, spacer, etc.)
	// it is used solely for holding a button and dragging the mouse so that the user doesn't have to click every space
	inline int last_edit = 0;

	// Board width and height indicate the grid of the picross board, higher numbers make for a harder puzzle
	inline const int BOARD_WIDTH = 5;
	inline const int BOARD_HEIGHT = 5;
	inline const int BOARD_SIZE = BOARD_WIDTH * BOARD_HEIGHT;

	// All Grid variables are somewhat deprecated, they are only used for initializers and change near immediately to fit to the window
	inline const int GRID_X = 0;
	inline const int GRID_Y = 0;
	inline const double GRID_DX = window_width / BOARD_WIDTH;
	inline const double GRID_DY = window_height / BOARD_HEIGHT;

	// The percentage (as a decimal) for how many correct spaces there should be. 
	// This is a random chance so it can vary. It is accurate to 6 decimal places
	inline const double PERCENT_CORRECT = 0.6;

	// Colors for each of the elements of the picross board
	inline const COLORREF BACKGROUND_COLOR = RGB(255, 255, 255);
	inline const COLORREF TEXT_COLOR = RGB(0, 0, 0);
	inline const COLORREF GRID_LINE_COLOR = RGB(100, 100, 100);
	inline const COLORREF SPACE_COLOR = RGB(0, 0, 0);
	inline const COLORREF BLOCK_SPACE_COLOR = RGB(255, 0, 0);
	inline const COLORREF SPACER_COLOR = RGB(150, 150, 150);
	inline const COLORREF SPACER_LINE_COLOR = RGB(150, 150, 150);
	inline const COLORREF NUM_GRID_LINE_COLOR = RGB(100, 100, 100);

	// Random puzzles relies on the time to create a seed, if it is off, the puzzles will start with a set seed
	inline const bool RANDOM_PUZZLES = true;
	// Starts with all the correct spaces filled in if this is true
	inline const bool SHOW_ANSWER = false;
};

#endif