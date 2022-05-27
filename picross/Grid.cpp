#include "Globals.h"
#include "Grid.h"

using namespace globals;
using picross::Grid;

Grid::Grid() {
	x = GRID_X;
	y = GRID_Y;
	dx = GRID_DX;
	dy = GRID_DY;
}