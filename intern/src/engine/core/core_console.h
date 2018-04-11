
#pragma once

#include "engine/engine_config.h"

#include "base/base_clock.h"
#include "base/base_defines.h"
#include "base/base_uncopyable.h"

#include <sstream>

#define ENGINE_CONSOLE_DEFAULT(_Message) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Default, _Message);
#define ENGINE_CONSOLE_ERROR(  _Message) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Error  , _Message);
#define ENGINE_CONSOLE_WARNING(_Message) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Warning, _Message);
#define ENGINE_CONSOLE_INFO(   _Message) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Info   , _Message);
#define ENGINE_CONSOLE_DEBUG(  _Message) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Debug  , _Message);

#define ENGINE_CONSOLE_DEFAULTV(_Format, ...) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Default, 0, _Format, __VA_ARGS__);
#define ENGINE_CONSOLE_ERRORV(  _Format, ...) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Error  , 0, _Format, __VA_ARGS__);
#define ENGINE_CONSOLE_WARNINGV(_Format, ...) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Warning, 0, _Format, __VA_ARGS__);
#define ENGINE_CONSOLE_INFOV(   _Format, ...) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Info   , 0, _Format, __VA_ARGS__);
#define ENGINE_CONSOLE_DEBUGV(  _Format, ...) ::Core::CConsole::GetInstance().Entry(::Core::CConsole::Debug  , 0, _Format, __VA_ARGS__);

#define ENGINE_CONSOLE_STREAMDEFAULT(_StreamData) ::Core::CConsole::GetInstance().StreamEntry(::Core::CConsole::Default) << _StreamData << std::endl;
#define ENGINE_CONSOLE_STREAMERROR(  _StreamData) ::Core::CConsole::GetInstance().StreamEntry(::Core::CConsole::Error  ) << _StreamData << std::endl;
#define ENGINE_CONSOLE_STREAMWARNING(_StreamData) ::Core::CConsole::GetInstance().StreamEntry(::Core::CConsole::Warning) << _StreamData << std::endl;
#define ENGINE_CONSOLE_STREAMINFO(   _StreamData) ::Core::CConsole::GetInstance().StreamEntry(::Core::CConsole::Info   ) << _StreamData << std::endl;
#define ENGINE_CONSOLE_STREAMDEBUG(  _StreamData) ::Core::CConsole::GetInstance().StreamEntry(::Core::CConsole::Debug  ) << _StreamData << std::endl;

namespace Core
{
    class ENGINE_API CConsole : public Base::CUncopyable
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

        static CConsole& GetInstance();

    public:

        void Entry(EConsoleLevel _ConsoleLevel, const char* _pText);
        void Entry(EConsoleLevel _ConsoleLevel, char*, const char* _pFormat, ...);
        std::ostringstream& StreamEntry(EConsoleLevel _ConsoleLevel);

        void SetVerbosityLevel(int _Level);
        
    public:
        
        const char* GetText() const;
        
    public:
        
        void Clear();
        
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

        void Out(EConsoleLevel _ConsoleLevel, const char* _pText) const;
    };

} // namespace Core

