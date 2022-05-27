// Add keyboard controls as alternate to mouse
// Add counter when you drag your mouse while holding a button

#include <windows.h>
#include <fstream>
#include <string>

#include "Globals.h"
#include "Board.h"

using namespace globals;
using namespace std;

using picross::Board;

Board board;

// Checks if a file exists
bool file_exists(const string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

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
			case 0:
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

			case 1: 
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
			

			case 2: 
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
			
			case 3: 
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
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
		
	case WM_DESTROY:
		running = false;
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
		handle_click(hwnd, wParam, lParam);
		SendMessage(hwnd, WM_PAINT, NULL, NULL);
		return 0;

	case WM_RBUTTONDOWN:
		handle_click(hwnd, wParam, lParam);
		SendMessage(hwnd, WM_PAINT, NULL, NULL);
		return 0;

	case WM_KEYDOWN: 
		// 0x52 is the R key
		// Resets the board, since this program isn't smart enough to tell if a board is completable, I have increased the amount of correct spaces
		// Increasing the spaces has made it much easier but the majority of games should be completable, however, if necessary the user can reset it
		if (wParam == 0x52) {
			board.generate_board(hwnd, SHOW_ANSWER);
		}
		if (wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
		}
		return 0;

	// Used to handle mouse dragging
	case WM_MOUSEMOVE: 
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

	case WM_SIZE:
		window_width = LOWORD(lParam);
		window_height = HIWORD(lParam);

		board.update(hwnd);
		[[fallthrough]];

	case WM_PAINT: 
		draw_window_objects(hwnd, true);
		return 0;

	default: 
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return result;
}

// This is the main function which is run first
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
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
		board.generate_board(hwnd, SHOW_ANSWER);
	}
	// If a bitstring file exists, it will use that to populate the picross board
	else if (file_exists("bitstring.txt")) {
		string line;
		ifstream bitFile ("bitstring.txt");

		// Creates a new board to add to the original board
		int new_board[BOARD_WIDTH][BOARD_HEIGHT];
		if (bitFile.is_open())
		{
			getline(bitFile, line);
			bitFile.close();
			
			for(int i = 0; i < BOARD_SIZE;i++)
			{
				new_board[i % BOARD_WIDTH][i / BOARD_HEIGHT] = (int) line[i] - '0';
			}
			board.add_board(hwnd, new_board, SHOW_ANSWER);
		}
		else board.generate_board(hwnd, SHOW_ANSWER);
	}

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
	return 0;
}