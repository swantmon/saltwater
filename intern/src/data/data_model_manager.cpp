
#include "data/data_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_math_constants.h"
#include "base/base_math_operations.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"
#include "base/base_pool.h"

#include "data/data_actor_facet.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_manager.h"
#include "data/data_model.h"
#include "data/data_model_manager.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_manager.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <unordered_map>
#include <functional>

using namespace Dt;
using namespace Dt::ModelManager;

namespace
{
	std::string g_PathToAssets	   = "../assets/";
	std::string g_PathToDataModels = "../data/graphic/models/";
} // namespace 

namespace
{
    class CDtModelManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtModelManager)
        
    public:
        
        CDtModelManager();
        ~CDtModelManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();
        
        CModel& CreateModel(const SModelDescriptor& _rDescriptor);

        CModel& CreateBox(float _Width, float _Height, float _Depth);
        CModel& CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
        CModel& CreateCone(float _Radius, float _Height, unsigned int _Slices);
        CModel& CreateRectangle(float _X, float _Y, float _Width, float _Height);
        
        void FreeModel(CModel& _rModel);
        
    private:
        
        class CInternModel : public CModel
        {
        private:
            
            friend class CDtModelManager;
        };
        
    private:
        
        typedef Base::CPool<CInternModel, 1024> CModels;
        
        typedef std::unordered_map<unsigned int, CInternModel*> CModelByIDs;
        typedef CModelByIDs::iterator                           CModelByIDPair;
        
    private:
        
        CModels     m_Models;       
        CModelByIDs m_ModelByID;
        
    private:

        CModel& AllocateModel(const Base::Char* _pModelname);
    };
} // namespace

namespace
{
    CDtModelManager::CDtModelManager()
        : m_Models   ()
        , m_ModelByID()
    {
        m_ModelByID.reserve(64);
    }
    
    // -----------------------------------------------------------------------------
    
    CDtModelManager::~CDtModelManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtModelManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtModelManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtModelManager::Clear()
    {
        // -----------------------------------------------------------------------------
        // Iterate over models and free everyone
        // -----------------------------------------------------------------------------
        CModels::CIterator CurrentModel = m_Models.Begin();
        CModels::CIterator EndOfModels  = m_Models.End();
        
        for (; CurrentModel != EndOfModels;)
        {
            CInternModel& rCurrentModel = *CurrentModel;
            
            CurrentModel = ++CurrentModel;
            
            FreeModel(rCurrentModel);
        }
        
        m_ModelByID.clear();
        
        m_Models  .Clear();
    }

    // -----------------------------------------------------------------------------

    CModel& CDtModelManager::CreateModel(const SModelDescriptor& _rDescriptor)
    {
		Assimp::Importer Importer;
		const aiScene*   pScene;
		std::string      PathToModel;

        // -----------------------------------------------------------------------------
        // Create model
        // -----------------------------------------------------------------------------
        CInternModel& rNewModel = static_cast<CInternModel&>(AllocateModel(_rDescriptor.m_pFileName));

        rNewModel.m_GenFlag = _rDescriptor.m_GenFlag;
       
        // -----------------------------------------------------------------------------
        // Build path to texture in file system and load model
        // -----------------------------------------------------------------------------
		PathToModel = g_PathToAssets + _rDescriptor.m_pFileName;
        
        pScene = Importer.ReadFile(PathToModel.c_str(), 0);

		if (!pScene)
		{
			PathToModel = g_PathToDataModels + _rDescriptor.m_pFileName;

			pScene = Importer.ReadFile(PathToModel.c_str(), 0);
		}
        
        if( !pScene)
        {
            BASE_THROWV("Can't load model file %s; Code: %s", _rDescriptor.m_pFileName, Importer.GetErrorString());
        }
        
        return rNewModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateBox(float _Width, float _Height, float _Depth)
    {        
        CInternModel& rModel = static_cast<CInternModel&>(AllocateModel(""));
        
        return rModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices)
    {
        CInternModel& rModel = static_cast<CInternModel&>(AllocateModel(""));

        return rModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateCone(float _Radius, float _Height, unsigned int _Slices)
    {        
        CInternModel& rModel = static_cast<CInternModel&>(AllocateModel(""));

        return rModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateRectangle(float _X, float _Y, float _Width, float _Height)
    {
        CInternModel& rModel = static_cast<CInternModel&>(AllocateModel(""));

        return rModel;
    }

    // -----------------------------------------------------------------------------

    void CDtModelManager::FreeModel(CModel& _rModel)
    {
        CInternModel& rInternModel = static_cast<CInternModel&>(_rModel);

        m_Models.Free(&rInternModel);
    }

    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::AllocateModel(const Base::Char* _pModelname)
    {
        // -----------------------------------------------------------------------------
        // Check existing model
        // -----------------------------------------------------------------------------
        unsigned int Hash = 0;
        
        if (_pModelname != 0)
        {
            Hash = Base::CRC32(_pModelname, static_cast<unsigned int>(strlen(_pModelname)));
            
            if (m_ModelByID.find(Hash) != m_ModelByID.end())
            {
                return *m_ModelByID.at(Hash);
            }
        }
        
        // -----------------------------------------------------------------------------
        // Create new model
        // -----------------------------------------------------------------------------
        CInternModel& rNewModel = m_Models.Allocate();
        
        rNewModel.m_Modelfile    = _pModelname != 0 ? _pModelname : "";
        
        // -----------------------------------------------------------------------------
        // Add model to hash table
        // -----------------------------------------------------------------------------
        if (Hash != 0)
        {
            m_ModelByID[Hash] = &rNewModel;
        }
        
        return rNewModel;
    }
} // namespace

namespace Dt
{
namespace ModelManager
{
    void OnStart()
    {
        CDtModelManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtModelManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CDtModelManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CModel& CreateModel(const SModelDescriptor& _rDescriptor)
    {
        return CDtModelManager::GetInstance().CreateModel(_rDescriptor);
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CreateBox(float _Width, float _Height, float _Depth)
    {
        return CDtModelManager::GetInstance().CreateBox(_Width, _Height, _Depth);
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices)
    {
        return CDtModelManager::GetInstance().CreateSphere(_Radius, _Stacks, _Slices);
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CreateCone(float _Radius, float _Height, unsigned int _Slices)
    {
        return CDtModelManager::GetInstance().CreateCone(_Radius, _Height, _Slices);
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CreateRectangle(float _X, float _Y, float _Width, float _Height)
    {
        return CDtModelManager::GetInstance().CreateRectangle(_X, _Y, _Width, _Height);
    }
    
    // -----------------------------------------------------------------------------
    
    void FreeModel(CModel& _rModel)
    {
        CDtModelManager::GetInstance().FreeModel(_rModel);
    }
} // namespace ModelManager
} // namespace Dt