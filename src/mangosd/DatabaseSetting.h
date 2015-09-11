#ifndef _DATABASE_SETTING
#define  _DATABASE_SETTING
#pragma once
#include <string>

enum nextStep
{
    NEXT_STEP_RETRY         = 0,
    NEXT_STEP_CONTINUE      = 1,
    NEXT_STEP_RETURN        = 2
};

class DatabaseSetting
{
public:
    DatabaseSetting();
    void ClearScreen();
    void Start();
    bool Complete();
    ~DatabaseSetting();
    bool GetUserInput(std::string &buffer);
    nextStep SetDBCredential();
    bool m_isRootCredential;
    std::string m_WorldDBName;
    std::string m_CharactersDBName;
    std::string m_RealmDBName;
    std::string m_serverAddress;
    std::string m_serverPort;
    std::string m_serverUserName;
    std::string m_serverPassword;
};
#endif

