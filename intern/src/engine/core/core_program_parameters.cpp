
#include "engine/engine_precompiled.h"

#include "base/base_input_event.h"

#include "engine/core/core_program_parameters.h"

#include "engine/gui/gui_event_handler.h"

#include <sstream>

namespace Core
{
    CProgramParameters& CProgramParameters::GetInstance()
    {
        static CProgramParameters s_Sinstance;

        return s_Sinstance;
    }
} // namespace Core

using namespace nlohmann;

namespace Core
{
    CProgramParameters::CProgramParameters()
    {
        // -----------------------------------------------------------------------------
        // Handle input commands
        // -----------------------------------------------------------------------------
        auto InputCommandDelegate = [&](const Base::CInputEvent& _rEvent)
        {
            if (_rEvent.GetType() != Base::CInputEvent::Command) return;

            std::string Command = _rEvent.GetCommand();

            if (Command.length() < 3 || !Command.compare(0, 2, "pp ")) return;

            Command = Command.substr(3);

            if (Command.length() == 0) return;

            size_t EqualPosition = Command.find('=');

            if (EqualPosition == std::string::npos)
            {
                if (IsNull(Command))
                {
                    ENGINE_CONSOLE_WARNINGV("Option %s does not exists", Command.c_str());
                }
                else
                {
                    auto Value = m_Container[ConvertOptionToJSONPointer(Command)];

                    ENGINE_CONSOLE_INFOV("%s is %s", Command.c_str(), Value.dump().c_str());
                }
            }
            else
            {
                std::string Option = Command.substr(0, EqualPosition);
                std::string Value  = Command.substr(EqualPosition + 1);

                if (Value.length() == 0 || Option.length() == 0) return;

                if (IsNull(Option))
                {
                    auto NewValue = nlohmann::json::parse(Value);

                    m_Container[ConvertOptionToJSONPointer(Option)] = NewValue;

                    auto Value = m_Container[ConvertOptionToJSONPointer(Option)];

                    ENGINE_CONSOLE_INFOV("New option %s with value %s", Option.c_str(), Value.dump().c_str());
                }
                else
                {
                    auto CurrentValue = m_Container[ConvertOptionToJSONPointer(Option)];

                    try
                    {
                        auto NewValue = nlohmann::json::parse(Value);

                        if (NewValue.type() == CurrentValue.type())
                        {
                            m_Container[ConvertOptionToJSONPointer(Option)] = NewValue;

                            auto Value = m_Container[ConvertOptionToJSONPointer(Option)];

                            ENGINE_CONSOLE_INFOV("%s is set to %s", Option.c_str(), Value.dump().c_str());
                        }
                        else
                        {
                            ENGINE_CONSOLE_ERRORV("Type mismatch for option %s", Option.c_str());
                        }
                    }
                    catch (json::exception& rException)
                    {
                        ENGINE_CONSOLE_ERRORV("%s", rException.what());
                    }
                }
            }
        };

        Gui::EventHandler::RegisterDirectUserListener(InputCommandDelegate);
    }

    // -----------------------------------------------------------------------------

    CProgramParameters::~CProgramParameters()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::ParseJSON(const std::string& _rJSON)
    {
        try
        {
            m_Container = json::parse(_rJSON);
        }
        catch (const json::exception& _rException)
        {
            ENGINE_CONSOLE_ERRORV("Failed parsing JSON with reason \"%s\". Container will be empty.", _rException.what());

            ParseJSON("{ }");
        }
        catch (...)
        {
            ENGINE_CONSOLE_ERROR("An undefined exception got up while parsing JSON file.");

            ParseJSON("{ }");
        }
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::ParseFile(const std::string& _rFile)
    {
        std::ifstream JSONFile(_rFile.c_str());

        if (JSONFile.is_open())
        {
            std::string FileContent((std::istreambuf_iterator<char>(JSONFile)), std::istreambuf_iterator<char>());

            ParseJSON(FileContent);
            
            JSONFile.close();
        }
        else
        {
            ENGINE_CONSOLE_WARNINGV("Config file %s could not be opened or does not exist! Container will be empty.", _rFile.c_str());

            ParseJSON("{ }");
        }
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::WriteFile(const std::string& _rFile)
    {
        std::ofstream JSONFile(_rFile.c_str());

        if (JSONFile.is_open())
        {
            JSONFile.clear();

            JSONFile << std::setw(4) << m_Container << std::endl;

            JSONFile.close();
        }
        else
        {
            ENGINE_CONSOLE_ERRORV("Save file %s could not be opened. No changes will be saved.", _rFile.c_str());
        }
    }

    // -----------------------------------------------------------------------------

    void CProgramParameters::Clear()
    {
        m_Container.clear();
    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::IsNull(const std::string& _rOption)
    {
        return m_Container[ConvertOptionToJSONPointer(_rOption)].is_null();
    }

    // -----------------------------------------------------------------------------

    json::json_pointer CProgramParameters::ConvertOptionToJSONPointer(const std::string& _rOption)
    {
        std::string Copy = _rOption;

        std::replace(Copy.begin(), Copy.end(), ':', '/');

        return json::json_pointer("/" + Copy);
    }

    // -----------------------------------------------------------------------------

    const std::string CProgramParameters::Get(const std::string& _rOption, const char* _Default)
    {
        return Get(_rOption, std::string(_Default));
    }
} // namespace Core