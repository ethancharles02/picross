#include <vector>
#include <array>

#include "Board.h"
#include "Functions.h"
#include "Globals.h"

using namespace globals;
using namespace std;
using picross::Board;

// Initializes the board
Board::Board() :cur_spaces{ 0 }, correct_spaces{ 0 }, highest_column_count{ 0 }, highest_row_count{ 0 } {
	width = BOARD_WIDTH;
	height = BOARD_HEIGHT;

	// The cur_spaces and correct_spaces variables are used to check if it is possible that the current board may be correct.
	// Whenever a space is added, this goes up and once they are equal, it starts checking if they are correct.
	// Sets the current board and the answer board to be completely empty
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			cur_board[x][y] = 0;
			correct_board[x][y] = 0;
		}
	}
}

void Board::update(HWND hwnd) {
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
void Board::draw_grid(HDC hdc, COLORREF color) {
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
void Board::draw_board(HDC hdc, COLORREF block_color, COLORREF x_color, COLORREF spacer_color, COLORREF spacer_line_color) {
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
				SetRect(&rect, x * grid.dx + grid.x + 1, y * grid.dy + grid.y + 1, (x + 1) * grid.dx + grid.x - 1, (y + 1) * grid.dy + grid.y - 1);
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
void Board::draw_num_hints(HDC hdc, COLORREF grid_color) {
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
void Board::add_board(HWND hwnd, int new_board[BOARD_WIDTH][BOARD_HEIGHT], bool current) {
	if (current) {
		add_board(hwnd, new_board);
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
void Board::set_board_space(HWND hwnd, POINT pt, int state) {
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
void Board::invalidate_board_space(HWND hwnd, POINT pt) {
	RECT rect;
	SetRect(&rect, pt.x * grid.dx + grid.x + 1, pt.y * grid.dy + grid.y + 1, (pt.x + 1) * grid.dx + grid.x - 1, (pt.y + 1) * grid.dy + grid.y - 1);
	InvalidateRect(hwnd, &rect, false);
}

// Generates a random board and updates the correct board (and the current board if current is true) with that new board
void Board::generate_board(HWND hwnd, bool current) {
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

	add_board(hwnd, new_board, current);
}

// Checks if the current board is equal to the correct board
// Has to update the whole screen if it is correct since a win message is displayed
bool Board::check_correct(HWND hwnd) {
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
POINT Board::point_to_coords(POINT pt) {
	POINT coords;
	coords.x = floor((pt.x - grid.x) / grid.dx);
	coords.y = floor((pt.y - grid.y) / grid.dy);

	return coords;
}

// Checks if a point is on the board
bool Board::pt_on_board(POINT pt) {
	if (pt.x > grid.x && pt.x < grid.x + grid.dx * width && pt.y > grid.y && pt.y < grid.y + grid.dy * height) {
		return true;
	}
	return false;
}

// Counts up the row spaces and updates the row_nums variable for use in drawing the number hints
void Board::update_row_nums() {

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
int Board::get_highest_row_count() {
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
void Board::update_column_nums() {

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
int Board::get_highest_column_count() {
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