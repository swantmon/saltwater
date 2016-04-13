//
//  base_memory.h
//  base
//
//  Created by Tobias Schwandt on 26/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <assert.h>
#include <memory>
#include <new>

namespace MEM
{
    class CMemory
    {

    public:
        
        inline static void* Allocate(Size _NumberOfBytes);
        inline static void* Copy(Size _NumberOfBytes, void* _pChunk);
        inline static void Copy(void* _pChunkTo, const void* _pChunkFrom, Size _NumberOfBytes);
        inline static void Free(void* _pChunk);
        inline static void Zero(void* _pChunk, Size _NumberOfBytes);
        
    public:
        
        template<class T>
        inline static T* ConstructObject(void* _pChunk);
        
        template<class T>
        inline static T* ConstructObjectAsCopy(void* _pChunk, const T& _rObject);
        
        template<class T>
        inline static void DestructObject(T* _pObject);
        
    public:
        
        template<class T>
        inline static T* NewObject();
        
        template<class T>
        inline static void DeleteObject(T* _pObject);
    };
} // namespace MEM

namespace MEM
{
    void* CMemory::Allocate(Size _NumberOfBytes)
    {
        return ::malloc(_NumberOfBytes);
    }
    
    // -----------------------------------------------------------------------------
    
    void* CMemory::Copy(Size _NumberOfBytes, void* _pChunk)
    {
        void* pCopy = Allocate(_NumberOfBytes);
        
        ::memcpy(pCopy, _pChunk, _NumberOfBytes);
        
        return pCopy;
    }

    // -----------------------------------------------------------------------------

    void CMemory::Copy(void* _pChunkTo, const void* _pChunkFrom, Size _NumberOfBytes)
    {
        ::memcpy(_pChunkTo, _pChunkFrom, _NumberOfBytes);
    }
    
    // -----------------------------------------------------------------------------
    
    void CMemory::Free(void* _pChunk)
    {
        ::free(_pChunk);
    }
    
    // -----------------------------------------------------------------------------
    
    void CMemory::Zero(void* _pChunk, Size _NumberOfBytes)
    {
        ::memset(_pChunk, 0, _NumberOfBytes);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline T* CMemory::ConstructObject(void* _pChunk)
    {
        assert(_pChunk != 0);
        
        return ::new (_pChunk) T();
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline T* CMemory::ConstructObjectAsCopy(void* _pChunk, const T& _rObject)
    {
        assert(_pChunk != 0);
        
        return ::new (_pChunk) T(_rObject);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline void CMemory::DestructObject(T* _pObject)
    {
        assert(_pObject != 0);
        
        _pObject->~T();
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline T* CMemory::NewObject()
    {
        void* pChunk;
        
        pChunk = static_cast<T*>(Allocate(sizeof(T)));
        
        if (pChunk == 0)
        {
            return 0;
        }
        
        return ConstructObject<T>(pChunk);
    }
    
    // -----------------------------------------------------------------------------
    
    template<class T>
    inline void CMemory::DeleteObject(T* _pObject)
    {
        assert(_pObject != 0);
        
        DestructObject(_pObject);
        
        Free(_pObject);
    }
} // namespace MEM