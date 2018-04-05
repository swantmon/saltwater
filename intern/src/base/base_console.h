
#pragma once

#include "engine/engine_config.h"

#include "base/base_clock.h"
#include "base/base_defines.h"
#include "base/base_uncopyable.h"

#include <sstream>

#define BASE_CONSOLE_DEFAULT(_Message) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Default, _Message);
#define BASE_CONSOLE_ERROR(  _Message) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Error  , _Message);
#define BASE_CONSOLE_WARNING(_Message) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Warning, _Message);
#define BASE_CONSOLE_INFO(   _Message) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Info   , _Message);
#define BASE_CONSOLE_DEBUG(  _Message) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Debug  , _Message);

#define BASE_CONSOLE_DEFAULTV(_Format, ...) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Default, BASE_NULL, _Format, __VA_ARGS__);
#define BASE_CONSOLE_ERRORV(  _Format, ...) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Error  , BASE_NULL, _Format, __VA_ARGS__);
#define BASE_CONSOLE_WARNINGV(_Format, ...) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Warning, BASE_NULL, _Format, __VA_ARGS__);
#define BASE_CONSOLE_INFOV(   _Format, ...) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Info   , BASE_NULL, _Format, __VA_ARGS__);
#define BASE_CONSOLE_DEBUGV(  _Format, ...) ::IO::CConsole::GetInstance().Entry(::IO::CConsole::Debug  , BASE_NULL, _Format, __VA_ARGS__);

#define BASE_CONSOLE_STREAMDEFAULT(_StreamData) ::IO::CConsole::GetInstance().StreamEntry(::IO::CConsole::Default) << _StreamData << std::endl;
#define BASE_CONSOLE_STREAMERROR(  _StreamData) ::IO::CConsole::GetInstance().StreamEntry(::IO::CConsole::Error  ) << _StreamData << std::endl;
#define BASE_CONSOLE_STREAMWARNING(_StreamData) ::IO::CConsole::GetInstance().StreamEntry(::IO::CConsole::Warning) << _StreamData << std::endl;
#define BASE_CONSOLE_STREAMINFO(   _StreamData) ::IO::CConsole::GetInstance().StreamEntry(::IO::CConsole::Info   ) << _StreamData << std::endl;
#define BASE_CONSOLE_STREAMDEBUG(  _StreamData) ::IO::CConsole::GetInstance().StreamEntry(::IO::CConsole::Debug  ) << _StreamData << std::endl;

namespace IO
{
    class CConsole : public Base::CUncopyable
    {
    
    public:

        enum EConsoleLevel
        {
            Default,
            Error,
            Warning,
            Info,
            Debug
        };

    public:

        ENGINE_API static CConsole& GetInstance();

    public:

        ENGINE_API void Entry(EConsoleLevel _ConsoleLevel, const Char* _pText);
        ENGINE_API void Entry(EConsoleLevel _ConsoleLevel, Char*, const Char* _pFormat, ...);
        ENGINE_API std::ostringstream& StreamEntry(EConsoleLevel _ConsoleLevel);

        ENGINE_API void SetVerbosityLevel(int _Level);
        
    public:
        
        ENGINE_API const Char* GetText() const;
        
    public:
        
        ENGINE_API void Clear();
        
    private:
        
        static const unsigned int s_MaxNumberOfFormatCharacters = 2048;

    private:

        CConsole();
        ~CConsole();

    private:

        std::ostringstream      m_OutputStream;
        Base::CPerformanceClock m_Clock;
        
        int m_VerbosityLevel;

    private:

        const std::string& GetLogLevelString(EConsoleLevel _ConsoleLevel) const;

        void Out(EConsoleLevel _ConsoleLevel, const Char* _pText) const;
    };

} // namespace IO

