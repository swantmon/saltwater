
#include "engine/engine_precompiled.h"

#include "base/base_console.h"

#include <stdarg.h>

#if PLATFORM_ANDROID
#include "android/log.h"
#endif // PLATFORM_ANDROID

namespace IO
{

	CConsole& CConsole::GetInstance()
    {
        static CConsole s_Singleton;

        return s_Singleton;
    }

    // -----------------------------------------------------------------------------

    CConsole::CConsole()
        : m_OutputStream  ( )
        , m_Clock         ( )
        , m_VerbosityLevel(3)
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
        if (m_VerbosityLevel >= _ConsoleLevel)
        {
            Out(_ConsoleLevel, _pText);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CConsole::Entry(EConsoleLevel _ConsoleLevel, Char*, const Char* _pFormat, ...)
    {
        if (m_VerbosityLevel >= _ConsoleLevel)
        {
            va_list pArguments;

            va_start(pArguments, _pFormat);

            char Buffer[s_MaxNumberOfFormatCharacters];

#if PLATFORM_ANDROID
            vsnprintf(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#else
            vsnprintf_s(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#endif

            va_end(pArguments);

            Out(_ConsoleLevel, Buffer);
        }
    }

    // -----------------------------------------------------------------------------

    std::ostringstream& CConsole::StreamEntry(EConsoleLevel _ConsoleLevel)
    {
        if (m_VerbosityLevel >= _ConsoleLevel)
        {
            // -----------------------------------------------------------------------------
            // Get current time stamp
            // -----------------------------------------------------------------------------
            m_Clock.OnFrame();

            double      CurrentTime = m_Clock.GetTime();
            std::string LogString = GetLogLevelString(_ConsoleLevel);

            // -----------------------------------------------------------------------------
            // Init new Console entry with timestamp and Console level
            // -----------------------------------------------------------------------------
            m_OutputStream << "(" << CurrentTime << " sec.) " << LogString << ": ";
        }

        return m_OutputStream;
    }
    
    // -----------------------------------------------------------------------------

    void CConsole::SetVerbosityLevel(int _Level)
    {
        assert(_Level >= -1 && _Level < 6);

        m_VerbosityLevel = _Level;
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
#ifdef PLATFORM_ANDROID
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
#endif // PLATFORM_ANDROID
    }
} // namespace IO