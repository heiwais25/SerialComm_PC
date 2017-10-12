#include "stdafx.h"
#include "utility.h"

BYTE GetOneCharKeyboardInput(void) {
	if (_kbhit() != 0)
		return _getch();

	return 0;
}

// Draw a line in the console to tell different session
void SplitLine(void) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns;

	// Get console size info pointer
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	std::cout << std::endl;
	while (columns--)
		std::cout << "=";
	//std::cout << std::endl;
}