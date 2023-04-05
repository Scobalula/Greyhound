#pragma once

class CommandLine
{
public:
    int argc = 0;
    LPWSTR* argv;

    CommandLine(int nArgsCount, LPWSTR* args);
    ~CommandLine();

    virtual int FindParam(LPWSTR psz) const;

    virtual bool HasParam(LPWSTR psz) const;
    virtual bool HasParam(const wchar_t* psz) const;

    virtual LPWSTR GetParamAtIdx(int idx) const;

    virtual unsigned int ArgC();

    virtual LPWSTR GetParamValue(LPWSTR pszArg, LPWSTR szDefault = (LPWSTR)L"") const;
    virtual LPWSTR GetParamValue(const wchar_t* pszArg, LPWSTR szDefault = (LPWSTR)L"") const;
};

