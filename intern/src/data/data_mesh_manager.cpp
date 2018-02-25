
#include "data/data_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_pool.h"

#include "core/core_asset_manager.h"

#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_manager.h"
#include "data/data_mesh.h"
#include "data/data_mesh_manager.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_manager.h"

#include <unordered_map>
#include <functional>

using namespace Dt;
using namespace Dt::MeshManager;

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
        
        void OnStart();
        void OnExit();

        void Clear();

        CMesh& CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag);
        CMesh& CreatePredefinedMesh(CMesh::EPredefinedMesh _PredefinedMesh);

        void FreeMesh(CMesh& _rModel);
        
    private:
        
        class CInternMesh : public CMesh
        {
        private:
            
            friend class CDtMeshManager;
        };
        
    private:
        
        typedef Base::CPool<CInternMesh, 64  > CMeshes;
        
        typedef std::unordered_map<unsigned int, CInternMesh*> CMeshByIDs;
        typedef CMeshByIDs::iterator                           CMeshByIDPair;
        
    private:
        
        CMeshes    m_Meshes;
        CMeshByIDs m_MeshByID;
    };
} // namespace

namespace
{
    CDtMeshManager::CDtMeshManager()
        : m_Meshes   ()
        , m_MeshByID()
    {
        m_MeshByID.reserve(64);
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMeshManager::~CDtMeshManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtMeshManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtMeshManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtMeshManager::Clear()
    {
        // -----------------------------------------------------------------------------
        // Iterate over models and free everyone
        // -----------------------------------------------------------------------------
        CMeshes::CIterator CurrentModel = m_Meshes.Begin();
        CMeshes::CIterator EndOfModels  = m_Meshes.End();
        
        for (; CurrentModel != EndOfModels;)
        {
            CInternMesh& rCurrentModel = *CurrentModel;
            
            CurrentModel = ++CurrentModel;
            
            FreeMesh(rCurrentModel);
        }
        
        m_MeshByID.clear();
        
        m_Meshes  .Clear();
    }

    // -----------------------------------------------------------------------------

    CMesh& CDtMeshManager::CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag)
    {
         if (_pFileName == 0)
         {
             BASE_THROWM("No filename set loading a mesh from file!")
         }

         unsigned int Hash = Base::CRC32(_pFileName, static_cast<unsigned int>(strlen(_pFileName)));

         if (m_MeshByID.find(Hash) != m_MeshByID.end())
         {
             return *m_MeshByID.at(Hash);
         }

        // -----------------------------------------------------------------------------
        // Create new model
        // -----------------------------------------------------------------------------
        CInternMesh& rNewModel = m_Meshes.Allocate();

        rNewModel.m_Filename       = _pFileName;
        rNewModel.m_GeneratorFlag  = _GenFlag;
        rNewModel.m_PredefinedMesh = CMesh::Nothing;

        // -----------------------------------------------------------------------------
        // Add model to hash table
        // -----------------------------------------------------------------------------
        if (Hash != 0)
        {
            m_MeshByID[Hash] = &rNewModel;
        }       

        return rNewModel;
    }

    // -----------------------------------------------------------------------------

    CMesh& CDtMeshManager::CreatePredefinedMesh(CMesh::EPredefinedMesh _PredefinedMesh)
    {
        CInternMesh& rNewModel = m_Meshes.Allocate();

        rNewModel.m_Filename       = "";
        rNewModel.m_GeneratorFlag  = 0;
        rNewModel.m_PredefinedMesh = _PredefinedMesh;

        return rNewModel;
    }
    
    // -----------------------------------------------------------------------------

    void CDtMeshManager::FreeMesh(CMesh& _rModel)
    {
        m_Meshes.Free(&static_cast<CInternMesh&>(_rModel));
    }
} // namespace

namespace Dt
{
namespace MeshManager
{
    void OnStart()
    {
        CDtMeshManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtMeshManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtMeshManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CMesh& CreateMeshFromFile(const Base::Char* _pFileName, int _GenFlag)
    {
        return CDtMeshManager::GetInstance().CreateMeshFromFile(_pFileName, _GenFlag);
    }

    // -----------------------------------------------------------------------------

    CMesh& CreatePredefinedMesh(CMesh::EPredefinedMesh _PredefinedMesh)
    {
        return CDtMeshManager::GetInstance().CreatePredefinedMesh(_PredefinedMesh);
    }
    
    // -----------------------------------------------------------------------------
    
    void FreeMesh(CMesh& _rMesh)
    {
        CDtMeshManager::GetInstance().FreeMesh(_rMesh);
    }
} // namespace MeshManager
} // namespace Dt