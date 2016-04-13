
#include "graphic/gfx_input_layout.h"

#include <assert.h>
#include <memory.h>

namespace Gfx
{
    CInputLayout::CInputLayout()
        : m_NumberOfElements(0)
    {
        memset(&m_Elements, 0, sizeof(m_Elements));
    }

    // -----------------------------------------------------------------------------

    CInputLayout::~CInputLayout()
    {
    }

    // -----------------------------------------------------------------------------

    bool CInputLayout::operator == (const CInputLayout& _rInputLayoutBase) const
    {
        unsigned int IndexOfElement;

        if (m_NumberOfElements != _rInputLayoutBase.m_NumberOfElements)
        {
            return false;
        }

        for (IndexOfElement = 0; IndexOfElement < m_NumberOfElements; ++ IndexOfElement)
        {
            if (m_Elements[IndexOfElement] != _rInputLayoutBase.m_Elements[IndexOfElement])
            {
                return false;
            }
        }

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CInputLayout::operator != (const CInputLayout& _rInputLayoutBase) const
    {
        unsigned int IndexOfElement;

        if (m_NumberOfElements != _rInputLayoutBase.m_NumberOfElements)
        {
            return true;
        }

        for (IndexOfElement = 0; IndexOfElement < m_NumberOfElements; ++ IndexOfElement)
        {
            if (m_Elements[IndexOfElement] != _rInputLayoutBase.m_Elements[IndexOfElement])
            {
                return true;
            }
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    const CInputLayout::CElement& CInputLayout::GetElement(unsigned int _Index) const
    {
        assert(_Index < m_NumberOfElements);

        return m_Elements[_Index];
    }

    // -----------------------------------------------------------------------------

    unsigned int CInputLayout::GetNumberOfElements() const
    {
        return m_NumberOfElements;
    }

    // -----------------------------------------------------------------------------

    void CInputLayout::SetSemanticName(CElement& _rElement, const char* _pName)
    {
        _rElement.m_SemanticName = _pName;
    }

    // -----------------------------------------------------------------------------

    void CInputLayout::SetSemanticIndex(CElement& _rElement, unsigned int _Index)
    {
        _rElement.m_SemanticIndex = _Index;
    }

    // -----------------------------------------------------------------------------

    void CInputLayout::SetFormat(CElement& _rElement, EFormat _Format)
    {
        _rElement.m_Format = _Format;
    }

    // -----------------------------------------------------------------------------

    void CInputLayout::SetInputSlot(CElement& _rElement, unsigned int _Slot)
    {
        _rElement.m_InputSlot = _Slot;
    }

    // -----------------------------------------------------------------------------

    void CInputLayout::SetAlignedByteOffset(CElement& _rElement, unsigned int _Offset)
    {
        _rElement.m_AlignedByteOffset = _Offset;
    }
    
    // -----------------------------------------------------------------------------
    
    void CInputLayout::SetStride(CElement& _rElement, unsigned int _Stride)
    {
        _rElement.m_Stride= _Stride;
    }

    // -----------------------------------------------------------------------------

    void CInputLayout::SetInputClassification(CElement& _rElement, EInputClassification _Class)
    {
        _rElement.m_InputSlotClass = _Class;
    }

    // -----------------------------------------------------------------------------

    void CInputLayout::SetInstanceDataStepRate(CElement& _rElement, unsigned int _Rate)
    {
        _rElement.m_InstanceDataStepRate = _Rate;
    }
} // namespace Gfx

namespace Gfx
{
    CInputLayout::CElement::CElement()
        : m_SemanticName        ()
        , m_SemanticIndex       (0)
        , m_Format              (UndefinedFormat)
        , m_InputSlot           (0)
        , m_AlignedByteOffset   (0)
        , m_Stride              (0)
        , m_InputSlotClass      (UndefinedInputClassification)
        , m_InstanceDataStepRate(0)
    {
    }

    // -----------------------------------------------------------------------------

    CInputLayout::CElement::~CElement()
    {
    }

    // -----------------------------------------------------------------------------

    bool CInputLayout::CElement::operator == (const CElement& _rElement) const
    {
        if (m_SemanticName         != _rElement.m_SemanticName        ) return false;
        if (m_SemanticIndex        != _rElement.m_SemanticIndex       ) return false;
        if (m_Format               != _rElement.m_Format              ) return false;
        if (m_InputSlot            != _rElement.m_InputSlot           ) return false;
        if (m_AlignedByteOffset    != _rElement.m_AlignedByteOffset   ) return false;
        if (m_Stride               != _rElement.m_Stride              ) return false;
        if (m_InputSlotClass       != _rElement.m_InputSlotClass      ) return false;
        if (m_InstanceDataStepRate != _rElement.m_InstanceDataStepRate) return false;

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CInputLayout::CElement::operator != (const CElement& _rElement) const
    {
        if (m_SemanticName         != _rElement.m_SemanticName        ) return true;
        if (m_SemanticIndex        != _rElement.m_SemanticIndex       ) return true;
        if (m_Format               != _rElement.m_Format              ) return true;
        if (m_InputSlot            != _rElement.m_InputSlot           ) return true;
        if (m_AlignedByteOffset    != _rElement.m_AlignedByteOffset   ) return true;
        if (m_Stride               != _rElement.m_Stride              ) return true;
        if (m_InputSlotClass       != _rElement.m_InputSlotClass      ) return true;
        if (m_InstanceDataStepRate != _rElement.m_InstanceDataStepRate) return true;

        return false;
    }

    // -----------------------------------------------------------------------------

    Base::U32 CInputLayout::CElement::CalcHashValue(Base::U32 _HashValue) const
    {
        /*
        _HashValue = Base::CHsieh::CalcHashValue(_HashValue, reinterpret_cast<const Base::U8*>(&m_SemanticName        ), sizeof(m_SemanticName        ));
        _HashValue = Base::CHsieh::CalcHashValue(_HashValue, reinterpret_cast<const Base::U8*>(&m_SemanticIndex       ), sizeof(m_SemanticIndex       ));
        _HashValue = Base::CHsieh::CalcHashValue(_HashValue, reinterpret_cast<const Base::U8*>(&m_Format              ), sizeof(m_Format              ));
        _HashValue = Base::CHsieh::CalcHashValue(_HashValue, reinterpret_cast<const Base::U8*>(&m_InputSlot           ), sizeof(m_InputSlot           ));
        _HashValue = Base::CHsieh::CalcHashValue(_HashValue, reinterpret_cast<const Base::U8*>(&m_AlignedByteOffset   ), sizeof(m_AlignedByteOffset   ));
        _HashValue = Base::CHsieh::CalcHashValue(_HashValue, reinterpret_cast<const Base::U8*>(&m_InputSlotClass      ), sizeof(m_InputSlotClass      ));
        _HashValue = Base::CHsieh::CalcHashValue(_HashValue, reinterpret_cast<const Base::U8*>(&m_InstanceDataStepRate), sizeof(m_InstanceDataStepRate));
        */

        return _HashValue;
    }

    // -----------------------------------------------------------------------------

    const char* CInputLayout::CElement::GetSemanticName() const
    {
        return m_SemanticName;
    }

    // -----------------------------------------------------------------------------

    unsigned int CInputLayout::CElement::GetSemanticIndex() const
    {
        return m_SemanticIndex;
    }

    // -----------------------------------------------------------------------------

    CInputLayout::EFormat CInputLayout::CElement::GetFormat() const
    {
        return m_Format;
    }

    // -----------------------------------------------------------------------------

    unsigned int CInputLayout::CElement::GetInputSlot() const
    {
        return m_InputSlot;
    }

    // -----------------------------------------------------------------------------

    unsigned int CInputLayout::CElement::GetAlignedByteOffset() const
    {
        return m_AlignedByteOffset;
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CInputLayout::CElement::GetStride() const
    {
        return m_Stride;
    }

    // -----------------------------------------------------------------------------

    CInputLayout::EInputClassification CInputLayout::CElement::GetInputClassification() const
    {
        return m_InputSlotClass;
    }

    // -----------------------------------------------------------------------------

    unsigned int CInputLayout::CElement::GetInstanceDataStepRate() const
    {
        return m_InstanceDataStepRate;
    }
} // namespace Gfx