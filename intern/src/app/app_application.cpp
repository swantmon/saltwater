
#include "app/app_application.h"

#include "base/base_clock.h"
#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "gui/gui_event_handler.h"

#include <vector>

namespace
{
    class CApplication : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CApplication)
        
    public:
        
        CApplication();
       ~CApplication();
        
    public:
        
        void OnStart();
        void OnExit();
        void Update();
        
        void OnInputEvent(Base::CInputEvent& _rInputEvent);
        
        void OnResize(int _Width, int _Height);
        
        void RegisterResizeHandler(App::Application::CResizeDelegate _NewDelgate);
        
        double GetDeltaTimeLastFrame();
        
        Base::U64 GetNumberOfFrame() const;
        
    private:
        
        typedef std::vector<App::Application::CResizeDelegate> CResizeDelagates;
        typedef CResizeDelagates::iterator                     CResizeDelegateIterator;
        
    private:
        
        CResizeDelagates        m_ResizeDelegates;
        Base::CDefaultClock     m_Clock;
        Base::U64               m_NumberOfFrame;
    };
} // namespace

namespace
{
    CApplication::CApplication()
        : m_ResizeDelegates()
        , m_Clock          ()
        , m_NumberOfFrame  (0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CApplication::~CApplication()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnExit()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::Update()
    {
        m_Clock.OnFrame();
                
        ++ m_NumberOfFrame;
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnInputEvent(Base::CInputEvent& _rInputEvent)
    {
        Gui::EventHandler::OnUserEvent(_rInputEvent);
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::OnResize(int _Width, int _Height)
    {
        CResizeDelegateIterator EndOfDelagates = m_ResizeDelegates.end();
        
        for (CResizeDelegateIterator CurrentDelagate = m_ResizeDelegates.begin(); CurrentDelagate != EndOfDelagates; ++CurrentDelagate)
        {
            (*CurrentDelagate)(_Width, _Height);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CApplication::RegisterResizeHandler(App::Application::CResizeDelegate _NewDelgate)
    {
        m_ResizeDelegates.push_back(_NewDelgate);
    }
    
    // -----------------------------------------------------------------------------
    
    double CApplication::GetDeltaTimeLastFrame()
    {
        return m_Clock.GetDurationOfFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    Base::U64 CApplication::GetNumberOfFrame() const
    {
        return m_NumberOfFrame;
    }
}

namespace App
{
namespace Application
{
    void OnStart()
    {
        CApplication::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CApplication::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CApplication::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnInputEvent(Base::CInputEvent& _rInputEvent)
    {
        CApplication::GetInstance().OnInputEvent(_rInputEvent);
    }
    
    // -----------------------------------------------------------------------------
    
    void OnResize(int _Width, int _Height)
    {
        CApplication::GetInstance().OnResize(_Width, _Height);
    }
    
    // -----------------------------------------------------------------------------
    
    void RegisterResizeHandler(CResizeDelegate _NewDelgate)
    {
        CApplication::GetInstance().RegisterResizeHandler(_NewDelgate);
    }
    
    // -----------------------------------------------------------------------------
    
    double GetDeltaTimeLastFrame()
    {
        return CApplication::GetInstance().GetDeltaTimeLastFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    Base::U64 GetNumberOfFrame()
    {
        return CApplication::GetInstance().GetNumberOfFrame();
    }
} // namespace Application
} // namespace App
