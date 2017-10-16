
#include "graphic/gfx_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_performance.h"

#include <cassert>
#include <string.h>
#include <vector>

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
        void Update();

        void BeginEvent(const Base::Char* _pEventName);
        void EndEvent();

        void StartDurationQuery(unsigned int _ID, Gfx::Performance::CDurationQueryDelegate _Delegate);
        void EndDurationQuery();
        float EndDurationQueryWithSync();

    private:
        struct SQueryStackItem
        {
            unsigned int m_ID;
            GLuint m_StartQuery;
            GLuint m_EndQuery;
            Gfx::Performance::CDurationQueryDelegate m_Callback;
        };

        std::vector<SQueryStackItem> m_Queries;
        std::vector<SQueryStackItem> m_QueryStack;
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

    void CGfxPerformance::Update()
    {
        for (auto i = m_Queries.begin(); i < m_Queries.end();)
        {
            SQueryStackItem& rItem = *i;

            GLuint StartQuery = rItem.m_StartQuery;
            GLuint EndQuery = rItem.m_EndQuery;

            GLint IsStartQueryAvailable;
            glGetQueryObjectiv(StartQuery, GL_QUERY_RESULT_AVAILABLE, &IsStartQueryAvailable);
            GLint IsEndQueryAvailable;
            glGetQueryObjectiv(EndQuery, GL_QUERY_RESULT_AVAILABLE, &IsEndQueryAvailable);

            if (IsStartQueryAvailable == GL_TRUE && IsEndQueryAvailable == GL_TRUE)
            {
                GLuint64 StartTime, EndTime;
                glGetQueryObjectui64v(rItem.m_StartQuery, GL_QUERY_RESULT, &StartTime);
                glGetQueryObjectui64v(rItem.m_EndQuery, GL_QUERY_RESULT, &EndTime);

                rItem.m_Callback(rItem.m_ID, (EndTime - StartTime) / 1000000.0f);

                i = m_Queries.erase(i);

                glDeleteQueries(1, &StartQuery);
                glDeleteQueries(1, &EndQuery);
            }
            else
            {
                ++i;
            }
        }
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
        assert(_Delegate);

        GLuint StartQuery = 0;
        GLuint EndQuery = 0;

        glCreateQueries(GL_TIMESTAMP, 1, &StartQuery);
        glCreateQueries(GL_TIMESTAMP, 1, &EndQuery);

        glQueryCounter(StartQuery, GL_TIMESTAMP);

        SQueryStackItem Item = { _ID, StartQuery, EndQuery, _Delegate };
        m_QueryStack.push_back(Item);
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::EndDurationQuery()
    {
        assert(!m_QueryStack.empty());

        SQueryStackItem Item = m_QueryStack.back();
        m_QueryStack.pop_back();

        glQueryCounter(Item.m_EndQuery, GL_TIMESTAMP);

        m_Queries.push_back(Item);
    }

    // -----------------------------------------------------------------------------

    float CGfxPerformance::EndDurationQueryWithSync()
    {
        assert(!m_QueryStack.empty());

        SQueryStackItem Item = m_QueryStack.back();
        m_QueryStack.pop_back();

        glQueryCounter(Item.m_EndQuery, GL_TIMESTAMP);

        GLint IsQueryAvailable = false;
        IsQueryAvailable = false;
        while (!IsQueryAvailable) {
            glGetQueryObjectiv(Item.m_StartQuery, GL_QUERY_RESULT_AVAILABLE, &IsQueryAvailable);
        }
        IsQueryAvailable = false;
        while (!IsQueryAvailable) {
            glGetQueryObjectiv(Item.m_EndQuery, GL_QUERY_RESULT_AVAILABLE, &IsQueryAvailable);
        }

        GLuint64 StartTime, EndTime;
        glGetQueryObjectui64v(Item.m_StartQuery, GL_QUERY_RESULT, &StartTime);
        glGetQueryObjectui64v(Item.m_EndQuery, GL_QUERY_RESULT, &EndTime);
        glDeleteQueries(1, &Item.m_StartQuery);
        glDeleteQueries(1, &Item.m_EndQuery);

        return (EndTime - StartTime) / 1000000.0f;
    }
} // namespace 

namespace Gfx
{
namespace Performance
{
    void Update()
    {
        CGfxPerformance::GetInstance().Update();
    }

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