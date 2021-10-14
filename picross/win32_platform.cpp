// Add keyboard controls as alternate to mouse
// Add counter when you drag your mouse while holding a button

// Packages that are used in the program
#include <windows.h>
#include <iostream>
#include <vector>
#include <array>
#include <time.h>

using namespace std;

// Running indicates if the program should still be checking for user inputs, when this is set to false, the program ends
bool running = true;
// Game over will change the screen to show a message that the puzzle has been completed
bool game_over = false;

// Last edit refers to the number identifier of what the user last did (adding a space, x, spacer, etc.)
// it is used solely for holding a button and dragging the mouse so that the user doesn't have to click every space
int last_edit = 0;

// Changes as the user resizes the window, used for resizing the picross board
int window_width = 800;
int window_height = 800;

// Board width and height indicate the grid of the picross board, higher numbers make for a harder puzzle
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 10;

// All Grid variables are somewhat deprecated, they are only used for initializers and change near immediately to fit to the window
const int GRID_X = 0;
const int GRID_Y = 0;
const double GRID_DX = window_width / BOARD_WIDTH;
const double GRID_DY = window_height / BOARD_HEIGHT;

// The percentage (as a decimal) for how many correct spaces there should be. 
// This is a random chance so it can vary. It is accurate to 6 decimal places
const double PERCENT_CORRECT = 0.7;

// Colors for each of the elements of the picross board
const COLORREF BACKGROUND_COLOR = RGB(255, 255, 255);
const COLORREF TEXT_COLOR = RGB(0, 0, 0);
const COLORREF GRID_LINE_COLOR = RGB(100, 100, 100);
const COLORREF SPACE_COLOR = RGB(0, 0, 0);
const COLORREF BLOCK_SPACE_COLOR = RGB(255, 0, 0);
const COLORREF SPACER_COLOR = RGB(150, 150, 150);
const COLORREF SPACER_LINE_COLOR = RGB(150, 150, 150);
const COLORREF NUM_GRID_LINE_COLOR = RGB(100, 100, 100);

// Random puzzles relies on the time to create a seed, if it is off, the puzzles will start with a set seed
const bool RANDOM_PUZZLES = true;
// Starts with all the correct spaces filled in if this is true
const bool SHOW_ANSWER = false;

// Returns true or false based on the percent (as a decimal). Accurate to 6 decimal places.
bool rand_chance(double percent) {
	// Skips any processing if the percent is equal to or above 100%
	if (percent >= 1) {
		return true;
	}

	// Counts the decimal places to decide the range for calculating the change, more decimals goes up in range since they need to be integers
	int num_decimals = 0;
	double num = percent;
	while (num_decimals <= 6) {
		num = num - int(num);
		if (num * 10 > 0) {
			num_decimals++;
			num *= 10;
		}
		else {
			break;
		}
	}
	
	int decimal_range = pow(10, num_decimals);

	bool result = rand() % decimal_range < percent * decimal_range;

	return result;
}

// Grid class for the board, this holds the basic structure for the grid for position and size
class GRID {
	public:
		int x = GRID_X;
		int y = GRID_Y;
		double dx = GRID_DX;
		double dy = GRID_DY;
};

// The board class is the main class for picross. It holds the information for the current board and the correct board as 
// well as providing methods for manipulating and checking these boards
class BOARD {
	public:
		int width = BOARD_WIDTH;
		int height = BOARD_HEIGHT;

		// The cur_spaces and correct_spaces variables are used to check if it is possible that the current board may be correct.
		// Whenever a space is added, this goes up and once they are equal, it starts checking if they are correct.
		int cur_board[BOARD_WIDTH][BOARD_HEIGHT];
		int cur_spaces = 0;

		int correct_board[BOARD_WIDTH][BOARD_HEIGHT];
		int correct_spaces = 0;

		// Holds the information for number hints in the columns and rows. They have to be vectors since the size is unknown before hand
		array<vector<int>, BOARD_WIDTH> column_nums;
		array<vector<int>, BOARD_HEIGHT> row_nums;

		// Used for changing the size of the grid since more number hints need more space
		int highest_column_count = 0;
		int highest_row_count = 0;

		GRID grid;

		BOARD();

		// Should be run whenever the window size changes so that the board size can be adjusted accordingly
		// Should also be run if the number hints have changed
		void update(HWND hwnd) {
			grid.dx = (window_width) / (BOARD_WIDTH + highest_row_count);
			grid.dy = (window_height) / (BOARD_HEIGHT + highest_column_count);

			// Restricts the aspect ratio of the board so that it is 1:1
			if (grid.dx > grid.dy) {
				grid.dx = grid.dy;
			}
			else {
				grid.dy = grid.dx;
			}

			// The actual position of the board is in the top left corner of the playing area
			grid.x = highest_row_count * grid.dx;
			grid.y = highest_column_count * grid.dy;

			// Informs the screen that the entire window needs to be redrawn since the size changed
			InvalidateRect(hwnd, NULL, false);
		}

		// Draws the grid
		void draw_grid(HDC hdc, COLORREF color) {
			HBRUSH grid_brush = CreateSolidBrush(color);

			RECT rect = { 0, 0, 0, 0 };

			for (int column = 0; column <= width; column++) {
				SetRect(&rect, column * grid.dx - 1 + grid.x, grid.y, column * grid.dx + 1 + grid.x, grid.dy * height + grid.y);
				FillRect(hdc, &rect, grid_brush);
			}

			for (int row = 0; row <= height; row++) {
				SetRect(&rect, grid.x, row * grid.dy - 1 + grid.y, grid.dx * width + grid.x, row * grid.dy + 1 + grid.y);
				FillRect(hdc, &rect, grid_brush);
			}

			// Deletes objects to prevent memory leaks
			DeleteObject(grid_brush);

			DeleteObject(&rect);
		}

		// Draws the current board, 0 is an empty space, 1 is a filled space, 2 is an x, 3 is a spacer
		// x is literally just an x in the Arial font. This causes issues if the board is unrealistically massive in rows and columns
		void draw_board(HDC hdc, COLORREF block_color, COLORREF x_color, COLORREF spacer_color, COLORREF spacer_line_color) {
			HBRUSH block_brush = CreateSolidBrush(block_color);
			HBRUSH x_brush = CreateSolidBrush(x_color);
			HBRUSH spacer_brush = CreateSolidBrush(spacer_color);
			HBRUSH spacer_brush_inner = CreateSolidBrush(BACKGROUND_COLOR);
			HPEN spacer_pen = CreatePen(PS_SOLID, 1, spacer_line_color);

			RECT rect = { 0, 0, 0, 0 };

			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					switch (cur_board[x][y])
					{
					case 1: {
						SetRect(&rect, x * grid.dx + grid.x + 1, y * grid.dy + grid.y + 1, (x + 1) * grid.dx + grid.x - 1, (y + 1) * grid.dy + grid.y - 1 );
						FillRect(hdc, &rect, block_brush);
						break;
					} 
					case 2: {
						HFONT hFont;
						hFont = CreateFont(static_cast<int>(grid.dy * 0.9), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
							CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
						SelectObject(hdc, hFont);

						DeleteObject(hFont);

						SetRect(&rect, x * grid.dx + grid.x + 1 + grid.dx / 2, y * grid.dy + grid.y + 1, x * grid.dx + grid.x + 1 + grid.dx / 2, y * grid.dy + grid.y + 1);
						SetTextColor(hdc, x_color);

						DrawText(hdc, L"x", -1, &rect, DT_NOCLIP);
						break;
					} 
					case 3: {
						SelectObject(hdc, spacer_brush);
						SelectObject(hdc, spacer_pen);
						Ellipse(hdc, x * grid.dx + grid.x + 1 + grid.dx / 5, y * grid.dy + grid.y + 1 + grid.dy / 5, (x + 1) * grid.dx + grid.x - 1 - grid.dx / 5, (y + 1) * grid.dy + grid.y - 1 - grid.dy / 5);
						SelectObject(hdc, spacer_brush_inner);
						Ellipse(hdc, x * grid.dx + grid.x + 1 + grid.dx / 3, y * grid.dy + grid.y + 1 + grid.dy / 3, (x + 1) * grid.dx + grid.x - 1 - grid.dx / 3, (y + 1) * grid.dy + grid.y - 1 - grid.dy / 3);
						break;
					}
					}
				}
			}

			DeleteObject(block_brush);
			DeleteObject(x_brush);
			DeleteObject(spacer_brush);
			DeleteObject(spacer_brush_inner);
			DeleteObject(spacer_pen);

			DeleteObject(&rect);
		}

		// Draws the number hints in their corresponding places
		void draw_num_hints(HDC hdc, COLORREF grid_color) {
			HBRUSH num_grid_brush = CreateSolidBrush(grid_color);

			RECT rect = { 0, 0, 0, 0 };

			for (int column = 0; column <= width; column++) {
				SetRect(&rect, column * grid.dx - 1 + grid.x, grid.y - highest_column_count * grid.dy, column * grid.dx + 1 + grid.x, grid.y);
				FillRect(hdc, &rect, num_grid_brush);
			}

			for (int row = 0; row <= height; row++) {
				SetRect(&rect, grid.x - highest_row_count * grid.dx, row * grid.dy - 1 + grid.y, grid.x, row * grid.dy + 1 + grid.y);
				FillRect(hdc, &rect, num_grid_brush);
			}

			DeleteObject(num_grid_brush);

			HFONT hFont;
			hFont = CreateFont(static_cast<int>(grid.dy * 0.9), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
				CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
			SelectObject(hdc, hFont);

			DeleteObject(hFont);

			for (int column = 0; column < width; column++) {
				int iterator = 0;
				for (auto i = column_nums[column].rbegin(); i != column_nums[column].rend(); ++i) {
					wchar_t buffer[8];
					wsprintfW(buffer, L"%d", *i);

					//Sets the coordinates for the rectangle in which the text is to be formatted.
					SetRect(&rect, grid.x + grid.dx * column + grid.dx / 2, grid.y - grid.dy * iterator - grid.dy, grid.x + grid.dx * column + grid.dx / 2, grid.y - grid.dy * iterator - grid.dy);
					SetTextColor(hdc, TEXT_COLOR);
					SetBkColor(hdc, BACKGROUND_COLOR);

					DrawText(hdc, buffer, -1, &rect, DT_NOCLIP);
					DeleteObject(buffer);
					iterator++;
				}
			}

			for (int row = 0; row < height; row++) {
				int iterator = 0;
				for (auto i = row_nums[row].rbegin(); i != row_nums[row].rend(); ++i) {
					wchar_t buffer[8];
					wsprintfW(buffer, L"%d", *i);

					//Sets the coordinates for the rectangle in which the text is to be formatted.
					SetRect(&rect, grid.x - grid.dx * iterator - grid.dx / 2, grid.y + grid.dy * row + 1, grid.x - grid.dx * iterator - grid.dx / 2, grid.y + grid.dy * row + grid.dy);
					SetTextColor(hdc, TEXT_COLOR);
					SetBkColor(hdc, BACKGROUND_COLOR);

					DrawText(hdc, buffer, -1, &rect, DT_NOCLIP);
					DeleteObject(buffer);
					iterator++;
				}
			}

			DeleteObject(&rect);
		}

		// Adds a board to replace the old correct one. If current, it will instead replace the current board
		void add_board(HWND hwnd, int new_board[BOARD_WIDTH][BOARD_HEIGHT], bool current = false) {
			if (current) {
				cur_spaces = 0;
			}
			else {
				correct_spaces = 0;
			}

			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					if (current) {
						cur_board[x][y] = new_board[x][y];
						if (new_board[x][y] == 1) {
							cur_spaces++;
						}
					}
					else {
						correct_board[x][y] = new_board[x][y];
						if (new_board[x][y] == 1) {
							correct_spaces++;
						}
					}
				}
			}

			if (!current) {
				update_column_nums();
				highest_column_count = get_highest_column_count();
				update_row_nums();
				highest_row_count = get_highest_row_count();
				update(hwnd);
			}
		}

		// Updates a position on the board with the state, 0 is an empty space, 1 is a filled space, 2 is an x, 3 is a spacer
		void set_board_space(HWND hwnd, POINT pt, int state) {
			if (cur_board[pt.x][pt.y] == 1 && state != 1) {
				cur_spaces--;
			}
			if (cur_board[pt.x][pt.y] != 1 && state == 1) {
				cur_spaces++;
			}
			cur_board[pt.x][pt.y] = state;
			last_edit = state;
			invalidate_board_space(hwnd, pt);
		}

		// Indicates that a specific part on the board needs to be redrawn since it was updated
		// This is done for optimization. If it updates the entire screen, elements will flicker as they get redrawn.
		void invalidate_board_space(HWND hwnd, POINT pt) {
			RECT rect;
			SetRect(&rect, pt.x * grid.dx + grid.x + 1, pt.y * grid.dy + grid.y + 1, (pt.x + 1) * grid.dx + grid.x - 1, (pt.y + 1) * grid.dy + grid.y - 1);
			InvalidateRect(hwnd, &rect, false);
		}

		// Generates a random board and updates the correct board (and the current board if current is true) with that new board
		void generate_board(HWND hwnd, bool current = false) {
			int new_board[BOARD_WIDTH][BOARD_HEIGHT];

			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					if (rand_chance(PERCENT_CORRECT)) {
						new_board[x][y] = 1;
					}
					
					cur_board[x][y] = 0;
				}
			}
			cur_spaces = 0;

			add_board(hwnd, new_board);
			if (current) {
				add_board(hwnd, new_board, current);
			}
		}

		// Checks if the current board is equal to the correct board
		// Has to update the whole screen if it is correct since a win message is displayed
		bool check_correct(HWND hwnd) {
			for (int x = 0; x < BOARD_WIDTH; x++) {
				for (int y = 0; y < BOARD_HEIGHT; y++) {
					if ((correct_board[x][y] == 1 && cur_board[x][y] != 1) || (correct_board[x][y] != 1 && cur_board[x][y] == 1)) {
						return false;
					}
				}
			}
			InvalidateRect(hwnd, NULL, false);
			return true;
		}

		// Converts a point on the screen to a specific grid space on the board
		POINT point_to_coords(POINT pt) {
			POINT coords;
			coords.x = floor((pt.x - grid.x) / grid.dx);
			coords.y = floor((pt.y - grid.y) / grid.dy);

			return coords;
		}

		// Checks if a point is on the board
		bool pt_on_board(POINT pt) {
			if (pt.x > grid.x && pt.x < grid.x + grid.dx * width && pt.y > grid.y && pt.y < grid.y + grid.dy * height) {
				return true;
			}
			return false;
		}

	private:
		// Counts up the row spaces and updates the row_nums variable for use in drawing the number hints
		void update_row_nums() {

			int iterator = 0;
			for (int y = 0; y < height; y++) {
				row_nums[y].clear();
				for (int x = 0; x < width; x++) {
					if (correct_board[x][y] == 1) {
						iterator++;
					}
					else if (iterator != 0) {
						row_nums[y].push_back(iterator);
						iterator = 0;
					}
				}
				if (iterator != 0) {
					row_nums[y].push_back(iterator);
					iterator = 0;
				}
			}
		}

		// Returns the count of the highest number of hints
		int get_highest_row_count() {
			int count = 0;
			int size = 0;
			for (int i = 0; i < BOARD_HEIGHT; i++) {
				size = row_nums[i].size();
				if (size > count) {
					count = size;
				}
			}
			return count;
		}

		// Same as the row updating method
		void update_column_nums() {

			int iterator = 0;
			for (int x = 0; x < width; x++) {
				column_nums[x].clear();
				for (int y = 0; y < height; y++) {
					if (correct_board[x][y] == 1) {
						iterator++;
					}
					else if (iterator != 0) {
						column_nums[x].push_back(iterator);
						iterator = 0;
					}
				}
				if (iterator != 0) {
					column_nums[x].push_back(iterator);
					iterator = 0;
				}
			}
		}

		// Same as the row count method
		int get_highest_column_count() {
			int count = 0;
			int size = 0;
			for (int i = 0; i < BOARD_WIDTH; i++) {
				size = column_nums[i].size();
				if (size > count) {
					count = size;
				}
			}
			return count;
		}
};

// Initializes the board to be completely empty
BOARD::BOARD(void) {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			cur_board[x][y] = 0;
		}
	}
}

BOARD board;

// This is the main drawing function, it either draws the win screen or the board
void draw_window_objects(HWND hwnd, bool clearscreen=true) {
	RECT rect;

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	SetTextAlign(hdc, TA_CENTER);

	HBRUSH brush = CreateSolidBrush(BACKGROUND_COLOR);
	SetRect(&rect, 0, 0, 10, 10);
	if (clearscreen) {
		SetRect(&rect, 0, 0, window_width - 1, window_height -1);
		FillRect(hdc, &rect, brush);
	}

	if (game_over) {
		LPCWSTR game_over_string = L"Puzzle Finished! Click to start a new puzzle";
		TextOut(hdc, window_width / 2, window_height / 2, game_over_string, wcslen(game_over_string));
	}
	else {
		board.draw_grid(hdc, GRID_LINE_COLOR);
		board.draw_board(hdc, SPACE_COLOR, BLOCK_SPACE_COLOR, SPACER_COLOR, SPACER_LINE_COLOR);
		board.draw_num_hints(hdc, NUM_GRID_LINE_COLOR);
	}

	EndPaint(hwnd, &ps);
}

// This function handles any click related actions by the user. If the user is holding a click button and dragging, mouse_moving is true
// Left clicking adds or removes a space or an x
// Right clicking adds or removes an x or a space
// Shift clicking adds a spacer
// Both left and right click will directly override a spacer with the corresponding element
// Clicking and dragging will add or remove the corresponding space to each grid hovered over
// If the space was removed, it will remove all elements on dragging, x's do not override spaces, spaces don't override x's, spacers will be overridden
void handle_click(HWND hwnd, WPARAM wParam, LPARAM lParam, bool mouse_moving = false) {
	if (game_over && !mouse_moving) {
		// Create a new board since the user clicked on the game over screen
		board.generate_board(hwnd, SHOW_ANSWER);
		game_over = false;
	}
	else {
		bool shiftClick = wParam == MK_LBUTTON + MK_SHIFT || wParam == MK_RBUTTON + MK_SHIFT;
		bool lClick = wParam == MK_LBUTTON && !shiftClick;
		bool rClick = wParam == MK_RBUTTON && !shiftClick;
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);

		if (board.pt_on_board(pt)) {
			POINT coords = board.point_to_coords(pt);

			switch (board.cur_board[coords.x][coords.y]) {
			case 0: {
				if (mouse_moving) {
					board.set_board_space(hwnd, coords, last_edit);
				}
				else {
					if (lClick) {
						board.set_board_space(hwnd, coords, 1);
					}
					else if (rClick) {
						board.set_board_space(hwnd, coords, 2);
					}
					else if (shiftClick) {
						board.set_board_space(hwnd, coords, 3);
					}
				}
				break;
			}
			case 1: {
				if (mouse_moving) {
					if (last_edit == 0) {
						board.set_board_space(hwnd, coords, last_edit);
					}
				}
				else {
					if (lClick || rClick) {
						board.set_board_space(hwnd, coords, 0);
					}
				}
				break;
			}
			case 2: {
				if (mouse_moving) {
					if (last_edit == 0) {
						board.set_board_space(hwnd, coords, last_edit);
					}
				}
				else {
					if (lClick || rClick) {
						board.set_board_space(hwnd, coords, 0);
					}
				}
				break;
			}
			case 3: {
				if (mouse_moving) {
					if (last_edit != 3) {
						board.set_board_space(hwnd, coords, last_edit);
					}
				}
				else {
					if (lClick) {
						board.set_board_space(hwnd, coords, 1);
					}
					else if (rClick) {
						board.set_board_space(hwnd, coords, 2);
					}
					else if (shiftClick) {
						board.set_board_space(hwnd, coords, 0);
					}
				}
				break;
			}
			}

			if (board.cur_spaces == board.correct_spaces) {
				game_over = board.check_correct(hwnd);
			}
		}
	}
}

// Windows message handling function
// This is the function that handles all of the user inputs (and messages that result from those inputs)
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (uMsg) 
	{
	case WM_CLOSE: {
		DestroyWindow(hwnd);
		return 0;
	}
		
	case WM_DESTROY: {
		running = false;
		PostQuitMessage(0);
		return 0;
	}

	case WM_LBUTTONDOWN: {
		handle_click(hwnd, wParam, lParam);
		SendMessage(hwnd, WM_PAINT, NULL, NULL);
		return 0;
	}

	case WM_RBUTTONDOWN: {
		handle_click(hwnd, wParam, lParam);
		SendMessage(hwnd, WM_PAINT, NULL, NULL);
		return 0;
	}

	case WM_KEYDOWN: {
		// 0x52 is the R key
		// Resets the board, since this program isn't smart enough to tell if a board is completable, I have increased the amount of correct spaces
		// Increasing the spaces has made it much easier but the majority of games should be completable, however, if necessary the user can reset it
		if (wParam == 0x52) {
			board.generate_board(hwnd, SHOW_ANSWER);
		}
		return 0;
	}

	// Used to handle mouse dragging
	case WM_MOUSEMOVE: {
		if (wParam == MK_LBUTTON || wParam == MK_RBUTTON || wParam == MK_LBUTTON + MK_SHIFT || wParam == MK_RBUTTON + MK_SHIFT) {
			POINT pt; 
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (board.pt_on_board(pt)) {
				handle_click(hwnd, wParam, lParam, true);
				SendMessage(hwnd, WM_PAINT, NULL, NULL);
			}
		}
		return 0;
	}

	case WM_SIZE: {
		window_width = LOWORD(lParam);
		window_height = HIWORD(lParam);

		board.update(hwnd);
	}

	case WM_PAINT: {
		draw_window_objects(hwnd, true);
		return 0;
	}

	default: {
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
	return result;
}

// This is the main function which is run first
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Picross Window Class";

	WNDCLASS wc = { };

	wc.lpfnWndProc = window_callback;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Picross",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	// Randomizes the seed
	if (RANDOM_PUZZLES) {
		srand(time(NULL));
	}
	board.generate_board(hwnd, SHOW_ANSWER);

	ShowWindow(hwnd, nCmdShow);

	// This is the main loop of the program, it handles inputs
	while (running) {
		MSG msg = { };
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}