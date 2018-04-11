
#pragma once

#include "base/base_type_info.h"

#include "engine/data/data_script_component.h"

namespace Scpt
{
    template<class T>
    class CScript : public Dt::CScriptComponent
    {
    public:

        static const Base::ID STATIC_SCRIPT_TYPE_ID;

    public:

        const Base::ID GetScriptTypeID() const override;

    public:

        Dt::CEntity* GetEntity();
        Dt::CTransformationFacet* GetTransformation();
    };
} // namespace Scpt

namespace Scpt
{
    template<class T>
    const Base::ID CScript<T>::STATIC_SCRIPT_TYPE_ID = Base::CTypeInfo::GetTypeID<T>();
} // namespace Scpt

namespace Scpt
{
    template<class T>
    const Base::ID CScript<T>::GetScriptTypeID() const
    {
        return STATIC_SCRIPT_TYPE_ID;
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