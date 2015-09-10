#include "DatabaseSetting.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
#include "Log.h"

DatabaseSetting::DatabaseSetting()
{
}


void DatabaseSetting::Start()
{
    clear_screen();
    sLog.outString("Database wizard.");
    sLog.outString("================");
    sLog.outString();
    do 
    {
        switch (TryToConnectAsRoot())
        {
            case NEXT_STEP_RETRY: continue; break;
            case NEXT_STEP_RETURN: return; break;
            case NEXT_STEP_CONTINUE: break;
            default: return;
        }
        break;
    } while (1);
}

bool DatabaseSetting::Complete()
{
    return false;
}

DatabaseSetting::~DatabaseSetting()
{
}

nextStep DatabaseSetting::TryToConnectAsRoot()
{
    throw std::logic_error("The method or operation is not implemented.");
    return NEXT_STEP_RETURN;
}
