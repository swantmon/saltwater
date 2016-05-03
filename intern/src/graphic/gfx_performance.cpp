
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
} // namespace Performance
} // namespace Gfx