#ifndef _DATABASE_SETTING
#define  _DATABASE_SETTING
#pragma once
#include <string>
#include "Database\DatabaseEnv.h"
#include "revision_sql.h"

enum nextStep
{
    NEXT_STEP_RETRY         = 0,
    NEXT_STEP_CONTINUE      = 1,
    NEXT_STEP_RETURN        = 2
};

enum dbIndexNames
{
    DB_INDEX_WORLD          = 0,
    DB_INDEX_REALM          = 1, 
    DB_INDEX_CHARACTERS     = 2,
    DB_INDEX_MAX
};

enum DbStatus
{
    DB_STATUS_NONE           = 0,
    DB_STATUS_NEED_UPGRADE   = 1,
    DB_STATUS_NEED_DOWNGRADE = 2,
    DB_STATUS_NEED_CREATE    = 3,
    DB_STATUS_IS_UP_TO_DATE  = 4
};

enum DbContentType
{
    DB_CONTENT_UDB            = 0,
    DB_CONTENT_YTDB           = 1,
    DB_CONTENT_PSDB           = 2,
    DB_CONTENT_MAX
};

extern const char* DefaultDatabasesName[DB_INDEX_MAX];
extern const char* DbRolesNames[DB_INDEX_MAX];
extern const char* DefaultDbVersionTablesNames[DB_INDEX_MAX];
extern const char* MinimumRevisions[DB_INDEX_MAX];
extern const char* SupportedDB[DB_CONTENT_MAX];

struct DbInfos
{
    std::string name;
    std::string version;
    bool        alreadyExist;
    DbStatus    status;
};

struct DbContentInfos
{
    bool            installed;
    DbContentType   type;
    uint32          version;
    std::string     fullVerion;
    std::string     acidVersion;
};

struct ConnectionData
{
    std::string     serverAdress;
    std::string     serverPort;
    std::string     serverUserName;
    std::string     serverUserPassword;
    DbInfos         dbInfos[DB_INDEX_MAX];
    bool            isRoot;
    DbContentInfos  dbContentInfos;

    ConnectionData()
    {
        for (uint32 i = 0; i < DB_INDEX_MAX; ++i)
        {
            dbInfos[i].name = DefaultDatabasesName[i];
            dbInfos[i].version.clear();
            dbInfos[i].alreadyExist = false;
            dbInfos[i].status = DB_STATUS_NONE;
        }
        isRoot = false;
        dbContentInfos.installed = false;
        dbContentInfos.type = DB_CONTENT_MAX;
    }

    std::string GetServerString(dbIndexNames index = DB_INDEX_MAX)
    {
        if (index >= DB_INDEX_MAX)
            return serverAdress + ';' + serverPort + ';' + serverUserName + ';' + serverUserPassword;
        else
            return serverAdress + ';' + serverPort + ';' + serverUserName + ';' + serverUserPassword + ';' + dbInfos[index].name;
    }
};

typedef DatabaseType databasesTab[DB_INDEX_MAX];

class DatabaseSetting
{
public:
    DatabaseSetting();
    void Start();
    bool Complete();
    ~DatabaseSetting();

private:
    void ClearScreen();
    bool GetUserInput(std::string &buffer);
    void GetDbContentTypeAndVersion(DbContentInfos &dbCI);
    std::string GetDbVersion(dbIndexNames index);
    nextStep SetDBCredential();
    nextStep TestingCredential();
    nextStep GetDBInformations();
    nextStep CheckSqlFolders();

    ConnectionData m_connectionData;
    databasesTab m_databases;
};
#endif

