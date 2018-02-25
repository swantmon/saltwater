
#include "data/data_precompiled.h"

#include "data/data_mesh_component.h"

#include <assert.h>

namespace Dt
{
    CMeshComponent::CMeshComponent()
        : m_Filename      ()
        , m_GeneratorFlag (SGeneratorFlag::Nothing)
        , m_PredefinedMesh(EPredefinedMesh::Nothing)
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

    void CMeshComponent::SetPredefinedMesh(EPredefinedMesh _Value)
    {
        m_PredefinedMesh = _Value;
    }

    // -----------------------------------------------------------------------------

    CMeshComponent::EPredefinedMesh CMeshComponent::GetPredefinedMesh() const
    {
        return m_PredefinedMesh;
    }
} // namespace Dt