
#pragma once

#include "base/base_managed_pool.h"
#include "base/base_typedef.h"

namespace Gfx
{
    class ENGINE_API CInputLayout : public Base::CManagedPoolItemBase
    {
    public:

        enum EFormat
        {
            SByte1Format,
            SByte2Format,
            SByte4Format,
            UByte1Format,
            UByte2Format,
            UByte4Format,
            SShort1Format,
            SShort2Format,
            SShort4Format,
            UShort1Format,
            UShort2Format,
            UShort4Format,
            SInt1Format,
            SInt2Format,
            SInt3Format,
            SInt4Format,
            UInt1Format,
            UInt2Format,
            UInt3Format,
            UInt4Format,
            Half1Format,
            Half2Format,
            Half4Format,
            Float1Format,
            Float2Format,
            Float3Format,
            Float4Format,
            NumberOfFormats,
            UndefinedFormat = -1,
        };

        enum EInputClassification
        {
            PerVertex,
            PerInstance,
            NumberOfInputClassifications,
            UndefinedInputClassification = -1
        };

        enum EElement
        {
            Position  =  0,
            Normal    =  1,
            Tangent   =  2,
            Color0    =  4,
            Color1    =  8,
            TexCoord0 = 16,
            TexCoord1 = 32,
            Animation = 64,
        };

    public:

        class CElement
        {
        public:

            const char* GetSemanticName() const;
            unsigned int GetSemanticIndex() const;
            EFormat GetFormat() const;
            unsigned int GetInputSlot() const;
            unsigned int GetAlignedByteOffset() const;
            unsigned int GetStride() const;
            EInputClassification GetInputClassification() const;
            unsigned int GetInstanceDataStepRate() const;

        private:

            const char*          m_SemanticName;
            unsigned int         m_SemanticIndex;
            EFormat              m_Format;
            unsigned int         m_InputSlot;
            unsigned int         m_AlignedByteOffset;
            unsigned int         m_Stride;
            EInputClassification m_InputSlotClass;
            unsigned int         m_InstanceDataStepRate;

        private:

            CElement();
           ~CElement();

        private:

            bool operator == (const CElement& _rElement) const;
            bool operator != (const CElement& _rElement) const;

        private:

            Base::U32 CalcHashValue(Base::U32 _HashValue) const;

        private:

            friend class CInputLayout;
        };

    public:

        static const unsigned int s_MaxNumberOfElements = 16;

    public:

        const CElement& GetElement(unsigned int _Index) const;

        unsigned int GetNumberOfElements() const;

    protected:

        unsigned int m_NumberOfElements;
        CElement     m_Elements[s_MaxNumberOfElements];

    protected:

        CInputLayout();
       ~CInputLayout();

    protected:

        bool operator == (const CInputLayout& _rInputLayoutBase) const;
        bool operator != (const CInputLayout& _rInputLayoutBase) const;

    protected:

        Base::U32 CalcHashValue() const;

    protected:

        void SetSemanticName(CElement& _rElement, const char* _pName);
        void SetSemanticIndex(CElement& _rElement, unsigned int _Index);
        void SetFormat(CElement& _rElement, EFormat _Format);
        void SetInputSlot(CElement& _rElement, unsigned int _Slot);
        void SetAlignedByteOffset(CElement& _rElement, unsigned int _Offset);
        void SetStride(CElement& _rElement, unsigned int _Stride);
        void SetInputClassification(CElement& _rElement, EInputClassification _Class);
        void SetInstanceDataStepRate(CElement& _rElement, unsigned int _Rate);
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CInputLayout> CInputLayoutPtr;
} // namespace Gfx