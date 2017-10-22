#pragma once
#include <Windows.h>
#include <conio.h>
#include <iostream>
#include <map>
#include <ctime>


BYTE GetOneCharKeyboardInput(void);

// Draw a line in the console to tell different session
void SplitLine(void);
void ReadColorFont(BYTE c);

// Indicate the input c is whether the interger value or not
BOOL isDecimalNumber(BYTE c);

// Return current time info
int getCurrentDayHourMinuteSec();

// Color code table
enum AnsiColorCode {
	ANSI_DEFAULT = 0,
	ANSI_BOLD = 1,
	ANSI_RED = 31,
	ANSI_GREEN = 32,
	ANSI_BLUE = 34,
	ANSI_MAGNETA = 35,
	ANSI_WHITE = 37,
};

enum CmdColorCode {
	CMD_DEFAULT = 0,
	CMD_BLUE = 1,
	CMD_GREEN = 2,
	CMD_RED = 4,
	CMD_MAGNETA = 5,
	CMD_WHITE = 7,
	CMD_BOLD = 8,
};

const std::map<AnsiColorCode, CmdColorCode> m_color_map =
{ {ANSI_DEFAULT, CMD_WHITE }, {ANSI_BOLD, CMD_BOLD}, {ANSI_BLUE, CMD_BLUE},{ANSI_RED, CMD_RED},
	{ANSI_GREEN, CMD_GREEN}, {ANSI_WHITE, CMD_WHITE}, {ANSI_MAGNETA, CMD_MAGNETA}};