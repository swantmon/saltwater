
#pragma once

#include "base/base_type_info.h"

#include "engine/data/data_script_component.h"

namespace Scpt
{
    template<class T>
    class CScript : public Dt::CScriptComponent
    {
    public:

        const Base::CTypeInfo::BInfo GetScriptTypeInfo() const override;

    public:

        Dt::CEntity* GetEntity();
        Dt::CTransformationFacet* GetTransformation();
    };
} // namespace Scpt

namespace Scpt
{
    template<class T>
    const Base::CTypeInfo::BInfo CScript<T>::GetScriptTypeInfo() const
    {
        return Base::CTypeInfo::Get<T>();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    Dt::CEntity* CScript<T>::GetEntity()
    {
        return m_pHostEntity;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    Dt::CTransformationFacet* CScript<T>::GetTransformation()
    {
        if (m_pHostEntity == nullptr) return nullptr;

        return m_pHostEntity->GetTransformationFacet();
    }
} // namespace Scpt