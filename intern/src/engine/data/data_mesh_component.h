
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

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

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            Base::Serialize(_rCodec, m_Filename);

            int MeshType;

            _rCodec >> m_GeneratorFlag;
            _rCodec >> m_MeshIndex;
            _rCodec >> MeshType;

            m_MeshType = (EMeshType)MeshType;
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            Base::Serialize(_rCodec, m_Filename);

            _rCodec << m_GeneratorFlag;
            _rCodec << m_MeshIndex;
            _rCodec << (int)m_MeshType;
        }

        inline IComponent* Allocate() override
        {
            return new CMeshComponent();
        }

    private:

        std::string m_Filename;
        int         m_GeneratorFlag;
        int         m_MeshIndex;
        EMeshType   m_MeshType;

    private:

        friend class CMeshComponentGUI;
    };
} // namespace Dt