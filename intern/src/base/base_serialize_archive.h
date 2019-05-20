
////////////////////////////////////////////////////////////////////////////////
///
/// \file base_serialize_archive.h
///
/// \author Tobias Schwandt
/// \author Credits to Joerg Sahm
/// \author Copyright (c) Tobias Schwandt. All rights reserved.
///
/// \date 2012-2013
///
/// \version 1.0
/// 
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base/base_defines.h"
#include "base/base_is_array.h"
#include "base/base_is_class.h"
#include "base/base_is_primitive.h"
#include "base/base_is_pointer.h"
#include "base/base_is_reference.h"
#include "base/base_is_std.h"
#include "base/base_logical_arithmetic.h"
#include "base/base_remove_pointer.h"
#include "base/base_remove_qualifier.h"
#include "base/base_serialize_std_vector.h"
#include "base/base_serialize_std_string.h"

#include <assert.h>

namespace SER
{
    class CArchive
    {
    public:

        typedef CArchive CThis;

    public:
        inline CArchive(unsigned int _Version);

    public:
        template<typename TElement>
        inline CThis& Write(const TElement& _rElement) = 0;

        template<typename TElement>
        inline CThis& operator << (const TElement& _rElement) = 0;

        template<typename TElement>
        inline CThis& Read(TElement& _rElement) = 0;

        template<typename TElement>
        inline CThis& operator >> (TElement& _rElement) = 0;

        template<typename TElement>
        inline CThis& operator & (const TElement& _rElement) = 0;

    public:
        template<typename TElement>
        inline void BeginCollection() = 0;

        template<typename TElement>
        inline void WriteCollection(const TElement* _pElements, unsigned int _NumberOfElements) = 0;

        template<typename TElement>
        inline void EndCollection() = 0;

        template<typename TElement>
        inline void WritePrimitive(const TElement& _rElement) = 0;

        template<typename TElement>
        inline void WriteClass(const TElement& _rElement) = 0;

        template<typename TElement>
        inline void ReadPrimitive(TElement& _rElement) = 0;

        template<typename TElement>
        inline void ReadClass(TElement& _rElement) = 0;

    protected:
        unsigned int m_ArchiveVersion;

    protected:
        template<class TArchive, typename TElement>
        inline static void DispatchWrite(TArchive& _rArchive, const TElement& _rElement)
        {
            using SDispatcher = typename SGetDispatcher<TElement>::SDispatcher;

            SDispatcher::Write(_rArchive, _rElement);
        }

        // -----------------------------------------------------------------------------

        template<class TArchive, typename TElement>
        inline static void DispatchRead(TArchive& _rArchive, TElement& _rElement)
        {
            using SDispatcher = typename SGetDispatcher<TElement>::SDispatcher;

            SDispatcher::Read(_rArchive, _rElement);
        }

    private:
        template<typename TElement>
        struct SPrimitiveDispatcher
        {
            template<class TArchive>
            inline static void Write(TArchive& _rArchive, const TElement& _rElement)
            {
                _rArchive.WritePrimitive(_rElement);
            }

            template<class TArchive>
            inline static void Read(TArchive& _rArchive, TElement& _rElement)
            {
                _rArchive.ReadPrimitive(_rElement);
            }
        };

    private:
        template<typename TElement>
        struct SPrimitivePtrDispatcher;

        template<typename TElement>
        struct SPrimitivePtrDispatcher<TElement*>
        {
            template<class TArchive>
            inline static void Write(TArchive& _rArchive, const TElement* _pElement)
            {
                _rArchive.WritePrimitive(*_pElement);
            }

            template<class TArchive>
            inline static void Read(TArchive& _rArchive, TElement*& _rpElement)
            {
                using X = typename SRemovePointer<TElement*>::X;
               
                X* pValue;

                pValue = static_cast<X*>(malloc(sizeof(X)));

                assert(pValue != 0);

                _rArchive.ReadPrimitive(*pValue);

                _rpElement = pValue;
            }
        };

     private:
        template<typename TElement>
        struct SClassDispatcher
        {
            template<class TArchive>
            inline static void Write(TArchive& _rArchive, const TElement& _rElement)
            {
                _rArchive.WriteClass(_rElement);
            }

            template<class TArchive>
            inline static void Read(TArchive& _rArchive, TElement& _rElement)
            {
                _rArchive.ReadClass(_rElement);
            }
        };

    private:

        template<typename TElement>
        struct SClassPtrDispatcher;

        template<typename TElement>
        struct SClassPtrDispatcher<TElement*>
        {
            template<class TArchive>
            inline static void Write(TArchive& _rArchive, const TElement* _pElement)
            {
                _rArchive.WriteClass(*const_cast<TElement*>(_pElement));
            }

            template<class TArchive>
            inline static void Read(TArchive& _rArchive, TElement*& _rpElement)
            {
                using X = typename SRemovePointer<TElement*>::X;
                
                X* pValue;

                pValue = new X[sizeof(X)];

                assert(pValue != 0);

                _rArchive.ReadClass(*pValue);

                _rpElement = pValue;
            }
        };

    private:
        template<typename TElement>
        struct SArrayDispatcher
        {
            template<class TArchive>
            inline static void Write(TArchive& _rArchive, const TElement& _rElement)
            {
                unsigned int NumberOfElements = sizeof(_rElement) / sizeof(_rElement[0]);

                _rArchive.template BeginCollection<TElement>(NumberOfElements);
                _rArchive.WriteCollection(_rElement);
                _rArchive.template EndCollection<TElement>();
            }

            template<class TArchive>
            inline static void Read(TArchive& _rArchive, TElement& _rElement)
            {
                unsigned int NumberOfElements = sizeof(_rElement) / sizeof(_rElement[0]);

                _rArchive.template BeginCollection<TElement>();
                _rArchive.ReadCollection(_rElement, NumberOfElements);
                _rArchive.template EndCollection<TElement>();
            }
        };

    private:
        template<typename TElement>
        struct SGetDispatcher
        {
        private:
            using XUnqualified = typename SRemoveQualifier<TElement>::X;
            using XUnqualifiedPointless = typename SRemovePointer<XUnqualified>::X;

        private:
            enum 
            {
                IsPrimitive      = SIsPrimitive<XUnqualified>::Value,
                IsPrimitivePtr   = SAnd<SIsPointer<XUnqualified>::Value, SIsPrimitive<XUnqualifiedPointless>::Value>::Value,
                IsArray          = SIsArray<XUnqualified>::Value,
                IsClass          = SIsClass<XUnqualified>::Value,
                IsClassPtr       = SAnd<SIsPointer<XUnqualified>::Value, SIsClass<XUnqualifiedPointless>::Value>::Value,
            };

        public:
            using SDispatcher = typename SIf<IsPrimitive   , SPrimitiveDispatcher<TElement>,
                    typename SIf<IsPrimitivePtr, SPrimitivePtrDispatcher<TElement>,
                    typename SIf<IsArray       , SArrayDispatcher<TElement>,
                    typename SIf<IsClassPtr    , SClassPtrDispatcher<TElement>, SClassDispatcher<TElement>>::X>::X>::X>::X;
        };
    };
} // namespace SER

namespace SER
{
    template<>
    struct CArchive::SPrimitivePtrDispatcher<char*>
    {
        template<class TArchive>
        inline static void Write(TArchive& _rArchive, const char* _pElement)
        {
            assert(_pElement != nullptr);
            
            auto NumberOfCharacters = static_cast<unsigned int>(strlen(_pElement));
            
            _rArchive.template BeginCollection<char>(NumberOfCharacters);
            _rArchive.WriteCollection(_pElement, NumberOfCharacters);
            _rArchive.template EndCollection<char>();
        }
        
        template<class TArchive>
        inline static void Read(TArchive& _rArchive, char*& _rpElement)
        {
            unsigned int NumberOfCharacters;
            char*        pString;
            
            NumberOfCharacters = _rArchive.template BeginCollection<char>();
            
            pString = new char[NumberOfCharacters + 1];
            
            assert(pString != nullptr);
            
            _rArchive.ReadCollection(pString, NumberOfCharacters);
            _rArchive.template EndCollection<char>();
            
            pString[NumberOfCharacters] = '\0';
            
            _rpElement = pString;
        }
    };
    
    // -----------------------------------------------------------------------------
    
    template<>
    struct CArchive::SPrimitivePtrDispatcher<wchar_t*>
    {
        template<class TArchive>
        inline static void Write(TArchive& _rArchive, const wchar_t* _pElement)
        {
            assert(_pElement != nullptr);
            
            auto NumberOfCharacters = static_cast<unsigned int>(wcslen(_pElement));
            
            _rArchive.template BeginCollection<wchar_t>(NumberOfCharacters);
            _rArchive.WriteCollection(_pElement);
            _rArchive.template EndCollection<wchar_t>();
        }
        
        template<class TArchive>
        inline static void Read(TArchive& _rArchive, wchar_t*& _rpElement)
        {
            unsigned int NumberOfCharacters;
            wchar_t*     pString;
            
            NumberOfCharacters = _rArchive.template BeginCollection<wchar_t>();
            
            pString = new wchar_t[NumberOfCharacters + 1];
            
            assert(pString != nullptr);
            
            _rArchive.ReadCollection(pString, NumberOfCharacters);
            _rArchive.template EndCollection<wchar_t>();
            
            pString[NumberOfCharacters] = '\0';
            
            _rpElement = pString;
        }
    };
} // namespace SER

namespace SER
{
    CArchive::CArchive(unsigned int _Version)
        : m_ArchiveVersion(_Version)
    {
    }
} // namespace SER