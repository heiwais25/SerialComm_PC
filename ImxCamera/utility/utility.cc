#include "stdafx.h"
#include "utility.h"

void ChangeColor(AnsiColorCode vec_color_code);


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

void ReadColorFont(BYTE c) {
	static int color_count = 0;
	static bool is_color_code = false;
	static int color_code = 0;
	if (!is_color_code) {
		if (c == 0x1b) {
			is_color_code = true;
		}
		else {
			std::cout << c;
		}
	}
	else {
		color_count++;
		// 1. [
		// 2~3 color code
		// 4. m
		if (color_count == 2 || color_count == 3) {
			// Start color message(c- 0x30 : Ascii to int)
			color_code = color_code * 10 + (c - 0x30);
		}
		else if (color_count == 4) {
			ChangeColor(static_cast<AnsiColorCode>(color_code));
			is_color_code = false;
			color_count = 0;
			color_code = 0;
		}
	}
}

void ChangeColor(AnsiColorCode color_code) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::map<AnsiColorCode, CmdColorCode>::const_iterator const_iter;
	const_iter = m_color_map.find(color_code);
	if (const_iter != m_color_map.end())
		SetConsoleTextAttribute(hConsole, const_iter->second);
	else
		std::cout << "It doesn't have color code at " << color_code << std::endl;
}