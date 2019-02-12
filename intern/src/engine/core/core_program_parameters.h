
#pragma once

#include "engine/engine_config.h"

#include "base/base_exception.h"
#include "base/base_json.h"

#include "engine/core/core_console.h"

#include "engine/gui/gui_event_handler.h"

#include <string>

namespace Core
{
    class ENGINE_API CProgramParameters
    {
    public:

        static CProgramParameters& GetInstance();

    public:

        void ParseJSON(const std::string& _rJSON);

        void ParseFile(const std::string& _rFile);
        void WriteFile(const std::string& _rFile);

    public:

        void Clear();

    public:

        template<typename T>
        void Add(const std::string& _rOption, const T _rParameter);
        
        template<typename T>
        const T Get(const std::string& _rOption, const T _Default);
        const std::string Get(const std::string& _rOption, const char* _Default);

        bool IsNull(const std::string& _rOption);

    private:

        nlohmann::json m_Container;

        Gui::EventHandler::CEventDelegate::HandleType m_OnEventDelegate;

    private:

        CProgramParameters();
        ~CProgramParameters();

    private:

        nlohmann::json::json_pointer ConvertOptionToJSONPointer(const std::string& _rOption);
    };
} // namespace Core

namespace Core
{
    template<typename T>
    void CProgramParameters::Add(const std::string& _rOption, const T _Parameter)
    {
        ENGINE_CONSOLE_INFOV("Creating new config parameter %s", _rOption.c_str());

        m_Container[ConvertOptionToJSONPointer(_rOption)] = _Parameter;
    }
    
    // -----------------------------------------------------------------------------

    template<typename T>
    const T CProgramParameters::Get(const std::string& _rOption, const T _Default)
    {
        try
        {
            if (IsNull(_rOption))
            {
                Add(_rOption, _Default);

                return _Default;
            }

            return m_Container[ConvertOptionToJSONPointer(_rOption)];
        }
        catch (const nlohmann::json::exception& _rException)
        {
            ENGINE_CONSOLE_ERRORV("Getting value of option \"%s\" from program parameters failed with error: \"%s\"", _rOption.c_str(), _rException.what());
        }

        return _Default;
    }
} // namespace Core