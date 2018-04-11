
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
        
        Base::U64 GetNumberOfFrame() const;
                
    private:
        
        Base::CPerformanceClock m_Clock;
        Base::U64               m_NumberOfFrame;
    };
} // namespace

namespace
{
    CTime::CTime()
        : m_Clock          ()
        , m_NumberOfFrame  (0)
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
                
        ++ m_NumberOfFrame;
    }
    
    // -----------------------------------------------------------------------------
    
    double CTime::GetDeltaTimeLastFrame()
    {
        return m_Clock.GetDurationOfFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    Base::U64 CTime::GetNumberOfFrame() const
    {
        return m_NumberOfFrame;
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
    
    Base::U64 GetNumberOfFrame()
    {
        return CTime::GetInstance().GetNumberOfFrame();
    }
} // namespace Time
} // namespace Core
