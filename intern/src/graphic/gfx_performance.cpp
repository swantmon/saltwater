
#include "graphic/gfx_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_performance.h"

#include <string.h>

#include "GL/glew.h"

namespace 
{
    class CGfxPerformance : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxPerformance);
    
    public:
    	CGfxPerformance();
       ~CGfxPerformance();
    
    public:
        void BeginEvent(const Base::Char* _pEventName);
        void EndEvent();

        void StartDurationQuery(unsigned int _ID, Gfx::Performance::CDurationQueryDelegate _Delegate);
        void EndDurationQuery();
        float EndDurationQueryWithSync();
    };
} // namespace 

namespace 
{
    CGfxPerformance::CGfxPerformance()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxPerformance::~CGfxPerformance()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::BeginEvent(const Base::Char* _pEventName)
    {
        GLsizei LengthOfEventName = static_cast<GLsizei>(strlen(_pEventName));

        glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, LengthOfEventName, _pEventName);
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::EndEvent()
    {
        glPopDebugGroup();
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::StartDurationQuery(unsigned int _ID, Gfx::Performance::CDurationQueryDelegate _Delegate)
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::EndDurationQuery()
    {

    }

    // -----------------------------------------------------------------------------

    float CGfxPerformance::EndDurationQueryWithSync()
    {
        return 0.0f;
    }
} // namespace 

namespace Gfx
{
namespace Performance
{
    void BeginEvent(const Base::Char* _pEventName)
    {
        CGfxPerformance::GetInstance().BeginEvent(_pEventName);
    }

    // -----------------------------------------------------------------------------

    void EndEvent()
    {
        CGfxPerformance::GetInstance().EndEvent();
    }

    // -----------------------------------------------------------------------------

    void StartDurationQuery(unsigned int _ID, CDurationQueryDelegate _Delegate)
    {
        CGfxPerformance::GetInstance().StartDurationQuery(_ID, _Delegate);
    }

    // -----------------------------------------------------------------------------

    void EndDurationQuery()
    {
        CGfxPerformance::GetInstance().EndDurationQuery();
    }

    // -----------------------------------------------------------------------------

    float EndDurationQueryWithSync()
    {
        return CGfxPerformance::GetInstance().EndDurationQueryWithSync();
    }
} // namespace Performance
} // namespace Gfx