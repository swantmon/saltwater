
#include "engine/engine_precompiled.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/gui/gui_input_manager.h"
#include "engine/gui/gui_event_handler.h"

#include <map>

namespace 
{
    class CGuiInputManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGuiInputManager)

    public:

        CGuiInputManager();
        ~CGuiInputManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

        bool IsAnyKey();
        bool IsAnyKeyDown();
        bool IsAnyKeyUp();

        bool GetKey(Base::CInputEvent::EKey _Key);
        bool GetKeyDown(Base::CInputEvent::EKey _Key);
        bool GetKeyUp(Base::CInputEvent::EKey _Key);

        const glm::vec2& GetGlobalMousePosition();
        const glm::vec2& GetLocalMousePosition();
        float GetMouseScrollDelta();

    private:

        enum EKeyState
        {
            Idle,
            Pressed,
            Released,
        };

        struct SMouse
        {
            glm::vec2 m_LatestGlobalMousePosition;
            glm::vec2 m_LatestLocalMousePosition;
            float     m_ScrollDelta;
        };

    private:

        typedef std::map<Base::CInputEvent::EKey, EKeyState> CKeyboardKeys;

    private:

        SMouse        m_Mouse;
        CKeyboardKeys m_KeyboardKeys;

    private:

        void OnEvent(const Base::CInputEvent& _rEvent);
    };
} // namespace 

namespace 
{
    CGuiInputManager::CGuiInputManager()
        : m_Mouse       ()
        , m_KeyboardKeys()
    {
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CGuiInputManager::OnEvent));
    }

    // -----------------------------------------------------------------------------

    CGuiInputManager::~CGuiInputManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGuiInputManager::OnStart()
    {
        m_KeyboardKeys.clear();
    }

    // -----------------------------------------------------------------------------

    void CGuiInputManager::OnExit()
    {
        m_KeyboardKeys.clear();
    }

    // -----------------------------------------------------------------------------

    void CGuiInputManager::Update()
    {
        for (auto CurrentKey : m_KeyboardKeys)
        {
            if (CurrentKey.second == EKeyState::Released) CurrentKey.second = EKeyState::Idle;
        }
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::IsAnyKey()
    {
        for (auto CurrentKey : m_KeyboardKeys)
        {
            if (CurrentKey.second != EKeyState::Idle) return true;
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::IsAnyKeyDown()
    {
        for (auto CurrentKey : m_KeyboardKeys)
        {
            if (CurrentKey.second == EKeyState::Pressed) return true;
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::IsAnyKeyUp()
    {
        for (auto CurrentKey : m_KeyboardKeys)
        {
            if (CurrentKey.second == EKeyState::Released) return true;
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetKey(Base::CInputEvent::EKey _Key)
    {
        return m_KeyboardKeys.find(_Key) != m_KeyboardKeys.end() && m_KeyboardKeys.at(_Key) != EKeyState::Idle;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetKeyDown(Base::CInputEvent::EKey _Key)
    {
        return m_KeyboardKeys.find(_Key) != m_KeyboardKeys.end() && m_KeyboardKeys.at(_Key) == EKeyState::Pressed;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetKeyUp(Base::CInputEvent::EKey _Key)
    {
        return m_KeyboardKeys.find(_Key) != m_KeyboardKeys.end() && m_KeyboardKeys.at(_Key) == EKeyState::Released;
    }

    // -----------------------------------------------------------------------------

    const glm::vec2& CGuiInputManager::GetGlobalMousePosition()
    {
        return m_Mouse.m_LatestGlobalMousePosition;
    }

    // -----------------------------------------------------------------------------

    const glm::vec2& CGuiInputManager::GetLocalMousePosition()
    {
        return m_Mouse.m_LatestLocalMousePosition;
    }

    // -----------------------------------------------------------------------------

    float CGuiInputManager::GetMouseScrollDelta()
    {
        return m_Mouse.m_ScrollDelta;
    }

    // -----------------------------------------------------------------------------

    void CGuiInputManager::OnEvent(const Base::CInputEvent& _rEvent)
    {
        if (_rEvent.GetAction() == Base::CInputEvent::KeyPressed)
        {
            Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(_rEvent.GetKey());

            m_KeyboardKeys[Key] = EKeyState::Pressed;
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::KeyReleased)
        {
            Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(_rEvent.GetKey());

            m_KeyboardKeys[Key] = EKeyState::Released;
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseWheel)
        {
            m_Mouse.m_ScrollDelta = _rEvent.GetDelta();
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseMove)
        {
            m_Mouse.m_LatestGlobalMousePosition = _rEvent.GetGlobalCursorPosition();
            m_Mouse.m_LatestLocalMousePosition = _rEvent.GetLocalCursorPosition();
        }
    }
} // namespace 

namespace Gui
{
namespace InputManager
{
    void OnStart()
    {
        CGuiInputManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGuiInputManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGuiInputManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    bool IsAnyKey()
    {
        return CGuiInputManager::GetInstance().IsAnyKey();
    }

    // -----------------------------------------------------------------------------

    bool IsAnyKeyDown()
    {
        return CGuiInputManager::GetInstance().IsAnyKeyDown();
    }

    // -----------------------------------------------------------------------------

    bool IsAnyKeyUp()
    {
        return CGuiInputManager::GetInstance().IsAnyKeyUp();
    }

    // -----------------------------------------------------------------------------

    bool GetKey(Base::CInputEvent::EKey _Key)
    {
        return CGuiInputManager::GetInstance().GetKey(_Key);
    }

    // -----------------------------------------------------------------------------

    bool GetKeyDown(Base::CInputEvent::EKey _Key)
    {
        return CGuiInputManager::GetInstance().GetKeyDown(_Key);
    }

    // -----------------------------------------------------------------------------

    bool GetKeyUp(Base::CInputEvent::EKey _Key)
    {
        return CGuiInputManager::GetInstance().GetKeyUp(_Key);
    }

    // -----------------------------------------------------------------------------

    const glm::vec2& GetGlobalMousePosition()
    {
        return CGuiInputManager::GetInstance().GetGlobalMousePosition();
    }

    // -----------------------------------------------------------------------------

    const glm::vec2& GetLocalMousePosition()
    {
        return CGuiInputManager::GetInstance().GetLocalMousePosition();
    }

    // -----------------------------------------------------------------------------

    float GetMouseScrollDelta()
    {
        return CGuiInputManager::GetInstance().GetMouseScrollDelta();
    }
} // namespace InputManager
} // namespace Gui