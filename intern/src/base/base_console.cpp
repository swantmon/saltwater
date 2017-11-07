
#include "base/base_precompiled.h"

#include "base/base_console.h"

#include <stdarg.h>

#include "android/log.h"

namespace IO
{

	CConsole& CConsole::GetInstance()
    {
        static CConsole s_Singleton;

        return s_Singleton;
    }

    // -----------------------------------------------------------------------------

    CConsole::CConsole()
        : m_OutputStream()
        , m_Clock       ()
    {
    }

    // -----------------------------------------------------------------------------

    CConsole::~CConsole() 
    {
        Out(Info, m_OutputStream.str().c_str());
    }
    
    // -----------------------------------------------------------------------------
    
    void CConsole::Entry(EConsoleLevel _ConsoleLevel, const Char* _pText)
    {
        Out(_ConsoleLevel, _pText);
    }
    
    // -----------------------------------------------------------------------------
    
    void CConsole::Entry(EConsoleLevel _ConsoleLevel, Char*, const Char* _pFormat, ...)
    {
        va_list pArguments;
        
        va_start(pArguments, _pFormat);
        
        char Buffer[s_MaxNumberOfFormatCharacters];
        
#if __APPLE__ || __ANDROID__
        vsnprintf(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#else
        vsnprintf_s(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#endif
        
        va_end(pArguments);
        
        Out(_ConsoleLevel, Buffer);
    }

    // -----------------------------------------------------------------------------

    std::ostringstream& CConsole::StreamEntry(EConsoleLevel _ConsoleLevel)
    {
        // -----------------------------------------------------------------------------
        // Get current time stamp
        // -----------------------------------------------------------------------------
        m_Clock.OnFrame();
        
        double      CurrentTime = m_Clock.GetTime();
        std::string LogString   = GetLogLevelString(_ConsoleLevel);

        // -----------------------------------------------------------------------------
        // Init new Console entry with timestamp and Console level
        // -----------------------------------------------------------------------------
        m_OutputStream << "(" << CurrentTime << " sec.) " << LogString << ": ";

        return m_OutputStream;
    }
    
    // -----------------------------------------------------------------------------
    
    const Char* CConsole::GetText() const
    {
        return m_OutputStream.str().c_str();
    }
    
    // -----------------------------------------------------------------------------
    
    void CConsole::Clear()
    {
        m_OutputStream.clear();
    }

    // -----------------------------------------------------------------------------

    const std::string& CConsole::GetLogLevelString(EConsoleLevel _ConsoleLevel) const
    {
        static const std::string s_LogString[] =
        {
            "Default",
            "Error",
            "Warning",
            "Info",
            "Debug"
        };

        return s_LogString[_ConsoleLevel];
    }

    // -----------------------------------------------------------------------------

    void CConsole::Out(EConsoleLevel _ConsoleLevel, const Char* _pText) const
    {
#ifdef __ANDROID__
        static const int s_LogLevel[] =
        {
            ANDROID_LOG_DEFAULT,
            ANDROID_LOG_ERROR,
            ANDROID_LOG_WARN,
            ANDROID_LOG_INFO,
            ANDROID_LOG_DEBUG,
    };

        __android_log_print(s_LogLevel[_ConsoleLevel], "Base.Console", "%s\n", _pText);
#else

        fprintf(stdout, "%s: %s\n", GetLogLevelString(_ConsoleLevel).c_str(), _pText);
        fflush(stdout);
#endif // __ANDROID__
    }
} // namespace IO