#ifndef _DATABASE_SETTING
#define  _DATABASE_SETTING

#ifdef WIN32
#include "windows.h"

void clear_screen(char fill = ' ') {
    COORD tl = { 0,0 };
    CONSOLE_SCREEN_BUFFER_INFO s;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);
}

#else
void clear_screen()
{
    write(1, "\E[H\E[2J", 7);
}
#endif

class DatabaseSetting
{
public:
    DatabaseSetting();
    ~DatabaseSetting();
};
#endif

