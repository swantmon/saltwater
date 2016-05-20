
#include "gui/gui_precompiled.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "gui/gui_input_manager.h"
#include "gui/gui_event_handler.h"

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

        bool GetMouseButton(Base::CInputEvent::EKey _Key);
        bool GetMouseButtonDown(Base::CInputEvent::EKey _Key);
        bool GetMouseButtonUp(Base::CInputEvent::EKey _Key);

        Base::Short2& GetMousePosition();
        float GetMouseScrollDelta();

    private:

        struct SMouse
        {
            int          m_ButtonLeft;
            int          m_ButtonMiddle;
            int          m_ButtonRight;
            Base::Short2 m_LatestMousePosition;
            float        m_ScrollDelta;
        };

    private:

        typedef std::map<Base::CInputEvent::EKey, int> CKeyboardKeys;

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
        // -----------------------------------------------------------------------------
        // register input event to gui
        // -----------------------------------------------------------------------------
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
        // -----------------------------------------------------------------------------
        // Keyboard
        // -----------------------------------------------------------------------------
        CKeyboardKeys::iterator CurrentKey = m_KeyboardKeys.begin();
        CKeyboardKeys::iterator EndOfKeys  = m_KeyboardKeys.end();

        for (; CurrentKey != EndOfKeys; ++CurrentKey)
        {
            if (CurrentKey->second >= 0)
            {
                ++ CurrentKey->second;
            }
        }
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::IsAnyKey()
    {
        // -----------------------------------------------------------------------------
        // Keyboard
        // -----------------------------------------------------------------------------
        CKeyboardKeys::iterator CurrentKey = m_KeyboardKeys.begin();
        CKeyboardKeys::iterator EndOfKeys = m_KeyboardKeys.end();

        for (; CurrentKey != EndOfKeys; ++CurrentKey)
        {
            if (CurrentKey->second != -2)
            {
                return true;
            }
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::IsAnyKeyDown()
    {
        // -----------------------------------------------------------------------------
        // Keyboard
        // -----------------------------------------------------------------------------
        CKeyboardKeys::iterator CurrentKey = m_KeyboardKeys.begin();
        CKeyboardKeys::iterator EndOfKeys = m_KeyboardKeys.end();

        for (; CurrentKey != EndOfKeys; ++CurrentKey)
        {
            if (CurrentKey->second == 0)
            {
                return true;
            }
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::IsAnyKeyUp()
    {
        // -----------------------------------------------------------------------------
        // Keyboard
        // -----------------------------------------------------------------------------
        CKeyboardKeys::iterator CurrentKey = m_KeyboardKeys.begin();
        CKeyboardKeys::iterator EndOfKeys = m_KeyboardKeys.end();

        for (; CurrentKey != EndOfKeys; ++CurrentKey)
        {
            if (CurrentKey->second == -1)
            {
                return true;
            }
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetKey(Base::CInputEvent::EKey _Key)
    {
        return m_KeyboardKeys.find(_Key) != m_KeyboardKeys.end() && m_KeyboardKeys.at(_Key) >= 0;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetKeyDown(Base::CInputEvent::EKey _Key)
    {
        return m_KeyboardKeys.find(_Key) != m_KeyboardKeys.end() && m_KeyboardKeys.at(_Key) == 0;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetKeyUp(Base::CInputEvent::EKey _Key)
    {
        bool Return = m_KeyboardKeys.find(_Key) != m_KeyboardKeys.end() && m_KeyboardKeys.at(_Key) == -1;

        if (Return)
        {
            m_KeyboardKeys.at(_Key) = -2;
        }

        return Return;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetMouseButton(Base::CInputEvent::EKey _Key)
    {
        return m_Mouse.m_ButtonLeft >= 0;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetMouseButtonDown(Base::CInputEvent::EKey _Key)
    {
        return m_Mouse.m_ButtonLeft == 0;
    }

    // -----------------------------------------------------------------------------

    bool CGuiInputManager::GetMouseButtonUp(Base::CInputEvent::EKey _Key)
    {
        bool Return = m_Mouse.m_ButtonLeft == -1;

        if (Return)
        {
            m_Mouse.m_ButtonLeft = -2;
        }

        return Return;
    }

    // -----------------------------------------------------------------------------

    Base::Short2& CGuiInputManager::GetMousePosition()
    {
        return m_Mouse.m_LatestMousePosition;
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

            CKeyboardKeys::iterator LowerBound = m_KeyboardKeys.lower_bound(Key);

            if (LowerBound != m_KeyboardKeys.end() && !(m_KeyboardKeys.key_comp()(Key, LowerBound->first)))
            {
                LowerBound->second = 0;
            }
            else
            {
                m_KeyboardKeys.insert(LowerBound, CKeyboardKeys::value_type(Key, 0));
            }
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::KeyReleased)
        {
            Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(_rEvent.GetKey());

            m_KeyboardKeys.at(Key) = -1;
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseLeftPressed)
        {
            m_Mouse.m_ButtonLeft = 0;
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseLeftReleased)
        {
            m_Mouse.m_ButtonLeft = -1;
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseWheel)
        {
            m_Mouse.m_ScrollDelta = _rEvent.GetWheelDelta();
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseMove)
        {
            m_Mouse.m_LatestMousePosition = _rEvent.GetCursorPosition();
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

    bool GetMouseButton(Base::CInputEvent::EKey _Key)
    {
        return CGuiInputManager::GetInstance().GetMouseButton(_Key);
    }

    // -----------------------------------------------------------------------------

    bool GetMouseButtonDown(Base::CInputEvent::EKey _Key)
    {
        return CGuiInputManager::GetInstance().GetMouseButtonDown(_Key);
    }

    // -----------------------------------------------------------------------------

    bool GetMouseButtonUp(Base::CInputEvent::EKey _Key)
    {
        return CGuiInputManager::GetInstance().GetMouseButtonUp(_Key);
    }

    // -----------------------------------------------------------------------------

    Base::Short2& GetMousePosition()
    {
        return CGuiInputManager::GetInstance().GetMousePosition();
    }

    // -----------------------------------------------------------------------------

    float GetMouseScrollDelta()
    {
        return CGuiInputManager::GetInstance().GetMouseScrollDelta();
    }
} // namespace InputManager
} // namespace Gui