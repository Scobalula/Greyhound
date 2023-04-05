#include "pch.h"
#include "CommandLine.h"

CommandLine::CommandLine(int nArgsCount, LPWSTR* args)
{
    if (!args)
    {
        LPWSTR tmp{};
        this->argc = 0;
        this->argv = &tmp;
    }
    else {
        this->argc = nArgsCount;
        this->argv = args;
    }
}

CommandLine::~CommandLine()
{
}

int CommandLine::FindParam(LPWSTR psz) const
{
    for (int i = 1; i < this->argc; ++i)
    {
        if (!_wcsicmp(this->argv[i], psz))
            return i;
    }
    return -1;
}

bool CommandLine::HasParam(LPWSTR psz) const
{
    return FindParam(psz) != -1;
}

bool CommandLine::HasParam(const wchar_t* psz) const
{
    return FindParam((LPWSTR)psz) != -1;
}

LPWSTR CommandLine::GetParamAtIdx(int idx) const
{
    // + 1 to skip the exe
    return this->argv[idx + 1];
}

unsigned int CommandLine::ArgC()
{
    return this->argc - 1;
}

LPWSTR CommandLine::GetParamValue(LPWSTR szArg, LPWSTR szDefault) const
{
    int idx = FindParam(szArg);
    if (idx == this->argc - 1 || idx == 0)
        return szDefault;

    LPWSTR szNextParam = this->argv[idx + 1];

    if (!szNextParam)
        return szDefault;

    if (szNextParam[0] == '-')
        return szDefault;

    return szNextParam;
}

LPWSTR CommandLine::GetParamValue(const wchar_t* szArg, LPWSTR szDefault) const
{
    int idx = FindParam((LPWSTR)szArg);
    if (idx == this->argc - 1 || idx <= 0)
        return szDefault;

    LPWSTR szNextParam = this->argv[idx + 1];

    if (!szNextParam)
        return szDefault;

    if (szNextParam[0] == '-')
        return szDefault;

    return szNextParam;
}