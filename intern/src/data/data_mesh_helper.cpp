
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

        CMeshComponent* CreateMeshFromFile(const std::string& _rFileName, int _GenFlag);

        CMeshComponent* CreateMeshFromAssimp(const std::string& _rFileName, int _GenFlag, int _MeshIndex, const void* _pImporter);

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

    CMeshComponent* CDtMeshManager::CreateMeshFromFile(const std::string& _rFileName, int _GenFlag)
    {
         unsigned int Hash = 0;

         Hash = Base::CRC32(Hash, _rFileName.c_str(), static_cast<unsigned int>(_rFileName.length()));

         Hash = Base::CRC32(Hash, &_GenFlag, sizeof(_GenFlag));

         int MeshIndex = 0;

         Hash = Base::CRC32(Hash, &MeshIndex, sizeof(MeshIndex));

         if (m_MeshByID.find(Hash) != m_MeshByID.end())
         {
             return m_MeshByID.at(Hash);
         }

        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMeshComponent>();

        pComponent->SetFilename(_rFileName);
        pComponent->SetGeneratorFlag(_GenFlag);
        pComponent->SetMeshType(CMeshComponent::File);
        pComponent->SetMeshIndex(MeshIndex);

        m_MeshByID[Hash] = pComponent;

        return pComponent;
    }

    // -----------------------------------------------------------------------------

    CMeshComponent* CDtMeshManager::CreateMeshFromAssimp(const std::string& _rFileName, int _GenFlag, int _MeshIndex, const void* _pImporter)
    {
        unsigned int Hash = 0;
        
        Hash = Base::CRC32(Hash, _rFileName.c_str(), static_cast<unsigned int>(_rFileName.length()));

        Hash = Base::CRC32(Hash, &_GenFlag, sizeof(_GenFlag));

        Hash = Base::CRC32(Hash, &_MeshIndex, sizeof(_MeshIndex));

        if (m_MeshByID.find(Hash) != m_MeshByID.end())
        {
            return m_MeshByID.at(Hash);
        }

        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMeshComponent>();

        pComponent->SetFilename(_rFileName);
        pComponent->SetGeneratorFlag(_GenFlag);
        pComponent->SetMeshIndex(_MeshIndex);
        pComponent->SetImporter(_pImporter);
        pComponent->SetMeshType(CMeshComponent::File);

        m_MeshByID[Hash] = pComponent;

        return pComponent;
    }
} // namespace

namespace Dt
{
namespace MeshHelper
{
    CMeshComponent* CreateMeshFromFile(const std::string& _rFileName, int _GenFlag)
    {
        return CDtMeshManager::GetInstance().CreateMeshFromFile(_rFileName, _GenFlag);
    }

    // -----------------------------------------------------------------------------

    CMeshComponent* CreateMeshFromAssimp(const std::string& _rFileName, int _GenFlag, int _MeshIndex, const void* _pImporter)
    {
        return CDtMeshManager::GetInstance().CreateMeshFromAssimp(_rFileName, _GenFlag, _MeshIndex, _pImporter);
    }
} // namespace MeshHelper
} // namespace Dt