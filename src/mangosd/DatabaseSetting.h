#ifndef _DATABASE_SETTING
#define  _DATABASE_SETTING
#pragma once

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
    void Start();
    bool Complete();
    ~DatabaseSetting();
    nextStep TryToConnectAsRoot();
};
#endif

