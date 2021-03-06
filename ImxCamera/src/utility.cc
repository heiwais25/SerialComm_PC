#include "stdafx.h"
#include "utility.h"


void ChangeColor(AnsiColorCode vec_color_code);

// Return keyboard input spontaneuously
BYTE GetOneCharKeyboardInput(void) {
	if (_kbhit() != 0)
		return _getch();
	return 0;
}

BOOL isDecimalNumber(BYTE c) {
	int number = c - 0x30;
	return number >= 0 && number <= 9 ? true : false;
}

int toDecimalNumber(BYTE c) {
	return isDecimalNumber(c) ? c - 0x30 : -1;
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
}

void ReadColorFont(BYTE c) {
	static int color_count = 0;
	static bool is_color_code = false;
	static int color_code = 0;
	if (!is_color_code) {
		if (c == 0x1b) 
			is_color_code = true;
		else 
			std::cout << c;
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

// It is for windows visual studio. If you change OS to linux, please change or remove this
void ChangeColor(AnsiColorCode color_code) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::map<AnsiColorCode, CmdColorCode>::const_iterator const_iter;
	const_iter = m_color_map.find(color_code);
	if (const_iter != m_color_map.end())
		SetConsoleTextAttribute(hConsole, const_iter->second);
	else
		std::cout << "It doesn't have color code at " << color_code << std::endl;
}

int getValueLowerThanMaximum(int maximum_value) {
	int output = 0;
	while (1) {
		std::cout << "Input lower than " << maximum_value << " : ";
		std::cin >> output;
		if (std::cin.fail()) {
			std::cout << "Input correct integer number" << std::endl;
			std::cin.clear();
			std::cin.ignore();
			continue;
		}
		if (output <= maximum_value) 
			return output;
		else 
			std::cout << "Please input lowerthan " << maximum_value << std::endl;
	}
}

int getCurrentDayHourMinuteSec() {
	std::time_t t = time(0);
	struct std::tm * now = localtime(&t);
	int time_info = time_info = now->tm_mday * 1000000  + now->tm_hour * 10000 + now->tm_min * 100 + now->tm_sec;
	return time_info;
}

// Take the address of position
void gotoxy(int x, int y) {
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

int DrawPercentageArrow(int parts_length, int total_length) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns, center_rows;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left;
	//center_rows = -1 * (csbi.srWindow.Top - csbi.srWindow.Bottom) / 2;
	center_rows = 0;
	std::vector<char> arrow(columns + 1, '=');
	int percentage = ((double)parts_length / (double)total_length) * (double)columns;

	gotoxy(percentage, center_rows);
	if (percentage == 0)
		for (int j = 0; j < arrow.size() - 1; j++) 
			std::cout << arrow[j];
	gotoxy(percentage, center_rows);
	arrow[percentage] = '>';
	std::cout << arrow[percentage];
	std::cout << std::endl;

	return 0;
}