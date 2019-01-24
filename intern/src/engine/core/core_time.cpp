
#include "engine/engine_precompiled.h"

#include "base/base_clock.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/core/core_time.h"

namespace
{
    class CTime : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CTime)
        
    public:
        
        CTime();
       ~CTime();
        
    public:
        
        void OnStart();
        void OnExit();
        void Update();
                
        double GetDeltaTimeLastFrame();

        double GetTime();
        
        Base::Size GetNumberOfFrame() const;
                
    private:
        
        Base::CDefaultClock m_Clock;
        Base::Size          m_NumberOfFrame;
        double              m_ActiveTime;
    };
} // namespace

namespace
{
    CTime::CTime()
        : m_Clock     ()
        , m_ActiveTime(0.0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CTime::~CTime()
    { }
    
    // -----------------------------------------------------------------------------
    
    void CTime::OnStart()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CTime::OnExit()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CTime::Update()
    {
        m_Clock.OnFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    double CTime::GetDeltaTimeLastFrame()
    {
        return m_Clock.GetDurationOfFrame();
    }

    // -----------------------------------------------------------------------------

    double CTime::GetTime()
    {
        return m_Clock.GetTime();
    }
    
    // -----------------------------------------------------------------------------
    
    Base::Size CTime::GetNumberOfFrame() const
    {
        return m_Clock.GetNumberOfFrame();
    }
}

namespace Core
{
namespace Time
{
    void OnStart()
    {
        CTime::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CTime::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CTime::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    double GetDeltaTimeLastFrame()
    {
        return CTime::GetInstance().GetDeltaTimeLastFrame();
    }

    // -----------------------------------------------------------------------------

    double GetTime()
    {
        return CTime::GetInstance().GetTime();
    }
    
    // -----------------------------------------------------------------------------
    
    Base::Size GetNumberOfFrame()
    {
        return CTime::GetInstance().GetNumberOfFrame();
    }
} // namespace Time
} // namespace Core
