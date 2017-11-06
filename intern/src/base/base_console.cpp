
#include "base/base_precompiled.h"

#include "base/base_console.h"

#include <stdarg.h>

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
        m_OutputStream << std::endl;

        fprintf(stdout, "%s", m_OutputStream.str().c_str());
        fflush(stdout);
    }
    
    // -----------------------------------------------------------------------------
    
    void CConsole::Entry(EConsoleLevel _ConsoleLevel, const Char* _pText)
    {
        std::string LogString = GetLevelString(_ConsoleLevel);
        
        fprintf(stdout, "%s: %s\n", LogString.c_str(), _pText);
        fflush(stdout);
    }
    
    // -----------------------------------------------------------------------------
    
    void CConsole::Entry(EConsoleLevel _ConsoleLevel, Char*, const Char* _pFormat, ...)
    {
        std::string LogString = GetLevelString(_ConsoleLevel);
        
        va_list pArguments;
        
        va_start(pArguments, _pFormat);
        
        char Buffer[s_MaxNumberOfFormatCharacters];
        
#if __APPLE__ || __ANDROID__
        vsnprintf(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#else
        vsnprintf_s(Buffer, s_MaxNumberOfFormatCharacters, _pFormat, pArguments);
#endif
        
        va_end(pArguments);
        
        fprintf(stdout, "%s: %s\n", LogString.c_str(), Buffer);
        fflush(stdout);
    }

    // -----------------------------------------------------------------------------

    std::ostringstream& CConsole::StreamEntry(EConsoleLevel _ConsoleLevel)
    {
        // -----------------------------------------------------------------------------
        // Get current time stamp
        // -----------------------------------------------------------------------------
        m_Clock.OnFrame();
        
        double      CurrentTime = m_Clock.GetTime();
        std::string LogString   = GetLevelString(_ConsoleLevel);

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
    
    std::string CConsole::GetLevelString(EConsoleLevel _Level)
    {
        static std::string s_LogString[] =
        {
            "Error",
            "Warning",
            "Info",
            "Debug"
        };
        
        return s_LogString[_Level];
    }

} // namespace IO