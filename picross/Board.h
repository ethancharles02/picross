#include <vector>
#include <array>

#include "Grid.h"
#include "Globals.h"

using picross::Grid;
using namespace std;
using namespace globals;

// The board class is the main class for picross. It holds the information for the current board and the correct board as 
// well as providing methods for manipulating and checking these boards
namespace picross {
	class Board {
	public:
		int width;
		int height;

		// The cur_spaces and correct_spaces variables are used to check if it is possible that the current board may be correct.
		// Whenever a space is added, this goes up and once they are equal, it starts checking if they are correct.
		int cur_board[BOARD_WIDTH][BOARD_HEIGHT];
		int cur_spaces;

		int correct_board[BOARD_WIDTH][BOARD_HEIGHT];
		int correct_spaces;

		// Holds the information for number hints in the columns and rows. They have to be vectors since the size is unknown before hand
		array<vector<int>, BOARD_WIDTH> column_nums;
		array<vector<int>, BOARD_HEIGHT> row_nums;

		// Used for changing the size of the grid since more number hints need more space
		int highest_column_count;
		int highest_row_count;

		Grid grid;

		Board();

		// Should be run whenever the window size changes so that the board size can be adjusted accordingly
		// Should also be run if the number hints have changed
		void update(HWND hwnd);

		// Draws the grid
		void draw_grid(HDC hdc, COLORREF color);

		// Draws the current board, 0 is an empty space, 1 is a filled space, 2 is an x, 3 is a spacer
		// x is literally just an x in the Arial font. This causes issues if the board is unrealistically massive in rows and columns
		void draw_board(HDC hdc, COLORREF block_color, COLORREF x_color, COLORREF spacer_color, COLORREF spacer_line_color);

		// Draws the number hints in their corresponding places
		void draw_num_hints(HDC hdc, COLORREF grid_color);

		// Adds a board to replace the old correct one. If current, it will instead replace the current board
		void add_board(HWND hwnd, int new_board[BOARD_WIDTH][BOARD_HEIGHT], bool current = false);

		// Updates a position on the board with the state, 0 is an empty space, 1 is a filled space, 2 is an x, 3 is a spacer
		void set_board_space(HWND hwnd, POINT pt, int state);

		// Indicates that a specific part on the board needs to be redrawn since it was updated
		// This is done for optimization. If it updates the entire screen, elements will flicker as they get redrawn.
		void invalidate_board_space(HWND hwnd, POINT pt);

		// Generates a random board and updates the correct board (and the current board if current is true) with that new board
		void generate_board(HWND hwnd, bool current = false);

		// Checks if the current board is equal to the correct board
		// Has to update the whole screen if it is correct since a win message is displayed
		bool check_correct(HWND hwnd);

		// Converts a point on the screen to a specific grid space on the board
		POINT point_to_coords(POINT pt);

		// Checks if a point is on the board
		bool pt_on_board(POINT pt);

	private:
		// Counts up the row spaces and updates the row_nums variable for use in drawing the number hints
		void update_row_nums();

		// Returns the count of the highest number of hints
		int get_highest_row_count();

		// Same as the row updating method
		void update_column_nums();

		// Same as the row count method
		int get_highest_column_count();
	};
}