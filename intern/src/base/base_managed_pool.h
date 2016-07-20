//
//  base_managed_pool.h
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

// -----------------------------------------------------------------------------
// Forword declaration
// -----------------------------------------------------------------------------
namespace CON
{
    template<class T>
    class CManagedPoolItemPtr;
    
    template<class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    class CManagedPool;
} // namespace CON

// -----------------------------------------------------------------------------
// Base item inside the managed pool with reference counting
// -----------------------------------------------------------------------------
namespace CON
{
    class CManagedPoolItemBase
    {
    protected:
        
        inline CManagedPoolItemBase();
        inline ~CManagedPoolItemBase();
        
    protected:
        
        inline static int AddRef(CManagedPoolItemBase& _rItem);
        inline static int Release(CManagedPoolItemBase& _rItem);
        inline static int GetNumberOfReferences(const CManagedPoolItemBase& _rItem);
        
    private:
        typedef void (*FRelease) (CManagedPoolItemBase&);
        
    private:
        int      m_NumberOfReferences;
        FRelease m_ReleaseFtr;
        
    private:
        template<class T>
        friend class CManagedPoolItemPtr;
        
        template<class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
        friend class CManagedPool;
    };
} // namespace CON

// -----------------------------------------------------------------------------
// Item inside the managed pool as a pointer.
// -----------------------------------------------------------------------------
namespace CON
{
    template<class T>
    class CManagedPoolItemPtr
    {
    public:
        
        typedef CManagedPoolItemPtr<T> CThis;
        typedef T                      X;
        typedef T*                     XPtr;
        typedef const T*               XConstPtr;
        typedef T&                     XRef;
        typedef const T&               XConstRef;
        typedef unsigned int           BSize;
        typedef unsigned int           BID;
        
    public:
        
        inline CManagedPoolItemPtr();
        inline CManagedPoolItemPtr(const CThis& _rOther);
        inline CManagedPoolItemPtr(XPtr _pItem);
        inline ~CManagedPoolItemPtr();
        
    public:
        
        inline CThis& operator = (XPtr _pItem);
        inline CThis& operator = (const CThis& _rOther);
        
    public:
        
        inline XRef operator * ();
        inline XConstRef operator * () const;
        
        inline XPtr operator -> ();
        inline XConstPtr operator -> () const;
        
    public:
        
        inline X** operator & ();
        
        inline bool operator ! () const;
        
    public:
        
        inline bool operator == (XConstPtr _pItem) const;
        inline bool operator == (const CThis& _rOther) const;
        
        inline bool operator != (XConstPtr _pItem) const;
        inline bool operator != (const CThis& _rOther) const;
        
        inline bool operator <  (const CThis& _rOther) const;
        inline bool operator <= (const CThis& _rOther) const;
        inline bool operator >  (const CThis& _rOther) const;
        inline bool operator >= (const CThis& _rOther) const;
        
    public:
        
        inline operator XPtr ();
        inline operator void* ();
        
    public:
        
        inline XPtr GetPtr();
        inline XConstPtr GetPtr() const;
        
    public:
        
        inline bool IsValid() const;
        
    private:
        
        inline static int AddRef(XPtr _pItem);
        inline static int Release(XPtr _pItem);
        
    private:
        
        XPtr m_pItem;
    };
} // namespace CON

// -----------------------------------------------------------------------------
// That is the managed pool with different items of template types.
// -----------------------------------------------------------------------------
namespace CON
{
    template <class T, unsigned int TNumberOfItemsPerPage = 64, unsigned int TDataPolicy = 0>
    class CManagedPool
    {
    public:
        
        typedef CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy> CThis;
        typedef CManagedPoolItemPtr<T>                 CPtr;
        typedef typename CPtr::X                       X;
        typedef typename CPtr::XPtr                    XPtr;
        typedef typename CPtr::XConstPtr               XConstPtr;
        typedef typename CPtr::XRef                    XRef;
        typedef typename CPtr::XConstRef               XConstRef;
        typedef typename CPtr::BSize                   BSize;
        typedef typename CPtr::BID                     BID;
        typedef unsigned int                           BDifference;
        
    public:
        
        static const BSize s_NumberOfItemsPerPage = TNumberOfItemsPerPage;
        
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
            typedef CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>  CContainer;
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
            
            friend class CManagedPool;
        };
        
        class CIterator : public CConstIterator
        {
        public:
            
            typedef CIterator                        CThis;
            typedef CConstIterator                   CBase;
            typedef CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>  CContainer;
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
            
            friend class CManagedPool;
        };
        
    public:
        
        inline CManagedPool();
        inline ~CManagedPool();
        
    public:
        
        inline void Clear();
        
    public:
        
        inline CPtr Allocate();
        
    public:
        
        inline CIterator GetIterator(XRef _rItem);
        inline CConstIterator GetIterator(XConstRef _rItem) const;
        
        inline CIterator Begin();
        inline CConstIterator Begin() const;
        
        inline CIterator Last();
        inline CConstIterator Last() const;
        
        inline CIterator End();
        inline CConstIterator End() const;
        
    public:
        
        inline BID GetID(XConstRef _rItem) const;
        
        inline CPtr GetItem(BID _ID);
        
    public:
        
        inline BSize GetNumberOfItems() const;
        
    private:
        
        struct SNode
        {
            X      m_Item;
            SLink  m_Link;
            BID    m_IndexInPage;
        };
        
    private:
        
        struct SPageHead
        {
            CThis* m_pOwner;
            BID    m_BaseID;
        };
        
        struct SPage
        {
            SPageHead m_Head;
            SNode     m_Nodes[s_NumberOfItemsPerPage];
        };
        
    private:
        
        BSize   m_NumberOfSetNodes;
        SPage** m_ppFirstPage;
        SPage** m_ppLastPage;
        SPage** m_ppEndPage;
        SLink*  m_pFirstFreeNode;
        SLink   m_EndOfSetNodes;
        
    private:
        
        inline static CThis* GetOwner(SNode& _rNode);
        inline static const CThis* GetOwner(const SNode& _rNode);
        
    private:
        
        inline static void FreeItem(CManagedPoolItemBase& _rItemBase);
        
    private:
        
        inline static SNode* GetNode(XPtr _pItem);
        inline static const SNode* GetNode(XConstPtr _pItem);
        
        inline static SNode* GetNode(SLink* _pLink);
        inline static const SNode* GetNode(const SLink* _pLink);
        
    private:
        
        inline void AddSetNode(SNode* _pNode);
        inline void RemoveSetNode(SNode* _pNode);
        inline bool HasSetNode() const;
        
        inline void PushFreeNode(SNode* _pNode);
        inline SNode* PopFreeNode();
        inline bool HasFreeNode() const;
    };
} // namespace CON

namespace CON
{
    inline CManagedPoolItemBase::CManagedPoolItemBase()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline CManagedPoolItemBase::~CManagedPoolItemBase()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline int CManagedPoolItemBase::AddRef(CManagedPoolItemBase& _rItem)
    {
        ++ _rItem.m_NumberOfReferences;
        
        return _rItem.m_NumberOfReferences;
    }
    
    // -----------------------------------------------------------------------------
    
    inline int CManagedPoolItemBase::Release(CManagedPoolItemBase& _rItem)
    {
        assert (_rItem.m_NumberOfReferences > 0);
        
        -- _rItem.m_NumberOfReferences;
        
        if (_rItem.m_NumberOfReferences == 0)
        {
            const FRelease ReleaseFtr = _rItem.m_ReleaseFtr;
            
            assert(ReleaseFtr != nullptr);
            
            ReleaseFtr(_rItem);
        }
        
        return _rItem.m_NumberOfReferences;
    }
    
    // -----------------------------------------------------------------------------
    
    inline int CManagedPoolItemBase::GetNumberOfReferences(const CManagedPoolItemBase& _rItem)
    {
        return _rItem.m_NumberOfReferences;
    }
} // namespace CON

namespace CON
{
    template<class T>
    inline CManagedPoolItemPtr<T>::CManagedPoolItemPtr()
        : m_pItem(nullptr)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline CManagedPoolItemPtr<T>::CManagedPoolItemPtr(const CThis& _rOther)
        : m_pItem(_rOther.m_pItem)
    {
        AddRef(m_pItem);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline CManagedPoolItemPtr<T>::CManagedPoolItemPtr(XPtr _pItem)
        : m_pItem(_pItem)
    {
        AddRef(m_pItem);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline CManagedPoolItemPtr<T>::~CManagedPoolItemPtr()
    {
        Release(m_pItem);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::CThis& CManagedPoolItemPtr<T>::operator = (XPtr _pItem)
    {
        XPtr pItem;
        
        pItem = m_pItem; m_pItem = _pItem;
        
        AddRef(m_pItem); Release(pItem);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::CThis& CManagedPoolItemPtr<T>::operator = (const CThis& _rOther)
    {
        XPtr pItem;
        
        pItem = m_pItem; m_pItem = _rOther.m_pItem;
        
        AddRef(m_pItem); Release(pItem);
        
        return *this;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::XRef CManagedPoolItemPtr<T>::operator * ()
    {
        return *m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::XConstRef CManagedPoolItemPtr<T>::operator * () const
    {
        return *m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::XPtr CManagedPoolItemPtr<T>::operator -> ()
    {
        return m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::XConstPtr CManagedPoolItemPtr<T>::operator -> () const
    {
        return m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::X** CManagedPoolItemPtr<T>::operator & ()
    {
        return &m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator ! () const
    {
        return m_pItem == nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator == (XConstPtr _pItem) const
    {
        return m_pItem == _pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator == (const CThis& _rOther) const
    {
        return m_pItem == _rOther.m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator != (XConstPtr _pItem) const
    {
        return m_pItem != _pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator != (const CThis& _rOther) const
    {
        return m_pItem != _rOther.m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator <  (const CThis& _rOther) const
    {
        return m_pItem < _rOther.m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator <= (const CThis& _rOther) const
    {
        return m_pItem <= _rOther.m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator >  (const CThis& _rOther) const
    {
        return m_pItem > _rOther.m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::operator >= (const CThis& _rOther) const
    {
        return m_pItem >= _rOther.m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline CManagedPoolItemPtr<T>::operator XPtr ()
    {
        return m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline CManagedPoolItemPtr<T>::operator void* ()
    {
        return m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::XPtr CManagedPoolItemPtr<T>::GetPtr()
    {
        return m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline typename CManagedPoolItemPtr<T>::XConstPtr CManagedPoolItemPtr<T>::GetPtr() const
    {
        return m_pItem;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline bool CManagedPoolItemPtr<T>::IsValid() const
    {
        return m_pItem != nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline int CManagedPoolItemPtr<T>::AddRef(XPtr _pItem)
    {
        if (_pItem == nullptr)
        {
            return 0;
        }
        
        return CManagedPoolItemBase::AddRef(*_pItem);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline int CManagedPoolItemPtr<T>::Release(XPtr _pItem)
    {
        if (_pItem == nullptr)
        {
            return 0;
        }
        
        return CManagedPoolItemBase::Release(*_pItem);
    }
} // namespace CON

namespace CON
{
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CManagedPool()
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
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::~CManagedPool()
    {
        Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline void CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::Clear()
    {
        SLink*  pLink;
        SPage** ppPage;

        // -----------------------------------------------------------------------------
        // If we are holder of this data we have to remove the object for ourself.
        // Otherwise the decrement of the reference will execute the destructor.
        // -----------------------------------------------------------------------------
#pragma warning(disable:4127)       
        if (TDataPolicy == 1)
        {
            for (pLink = m_EndOfSetNodes.m_pNext; pLink != &m_EndOfSetNodes; )
            {
                SNode* pNode = GetNode(pLink);
                
                pLink = pLink->m_pNext;
                
                assert(static_cast<CManagedPoolItemBase&>(pNode->m_Item).m_NumberOfReferences == 1);
                
                Base::CMemory::DestructObject(&pNode->m_Item);
            }
        }
#pragma warning(default:4127)
    
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
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CPtr CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::Allocate()
    {
        BSize   OldNumberOfPages;
        BSize   NewNumberOfPages;
        SNode*  pNodes;
        XPtr    pItem;
        SPage*  pPage;
        SPage** ppFirstPage;
        
        if (HasFreeNode())
        {
            SNode& rNode = *GetNode(m_pFirstFreeNode);
            
            pItem = new (&rNode.m_Item) X;
            
            PopFreeNode();
            
            AddSetNode(&rNode);
        }
        else
        {
            // -----------------------------------------------------------------------------
            // There is not a free node, so we have to allocate a new page.
            // -----------------------------------------------------------------------------
            ppFirstPage = nullptr;
            
            pPage = static_cast<SPage*>(Base::CMemory::Allocate(sizeof(SPage)));
            
            if (pPage == nullptr)
            {
                throw std::bad_alloc();
            }
            
            pNodes = pPage->m_Nodes;
            pItem  = nullptr;
            
            try
            {
                // -----------------------------------------------------------------------------
                // Construct the new item in the first node of the new page.
                // -----------------------------------------------------------------------------
                pItem = new (&pNodes[0].m_Item) X;
                
                // -----------------------------------------------------------------------------
                // Check if we have to resize the array with the page addresses.
                // -----------------------------------------------------------------------------
                if (m_ppLastPage == m_ppEndPage)
                {
                    OldNumberOfPages = static_cast<BSize>(m_ppLastPage - m_ppFirstPage);
                    NewNumberOfPages = (OldNumberOfPages != 0) ? OldNumberOfPages + (OldNumberOfPages >> 1) : 8;
                    
                    ppFirstPage = static_cast<SPage**>(Base::CMemory::Allocate(NewNumberOfPages * sizeof(SPage)));
                    
                    if (ppFirstPage == nullptr)
                    {
                        throw std::bad_alloc();
                    }
                    
                    // -----------------------------------------------------------------------------
                    // No exception from now on, so free the old page array.
                    // -----------------------------------------------------------------------------
                    if (m_ppFirstPage != nullptr)
                    {
                        ::memcpy(ppFirstPage, m_ppFirstPage, OldNumberOfPages * sizeof(*m_ppFirstPage));
                        
                        Base::CMemory::Free(m_ppFirstPage);
                    }
                    
                    // -----------------------------------------------------------------------------
                    // Set the members.
                    // -----------------------------------------------------------------------------
                    m_ppFirstPage = ppFirstPage;
                    m_ppLastPage  = ppFirstPage + OldNumberOfPages;
                    m_ppEndPage   = ppFirstPage + NewNumberOfPages;
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
            // Setup page with header informations
            // -----------------------------------------------------------------------------
            pPage->m_Head.m_pOwner = this;
            pPage->m_Head.m_BaseID = static_cast<BID>(m_NumberOfSetNodes);
            
            // -----------------------------------------------------------------------------
            // Register the new page in the page array.
            // -----------------------------------------------------------------------------
            *m_ppLastPage = pPage;
            
            ++ m_ppLastPage;
            
            // -----------------------------------------------------------------------------
            // Add the node to the list containing all allocated nodes.
            // -----------------------------------------------------------------------------
            SNode& rNode = pNodes[0];
            
            rNode.m_IndexInPage = 0;
            
            AddSetNode(&rNode);
            
            // -----------------------------------------------------------------------------
            // Initialize all other nodes of the new page and add them to the free list.
            // -----------------------------------------------------------------------------
            for (unsigned int IndexOfNode = s_NumberOfItemsPerPage - 1; IndexOfNode > 0 ; -- IndexOfNode)
            {
                SNode& rCurrentNode = pNodes[IndexOfNode];
                
                rCurrentNode.m_IndexInPage = static_cast<BID>(IndexOfNode);
                
                PushFreeNode(&rCurrentNode);
            }
        }
        
        // -----------------------------------------------------------------------------
        // Setup the base of the new item
        // -----------------------------------------------------------------------------
        CManagedPoolItemBase& rItemBase = static_cast<CManagedPoolItemBase&>(*pItem);
        
        rItemBase.m_NumberOfReferences = TDataPolicy;
        rItemBase.m_ReleaseFtr         = &CThis::FreeItem;
        
        return CPtr(pItem);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline void CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::AddSetNode(SNode* _pNode)
    {
        assert(_pNode != nullptr);
        
        _pNode->m_Link.m_pNext     = &m_EndOfSetNodes;
        _pNode->m_Link.m_pPrevious =  m_EndOfSetNodes.m_pPrevious;
        
        m_EndOfSetNodes.m_pPrevious->m_pNext = &_pNode->m_Link;
        m_EndOfSetNodes.m_pPrevious          = &_pNode->m_Link;
        
        ++ m_NumberOfSetNodes;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline void CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::RemoveSetNode(SNode* _pNode)
    {
        assert(_pNode != nullptr);
        
        _pNode->m_Link.m_pPrevious->m_pNext = _pNode->m_Link.m_pNext;
        _pNode->m_Link.m_pNext->m_pPrevious = _pNode->m_Link.m_pPrevious;
        
        -- m_NumberOfSetNodes;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline bool CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::HasSetNode() const
    {
        return m_NumberOfSetNodes != 0;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline void CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::PushFreeNode(SNode* _pNode)
    {
        assert(_pNode != nullptr);
        
        _pNode->m_Link.m_pNext = m_pFirstFreeNode;
        
        m_pFirstFreeNode = &_pNode->m_Link;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::SNode* CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::PopFreeNode()
    {
        SNode* pNode;
        
        pNode = GetNode(m_pFirstFreeNode);
        
        m_pFirstFreeNode = m_pFirstFreeNode->m_pNext;
        
        return pNode;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline bool CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::HasFreeNode() const
    {
        return m_pFirstFreeNode != nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CThis* CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetOwner(SNode& _rNode)
    {
        SPageHead* pPageHead;
        
        Size HeadMemberOffset      = (Size) &reinterpret_cast<const volatile Base::U8&>((((SPage*) nullptr)->SPage::m_Head));
        Size NodesMemberOffset     = (Size) &reinterpret_cast<const volatile Base::U8&>((((SPage*) nullptr)->SPage::m_Nodes));
        Size NumberOfPageHeadBytes = NodesMemberOffset - HeadMemberOffset;
        Size NodePosition          = (Size) &_rNode;
        Size SizeOfNode            = (Size) sizeof(SNode);
        
        pPageHead = reinterpret_cast<SPageHead*>(NodePosition - (NumberOfPageHeadBytes + _rNode.m_IndexInPage * SizeOfNode));
        
        return pPageHead->m_pOwner;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline const typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CThis* CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetOwner(const SNode& _rNode)
    {
        SPageHead* pPageHead;
        
        Size HeadMemberOffset      = (Size) &reinterpret_cast<const volatile Base::U8&>((((SPage*) nullptr)->SPage::m_Head));
        Size NodesMemberOffset     = (Size) &reinterpret_cast<const volatile Base::U8&>((((SPage*) nullptr)->SPage::m_Nodes));
        Size NumberOfPageHeadBytes = NodesMemberOffset - HeadMemberOffset;
        Size NodePosition          = (Size) &_rNode;
        Size SizeOfNode            = (Size) sizeof(SNode);
        
        pPageHead = reinterpret_cast<SPageHead*>(NodePosition - (NumberOfPageHeadBytes + _rNode.m_IndexInPage * SizeOfNode));
        
        return pPageHead->m_pOwner;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline void CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::FreeItem(Base::CManagedPoolItemBase& _rItemBase)
    {
        CThis* pOwner;
        
        assert(_rItemBase.m_NumberOfReferences == 0);
        
        XRef rItem = static_cast<XRef>(_rItemBase);
        
        SNode& rNode = *GetNode(&rItem);
        
        Base::CMemory::DestructObject(&rNode.m_Item);
        
        pOwner = GetOwner(rNode);
        
        pOwner->RemoveSetNode(&rNode);
        
        pOwner->PushFreeNode(&rNode);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::SNode* CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetNode(XPtr _pItem)
    {
        assert(_pItem != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pItem) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Item))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline const typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::SNode* CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetNode(XConstPtr _pItem)
    {
        assert(_pItem != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pItem) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Item))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::SNode* CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetNode(SLink* _pLink)
    {
        assert(_pLink != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pLink) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Link))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline const typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::SNode* CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetNode(const SLink* _pLink)
    {
        assert(_pLink != nullptr);
        
        return reinterpret_cast<SNode*>(reinterpret_cast<ptrdiff_t>(_pLink) - (reinterpret_cast<ptrdiff_t>(&(static_cast<SNode*>(nullptr)->*(&SNode::m_Link))) - reinterpret_cast<ptrdiff_t>(static_cast<void*>(nullptr))));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::Begin()
    {
        return CIterator(m_EndOfSetNodes.m_pNext);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::Begin() const
    {
        return CConstIterator(m_EndOfSetNodes.m_pNext);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::Last()
    {
        return CIterator(m_EndOfSetNodes.m_pPrevious);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::Last() const
    {
        return CConstIterator(m_EndOfSetNodes.m_pPrevious);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::End()
    {
        return CIterator(&m_EndOfSetNodes);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::End() const
    {
        return CConstIterator(const_cast<SLink*>(&m_EndOfSetNodes));
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::BID CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetID(XConstRef _rItem) const
    {
        const SNode* pNode;
        
        pNode = GetNode(&_rItem);
        
        return pNode->m_ID;
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CPtr CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetItem(BID _ID)
    {
        BSize IndexOfPage;
        BSize IndexOfItem;
        
        IndexOfPage = _ID / TNumberOfItemsPerPage;
        IndexOfItem = _ID % TNumberOfItemsPerPage;
        
        assert((m_ppFirstPage != nullptr) && (IndexOfPage < static_cast<BSize>(m_ppLastPage - m_ppFirstPage)));
        
        return CPtr(m_ppFirstPage[IndexOfPage][IndexOfItem].m_Item);
    }
    
    // -----------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::BSize CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::GetNumberOfItems() const
    {
        return m_NumberOfSetNodes;
    }
} // namespace CON

namespace CON
{
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CConstIterator()
        : m_pLink(nullptr)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CConstIterator(const CThis& _rIterator)
        : m_pLink(_rIterator.m_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CConstIterator(SLink* _pLink)
        : m_pLink(_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CThis& CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator = (const CThis& _rIterator)
    {
        m_pLink = _rIterator.m_pLink;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::XRef CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator * () const
    {
        SNode* pNode;
        
        assert(m_pLink != nullptr);
        
        pNode = CContainer::GetNode(m_pLink);
        
        return pNode->m_Item;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::XPtr CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator -> () const
    {
        return &(*(*this));
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CThis& CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator ++ ()
    {
        assert(m_pLink != nullptr);
        
        m_pLink = m_pLink->m_pNext;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CThis CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator ++ (int)
    {
        CThis Iterator = *this;
        
        ++ (*this);
        
        return Iterator;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CThis& CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator -- ()
    {
        assert(m_pLink != nullptr);
        
        m_pLink = m_pLink->m_pPrevious;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::CThis CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator -- (int)
    {
        CThis Iterator = *this;
        
        -- (*this);
        
        return Iterator;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline bool CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator == (const CThis& _rIterator) const
    {
        return m_pLink == _rIterator.m_pLink;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline bool CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CConstIterator::operator != (const CThis& _rIterator) const
    {
        return m_pLink != _rIterator.m_pLink;
    }
} // namespace CON

namespace CON
{
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CIterator()
    : CBase()
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CIterator(const CThis& _rIterator)
    : CBase(_rIterator.m_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CIterator(SLink* _pLink)
    : CBase(_pLink)
    {
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CThis& CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::operator = (const CThis& _rIterator)
    {
        this->m_pLink = _rIterator.m_pLink;
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::XRef CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::operator * () const
    {
        SNode* pNode;
        
        assert(this->m_pLink != nullptr);
        
        pNode = CContainer::GetNode(this->m_pLink);
        
        return pNode->m_Item;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::XPtr CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::operator -> () const
    {
        return &(*(*this));
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CThis& CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::operator ++ ()
    {
        ++ (*static_cast<CBase*>(this));
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CThis CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::operator ++ (int)
    {
        CThis Iterator = *this;
        
        ++ (*this);
        
        return Iterator;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CThis& CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::operator -- ()
    {
        -- (*static_cast<CBase*>(this));
        
        return *this;
    }
    
    // -------------------------------------------------------------------------------
    
    template <class T, unsigned int TNumberOfItemsPerPage, unsigned int TDataPolicy>
    inline typename CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::CThis CManagedPool<T, TNumberOfItemsPerPage, TDataPolicy>::CIterator::operator -- (int)
    {
        CThis Iterator = *this;
        
        -- (*this);
        
        return Iterator;
    }
} // namespace CON
