
// -----------------------------------------------------------------------------
// Code based on: http://note.sonots.com/Comp/CompLang/cpp/getopt.html
// -----------------------------------------------------------------------------

#include "base/base_precompiled.h"

#include "base/base_console.h"
#include "base/base_getopt.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include <stdio.h>

namespace 
{
	class CGetOpt : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGetOpt);

    public:

        int GetOption(int _Argc, char* _pArgv[], char* _pOptions);
        const char* GetArgument();
        int GetOptionIndex();
        int GetOptionError();
        int GetOptionName();

    private:

        int m_ArgumentIndex;
        int m_OptionError;
        int m_OptionIndex;
        int m_OptionName;
        char* m_pOptionArgument;

    private:

        CGetOpt();
        ~CGetOpt();
    };
} // namespace 

namespace 
{
    CGetOpt::CGetOpt()
        : m_ArgumentIndex  (1)
        , m_OptionError    (1)
        , m_OptionIndex    (1)
        , m_OptionName     (0)
        , m_pOptionArgument(0)
    {
    }

    // -----------------------------------------------------------------------------

    CGetOpt::~CGetOpt()
    {

    }

    // -----------------------------------------------------------------------------

    int CGetOpt::GetOption(int _Argc, char* _pArgv[], char* _pOptions)
    {
        auto WriteError = [&](const char* _pDescription, int _Argument)
        {
            if (m_OptionError)
            {
				BASE_CONSOLE_ERRORV("Getting option \"%s\" and argument \"%s\" failed with message: %s", static_cast<char>(m_OptionError), static_cast<char>(_Argument), _pDescription);
            }
        };

        // -----------------------------------------------------------------------------

        int   Argument;
        char* pArgumentCompare;

        if (m_ArgumentIndex == 1)
        {
            if (m_OptionIndex >= _Argc || _pArgv[m_OptionIndex][0] != '-' || _pArgv[m_OptionIndex][1] == '\0')
            {
                return -1;
            }
            else if (strcmp(_pArgv[m_OptionIndex], "--") == 0)
            {
                m_OptionIndex++;

                return -1;
            }
        }

        m_OptionName = Argument = _pArgv[m_OptionIndex][m_ArgumentIndex];

        if (Argument == ':' || (pArgumentCompare = strchr(_pOptions, Argument)) == NULL) 
        {
            WriteError(": illegal option -- ", Argument);

            if (_pArgv[m_OptionIndex][++m_ArgumentIndex] == '\0') 
            {
                m_OptionIndex++;
                m_ArgumentIndex = 1;
            }

            return('?');
        }

        if (*++pArgumentCompare == ':') 
        {
            if (_pArgv[m_OptionIndex][m_ArgumentIndex + 1] != '\0')
            {
                m_pOptionArgument = &_pArgv[m_OptionIndex++][m_ArgumentIndex + 1];
            }
            else if (++m_OptionIndex >= _Argc) 
            {
                WriteError(": option requires an argument -- ", Argument);
                m_ArgumentIndex = 1;

                return('?');
            }
            else
            {
                m_pOptionArgument = _pArgv[m_OptionIndex++];
            }

            m_ArgumentIndex = 1;
        }
        else 
        {
            if (_pArgv[m_OptionIndex][++m_ArgumentIndex] == '\0') 
            {
                m_ArgumentIndex = 1;
                m_OptionIndex++;
            }

            m_pOptionArgument = 0;
        }

        return(Argument);
    }

    // -----------------------------------------------------------------------------

    const char* CGetOpt::GetArgument()
    {
        return m_pOptionArgument;
    }

    // -----------------------------------------------------------------------------

    int CGetOpt::GetOptionIndex()
    {
        return m_OptionIndex;
    }

    // -----------------------------------------------------------------------------

    int CGetOpt::GetOptionError()
    {
        return m_OptionError;
    }

    // -----------------------------------------------------------------------------

    int CGetOpt::GetOptionName()
    {
        return m_OptionName;
    }
} // namespace 

namespace IO
{
    int GetOption(int _Argc, char* _pArgv[], char* _pOptions)
    {
        return CGetOpt::GetInstance().GetOption(_Argc, _pArgv, _pOptions);
    }

    // -----------------------------------------------------------------------------

    const char* GetArgument()
    {
        return CGetOpt::GetInstance().GetArgument();
    }

    // -----------------------------------------------------------------------------

    int GetOptionIndex()
    {
        return CGetOpt::GetInstance().GetOptionIndex();
    }

    // -----------------------------------------------------------------------------

    int GetOptionError()
    {
        return CGetOpt::GetInstance().GetOptionError();
    }

    // -----------------------------------------------------------------------------

    int GetOptionName()
    {
        return CGetOpt::GetInstance().GetOptionName();
    }
} // namespace IO