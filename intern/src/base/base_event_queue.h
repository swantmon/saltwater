
#pragma once

#include "base/base_defines.h"
#include "base/base_uncopyable.h"

#include <queue>

namespace CON
{

    template<class TEvent>
	class CEventQueue : private Base::CUncopyable
    {

    public:

        CEventQueue();
        ~CEventQueue();

    public:

        inline void          AddEvent(const TEvent& _rEvent);
        inline const TEvent& PeekEvent() const;
        inline void          RemoveEvent();
        inline const bool    HasEvent() const;

    private:

        std::queue<TEvent> m_Queue;

    };

} // namespace CON


namespace CON
{

    template<class TEvent>
    inline CEventQueue<TEvent>::CEventQueue()
        : m_Queue()
    {

    }

    // -----------------------------------------------------------------------------

    template<class TEvent>
    inline CEventQueue<TEvent>::~CEventQueue()
    {
    }

    // -----------------------------------------------------------------------------

    template<class TEvent>
    inline void CEventQueue<TEvent>::AddEvent(const TEvent& _rEvent)
    {
        m_Queue.push(_rEvent);
    }

    // -----------------------------------------------------------------------------

    template<class TEvent>
    inline const TEvent& CEventQueue<TEvent>::PeekEvent() const
    {
        return m_Queue.front();
    }

    // -----------------------------------------------------------------------------

    template<class TEvent>
    inline void CEventQueue<TEvent>::RemoveEvent()
    {
        m_Queue.pop();
    }

    // -----------------------------------------------------------------------------

    template<class TEvent>
    inline const bool CEventQueue<TEvent>::HasEvent() const
    {
        return m_Queue.size() != 0;
    }

} // namespace CON