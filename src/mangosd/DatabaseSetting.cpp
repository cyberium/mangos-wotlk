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

using namespace std;

DatabaseSetting::DatabaseSetting()
{
}


void DatabaseSetting::ClearScreen()
{
    clear_screen();
    sLog.outString("Database installation and update wizard.");
    sLog.outString("========================================");
    sLog.outString();
}

void DatabaseSetting::Start()
{
    do 
    {
        ClearScreen();
        switch (SetDBCredential())
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

bool DatabaseSetting::GetUserInput(std::string &buffer)
{
    buffer.clear();
    char buff[256];
    char* pbuff = fgets(buff, sizeof(buff), stdin);
    if (pbuff != NULL)
    {
        for (int x = 0; x < sizeof(buff); ++x)
        {
            if (pbuff[x] == '\r' || pbuff[x] == '\n')
                break;
            else
                buffer += pbuff[x];
        }
    }
    else
        return false;
    return true;
}


nextStep DatabaseSetting::SetDBCredential()
{
    string answer;
    sLog.outString("1 - Server address configuration.");
    sLog.outString("--------------------------------");
    sLog.outString("The following setting is your mysql server address.");
    sLog.outString("Please enter your address like '192.168.1.11' or 'your.server.com' then your port (default is 'localhost' and '3306').");
    sLog.SetColor(true, YELLOW);
    do
    {
        printf("Please type your server address [localhost]:");
        sLog.SetColor(true, WHITE);
        if (GetUserInput(m_serverAddress))
            m_serverAddress = "localhost";
        sLog.SetColor(true, YELLOW);
        printf("You choose '%s' as server address, is it correct(Y/N)?", m_serverAddress.c_str());
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(answer) || (answer[0] != 'y' && answer[0] != 'Y'))
            continue;
        else
            break;
    } while (1);
    do
    {
        sLog.SetColor(true, YELLOW);
        printf("Please type your server Port [3306]:");
        sLog.SetColor(true, WHITE);
        if (GetUserInput(m_serverPort))
            m_serverPort = "3306";
        sLog.SetColor(true, YELLOW);
        printf("You choose '%s' as server port, is it correct(Y/N)?", m_serverPort.c_str());
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(answer) || (answer[0] != 'y' && answer[0] != 'Y'))
            continue;
        else
            break;
    } while (1);
    sLog.ResetColor(true);

    ClearScreen();
    sLog.outString("2 - Server access configuration.");
    sLog.outString("--------------------------------");
    sLog.outString("The following setting require db access, we need to know if you have root access or normal access.");
    sLog.outString("If you have root access we could handle the 3 databases and the normal user access creation automatically.");
    sLog.outString("If you have only normal access you should have by any other way create yourself the required database and provide us the correct credentials to access them.");
    sLog.outString();
    sLog.SetColor(true, YELLOW);
    do
    {
        printf("Do you have root access to db or normal access(R/N)?");
        sLog.SetColor(true, WHITE);
        if (GetUserInput(answer) && (answer[0] == 'R' || answer[0] == 'r' || answer[0] == 'N' || answer[0] == 'n'))
            break;
    } while (1);

    if (answer[0] == 'R' || answer[0] == 'r')
        m_isRootCredential = true;
    do
    {
        sLog.SetColor(true, YELLOW);
        printf("Please enter your root username:", m_isRootCredential ? "root" : "normal");
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(m_serverUserName))
            continue;
        sLog.SetColor(true, YELLOW);
        printf("You choose '%s' as user name, is it correct(Y/N)?", m_serverUserName.c_str());
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(answer) || (answer[0] != 'y' && answer[0] != 'Y'))
            continue;
        break;
    } while (1);

    do
    {
        sLog.SetColor(true, YELLOW);
        printf("Please enter your password:");
        sLog.SetColor(true, CYAN);
        if (!GetUserInput(m_serverPassword))
            continue;
        sLog.SetColor(true, YELLOW);
        printf("Please confirm your password:");
        sLog.SetColor(true, CYAN);
        if (!GetUserInput(answer))
            continue;
        if (answer != m_serverPassword)
            continue;
        break;
    } while (1);

    if (!m_isRootCredential)
    {
        do
        {
            sLog.SetColor(true, YELLOW);
            printf("Please confirm world database name [mangos]:");
            sLog.SetColor(true, WHITE);
            if (!GetUserInput(m_WorldDBName))
                m_WorldDBName = "mangos";
            sLog.SetColor(true, YELLOW);
            printf("Please confirm characters database name [characters]:");
            sLog.SetColor(true, WHITE);
            if (!GetUserInput(m_CharactersDBName))
                m_CharactersDBName = "characters";
            sLog.SetColor(true, YELLOW);
            printf("Please confirm realm database [realmd]:");
            sLog.SetColor(true, WHITE);
            if (!GetUserInput(m_RealmDBName))
                m_WorldDBName = "realmd";
            break;
        } while (1);
    }

    return NEXT_STEP_CONTINUE;
}
