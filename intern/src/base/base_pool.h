//
//  base_pool.h
//  base
//
//  Created by Tobias Schwandt on 22/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"
#include "base/base_memory.h"

#include <assert.h>
#include <exception>
#include <memory.h>
#include <new>
#include <stdlib.h>

namespace CON
{
    template <class T, Size TNumberOfItemsPerPage = 64>
    class CPool
    {
    public:
        
        typedef CPool<T>         CThis;
        typedef T                X;
        typedef T*               XPtr;
        typedef const T*         XConstPtr;
        typedef T&               XRef;
        typedef const T&         XConstRef;
        typedef unsigned int     BDifference;
        typedef Size             BSize;
        typedef Size             BID;
        
    private:
        
        struct SLink
        {
            SLink* m_pNext;
            SLink* m_pPrevious;
        };
        
    public:
        
        class CConstIterator
        {
        public:
            
            typedef CConstIterator                   CThis;
            typedef CPool<T, TNumberOfItemsPerPage>  CContainer;
            typedef typename CContainer::X           X;
            typedef typename CContainer::XConstPtr   XPtr;
            typedef typename CContainer::XConstPtr   XConstPtr;
            typedef typename CContainer::XConstRef   XRef;
            typedef typename CContainer::XConstRef   XConstRef;
            typedef typename CContainer::BDifference BDifference;
            typedef typename CContainer::BSize       BSize;
            typedef typename CContainer::BID         BID;
            
        public:
            
            inline CConstIterator();
            inline CConstIterator(const CThis& _rIterator);
            
        public:
            
            inline CThis& operator = (const CThis& _rIterator);
            
        public:
            
            inline XRef operator * () const;
            
            inline XPtr operator -> () const;
            
            inline CThis& operator ++ ();
            inline CThis& operator -- ();
            
            inline CThis operator ++ (int);
            inline CThis operator -- (int);
            
            inline bool operator == (const CThis& _rIterator) const;
            inline bool operator != (const CThis& _rIterator) const;
            
        protected:
            
            SLink* m_pLink;
            
        protected:
            
            inline explicit CConstIterator(SLink* _pLink);
            
        private:
            
            friend class CPool;
        };
        
        class CIterator : public CConstIterator
        {
        public:
            
            typedef CIterator                        CThis;
            typedef CConstIterator                   CBase;
            typedef CPool<T, TNumberOfItemsPerPage>  CContainer;
            typedef typename CContainer::X           X;
            typedef typename CContainer::XPtr        XPtr;
            typedef typename CContainer::XConstPtr   XConstPtr;
            typedef typename CContainer::XRef        XRef;
            typedef typename CContainer::XConstRef   XConstRef;
            typedef typename CContainer::BDifference BDifference;
            typedef typename CContainer::BSize       BSize;
            typedef typename CContainer::BID         BID;
            
        public:
            
            inline CIterator();
            inline CIterator(const CThis& _rIterator);
            
        public:
            
            inline CThis& operator = (const CThis& _rIterator);
            
        public:
            
            inline XRef operator * () const;
            
            inline XPtr operator -> () const;
            
            inline CThis& operator ++ ();
            inline CThis& operator -- ();
            
            inline CThis operator ++ (int);
            inline CThis operator -- (int);
            
        private:
            
            inline explicit CIterator(SLink* _pLink);
            
        private:
            
            friend class CPool;
        };
        
    public:
        
        inline CPool();
        inline virtual ~CPool();
        
    public:
        
        inline void Clear();
        
    public:
        
        inline XRef Allocate();
        inline void Free(XPtr _pItem);
        
    public:
        
        inline CIterator Begin();
        inline CConstIterator Begin() const;
        
        inline CIterator Last();
        inline CConstIterator Last() const;
        
        inline CIterator End();
        inline CConstIterator End() const;
        
    public:
        
        inline BID GetID(XConstRef _rItem) const;
        
        inline XRef GetItem(BID _ID);
        inline XConstRef GetItem(BID _ID) const;
        
    public:
        
        inline BSize GetNumberOfItems() const;
        
    private:
        
        struct SNode
        {
            SLink m_Link;
            BID   m_ID;
            X     m_Item;
        };
        
    private:
        
        BSize   m_NumberOfSetNodes;
        SNode** m_ppFirstPage;
        SNode** m_ppLastPage;
        SNode** m_ppEndPage;
        SLink*  m_pFirstFreeNode;
        SLink   m_EndOfSetNodes;
        
    private:
        
        inline static SNode* GetNode(XPtr _pItem);
        inline static const SNode* GetNode(XConstPtr _pItem);
        
        inline static SNode* GetNode(SLink* _pLink);
        inline static const SNode* GetNode(const SLink* _pLink);
        
    private:
        
        inline void AddSetNode(SNode* _pNode);
        inline void RemoveSetNode(SNode* _pNode);
        
        inline void PushFreeNode(SNode* _pNode);
        inline SNode* PopFreeNode();
        inline bool HasFreeNode() const;
    };
} // namespace CON

namespace CON
{
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::CPool()
        : m_NumberOfSetNodes(0)
        , m_ppFirstPage     (nullptr)
        , m_ppLastPage      (nullptr)
        , m_ppEndPage       (nullptr)
        , m_pFirstFreeNode  (nullptr)
    {
        m_EndOfSetNodes.m_pNext     = &m_EndOfSetNodes;
        m_EndOfSetNodes.m_pPrevious = &m_EndOfSetNodes;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::~CPool()
    {
        Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline void CPool<T, TNumberOfItemsPerPage>::Clear()
    {
        SLink*  pLink;
        SNode** ppPage;
        
        // -----------------------------------------------------------------------------
        // Call the destructor of all the set items.
        // -----------------------------------------------------------------------------
        for (pLink = m_EndOfSetNodes.m_pNext; pLink != &m_EndOfSetNodes; pLink = pLink->m_pNext)
        {
            GetNode(pLink)->m_Item.~X();
        }
        
        // -----------------------------------------------------------------------------
        // Free the pages.
        // -----------------------------------------------------------------------------
        for (ppPage = m_ppFirstPage; ppPage != m_ppLastPage; ++ ppPage)
        {
            Base::CMemory::Free(*ppPage);
        }
        
        // -----------------------------------------------------------------------------
        // Free the page pointer array.
        // -----------------------------------------------------------------------------
        if (m_ppFirstPage != nullptr)
        {
            Base::CMemory::Free(m_ppFirstPage);
        }
        
        // -----------------------------------------------------------------------------
        // Reset the members.
        // -----------------------------------------------------------------------------
        m_NumberOfSetNodes = 0;
        m_pFirstFreeNode   = nullptr;
        m_ppFirstPage      = nullptr;
        m_ppLastPage       = nullptr;
        m_ppEndPage        = nullptr;
        
        // -----------------------------------------------------------------------------
        // Reset the used node list.
        // -----------------------------------------------------------------------------
        m_EndOfSetNodes.m_pNext     = &m_EndOfSetNodes;
        m_EndOfSetNodes.m_pPrevious = &m_EndOfSetNodes;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::XRef CPool<T, TNumberOfItemsPerPage>::Allocate()
    {
        BSize   OldNumberOfPages;
        BSize   NewNumberOfPages;
        SNode*  pNode;
        XPtr    pItem;
        SNode*  pPage;
        SNode** ppFirstPage;
        
        if (HasFreeNode())
        {
            pNode = GetNode(m_pFirstFreeNode);
            
            pItem = new (&pNode->m_Item) X;
            
            PopFreeNode();
            
            AddSetNode(pNode);
        }
        else
        {
            // -----------------------------------------------------------------------------
            // There is not a free node, so we have to allocate a new page.
            // -----------------------------------------------------------------------------
            ppFirstPage = nullptr;
            
            pPage = static_cast<SNode*>(Base::CMemory::Allocate(TNumberOfItemsPerPage * sizeof(*pPage)));
            
            if (pPage == nullptr)
            {
                throw std::bad_alloc();
            }
            
            try
            {
                // -----------------------------------------------------------------------------
                // Check if we have to resize the array with the page addresses.
                // -----------------------------------------------------------------------------
                if (m_ppLastPage == m_ppEndPage)
                {
                    OldNumberOfPages = m_ppLastPage - m_ppFirstPage;
                    NewNumberOfPages = (OldNumberOfPages != 0) ? OldNumberOfPages + (OldNumberOfPages >> 1) : 8;
                    
                    ppFirstPage = static_cast<SNode**>(Base::CMemory::Allocate(NewNumberOfPages * sizeof(*ppFirstPage)));
                    
                    if (ppFirstPage == nullptr)
                    {
                        throw std::bad_alloc();
                    }
                    
                    if (m_ppFirstPage != nullptr)
                    {
                        Base::CMemory::Copy(ppFirstPage, m_ppFirstPage, OldNumberOfPages * sizeof(*ppFirstPage));
                    }
                    
                    // -----------------------------------------------------------------------------
                    // Construct the new item in the first node of the new page.
                    // -----------------------------------------------------------------------------
                    pItem = new (&pPage->m_Item) X;
                    
                    // -----------------------------------------------------------------------------
                    // No exception from now on, so free the old page array.
                    // -----------------------------------------------------------------------------
                    if (m_ppFirstPage != nullptr)
                    {
                        Base::CMemory::Free(m_ppFirstPage);
                    }
                    
                    // -----------------------------------------------------------------------------
                    // Set the members.
                    // -----------------------------------------------------------------------------
                    m_ppFirstPage = ppFirstPage;
                    m_ppLastPage  = ppFirstPage + OldNumberOfPages;
                    m_ppEndPage   = ppFirstPage + NewNumberOfPages;
                }
                else
                {
                    // -----------------------------------------------------------------------------
                    // Construct the new item in the first node of the new page.
                    // -----------------------------------------------------------------------------
                    pItem = new (&pPage->m_Item) X;
                }
            }
            catch (...)
            {
                // -----------------------------------------------------------------------------
                // Free the allocated chunks again.
                // -----------------------------------------------------------------------------
                Base::CMemory::Free(ppFirstPage);
                Base::CMemory::Free(pPage);
                
                throw;
            }
            
            // -----------------------------------------------------------------------------
            // Register the new page in the page array.
            // -----------------------------------------------------------------------------
            *m_ppLastPage = pPage;
            
            ++ m_ppLastPage;
            
            // -----------------------------------------------------------------------------
            // Since all pages are set the ID of the first node in the new page has to be
            // equal to the number of currently set nodes.
            // -----------------------------------------------------------------------------
            pPage->m_ID = m_NumberOfSetNodes;
            
            // -----------------------------------------------------------------------------
            // Add the node to the list containing all allocated nodes.
            // -----------------------------------------------------------------------------
            AddSetNode(pPage);
            
            // -----------------------------------------------------------------------------
            // Initialize all other nodes of the new page and add them to the free list.
            // -----------------------------------------------------------------------------
            for (pNode = pPage + TNumberOfItemsPerPage - 1; pNode != pPage; -- pNode)
            {
                pNode->m_ID = pPage->m_ID + (pNode - pPage);
                
                PushFreeNode(pNode);
            }
        }
        
        return *pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline void CPool<T, TNumberOfItemsPerPage>::Free(XPtr _pItem)
    {
        SNode* pNode;
        
        assert(_pItem != nullptr);
        
        pNode = GetNode(_pItem);
        
        pNode->m_Item.~X();
        
        RemoveSetNode(pNode);
        
        PushFreeNode(pNode);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline void CPool<T, TNumberOfItemsPerPage>::AddSetNode(SNode* _pNode)
    {
        assert(_pNode != nullptr);
        
        _pNode->m_Link.m_pNext     = &m_EndOfSetNodes;
        _pNode->m_Link.m_pPrevious =  m_EndOfSetNodes.m_pPrevious;
        
        m_EndOfSetNodes.m_pPrevious->m_pNext = &_pNode->m_Link;
        m_EndOfSetNodes.m_pPrevious          = &_pNode->m_Link;
        
        ++ m_NumberOfSetNodes;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline void CPool<T, TNumberOfItemsPerPage>::RemoveSetNode(SNode* _pNode)
    {
        assert(_pNode != nullptr);
        
        _pNode->m_Link.m_pPrevious->m_pNext = _pNode->m_Link.m_pNext;
        _pNode->m_Link.m_pNext->m_pPrevious = _pNode->m_Link.m_pPrevious;
        
        -- m_NumberOfSetNodes;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline void CPool<T, TNumberOfItemsPerPage>::PushFreeNode(SNode* _pNode)
    {
        assert(_pNode != nullptr);
        
        _pNode->m_Link.m_pNext = m_pFirstFreeNode;
        
        m_pFirstFreeNode = &_pNode->m_Link;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::SNode* CPool<T, TNumberOfItemsPerPage>::PopFreeNode()
    {
        SNode* pNode;
        
        pNode = GetNode(m_pFirstFreeNode);
        
        m_pFirstFreeNode = m_pFirstFreeNode->m_pNext;
        
        return pNode;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline bool CPool<T, TNumberOfItemsPerPage>::HasFreeNode() const
    {
        return m_pFirstFreeNode != nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::SNode* CPool<T, TNumberOfItemsPerPage>::GetNode(XPtr _pItem)
    {
        assert(_pItem != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pItem) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Item))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline const typename CPool<T, TNumberOfItemsPerPage>::SNode* CPool<T, TNumberOfItemsPerPage>::GetNode(XConstPtr _pItem)
    {
        assert(_pItem != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pItem) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Item))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::SNode* CPool<T, TNumberOfItemsPerPage>::GetNode(SLink* _pLink)
    {
        assert(_pLink != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pLink) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Link))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline const typename CPool<T, TNumberOfItemsPerPage>::SNode* CPool<T, TNumberOfItemsPerPage>::GetNode(const SLink* _pLink)
    {
        assert(_pLink != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pLink) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Link))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator CPool<T, TNumberOfItemsPerPage>::Begin()
    {
        return CIterator(m_EndOfSetNodes.m_pNext);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator CPool<T, TNumberOfItemsPerPage>::Begin() const
    {
        return CConstIterator(m_EndOfSetNodes.m_pNext);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator CPool<T, TNumberOfItemsPerPage>::Last()
    {
        return CIterator(m_EndOfSetNodes.m_pPrevious);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator CPool<T, TNumberOfItemsPerPage>::Last() const
    {
        return CConstIterator(m_EndOfSetNodes.m_pPrevious);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator CPool<T, TNumberOfItemsPerPage>::End()
    {
        return CIterator(&m_EndOfSetNodes);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator CPool<T, TNumberOfItemsPerPage>::End() const
    {
        return CConstIterator(const_cast<SLink*>(&m_EndOfSetNodes));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::BID CPool<T, TNumberOfItemsPerPage>::GetID(XConstRef _rItem) const
    {
        const SNode* pNode;
        
        pNode = GetNode(&_rItem);
        
        return pNode->m_ID;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::XRef CPool<T, TNumberOfItemsPerPage>::GetItem(BID _ID)
    {
        BSize IndexOfPage;
        BSize IndexOfItem;
        
        IndexOfPage = _ID / TNumberOfItemsPerPage;
        IndexOfItem = _ID % TNumberOfItemsPerPage;
        
        assert((m_ppFirstPage != nullptr) && (IndexOfPage < static_cast<BSize>(m_ppLastPage - m_ppFirstPage)));
        
        return m_ppFirstPage[IndexOfPage][IndexOfItem].m_Item;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::XConstRef CPool<T, TNumberOfItemsPerPage>::GetItem(BID _ID) const
    {
        BSize IndexOfPage;
        BSize IndexOfItem;
        
        IndexOfPage = _ID / TNumberOfItemsPerPage;
        IndexOfItem = _ID % TNumberOfItemsPerPage;
        
        assert((m_ppFirstPage != nullptr) && (IndexOfPage < static_cast<BSize>(m_ppLastPage - m_ppFirstPage)));
        
        return m_ppFirstPage[IndexOfPage][IndexOfItem].m_Item;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::BSize CPool<T, TNumberOfItemsPerPage>::GetNumberOfItems() const
    {
        return m_NumberOfSetNodes;
    }
} // namespace CON

namespace CON
{
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::CConstIterator::CConstIterator()
        : m_pLink(nullptr)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::CConstIterator::CConstIterator(const CThis& _rIterator)
        : m_pLink(_rIterator.m_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::CConstIterator::CConstIterator(SLink* _pLink)
        : m_pLink(_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator::CThis& CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator = (const CThis& _rIterator)
    {
        m_pLink = _rIterator.m_pLink;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator::XRef CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator * () const
    {
        SNode* pNode;
        
        assert(m_pLink != nullptr);
        
        pNode = CContainer::GetNode(m_pLink);
        
        return pNode->m_Item;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator::XPtr CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator -> () const
    {
        return &(*(*this));
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator::CThis& CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator ++ ()
    {
        assert(m_pLink != nullptr);
        
        m_pLink = m_pLink->m_pNext;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator::CThis CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator ++ (int)
    {
        CThis Iterator = *this;
        
        ++ (*this);
        
        return Iterator;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator::CThis& CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator -- ()
    {
        assert(m_pLink != nullptr);
        
        m_pLink = m_pLink->m_pPrevious;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CConstIterator::CThis CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator -- (int)
    {
        CThis Iterator = *this;
        
        -- (*this);
        
        return Iterator;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline bool CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator == (const CThis& _rIterator) const
    {
        return m_pLink == _rIterator.m_pLink;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline bool CPool<T, TNumberOfItemsPerPage>::CConstIterator::operator != (const CThis& _rIterator) const
    {
        return m_pLink != _rIterator.m_pLink;
    }
} // namespace CON

namespace CON
{
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::CIterator::CIterator()
        : CBase()
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::CIterator::CIterator(const CThis& _rIterator)
        : CBase(_rIterator.m_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline CPool<T, TNumberOfItemsPerPage>::CIterator::CIterator(SLink* _pLink)
        : CBase(_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator::CThis& CPool<T, TNumberOfItemsPerPage>::CIterator::operator = (const CThis& _rIterator)
    {
        this->m_pLink = _rIterator.m_pLink;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator::XRef CPool<T, TNumberOfItemsPerPage>::CIterator::operator * () const
    {
        SNode* pNode;
        
        assert(this->m_pLink != nullptr);
        
        pNode = CContainer::GetNode(this->m_pLink);
        
        return pNode->m_Item;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator::XPtr CPool<T, TNumberOfItemsPerPage>::CIterator::operator -> () const
    {
        return &(*(*this));
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator::CThis& CPool<T, TNumberOfItemsPerPage>::CIterator::operator ++ ()
    {
        ++ (*static_cast<CBase*>(this));
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator::CThis CPool<T, TNumberOfItemsPerPage>::CIterator::operator ++ (int)
    {
        CThis Iterator = *this;
        
        ++ (*this);
        
        return Iterator;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator::CThis& CPool<T, TNumberOfItemsPerPage>::CIterator::operator -- ()
    {
        -- (*static_cast<CBase*>(this));
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, Size TNumberOfItemsPerPage>
    inline typename CPool<T, TNumberOfItemsPerPage>::CIterator::CThis CPool<T, TNumberOfItemsPerPage>::CIterator::operator -- (int)
    {
        CThis Iterator = *this;
        
        -- (*this);
        
        return Iterator;
    }
} // namespace CON
