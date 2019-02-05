
#pragma once

#include "base/base_defines.h"
#include "base/base_include_glm.h"

#include <string>

namespace IO
{
namespace Native
{
    static const int s_KeyLeft    = 1073741904;  ///< Cursor left
    static const int s_KeyRight   = 1073741903;  ///< Cursor right
    static const int s_KeyUp      = 1073741906;  ///< Cursor up
    static const int s_KeyDown    = 1073741905;  ///< Cursor down
    static const int s_KeyEscape  = 27;          ///< Escape
    static const int s_KeyEnter   = 13;          ///< Enter
    static const int s_KeyBack    = 8;           ///< Backspace
    static const int s_KeyShift   = 1073742049;  ///< Shift
    static const int s_KeyDelete  = 127;         ///< Delete
    static const int s_KeyEnd     = 1073741901;  ///< End
    static const int s_KeyHome    = 1073741898;  ///< Home
    static const int s_KeyCapital = 1073741881;  ///< Caps lock
    static const int s_KeyControl = 1073742048;  ///< Control
    static const int s_KeyConsole = 60;          ///< Console ^
    static const int s_KeyRhombos = 35;          ///< #
    static const int s_Key0       = 48;          ///< 0
    static const int s_Key1       = 49;          ///< 1
    static const int s_Key2       = 50;          ///< 2
    static const int s_Key3       = 51;          ///< 3
    static const int s_Key4       = 52;          ///< 4
    static const int s_Key5       = 53;          ///< 5
    static const int s_Key6       = 54;          ///< 6
    static const int s_Key7       = 55;          ///< 7
    static const int s_Key8       = 56;          ///< 8
    static const int s_Key9       = 57;          ///< 9
    static const int s_KeyA       = 'a';         ///< A
    static const int s_KeyD       = 'd';         ///< D
    static const int s_KeyS       = 's';         ///< S
    static const int s_KeyW       = 'w';         ///< W
    
    static const int s_KeyModifierAlt     = 256;
    static const int s_KeyModifierShift   = 1;
    static const int s_KeyModifierCapital = 8192;
    static const int s_KeyModifierControl = 64;
} // namespace Native
} // namespace IO

namespace IO
{
    class CInputEvent
    {
    public:
        
        enum EType
        {
            Input,
            Command,
            Exit,
        };
        
        enum EAction
        {
            MouseMove,
            MouseWheel,
            MouseLeftPressed,
            MouseLeftReleased,
            MouseMiddlePressed,
            MouseMiddleReleased,
            MouseRightPressed,
            MouseRightReleased,
            KeyPressed,
            KeyCharPressed,
            KeyReleased,
            TouchMove,
            TouchPressed,
            TouchReleased,
            GamepadKeyPressed,
            GamepadKeyReleased,
            GamepadAxisMotion,
            GamepadTriggerMotion,
            UndefinedAction = -1,
        };
        
        enum EKey
        {
            KeyLeft    = Native::s_KeyLeft   ,  
            KeyRight   = Native::s_KeyRight  ,  
            KeyUp      = Native::s_KeyUp     ,  
            KeyDown    = Native::s_KeyDown   ,  
            KeyEscape  = Native::s_KeyEscape ,  
            KeyEnter   = Native::s_KeyEnter  ,  
            KeyBack    = Native::s_KeyBack   ,  
            KeyShift   = Native::s_KeyShift  ,
            KeyDelete  = Native::s_KeyDelete ,
            KeyEnd     = Native::s_KeyEnd    ,
            KeyHome    = Native::s_KeyHome   ,
            KeyCapital = Native::s_KeyCapital,
            KeyControl = Native::s_KeyControl,
            KeyConsole = Native::s_KeyConsole,
            KeyRhombos = Native::s_KeyRhombos,
            Key0       = Native::s_Key0      ,
            Key1       = Native::s_Key1      ,
            Key2       = Native::s_Key2      ,
            Key3       = Native::s_Key3      ,
            Key4       = Native::s_Key4      ,
            Key5       = Native::s_Key5      ,
            Key6       = Native::s_Key6      ,
            Key7       = Native::s_Key7      ,
            Key8       = Native::s_Key8      ,
            Key9       = Native::s_Key9      ,
            KeyA       = Native::s_KeyA      ,
            KeyD       = Native::s_KeyD      ,
            KeyS       = Native::s_KeyS      ,
            KeyW       = Native::s_KeyW      ,
            Mouse      = 0x00                ,
            Pointer    = 0x00                ,
            Start,
            Select,
            Up,
            Down,
            Left,
            Right,
            LeftBumper,
            RightBumper,
            LeftStick,
            RightStick,
            LeftTrigger,
            RightTrigger,
        };
                
        enum EKeyModifier
        {
            KeyModifierAlt     = Native::s_KeyModifierAlt,
            KeyModifierShift   = Native::s_KeyModifierShift,
            KeyModifierCapital = Native::s_KeyModifierCapital,
            KeyModifierControl = Native::s_KeyModifierControl,
        };
        
    public:
        
        inline CInputEvent(unsigned int _Type);
        inline CInputEvent(unsigned int _Type, const std::string& _rCommand);
        inline CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, unsigned int _KeyModifier);
        inline CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, const glm::ivec2& _rPointerPosition, const glm::ivec2& _rLocalPointerPosition);
        inline CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, const glm::ivec2& _rPointerPosition, const glm::ivec2& _rLocalPointerPosition, float _WheelDelta);

        inline CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key);
        inline CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, float _Delta);
        inline CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, unsigned int _Axis, float _Delta);

        inline CInputEvent(const CInputEvent& _rEvent);
        inline ~CInputEvent();
        
    public:
        
        inline CInputEvent& operator = (const CInputEvent& _rEvent);
        
    public:
        
        inline EType GetType() const;
        
        inline unsigned int GetAction() const;
        
        inline unsigned int GetKey() const;
        
        inline bool HasAlt() const;
        inline bool HasShift() const;
        inline bool HasControl() const;
        inline bool HasModifier() const;
        
        inline const glm::ivec2& GetGlobalCursorPosition() const;
        inline const glm::ivec2& GetLocalCursorPosition() const;
        
        inline float GetDelta() const;

        inline int GetAxis() const;

        inline const std::string& GetCommand() const;
        
    private:
        
        struct SBits
        {
            unsigned int m_Action;
            unsigned int m_Key;
            unsigned int m_Type;
            unsigned int m_KeyModifier;
            int m_Axis;
        };
        
    private:
        
        SBits       m_Bits;
        std::string m_Command;
        glm::ivec2  m_GlobalPointerPosition;
        glm::ivec2  m_LocalPointerPosition;
        float       m_WheelDelta;
    };
} // namespace IO

namespace IO
{
    inline CInputEvent::CInputEvent(unsigned int _Type)
        : m_Command              ()
        , m_GlobalPointerPosition(-1, -1)
        , m_LocalPointerPosition (-1, -1)
        , m_WheelDelta           (0)
    {
        m_Bits.m_Action      = 0;
        m_Bits.m_Key         = 0;
        m_Bits.m_Type        = _Type;
        m_Bits.m_Axis        = -1;
        m_Bits.m_KeyModifier = 0;
    }

    // -----------------------------------------------------------------------------

    inline CInputEvent::CInputEvent(unsigned int _Type, const std::string& _rCommand)
        : CInputEvent(_Type)
    {
        m_Command = _rCommand;
    }
    
    // -----------------------------------------------------------------------------
    
    inline CInputEvent::CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, unsigned int _KeyCombo)
        : m_Command              ()
        , m_GlobalPointerPosition(-1, -1)
        , m_LocalPointerPosition (-1, -1)
        , m_WheelDelta           (0)
    {
        m_Bits.m_Action      = _Action;
        m_Bits.m_Key         = _Key;
        m_Bits.m_Type        = _Type;
        m_Bits.m_KeyModifier = _KeyCombo;
    }
    
    // -----------------------------------------------------------------------------
    
    inline CInputEvent::CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, const glm::ivec2& _rPointerPosition, const glm::ivec2& _rLocalPointerPosition)
        : m_Command              ()
        , m_GlobalPointerPosition(_rPointerPosition)
        , m_LocalPointerPosition (_rLocalPointerPosition)
        , m_WheelDelta           (0)
    {
        m_Bits.m_Type        = _Type;
        m_Bits.m_Action      = _Action;
        m_Bits.m_Key         = _Key;
        m_Bits.m_KeyModifier = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    inline CInputEvent::CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, const glm::ivec2& _rPointerPosition, const glm::ivec2& _rLocalPointerPosition, float _WheelDelta)
        : m_Command              ()
        , m_GlobalPointerPosition(_rPointerPosition)
        , m_LocalPointerPosition (_rLocalPointerPosition)
        , m_WheelDelta           (_WheelDelta)
    {
        m_Bits.m_Type        = _Type;
        m_Bits.m_Action      = _Action;
        m_Bits.m_Key         = _Key;
        m_Bits.m_KeyModifier = 0;
    }
    
    // -----------------------------------------------------------------------------

    inline CInputEvent::CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key)
        : m_Command              ()
        , m_GlobalPointerPosition(-1, -1)
        , m_LocalPointerPosition (-1, -1)
        , m_WheelDelta           (0)
    {
        m_Bits.m_Type        = _Type;
        m_Bits.m_Action      = _Action;
        m_Bits.m_Key         = _Key;
        m_Bits.m_KeyModifier = 0;
    }

    // -----------------------------------------------------------------------------

    inline CInputEvent::CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, float _Delta)
        : m_Command              ()
        , m_GlobalPointerPosition(-1, -1)
        , m_LocalPointerPosition (-1, -1)
    {
        m_Bits.m_Type        = _Type;
        m_Bits.m_Action      = _Action;
        m_Bits.m_Key         = _Key;
        m_Bits.m_KeyModifier = 0;
        m_WheelDelta         = _Delta;
    }

    // -----------------------------------------------------------------------------

    inline CInputEvent::CInputEvent(unsigned int _Type, unsigned int _Action, unsigned int _Key, unsigned int _Axis, float _Delta)
        : m_Command              ()
        , m_GlobalPointerPosition(-1, -1)
        , m_LocalPointerPosition (-1, -1)
    {
        m_Bits.m_Type        = _Type;
        m_Bits.m_Action      = _Action;
        m_Bits.m_Key         = _Key;
        m_Bits.m_KeyModifier = 0;
        m_Bits.m_Axis        = _Axis;
        m_WheelDelta         = _Delta;
    }

    // -----------------------------------------------------------------------------
    
    inline CInputEvent::CInputEvent(const CInputEvent& _rEvent)
        : m_Command              (_rEvent.m_Command)
        , m_GlobalPointerPosition(_rEvent.m_GlobalPointerPosition)
        , m_LocalPointerPosition (_rEvent.m_LocalPointerPosition)
        , m_WheelDelta           (_rEvent.m_WheelDelta)
    {
        m_Bits.m_Type        = _rEvent.m_Bits.m_Type;
        m_Bits.m_Action      = _rEvent.m_Bits.m_Action;
        m_Bits.m_Key         = _rEvent.m_Bits.m_Key;
        m_Bits.m_KeyModifier = _rEvent.m_Bits.m_KeyModifier;
        m_Bits.m_Axis        = _rEvent.m_Bits.m_Axis;
    }
    
    // -----------------------------------------------------------------------------
    
    inline CInputEvent::~CInputEvent()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CInputEvent& CInputEvent::operator = (const CInputEvent& _rEvent)
    {
        m_Bits.m_Type           = _rEvent.m_Bits.m_Type;
        m_Bits.m_Action         = _rEvent.m_Bits.m_Action;
        m_Bits.m_Key            = _rEvent.m_Bits.m_Key;
        m_Bits.m_KeyModifier    = _rEvent.m_Bits.m_KeyModifier;
        m_Bits.m_Axis           = _rEvent.m_Bits.m_Axis;
        m_Command               = _rEvent.m_Command;
        m_GlobalPointerPosition = _rEvent.m_GlobalPointerPosition;
        m_LocalPointerPosition  = _rEvent.m_LocalPointerPosition;
        m_WheelDelta            = _rEvent.m_WheelDelta;
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    inline CInputEvent::EType CInputEvent::GetType() const
    {
        return static_cast<CInputEvent::EType>(m_Bits.m_Type);
    }
    
    // -----------------------------------------------------------------------------
    
    inline unsigned int CInputEvent::GetAction() const
    {
        return static_cast<CInputEvent::EAction>(m_Bits.m_Action);
    }
    
    // -----------------------------------------------------------------------------
    
    inline unsigned int CInputEvent::GetKey() const
    {
        return static_cast<CInputEvent::EKey>(m_Bits.m_Key);
    }
    
    // -----------------------------------------------------------------------------
    
    inline bool CInputEvent::HasAlt() const
    {
        return (m_Bits.m_KeyModifier & KeyModifierAlt) != 0;
    }
    
    // -----------------------------------------------------------------------------
    
    inline bool CInputEvent::HasShift() const
    {
        return (m_Bits.m_KeyModifier & KeyModifierShift) != 0;
    }
    
    // -----------------------------------------------------------------------------
    
    inline bool CInputEvent::HasControl() const
    {
        return (m_Bits.m_KeyModifier & KeyModifierControl) != 0;
    }
    
    // -----------------------------------------------------------------------------
    
    inline bool CInputEvent::HasModifier() const
    {
        return m_Bits.m_KeyModifier > 0;
    }
    
    // -----------------------------------------------------------------------------
    
    inline const glm::ivec2& CInputEvent::GetGlobalCursorPosition() const
    {
        return m_GlobalPointerPosition;
    }

    // -----------------------------------------------------------------------------

    inline const glm::ivec2& CInputEvent::GetLocalCursorPosition() const
    {
        return m_LocalPointerPosition;
    }
    
    // -----------------------------------------------------------------------------
    
    inline float CInputEvent::GetDelta() const
    {
        return m_WheelDelta;
    }

    // -----------------------------------------------------------------------------

    inline int CInputEvent::GetAxis() const
    {
        assert(m_Bits.m_Axis == 0 || m_Bits.m_Axis == 1);

        return m_Bits.m_Axis;
    }

    // -----------------------------------------------------------------------------

    inline const std::string& CInputEvent::GetCommand() const
    {
        return m_Command;
    }
} // namespace IO