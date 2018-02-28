
#include "data/data_precompiled.h"

#include "core/core_asset_importer.h"

#include "data/data_mesh_component.h"

#include <assert.h>

namespace Dt
{
    CMeshComponent::CMeshComponent()
        : m_Filename      ()
        , m_GeneratorFlag (Core::AssetImporter::SGeneratorFlag::Nothing)
        , m_MeshIndex     (0)
        , m_pImporter     (nullptr)
        , m_MeshType      (EMeshType::File)
    {
    }

    // -----------------------------------------------------------------------------

    CMeshComponent::~CMeshComponent()
    {
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetFilename(const std::string& _rValue)
    {
        m_Filename = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMeshComponent::GetFilename() const
    {
        return m_Filename;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetGeneratorFlag(int _Flag)
    {
        m_GeneratorFlag = _Flag;
    }

    // -----------------------------------------------------------------------------

    int CMeshComponent::GetGeneratorFlag() const
    {
        return m_GeneratorFlag;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMeshIndex(const int _Value)
    {
        m_MeshIndex = _Value;
    }

    // -----------------------------------------------------------------------------

    int CMeshComponent::GetMeshIndex() const
    {
        return m_MeshIndex;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetImporter(const void* _pImporter)
    {
        m_pImporter = _pImporter;
    }

    // -----------------------------------------------------------------------------

    const void* CMeshComponent::GetImporter() const
    {
        return m_pImporter;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMeshType(EMeshType _Value)
    {
        m_MeshType = _Value;
    }

    // -----------------------------------------------------------------------------

    CMeshComponent::EMeshType CMeshComponent::GetMeshType() const
    {
        return m_MeshType;
    }
} // namespace Dt