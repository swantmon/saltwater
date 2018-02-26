
#include "data/data_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_manager.h"

#include "data/data_component.h"
#include "data/data_mesh_component.h"
#include "data/data_mesh_helper.h"

#include <unordered_map>

using namespace Dt;
using namespace Dt::MeshHelper;

namespace
{
    std::string g_PathToDataModels = "/graphic/models/";
} // namespace 

namespace
{
    class CDtMeshManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtMeshManager)
        
    public:
        
        CDtMeshManager();
        ~CDtMeshManager();
        
    public:

        CMeshComponent* CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag);

    private:
        
        typedef std::unordered_map<unsigned int, CMeshComponent*> CMeshByIDs;
        typedef CMeshByIDs::iterator                              CMeshByIDPair;
        
    private:

        CMeshByIDs m_MeshByID;
    };
} // namespace

namespace
{
    CDtMeshManager::CDtMeshManager()
        : m_MeshByID()
    {
        m_MeshByID.reserve(64);
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMeshManager::~CDtMeshManager()
    {
        m_MeshByID.clear();
    }

    // -----------------------------------------------------------------------------

    CMeshComponent* CDtMeshManager::CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag)
    {
         if (_pFileName == 0)
         {
             BASE_THROWM("No filename set loading a mesh from file!")
         }

         unsigned int Hash = Base::CRC32(_pFileName, static_cast<unsigned int>(strlen(_pFileName)));

         if (m_MeshByID.find(Hash) != m_MeshByID.end())
         {
             return m_MeshByID.at(Hash);
         }

        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMeshComponent>();

        pComponent->SetFilename(_pFileName);
        pComponent->SetGeneratorFlag(_GenFlag);
        pComponent->SetPredefinedMesh(CMeshComponent::Nothing);

        if (Hash != 0)
        {
            m_MeshByID[Hash] = pComponent;
        }       

        return pComponent;
    }
} // namespace

namespace Dt
{
namespace MeshHelper
{
    CMeshComponent* CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag)
    {
        return CDtMeshManager::GetInstance().CreateMeshFromFile(_pFileName, _GenFlag);
    }
} // namespace MeshHelper
} // namespace Dt