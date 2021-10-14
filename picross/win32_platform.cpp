// Add a button to reset the board
// Add keyboard controls as alternate to mouse
// Add counter when you drag your mouse while holding a button

#include <windows.h>
#include <iostream>
#include <vector>
#include <array>
#include <time.h>

using namespace std;

bool running = true;
bool game_over = false;
bool skip_size = false;
bool skip_paint = false;
int last_edit = 0;

//const int screen_width = 1920;
//const int screen_height = 1080;
int window_width = 800;
int window_height = 800;

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 10;
const int GRID_X = 0;
const int GRID_Y = 0;
const double GRID_DX = window_width / BOARD_WIDTH;
const double GRID_DY = window_height / BOARD_HEIGHT;

const double PERCENT_CORRECT = 0.7;

const COLORREF GRID_LINE_COLOR = RGB(100, 100, 100);
const COLORREF SPACE_COLOR = RGB(0, 0, 0);
const COLORREF BLOCK_SPACE_COLOR = RGB(255, 0, 0);
const COLORREF SPACER_COLOR = RGB(150, 150, 150);
const COLORREF SPACER_LINE_COLOR = RGB(150, 150, 150);
const COLORREF NUM_GRID_LINE_COLOR = RGB(100, 100, 100);
const COLORREF NUM_COLOR = RGB(0, 0, 0);

const bool random_puzzles = true;
const bool show_answer = false;

//int window_x = 0;
//int window_y = 0;

bool rand_chance(double percent) {
	if (percent >= 1) {
		return true;
	}

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

class GRID {
	public:
		int x = GRID_X;
		int y = GRID_Y;
		double dx = GRID_DX;
		double dy = GRID_DY;
};

class BOARD {
	public:
		int width = BOARD_WIDTH;
		int height = BOARD_HEIGHT;

		int cur_board[BOARD_WIDTH][BOARD_HEIGHT];
		//vector<int> cur_board;
		int cur_spaces = 0;

		int correct_board[BOARD_WIDTH][BOARD_HEIGHT];
		//vector<int> correct_board;
		int correct_spaces = 0;

		array<vector<int>, BOARD_WIDTH> column_nums;
		array<vector<int>, BOARD_HEIGHT> row_nums;

		int highest_column_count = 0;
		int highest_row_count = 0;

		GRID grid;

		BOARD();

		void update(HWND hwnd) {
			grid.dx = (window_width) / (BOARD_WIDTH + highest_row_count);
			grid.dy = (window_height) / (BOARD_HEIGHT + highest_column_count);

			if (grid.dx > grid.dy) {
				grid.dx = grid.dy;
			}
			else {
				grid.dy = grid.dx;
			}

			grid.x = highest_row_count * grid.dx;
			grid.y = highest_column_count * grid.dy;

			InvalidateRect(hwnd, NULL, false);
		}

		void draw_grid(HDC hdc, COLORREF color) {
			HBRUSH grid_brush = CreateSolidBrush(color);

			RECT rect = { 0, 0, 0, 0 };
			/*float dx = window_width / width;
			float dy = window_height / height;*/

			for (int column = 0; column <= width; column++) {
				SetRect(&rect, column * grid.dx - 1 + grid.x, grid.y, column * grid.dx + 1 + grid.x, grid.dy * height + grid.y);
				FillRect(hdc, &rect, grid_brush);
			}

			for (int row = 0; row <= height; row++) {
				SetRect(&rect, grid.x, row * grid.dy - 1 + grid.y, grid.dx * width + grid.x, row * grid.dy + 1 + grid.y);
				FillRect(hdc, &rect, grid_brush);
			}

			DeleteObject(grid_brush);

			DeleteObject(&rect);
		}

		void draw_board(HDC hdc, COLORREF block_color, COLORREF x_color, COLORREF spacer_color, COLORREF spacer_line_color) {
			HBRUSH block_brush = CreateSolidBrush(block_color);
			HBRUSH x_brush = CreateSolidBrush(x_color);
			HBRUSH spacer_brush = CreateSolidBrush(spacer_color);
			HBRUSH spacer_brush_inner = CreateSolidBrush(RGB(255, 255, 255));
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

						//SetRect(&rect, x * grid.dx + grid.x + 1, y * grid.dy + grid.y + 1, (x + 1) * grid.dx + grid.x - 1, (y + 1) * grid.dy + grid.y - 1);
						SetRect(&rect, x * grid.dx + grid.x + 1 + grid.dx / 2, y * grid.dy + grid.y + 1, x * grid.dx + grid.x + 1 + grid.dx / 2, y * grid.dy + grid.y + 1);
						//FillRect(hdc, &rect, x_brush);
						SetTextColor(hdc, x_color);

						DrawText(hdc, L"x", -1, &rect, DT_NOCLIP);
						break;
					} 
					case 3: {
						//spacer_brush
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

		void draw_num_hints(HDC hdc, COLORREF grid_color, COLORREF num_color) {
			HBRUSH num_grid_brush = CreateSolidBrush(grid_color);

			RECT rect = { 0, 0, 0, 0 };
			/*float dx = window_width / width;
			float dy = window_height / height;*/

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
					SetTextColor(hdc, RGB(0, 0, 0));

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
					SetTextColor(hdc, RGB(0, 0, 0));

					DrawText(hdc, buffer, -1, &rect, DT_NOCLIP);
					DeleteObject(buffer);
					iterator++;
				}
			}

			DeleteObject(&rect);
		}

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

		void set_grid_space(HWND hwnd, POINT pt, int state) {
			if (cur_board[pt.x][pt.y] == 1 && state != 1) {
				cur_spaces--;
			}
			if (cur_board[pt.x][pt.y] != 1 && state == 1) {
				cur_spaces++;
			}
			cur_board[pt.x][pt.y] = state;
			last_edit = state;
			invalidate_grid_space(hwnd, pt);
		}

		void invalidate_grid_space(HWND hwnd, POINT pt) {
			RECT rect;
			SetRect(&rect, pt.x * grid.dx + grid.x + 1, pt.y * grid.dy + grid.y + 1, (pt.x + 1) * grid.dx + grid.x - 1, (pt.y + 1) * grid.dy + grid.y - 1);
			InvalidateRect(hwnd, &rect, false);
		}

		void generate_board(HWND hwnd, bool current = false) {
			int new_board[BOARD_WIDTH][BOARD_HEIGHT];

			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					if (rand_chance(PERCENT_CORRECT)) {
						new_board[x][y] = 1;
					}
					
					//int num = rand() % 2;
					cur_board[x][y] = 0;
				}
			}
			cur_spaces = 0;

			add_board(hwnd, new_board);
			if (current) {
				add_board(hwnd, new_board, current);
			}
		}

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

		POINT point_to_coords(POINT pt) {
			POINT coords;
			coords.x = floor((pt.x - grid.x) / grid.dx);
			coords.y = floor((pt.y - grid.y) / grid.dy);

			return coords;
		}

		bool pt_on_board(POINT pt) {
			if (pt.x > grid.x && pt.x < grid.x + grid.dx * width && pt.y > grid.y && pt.y < grid.y + grid.dy * height) {
				return true;
			}
			return false;
		}

	private:
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

BOARD::BOARD(void) {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			cur_board[x][y] = 0;
		}
	}
}

BOARD board;


void draw_window_objects(HWND hwnd, bool clearscreen=true) {
	//RECT rect = { 0, 0, window_width - 1, window_height - 1 };
	RECT rect;

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	SetTextAlign(hdc, TA_CENTER);

	HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
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
		board.draw_num_hints(hdc, NUM_GRID_LINE_COLOR, NUM_COLOR);
	}
	//draw_grid(hdc, 100, 100, 20, 20, 4, 4, RGB(100, 100, 100));

	EndPaint(hwnd, &ps);
}

void handle_click(HWND hwnd, WPARAM wParam, LPARAM lParam, bool mouse_moving = false) {
	if (game_over && !mouse_moving) {
		board.generate_board(hwnd, show_answer);
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
					board.set_grid_space(hwnd, coords, last_edit);
				}
				else {
					if (lClick) {
						board.set_grid_space(hwnd, coords, 1);
					}
					else if (rClick) {
						board.set_grid_space(hwnd, coords, 2);
					}
					else if (shiftClick) {
						board.set_grid_space(hwnd, coords, 3);
					}
				}
				break;
			}
			case 1: {
				if (mouse_moving) {
					if (last_edit == 0) {
						board.set_grid_space(hwnd, coords, last_edit);
					}
				}
				else {
					if (lClick || rClick) {
						board.set_grid_space(hwnd, coords, 0);
					}
				}
				break;
			}
			case 2: {
				if (mouse_moving) {
					if (last_edit == 0) {
						board.set_grid_space(hwnd, coords, last_edit);
					}
				}
				else {
					if (lClick || rClick) {
						board.set_grid_space(hwnd, coords, 0);
					}
				}
				break;
			}
			case 3: {
				if (mouse_moving) {
					if (last_edit != 3) {
						board.set_grid_space(hwnd, coords, last_edit);
					}
				}
				else {
					if (lClick) {
						board.set_grid_space(hwnd, coords, 1);
					}
					else if (rClick) {
						board.set_grid_space(hwnd, coords, 2);
					}
					else if (shiftClick) {
						board.set_grid_space(hwnd, coords, 0);
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
	//skip_size = true;
}

LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (uMsg) 
	{
	case WM_CLOSE: {
		//MessageBox(hwnd, L"Test", L"Test", MB_OKCANCEL);
		DestroyWindow(hwnd);
		return 0;
	}
		
	case WM_DESTROY: {
		running = false;
		PostQuitMessage(0);
		//DestroyWindow(hwnd);
		return 0;
	}

	case WM_LBUTTONDOWN: {
		//if (wParam != MK_RBUTTON) {
		handle_click(hwnd, wParam, lParam);
		SendMessage(hwnd, WM_PAINT, NULL, NULL);
		//}
		return 0;
	}

	case WM_RBUTTONDOWN: {
		//if (wParam != MK_LBUTTON && wParam != MK_LBUTTON + MK_SHIFT) {
		handle_click(hwnd, wParam, lParam);
		SendMessage(hwnd, WM_PAINT, NULL, NULL);
		//}
		return 0;
	}

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
		if (!skip_size) {
			window_width = LOWORD(lParam);
			window_height = HIWORD(lParam);

			board.update(hwnd);
		}
		else {
			skip_size = false;
		}

		//draw_window_objects(hwnd, true);
	}

	case WM_PAINT: {
		if (!skip_paint) {
			draw_window_objects(hwnd, true);
		}
		else {
			skip_paint = false;
		}
		return 0;
	}

	default: {
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
	return result;
}

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

	if (random_puzzles) {
		srand(time(NULL));
	}
	board.generate_board(hwnd, show_answer);

	/*board.cur_board[0][0] = 1;
	board.cur_board[1][0] = 1;
	board.cur_board[0][1] = 1;*/

	ShowWindow(hwnd, nCmdShow);
	//UpdateWindow(hwnd);

	while (running) {
		MSG msg = { };
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}