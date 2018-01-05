
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_performance.h"

#include <cassert>
#include <unordered_map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#ifdef __ANDROID__
GfxQueryCounterEXT glQueryCounter = 0;
GfxGetQueryObjectui64vEXT glGetQueryObjectui64v = 0;
#endif // __ANDROID__

namespace
{
    bool g_QueryPerformanceMarkerDurations = true;
}

namespace 
{
    class CGfxPerformance : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxPerformance);
    
    public:

        CGfxPerformance();
       ~CGfxPerformance();
    
    public:

        void OnStart();
        void Update();
        void OnExit();

        void BeginEvent(const Base::Char* _pEventName);
        void BeginDurationEvent(const Base::Char* _pEventName);
        void ResetEventStatistics(const Base::Char* _pEventName);
        void EndEvent();

        void StartDurationQuery(unsigned int _ID, Gfx::Performance::CDurationQueryDelegate _Delegate);
        void EndDurationQuery();
        float EndDurationQueryWithSync();

    private:

        void CheckDurationQueries();
        void CheckPerformanceMarkerQueries();

        struct SQueryStackItem
        {
            Base::U32 m_ID;
            GLuint m_StartQuery;
            GLuint m_EndQuery;
            Base::U64 m_Frame;
            Gfx::Performance::CDurationQueryDelegate m_Callback;
        };

        struct SPerformanceMarker
        {
            int m_NumberOfMarkers;
            float m_AccumulatedTime;

            std::vector<std::pair<GLuint, GLuint>> m_PendingQueries;

            bool m_HasStatistics;

            SPerformanceMarker()
                : m_NumberOfMarkers(0)
                , m_AccumulatedTime(0.0f)
                , m_HasStatistics(false)
            {

            }
        };

        std::vector<SQueryStackItem> m_Queries;
        std::vector<SQueryStackItem> m_QueryStack;

        std::unordered_map<std::string, SPerformanceMarker> m_PerformanceMarkerTimings;
        std::stack<SPerformanceMarker*> m_OpenedMarkerStack;
    };
} // namespace 

namespace 
{
    CGfxPerformance::CGfxPerformance()
        : m_Queries()
        , m_QueryStack()
        , m_PerformanceMarkerTimings()
        , m_OpenedMarkerStack()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxPerformance::~CGfxPerformance()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::OnStart()
    {
#ifdef __ANDROID__
        if (Gfx::Main::IsExtensionAvailable("GL_EXT_disjoint_timer_query"))
        {
            glQueryCounter = reinterpret_cast<GfxQueryCounterEXT>(eglGetProcAddress("glQueryCounterEXT"));
            glGetQueryObjectui64v = reinterpret_cast<GfxGetQueryObjectui64vEXT>(eglGetProcAddress("glGetQueryObjectui64vEXT"));
        }
        else
        {
            BASE_THROWM("GL_EXT_disjoint_timer_query is not supported but it is highly needed!");
        }
#endif
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::Update()
    {
        CheckDurationQueries();
        if (g_QueryPerformanceMarkerDurations)
        {
            CheckPerformanceMarkerQueries();
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::OnExit()
    {
        if (g_QueryPerformanceMarkerDurations)
        {
            for (auto& rItemPair : m_PerformanceMarkerTimings)
            {
                auto& rItem = rItemPair.second;

                if (rItem.m_HasStatistics)
                {
                    while (!rItem.m_PendingQueries.empty())
                    {
                        auto QueryPair = rItem.m_PendingQueries.back();
                        rItem.m_PendingQueries.pop_back();

                        GLuint64 StartTime, EndTime;
                        glGetQueryObjectui64v(QueryPair.first, GL_QUERY_RESULT, &StartTime);
                        glGetQueryObjectui64v(QueryPair.second, GL_QUERY_RESULT, &EndTime);

                        float QueryDuration = (EndTime - StartTime) / 1000000.0f;

                        rItem.m_AccumulatedTime += QueryDuration;
                        ++rItem.m_NumberOfMarkers;
                    }

                    std::stringstream Stream;

                    Stream << '\n' << rItemPair.first << '\n'
                        << rItem.m_NumberOfMarkers << " Times called\n"
                        << rItem.m_AccumulatedTime / rItem.m_NumberOfMarkers << " ms average time\n";

                    BASE_CONSOLE_STREAMINFO(Stream.str());
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::CheckDurationQueries()
    {
        for (auto i = m_Queries.begin(); i < m_Queries.end();)
        {
            SQueryStackItem& rItem = *i;

            GLuint StartQuery = rItem.m_StartQuery;
            GLuint EndQuery = rItem.m_EndQuery;


            GLuint IsStartQueryAvailable;
            glGetQueryObjectuiv(StartQuery, GL_QUERY_RESULT_AVAILABLE, &IsStartQueryAvailable);
            GLuint IsEndQueryAvailable;
            glGetQueryObjectuiv(EndQuery, GL_QUERY_RESULT_AVAILABLE, &IsEndQueryAvailable);

            if (IsStartQueryAvailable == GL_TRUE && IsEndQueryAvailable == GL_TRUE)
            {
                GLuint64 StartTime, EndTime;
                glGetQueryObjectui64v(rItem.m_StartQuery, GL_QUERY_RESULT, &StartTime);
                glGetQueryObjectui64v(rItem.m_EndQuery, GL_QUERY_RESULT, &EndTime);

                assert(rItem.m_Callback);

                rItem.m_Callback(rItem.m_ID, (EndTime - StartTime) / 1000000.0f, rItem.m_Frame);

                i = m_Queries.erase(i);

                glDeleteQueries(1, &StartQuery);
                glDeleteQueries(1, &EndQuery);
            }
            else
            {
                ++ i;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::CheckPerformanceMarkerQueries()
    {
        for (auto& rItemPair : m_PerformanceMarkerTimings)
        {
            auto& rItem = rItemPair.second;

            for (auto i = rItem.m_PendingQueries.begin(); i < rItem.m_PendingQueries.end();)
            {
                GLuint IsStartQueryAvailable;
                glGetQueryObjectuiv(i->first, GL_QUERY_RESULT_AVAILABLE, &IsStartQueryAvailable);
                GLuint IsEndQueryAvailable;
                glGetQueryObjectuiv(i->second, GL_QUERY_RESULT_AVAILABLE, &IsEndQueryAvailable);

                if (IsStartQueryAvailable && IsEndQueryAvailable)
                {
                    GLuint64 StartTime, EndTime;
                    glGetQueryObjectui64v(i->first, GL_QUERY_RESULT, &StartTime);
                    glGetQueryObjectui64v(i->second, GL_QUERY_RESULT, &EndTime);

                    glDeleteQueries(1, &i->first);
                    glDeleteQueries(1, &i->second);

                    float QueryDuration = (EndTime - StartTime) / 1000000.0f;

                    rItem.m_AccumulatedTime += QueryDuration;
                    ++ rItem.m_NumberOfMarkers;

                    i = rItem.m_PendingQueries.erase(i);
                }
                else
                {
                    ++ i;
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::BeginEvent(const Base::Char* _pEventName)
    {
        GLsizei LengthOfEventName = static_cast<GLsizei>(strlen(_pEventName));

        glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, LengthOfEventName, _pEventName);

        if (Gfx::Main::GetGraphicsAPI() != Gfx::OpenGLES)
        {
            if (g_QueryPerformanceMarkerDurations)
            {
                std::string Name = _pEventName;

                auto& Item = m_PerformanceMarkerTimings[Name];

                Item.m_HasStatistics = false;

                GLuint StartQuery;
#ifdef __ANDROID__
                glGenQueries(1, &StartQuery);
                glQueryCounter(StartQuery, GL_TIMESTAMP_EXT);
#else
                glCreateQueries(GL_TIMESTAMP, 1, &StartQuery);
                glQueryCounter(StartQuery, GL_TIMESTAMP);
#endif

                Item.m_PendingQueries.push_back(std::make_pair(StartQuery, 0));

                m_OpenedMarkerStack.push(&Item);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::BeginDurationEvent(const Base::Char* _pEventName)
    {
        GLsizei LengthOfEventName = static_cast<GLsizei>(strlen(_pEventName));

        glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, LengthOfEventName, _pEventName);

        if (Gfx::Main::GetGraphicsAPI() != Gfx::OpenGLES)
        {
            if (g_QueryPerformanceMarkerDurations)
            {
                std::string Name = _pEventName;

                auto& Item = m_PerformanceMarkerTimings[Name];

                Item.m_HasStatistics = true;

                GLuint StartQuery;
#ifdef __ANDROID__
                glGenQueries(1, &StartQuery);
                glQueryCounter(StartQuery, GL_TIMESTAMP_EXT);
#else
                glCreateQueries(GL_TIMESTAMP, 1, &StartQuery);
                glQueryCounter(StartQuery, GL_TIMESTAMP);
#endif

                Item.m_PendingQueries.push_back(std::make_pair(StartQuery, 0));

                m_OpenedMarkerStack.push(&Item);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::ResetEventStatistics(const Base::Char* _pEventName)
    {
        auto Iter = m_PerformanceMarkerTimings.find(_pEventName);

        if (Iter != m_PerformanceMarkerTimings.end())
        {
            Iter->second.m_AccumulatedTime = 0;
            Iter->second.m_NumberOfMarkers = 0;

            for (auto QueryPair : Iter->second.m_PendingQueries)
            {
                glDeleteQueries(1, &QueryPair.first);
                glDeleteQueries(1, &QueryPair.second);
            }
            Iter->second.m_PendingQueries.clear();
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::EndEvent()
    {
        glPopDebugGroup();

        if (Gfx::Main::GetGraphicsAPI() != Gfx::OpenGLES)
        {
            if (g_QueryPerformanceMarkerDurations)
            {
                GLuint EndQuery;
#ifdef __ANDROID__
                glGenQueries(1, &EndQuery);

                glQueryCounter(EndQuery, GL_TIMESTAMP_EXT);
#else
                glCreateQueries(GL_TIMESTAMP, 1, &EndQuery);
                glQueryCounter(EndQuery, GL_TIMESTAMP);
#endif

                m_OpenedMarkerStack.top()->m_PendingQueries.back().second = EndQuery;
                m_OpenedMarkerStack.pop();
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::StartDurationQuery(unsigned int _ID, Gfx::Performance::CDurationQueryDelegate _Delegate)
    {
        GLuint StartQuery = 0;
        GLuint EndQuery   = 0;

#ifdef __ANDROID__
        glGenQueries(1, &StartQuery);
        glGenQueries(1, &EndQuery);

        glQueryCounter(StartQuery, GL_TIMESTAMP_EXT);
#else
        glCreateQueries(GL_TIMESTAMP, 1, &StartQuery);
        glCreateQueries(GL_TIMESTAMP, 1, &EndQuery);

        glQueryCounter(StartQuery, GL_TIMESTAMP);
#endif

        SQueryStackItem Item = { _ID, StartQuery, EndQuery, Core::Time::GetNumberOfFrame(), _Delegate };
        m_QueryStack.push_back(Item);
    }

    // -----------------------------------------------------------------------------

    void CGfxPerformance::EndDurationQuery()
    {
        assert(!m_QueryStack.empty());

        SQueryStackItem Item = m_QueryStack.back();
        m_QueryStack.pop_back();

#ifdef __ANDROID__
        glQueryCounter(Item.m_EndQuery, GL_TIMESTAMP_EXT);
#else
        glQueryCounter(Item.m_EndQuery, GL_TIMESTAMP);
#endif

        m_Queries.push_back(Item);
    }

    // -----------------------------------------------------------------------------

    float CGfxPerformance::EndDurationQueryWithSync()
    {
        assert(!m_QueryStack.empty());

        SQueryStackItem Item = m_QueryStack.back();
        m_QueryStack.pop_back();

#ifdef __ANDROID__
        glQueryCounter(Item.m_EndQuery, GL_TIMESTAMP_EXT);
#else
        glQueryCounter(Item.m_EndQuery, GL_TIMESTAMP);
#endif

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
    void OnStart()
    {
        CGfxPerformance::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CGfxPerformance::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxPerformance::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void BeginEvent(const Base::Char* _pEventName)
    {
        CGfxPerformance::GetInstance().BeginEvent(_pEventName);
    }

    // -----------------------------------------------------------------------------

    void BeginDurationEvent(const Base::Char* _pEventName)
    {
        CGfxPerformance::GetInstance().BeginDurationEvent(_pEventName);
    }

    // -----------------------------------------------------------------------------

    void ResetEventStatistics(const Base::Char* _pEventName)
    {
        CGfxPerformance::GetInstance().ResetEventStatistics(_pEventName);
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