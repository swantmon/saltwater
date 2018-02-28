
#include "data/data_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_component.h"
#include "data/data_mesh_component.h"
#include "data/data_mesh_helper.h"

using namespace Dt;
using namespace Dt::MeshHelper;

namespace
{
    class CDtMeshManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtMeshManager)
        
    public:
        
        CDtMeshManager();
        ~CDtMeshManager();
        
    public:

        CMeshComponent* CreateMeshFromFile(const std::string& _rFileName, int _GenFlag, int _MeshIndex);
    };
} // namespace

namespace
{
    CDtMeshManager::CDtMeshManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMeshManager::~CDtMeshManager()
    {
    }

    // -----------------------------------------------------------------------------

    CMeshComponent* CDtMeshManager::CreateMeshFromFile(const std::string& _rFileName, int _GenFlag, int _MeshIndex)
    {
        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMeshComponent>();

        pComponent->SetFilename(_rFileName);
        pComponent->SetGeneratorFlag(_GenFlag);
        pComponent->SetMeshType(CMeshComponent::Asset);
        pComponent->SetMeshIndex(_MeshIndex);

        return pComponent;
    }
} // namespace

namespace Dt
{
namespace MeshHelper
{
    CMeshComponent* CreateMeshFromFile(const std::string& _rFileName, int _GenFlag, int _MeshIndex)
    {
        return CDtMeshManager::GetInstance().CreateMeshFromFile(_rFileName, _GenFlag, _MeshIndex);
    }
} // namespace MeshHelper
} // namespace Dt