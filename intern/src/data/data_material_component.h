
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

#include "data/data_component.h"

#include <string>

namespace Dt
{
    class CMaterial;
} // namespace Dt

namespace Dt
{
    class ENGINE_API CMaterialComponent : public CComponent<CMaterialComponent>
    {
    public:

        void SetMaterial(CMaterial* _pValue);
        CMaterial* GetMaterial() const;

    public:

        CMaterialComponent();
        ~CMaterialComponent();

    private:

        CMaterial* m_pMaterial;
    };
} // namespace Dt