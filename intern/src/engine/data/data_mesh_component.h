
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"

#include <string>

namespace Dt
{
    class ENGINE_API CMeshComponent : public CComponent<CMeshComponent>
    {
    public:

        enum EMeshType
        {
            Asset,
            Box,
            Sphere,
            IsometricSphere,
            Cone,
            Rectangle,
        };

    public:

        void SetFilename(const std::string& _rValue);
        const std::string& GetFilename() const;

        void SetGeneratorFlag(int _Flag);
        int GetGeneratorFlag() const;

        void SetMeshIndex(const int _Value);
        int GetMeshIndex() const;

        void SetMeshType(EMeshType _Value);
        EMeshType GetMeshType() const;

    public:

        CMeshComponent();
        ~CMeshComponent();

    private:

        std::string m_Filename;
        int         m_GeneratorFlag;
        int         m_MeshIndex;
        EMeshType   m_MeshType;
    };
} // namespace Dt