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
#include "Database\DatabaseEnv.h"

const char* DefaultDatabasesName[DB_INDEX_MAX] = { "mangos", "realmd", "characters" };
const char* DbRolesNames[DB_INDEX_MAX] = { "world", "realm", "characters" };
const char* DefaultDbVersionTablesNames[DB_INDEX_MAX] = { "db_version", "realmd_db_version", "character_db_version" };
const char* MinimumRevisions[DB_INDEX_MAX] = { REVISION_DB_MANGOS, REVISION_DB_REALMD, REVISION_DB_CHARACTERS };
const char* SupportedDB[DB_INDEX_MAX] = { "UDB", "YTDB", "PSDB" };

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
        switch (SetDBCredential())
        {
            case NEXT_STEP_RETRY: continue; break;
            case NEXT_STEP_RETURN: return; break;
            case NEXT_STEP_CONTINUE: break;
            default: return;
        }

        switch (TestingCredential())
        {
            case NEXT_STEP_RETRY: continue; break;
            case NEXT_STEP_RETURN: return; break;
            case NEXT_STEP_CONTINUE: break;
            default: return;
        }

        switch (GetDBInformations())
        {
            case NEXT_STEP_RETRY: continue; break;
            case NEXT_STEP_RETURN: return; break;
            case NEXT_STEP_CONTINUE: break;
            default: return;
        }

        switch (CheckSqlFolders())
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

    if (buffer.empty())
        return false;
    return true;
}

std::string DatabaseSetting::GetDbVersion(dbIndexNames index)
{
    std::string reqName;
    reqName.clear();

    if (index >= DB_INDEX_MAX)
        return reqName;

    string tableName;
    uint32 dbIndex = uint32(index);

    if (m_connectionData.isRoot)
    {
        tableName = m_connectionData.dbInfos[index].name + '.' + DefaultDbVersionTablesNames[index];
        dbIndex = DB_INDEX_WORLD;
    }
    else
        tableName = DefaultDbVersionTablesNames[index];

    QueryNamedResult* result = m_databases[dbIndex].PQueryNamed("SELECT * FROM %s LIMIT 1", tableName.c_str());
    if (result)
    {
        QueryFieldNames const& namesMap = result->GetFieldNames();
        for (QueryFieldNames::const_iterator itr = namesMap.begin(); itr != namesMap.end(); ++itr)
        {
            if (itr->substr(0, 9) == "required_")
            {
                reqName = *itr;
                break;
            }
        }

        delete result;
        return reqName;
    }
    return reqName;
}

void DatabaseSetting::GetDbContentTypeAndVersion(DbContentInfos &dbCI)
{
    string tableName;

    if (m_connectionData.isRoot)
        tableName = m_connectionData.dbInfos[DB_INDEX_WORLD].name + '.' + DefaultDbVersionTablesNames[DB_INDEX_WORLD];
    else
        tableName = DefaultDbVersionTablesNames[DB_INDEX_WORLD];

    QueryResult* result = m_databases[DB_INDEX_WORLD].PQuery("SELECT version, creature_ai_version, cache_id FROM %s LIMIT 1", tableName.c_str());
    if (result)
    {
        Field* fields       = result->Fetch();
        dbCI.fullVerion     = fields[0].GetString();
        dbCI.acidVersion    = fields[1].GetString();
        dbCI.version        = fields[2].GetUInt32();
        delete result;

        // TODO:: maybe add better check
        if (!dbCI.fullVerion.empty() && dbCI.version != 0)
            dbCI.installed = true;
    }
}

nextStep DatabaseSetting::SetDBCredential()
{
    string answer;
    string dbNames[DB_INDEX_MAX];
    string serverAddress;
    string serverPort;
    string serverUserName;
    string serverPassword;
    do
    {
        ClearScreen();
        sLog.outString("1 - Server address configuration.");
        sLog.outString("--------------------------------");
        sLog.outString("The following setting is your mysql server address.");
        sLog.outString("Please enter your address like '192.168.1.11' or 'your.server.com' then your port (default is 'localhost' and '3306').");
        sLog.outString();

        sLog.SetColor(true, YELLOW);
        printf("Please type your server address [localhost]: ");
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(serverAddress))
            serverAddress = "localhost";
        sLog.SetColor(true, YELLOW);
        printf("Please type your server Port [3306]: ");
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(serverPort))
            serverPort = "3306";
        sLog.SetColor(true, YELLOW);
        printf("You choose '%s' as server address and '%s' as server port, is it correct(Y/N)? ", serverAddress.c_str(), serverPort.c_str());
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(answer) || (answer[0] != 'y' && answer[0] != 'Y'))
            continue;
        else
            break;
    } while (1);
    sLog.ResetColor(true);

    do
    {
        ClearScreen();
        sLog.outString("2 - Server access configuration.");
        sLog.outString("--------------------------------");
        sLog.outString();
        sLog.outString("The following setting require db access, we need to know if you have root access or normal access.");
        sLog.outString("If you have root access we could handle the 3 databases and the normal user access creation automatically.");
        sLog.outString("If you have only normal access you should have by any other way create yourself the required database and provide us the correct credentials to access them.");
        sLog.outString();

        // set the type of access
        sLog.SetColor(true, YELLOW);
        printf("Do you have root access to db or normal access(R/N)? ");
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(answer) || (answer[0] != 'R' && answer[0] != 'r' && answer[0] != 'N' && answer[0] != 'n'))
            continue;

        m_connectionData.isRoot = (answer[0] == 'R' || answer[0] == 'r');

        // set the user name and password
        sLog.SetColor(true, YELLOW);
        printf("Please enter your %s username: ", m_connectionData.isRoot ? "root" : "normal");
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(serverUserName))
            continue;
        sLog.SetColor(true, YELLOW);
        printf("Please enter your password: ");
        sLog.SetColor(true, CYAN);
        if (!GetUserInput(serverPassword))
            serverPassword.clear();

        // set the db names
        for (uint32 i = 0; i < DB_INDEX_MAX; ++i)
        {
            DbInfos &dbInfos = m_connectionData.dbInfos[i];
            sLog.SetColor(true, YELLOW);
            printf("Please confirm '%s' database name [%s]: ", DbRolesNames[i], dbInfos.name.c_str());
            sLog.SetColor(true, WHITE);
            if (GetUserInput(answer))
                dbNames[i] = answer;
            else
                dbNames[i] = dbInfos.name;
        }
        sLog.ResetColor(true);

        sLog.outString();
        sLog.outString("Your settings are:");
        sLog.outString("------------------");
        sLog.outString();
        sLog.outString("Server access type : \t%s", m_connectionData.isRoot ? "root": "normal");
        sLog.outString("User name : \t\t%s", serverUserName.c_str());
        sLog.outString("Password : \t\t%s", serverPassword.empty() ? "Not Provided" : "Provided");
        sLog.outString("World db name : \t%s", dbNames[DB_INDEX_WORLD].c_str());
        sLog.outString("Characters db name : \t%s", dbNames[DB_INDEX_CHARACTERS].c_str());
        sLog.outString("Realm db name : \t%s", dbNames[DB_INDEX_REALM].c_str());
        sLog.SetColor(true, YELLOW);
        printf("Is this correct(Y/N)? ");
        sLog.SetColor(true, WHITE);
        if (!GetUserInput(answer) || (answer[0] != 'y' && answer[0] != 'Y'))
            continue;
        break;
    } while (1);
    sLog.ResetColor(true);
    m_connectionData.serverAdress = serverAddress;
    m_connectionData.serverPort = serverPort;
    m_connectionData.serverUserName = serverUserName;
    m_connectionData.serverUserPassword = serverPassword;
    for (uint32 i = 0; i < DB_INDEX_MAX; ++i)
        m_connectionData.dbInfos[i].name = dbNames[i];

    return NEXT_STEP_CONTINUE;
}

nextStep DatabaseSetting::TestingCredential()
{
    string answer;

    ClearScreen();
    sLog.outString("3 - Server access testing.");
    sLog.outString("--------------------------");
    sLog.outString();
    sLog.outString("We'll now check if we can establish a connection to your SQL server.");
    sLog.outString("Note that if it fail, you'll have to verify all the previous settings and be sure your SQL server is up.");
    sLog.outString();

    if (m_connectionData.isRoot)
    {
        do
        {
            if (!m_databases[DB_INDEX_WORLD].Initialize(m_connectionData.GetServerString().c_str()))
            {
                sLog.outError("Cannot connect to the DB!");
                sLog.SetColor(true, YELLOW);
                printf("Do you want to retry, configure credential or quit(R/C/Q)? ");
                sLog.SetColor(true, WHITE);
                if (!GetUserInput(answer) || (answer[0] != 'C' && answer[0] != 'c' && answer[0] != 'Q' && answer[0] != 'q'))
                    continue;
                if (answer[0] == 'Q' || answer[0] == 'q')
                    return NEXT_STEP_RETURN;
                return NEXT_STEP_RETRY;
            }
            break;
        } while (1);
    }
    else
    {
        uint32 i = 0;
        do
        {
            if (!m_databases[i].Initialize(m_connectionData.GetServerString(dbIndexNames(i)).c_str()))
            {
                sLog.outError("Cannot connect to %s DB!", m_connectionData.dbInfos[i].name.c_str());
                sLog.SetColor(true, YELLOW);
                printf("Do you want to retry, configure credential or quit(R/C/Q)? ");
                sLog.SetColor(true, WHITE);
                if (!GetUserInput(answer) || (answer[0] != 'C' && answer[0] != 'c' && answer[0] != 'Q' && answer[0] != 'q'))
                    continue;
                if (answer[0] == 'Q' || answer[0] == 'q')
                    return NEXT_STEP_RETURN;
                return NEXT_STEP_RETRY;
            }

            // found the database
            m_connectionData.dbInfos[i].alreadyExist = true;

            ++i;
            if (i >= DB_INDEX_MAX)
                break;
        } while (1);
    }
    sLog.outString();
    sLog.outString();
    sLog.SetColor(true, LGREEN);
    printf("Initialization is done, everything seem right.\n");
    sLog.ResetColor(true);
    sLog.outString();
    sLog.outString("Press enter for next step.");
    GetUserInput(answer);
    return NEXT_STEP_CONTINUE;
}

nextStep DatabaseSetting::GetDBInformations()
{
    ClearScreen();
    sLog.outString("4 - Getting database informations.");
    sLog.outString("----------------------------------");
    sLog.outString();

    bool foundAlreadyExistTables = false;

    if (m_connectionData.isRoot)
    {
        // we have to check if there is any already existing tables
        QueryResult* result = m_databases[DB_INDEX_WORLD].Query("SHOW DATABASES;");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                string dbName = fields[0].GetString();
                for (int i = 0; i < DB_INDEX_MAX; ++i)
                {
                    DbInfos &dbInfos = m_connectionData.dbInfos[i];
                    if (dbInfos.name.find(dbName) != string::npos)
                    {
                        sLog.outString("Found already existing DB '%s'.", dbName.c_str());
                        // found the database
                        dbInfos.alreadyExist = true;

                        // now try to get table version if exist
                        dbInfos.version = GetDbVersion(dbIndexNames(i));

                        // at least one database found so we can't directly create a new clean install
                        foundAlreadyExistTables = true;
                    }
                }
            } while (result->NextRow());
            delete result;
        }
    }
    else
        foundAlreadyExistTables = true;

    if (!foundAlreadyExistTables)
    {
        for (int i = 0; i < DB_INDEX_MAX; ++i)
            m_connectionData.dbInfos[i].status = DB_STATUS_NEED_CREATE;
    }
    else
    {
        for (int i = 0; i < DB_INDEX_MAX; ++i)
        {
            DbInfos &dbInfos = m_connectionData.dbInfos[i];

            // now try to get table version if exist
            dbInfos.version = GetDbVersion(dbIndexNames(i));

            if (dbInfos.alreadyExist && !dbInfos.version.empty())
            {
                if (dbInfos.version > MinimumRevisions[i])
                    dbInfos.status = DB_STATUS_NEED_DOWNGRADE;
                else if (dbInfos.version < MinimumRevisions[i])
                    dbInfos.status = DB_STATUS_NEED_UPGRADE;
                else
                    dbInfos.status = DB_STATUS_IS_UP_TO_DATE;
                continue;
            }
            dbInfos.status = DB_STATUS_NEED_CREATE;
        }
    }

    GetDbContentTypeAndVersion(m_connectionData.dbContentInfos);

    sLog.outString();
    bool warnUpgrade = false;
    bool warnDowngrade = false;
    for (int i = 0; i < DB_INDEX_MAX; ++i)
    {
        DbInfos &dbInfos = m_connectionData.dbInfos[i];
        sLog.SetColor(true, YELLOW);
        printf("Main core database '%s' ", dbInfos.name.c_str());
        switch (dbInfos.status)
        {
            case DB_STATUS_NEED_CREATE:
                sLog.SetColor(true, WHITE);
                printf("will be created!\n");
                break;
            case DB_STATUS_NEED_DOWNGRADE:
                sLog.SetColor(true, LRED);
                printf("will be erased and rebuilded!\n");
                warnDowngrade = true;
                break;
            case DB_STATUS_NEED_UPGRADE:
                sLog.SetColor(true, LMAGENTA);
                printf("will be updated!\n");
                warnUpgrade = true;
                break;
            case DB_STATUS_IS_UP_TO_DATE:
                sLog.SetColor(true, LGREEN);
                printf("is up to date.\n");
                break;
            default:
                return NEXT_STEP_RETURN;
        }
        sLog.ResetColor(true);
    }

    if (m_connectionData.dbContentInfos.installed)
    {
        if (m_connectionData.dbContentInfos.fullVerion.find(SupportedDB[DB_CONTENT_UDB]) != string::npos)
            m_connectionData.dbContentInfos.type = DB_CONTENT_UDB;
        else if (m_connectionData.dbContentInfos.fullVerion.find(SupportedDB[DB_CONTENT_YTDB]) != string::npos)
            m_connectionData.dbContentInfos.type = DB_CONTENT_YTDB;
        else if (m_connectionData.dbContentInfos.fullVerion.find(SupportedDB[DB_CONTENT_PSDB]) != string::npos)
            m_connectionData.dbContentInfos.type = DB_CONTENT_PSDB;
    }

    sLog.outString();
    if (m_connectionData.dbContentInfos.type != DB_CONTENT_MAX)
    {
        sLog.SetColor(true, YELLOW);
        printf("DB content '");
        sLog.SetColor(true, WHITE);
        printf("%s (%u)'", SupportedDB[m_connectionData.dbContentInfos.type], m_connectionData.dbContentInfos.version);
        sLog.SetColor(true, YELLOW);
        printf("is installed.\n");
    }
    else
        sLog.outString("No content DB is installed (empty world)");
    sLog.outString();
    sLog.outString();

    if (warnUpgrade || warnDowngrade)
    {
        sLog.SetColor(true, LRED);
        printf("Warning!!!\n");
        printf("----------\n");
        if (warnDowngrade)
        {
            printf("At least one of your database need to be downgraded for this core.\n");
            printf("Warning all your precedent customization, data or any translation will be discarded!\n");
            printf("-> Better in this case is to cancel the wizard and recompile the core to its last available version.\n");
        }

        if (warnUpgrade)
        {
            printf("At least one of your database will be updated, we advise you to make a backup before.\n");
        }
    }
    sLog.ResetColor(true);
    if (!foundAlreadyExistTables)
    {
        sLog.outString("The wizard found that we can start a new clean installation.");
    }
    sLog.outString();

    string answer;
    sLog.outString("Press enter for next step.");
    GetUserInput(answer);
    return NEXT_STEP_CONTINUE;
}

nextStep DatabaseSetting::CheckSqlFolders()
{
    ClearScreen();
    sLog.outString("5 - Checking sql folder and download if needed.");
    sLog.outString("-----------------------------------------------");
    sLog.outString();
    sLog.outError("Not yet implemented");

    sLog.outString();
    sLog.outString();

    string answer;
    sLog.outString("Press enter for next step.");
    GetUserInput(answer);
    return NEXT_STEP_RETURN;
}
