
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
#include "data/data_lod.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_manager.h"
#include "data/data_model.h"
#include "data/data_model_manager.h"
#include "data/data_surface.h"
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
        
        CModel& CreateModel(const SModelFileDescriptor& _rDescriptor);
        CModel& CreateModel(const SModelAssimpDescriptor& _rDescriptor);

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
        
        class CInternLOD : public CLOD
        {
        private:
            
            friend class CDtModelManager;
        };
        
        class CInternSurface : public CSurface
        {
        private:
            
            friend class CDtModelManager;
        };
        
    private:
        
        typedef Base::CPool<CInternModel  , 64  > CModels;
        typedef Base::CPool<CInternLOD    , 256 > CLODs;
        typedef Base::CPool<CInternSurface, 1024> CSurfaces;
        
        typedef std::unordered_map<unsigned int, CInternModel*> CModelByIDs;
        typedef CModelByIDs::iterator                           CModelByIDPair;
        
    private:
        
        CModels   m_Models;
        CLODs     m_LODs;
        CSurfaces m_Surfaces;
        
        CModelByIDs m_ModelByID;
        
    private:

        CModel& AllocateModel(const Base::Char* _pModelname);
        CLOD& AllocateLOD();
        CSurface& AllocateSurface(unsigned int _NumberOfVertices, unsigned int _NumberOfIndices, unsigned int _Elements);
        unsigned int ConvertGenerationPresets(unsigned int _EngineFlag);
    };
} // namespace

namespace
{
    CDtModelManager::CDtModelManager()
        : m_Models   ()
        , m_LODs     ()
        , m_Surfaces ()
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
        
        m_Surfaces.Clear();
        m_LODs    .Clear();
        m_Models  .Clear();
    }

    // -----------------------------------------------------------------------------

    CModel& CDtModelManager::CreateModel(const SModelFileDescriptor& _rDescriptor)
    {
		Assimp::Importer Importer;
		const aiScene*   pScene;
		std::string      PathToModel;
		unsigned int	 Flags;

        // -----------------------------------------------------------------------------
        // Create model
        // -----------------------------------------------------------------------------
        CInternModel& rNewModel = static_cast<CInternModel&>(AllocateModel(_rDescriptor.m_pFileName));
        
        if (rNewModel.m_NumberOfLODs > 0)
        {
            return rNewModel;
        }

		// -----------------------------------------------------------------------------
		// Flags
		// -----------------------------------------------------------------------------
		Flags = ConvertGenerationPresets(_rDescriptor.m_GenFlag);

        // -----------------------------------------------------------------------------
        // Build path to texture in file system and load model
        // -----------------------------------------------------------------------------
		PathToModel = g_PathToAssets + _rDescriptor.m_pFileName;
        
        pScene = Importer.ReadFile(PathToModel.c_str(), Flags);

		if (!pScene)
		{
			PathToModel = g_PathToDataModels + _rDescriptor.m_pFileName;

			pScene = Importer.ReadFile(PathToModel.c_str(), Flags);
		}
        
        if( !pScene)
        {
            BASE_THROWV("Can't load model file %s; Code: %s", _rDescriptor.m_pFileName, Importer.GetErrorString());
        }
        
        // -----------------------------------------------------------------------------
        // Get current node of first mesh inside scene
        // -----------------------------------------------------------------------------
        aiNode* pCurrentObject = pScene->mRootNode;
        
        unsigned int NumberOfSurfaces = pCurrentObject->mNumMeshes;
        
        assert(NumberOfSurfaces < Dt::CLOD::s_NumberOfSurfaces);
        
        // -----------------------------------------------------------------------------
        // Setup model
        // -----------------------------------------------------------------------------
        rNewModel.m_NumberOfLODs = 1;
        
        // -----------------------------------------------------------------------------
        // For every LOD we have to create everything (data, material, informations,
        // ...)
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfLOD = 0; IndexOfLOD < rNewModel.m_NumberOfLODs; ++IndexOfLOD)
        {
            // -----------------------------------------------------------------------------
            // Create LOD
            // -----------------------------------------------------------------------------
            CInternLOD& rNewLOD = static_cast<CInternLOD&>(AllocateLOD());
            
            // -----------------------------------------------------------------------------
            // Link
            // -----------------------------------------------------------------------------
            rNewModel.m_LODs[IndexOfLOD] = & rNewLOD;
            
            // -----------------------------------------------------------------------------
            // Iterate throw the mesh data of current LOD and create a surface for every
            // mesh
            // -----------------------------------------------------------------------------
            std::function<void(const aiNode*, const aiScene*, Dt::CLOD&, unsigned int, unsigned int&)> CreateSurfaceFromNode = [&](const aiNode* _pNode, const aiScene* _pScene, Dt::CLOD& _rModelLOD, unsigned int _LODLevel, unsigned int& _rSurfaceNumber)->void
            {
                // -----------------------------------------------------------------------------
                // If we have some mesh data send this mesh to graphic and set facet on entity.
                // This collection of mesh is a model with different surfaces!
                // -----------------------------------------------------------------------------
                if (_pNode->mNumMeshes > 0)
                {
                    assert(_rSurfaceNumber + _pNode->mNumMeshes < Dt::CLOD::s_NumberOfSurfaces);
                    
                    // -----------------------------------------------------------------------------
                    // Create surface data
                    // -----------------------------------------------------------------------------
                    for (unsigned int IndexOfMesh = 0; IndexOfMesh < _pNode->mNumMeshes; ++IndexOfMesh)
                    {
                        // -----------------------------------------------------------------------------
                        // Get data from Assimp
                        // -----------------------------------------------------------------------------
                        unsigned int MeshIndex = _pNode->mMeshes[IndexOfMesh];
                        
                        aiMesh* pMesh = pScene->mMeshes[MeshIndex];
                        
                        unsigned int NumberOfVertices       = pMesh->mNumVertices;
                        unsigned int NumberOfFaces          = pMesh->mNumFaces;
                        unsigned int NumberOfIndicesPerFace = pMesh->mFaces->mNumIndices;
                        unsigned int NumberOfIndices        = NumberOfFaces * NumberOfIndicesPerFace;
                        
                        assert(NumberOfIndicesPerFace == 3);
                        
                        aiVector3D* pVertexData    = pMesh->mVertices;
                        aiVector3D* pNormalData    = pMesh->mNormals;
                        aiVector3D* pTangentData   = pMesh->mTangents;
                        aiVector3D* pBitangentData = pMesh->mBitangents;
                        aiVector3D* pTextureData   = pMesh->mTextureCoords[0];
                        
                        unsigned int Elements = CSurface::Position;
                        
                        assert(pVertexData != 0);
                        
                        if (pMesh->mNormals          != nullptr) Elements |= CSurface::Normal;
                        if (pMesh->mTangents         != nullptr) Elements |= CSurface::Tangent;
                        if (pMesh->mBitangents       != nullptr) Elements |= CSurface::Tangent;
                        if (pMesh->mTextureCoords[0] != nullptr) Elements |= CSurface::TexCoord0;
                                                
                        // -----------------------------------------------------------------------------
                        // Create surface
                        // -----------------------------------------------------------------------------
                        CInternSurface& rNewSurface = static_cast<CInternSurface&>(AllocateSurface(NumberOfVertices, NumberOfIndices, Elements));
                        
                        // -----------------------------------------------------------------------------
                        // Link
                        // -----------------------------------------------------------------------------
                        _rModelLOD.m_Surfaces[_rSurfaceNumber] = &rNewSurface;
                        
                        // -----------------------------------------------------------------------------
                        // Setup surface
                        // -----------------------------------------------------------------------------
                        for (unsigned int IndexOfFace = 0; IndexOfFace < NumberOfFaces; ++IndexOfFace)
                        {
                            aiFace CurrentFace = pMesh->mFaces[IndexOfFace];
                            
                            for (unsigned int IndexOfIndice = 0; IndexOfIndice < NumberOfIndicesPerFace; ++IndexOfIndice)
                            {
                                rNewSurface.m_pIndices[IndexOfFace * NumberOfIndicesPerFace + IndexOfIndice] = CurrentFace.mIndices[IndexOfIndice];
                            }
                        }
                        
                        Base::Float3 StartPosition(pVertexData[0].x, pVertexData[0].y, pVertexData[0].z);
                        
                        rNewModel.m_AABB.SetMin(StartPosition);
                        rNewModel.m_AABB.SetMax(StartPosition);
                        
                        for (unsigned int CurrentVertex = 0; CurrentVertex < NumberOfVertices; ++CurrentVertex)
                        {
                            Base::Float3 CurrentPosition(pVertexData[CurrentVertex].x, pVertexData[CurrentVertex].y, pVertexData[CurrentVertex].z);
                            
                            rNewModel.m_AABB.Extend(CurrentPosition);
                            
                            rNewSurface.m_pPositions[CurrentVertex][0] = pVertexData[CurrentVertex].x;
                            rNewSurface.m_pPositions[CurrentVertex][1] = pVertexData[CurrentVertex].y;
                            rNewSurface.m_pPositions[CurrentVertex][2] = pVertexData[CurrentVertex].z;
                            
                            if (rNewSurface.m_Elements & CSurface::Normal)
                            {
                                rNewSurface.m_pNormals[CurrentVertex][0] = pNormalData[CurrentVertex].x;
                                rNewSurface.m_pNormals[CurrentVertex][1] = pNormalData[CurrentVertex].y;
                                rNewSurface.m_pNormals[CurrentVertex][2] = pNormalData[CurrentVertex].z;
                            }
                            
                            if (rNewSurface.m_Elements & CSurface::Tangent)
                            {
                                assert(pTangentData != 0);
                                
                                rNewSurface.m_pTangents[CurrentVertex][0] = pTangentData[CurrentVertex].x;
                                rNewSurface.m_pTangents[CurrentVertex][1] = pTangentData[CurrentVertex].y;
                                rNewSurface.m_pTangents[CurrentVertex][2] = pTangentData[CurrentVertex].z;
                            }
                            
                            if (rNewSurface.m_Elements & CSurface::Tangent)
                            {
                                assert(pBitangentData != 0);
                                
                                rNewSurface.m_pBitangents[CurrentVertex][0] = pBitangentData[CurrentVertex].x;
                                rNewSurface.m_pBitangents[CurrentVertex][1] = pBitangentData[CurrentVertex].y;
                                rNewSurface.m_pBitangents[CurrentVertex][2] = pBitangentData[CurrentVertex].z;
                            }
                            
                            if (rNewSurface.m_Elements & CSurface::TexCoord0)
                            {
                                rNewSurface.m_pTexCoords[CurrentVertex][0] = pTextureData[CurrentVertex].x;
                                rNewSurface.m_pTexCoords[CurrentVertex][1] = pTextureData[CurrentVertex].y;
                            }
                        }
                        
                        // -----------------------------------------------------------------------------
                        // Create default material from file
                        // -----------------------------------------------------------------------------
                        unsigned int MaterialIndex = pScene->mMeshes[_pNode->mMeshes[IndexOfMesh]]->mMaterialIndex;;
                        
                        aiMaterial* pMaterial = pScene->mMaterials[MaterialIndex];
                        
                        // -----------------------------------------------------------------------------
                        // Check if material has an engine material indicator
                        // -----------------------------------------------------------------------------
                        aiString    NativeMaterialExaminer;
                        std::string MaterialExaminer;
                        
                        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &NativeMaterialExaminer) == AI_SUCCESS)
                        {
                            // -----------------------------------------------------------------------------
                            // Load material from file and link with surface
                            // -----------------------------------------------------------------------------
                            MaterialExaminer = std::string(NativeMaterialExaminer.data);
                            
                            if (MaterialExaminer.find(".mat") != std::string::npos)
                            {
                                SMaterialFileDescriptor MaterialFileDesc;

                                MaterialFileDesc.m_pFileName = MaterialExaminer.c_str();

                                rNewSurface.m_pDefaultMaterial = &MaterialManager::CreateMaterial(MaterialFileDesc);
                            }
                        }
                        else
                        {
                            // -----------------------------------------------------------------------------
                            // Create new material and load with model file settings
                            // -----------------------------------------------------------------------------
                            CMaterial& rNewMaterial = MaterialManager::CreateEmptyMaterial();
                            
                            // -----------------------------------------------------------------------------
                            // Link
                            // -----------------------------------------------------------------------------
                            rNewSurface.m_pDefaultMaterial = &rNewMaterial;
                            
                            // -----------------------------------------------------------------------------
                            // Get model specific attributes
                            // -----------------------------------------------------------------------------
                            aiString  TextureName;
                            aiColor4D DiffuseColor;
                            
                            float ColorR = 1.0f;
                            float ColorG = 1.0f;
                            float ColorB = 1.0f;
                            
                            const char* pColorMap = 0;
                            
                            if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
                            {
                                ColorR = DiffuseColor.r;
                                ColorG = DiffuseColor.g;
                                ColorB = DiffuseColor.b;
                            }
                            
                            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &TextureName) == AI_SUCCESS)
                            {
                                pColorMap = TextureName.data;
                            }
                            
                            // -----------------------------------------------------------------------------
                            // Normal
                            // -----------------------------------------------------------------------------
                            const char* pNormalMap = 0;
                            
                            if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &TextureName) == AI_SUCCESS)
                            {
                                pNormalMap = TextureName.data;
                            }
                            
                            // -----------------------------------------------------------------------------
                            // Setup material
                            // -----------------------------------------------------------------------------
                            rNewMaterial.m_Materialname    = "";
                            rNewMaterial.m_Color           = Base::Float3(ColorR, ColorG, ColorB);
                            rNewMaterial.m_Roughness       = 0.0f;
                            rNewMaterial.m_Reflectance     = 0.0f;
                            rNewMaterial.m_MetalMask       = 0.0f;
                            rNewMaterial.m_pColorMap       = 0;
                            rNewMaterial.m_pNormalMap      = 0;
                            rNewMaterial.m_pRoughnessMap   = 0;
                            rNewMaterial.m_pReflectanceMap = 0;
                            rNewMaterial.m_pMetalMaskMap   = 0;
                            rNewMaterial.m_pAOMap          = 0;
                            rNewMaterial.m_pBumpMap        = 0;

                            // -----------------------------------------------------------------------------
                            // Setup material textures
                            // -----------------------------------------------------------------------------
                            STextureDescriptor TextureDescriptor;

                            TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                            TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                            TextureDescriptor.m_NumberOfPixelsW  = 1;
                            TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
                            TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
                            TextureDescriptor.m_pPixels          = 0;
                            TextureDescriptor.m_pFileName        = 0;
                            TextureDescriptor.m_pIdentifier      = 0;

                            if (pColorMap)
                            {
                                TextureDescriptor.m_pFileName = pColorMap;

                                rNewMaterial.m_pColorMap = TextureManager::CreateTexture2D(TextureDescriptor);
                            }

                            if (pNormalMap)
                            {
                                TextureDescriptor.m_pFileName = pNormalMap;

                                rNewMaterial.m_pNormalMap = TextureManager::CreateTexture2D(TextureDescriptor);
                            }
                        }
                        
                        ++ _rSurfaceNumber;
                    }
                    
                    _rModelLOD.m_NumberOfSurfaces = _rSurfaceNumber;
                }
                
                // -----------------------------------------------------------------------------
                // Check sub children
                // -----------------------------------------------------------------------------
                unsigned int NumberOfEntities = _pNode->mNumChildren;
                
                for (unsigned int IndexOfEntity = 0; IndexOfEntity < NumberOfEntities; ++IndexOfEntity)
                {
                    aiNode* pChildren = _pNode->mChildren[IndexOfEntity];
                    
                    // -----------------------------------------------------------------------------
                    // Take care on more children
                    // -----------------------------------------------------------------------------
                    CreateSurfaceFromNode(pChildren, _pScene, rNewLOD, _LODLevel, _rSurfaceNumber);
                }
            };
            
            unsigned int NumberOfSurfaces = 0;
            
            CreateSurfaceFromNode(pCurrentObject, pScene, rNewLOD, IndexOfLOD, NumberOfSurfaces);
        }
        
        return rNewModel;
    }

    // -----------------------------------------------------------------------------

    CModel& CDtModelManager::CreateModel(const SModelAssimpDescriptor& _rDescriptor)
    {
        const aiNode*  pNode  = static_cast<const aiNode*>(_rDescriptor.m_pNode);
        const aiScene* pScene = static_cast<const aiScene*>(_rDescriptor.m_pScene);

        // -----------------------------------------------------------------------------
        // If we have some mesh data send this mesh to graphic and set facet on entity.
        // This collection of mesh is a model with different surfaces!
        // -----------------------------------------------------------------------------
        if (pNode == 0 || pScene == 0)
        {
            BASE_THROWM("Can't create model because of missing informations!");
        }

        // -----------------------------------------------------------------------------
        // Create model
        // -----------------------------------------------------------------------------
        CInternModel& rNewModel = static_cast<CInternModel&>(AllocateModel(pNode->mName.C_Str()));
                                
        // -----------------------------------------------------------------------------
        // Setup model
        // -----------------------------------------------------------------------------
        if (rNewModel.m_NumberOfLODs == 0)
        {
            rNewModel.m_NumberOfLODs = 1;
                    
            for (unsigned int IndexOfLOD = 0; IndexOfLOD < rNewModel.m_NumberOfLODs; IndexOfLOD++)
            {
                // -----------------------------------------------------------------------------
                // Create LOD
                // -----------------------------------------------------------------------------
                CInternLOD& rNewLOD = static_cast<CInternLOD&>(AllocateLOD());
                        
                // -----------------------------------------------------------------------------
                // Link
                // -----------------------------------------------------------------------------
                rNewModel.m_LODs[IndexOfLOD] = & rNewLOD;
                        
                // -----------------------------------------------------------------------------
                // Setup
                // -----------------------------------------------------------------------------
                rNewLOD.m_NumberOfSurfaces = pNode->mNumMeshes;
                        
                for (unsigned int IndexOfSurface = 0; IndexOfSurface < rNewLOD.m_NumberOfSurfaces; ++IndexOfSurface)
                {
                    // -----------------------------------------------------------------------------
                    // Get data from Assimp
                    // -----------------------------------------------------------------------------
                    unsigned int MeshIndex = pNode->mMeshes[IndexOfSurface];
                            
                    aiMesh* pMesh = pScene->mMeshes[MeshIndex];
                            
                    unsigned int NumberOfVertices       = pMesh->mNumVertices;
                    unsigned int NumberOfFaces          = pMesh->mNumFaces;
                    unsigned int NumberOfIndicesPerFace = pMesh->mFaces->mNumIndices;
                    unsigned int NumberOfIndices        = NumberOfFaces * NumberOfIndicesPerFace;
                            
                    assert(NumberOfIndicesPerFace == 3);
                            
                    aiVector3D* pVertexData    = pMesh->mVertices;
                    aiVector3D* pNormalData    = pMesh->mNormals;
                    aiVector3D* pTangentData   = pMesh->mTangents;
                    aiVector3D* pBitangentData = pMesh->mBitangents;
                    aiVector3D* pTextureData   = pMesh->mTextureCoords[0];
                            
                    unsigned int Elements = CSurface::Position;
                            
                    assert(pVertexData != 0);
                            
                    if (pMesh->mNormals          != nullptr) Elements |= CSurface::Normal;
                    if (pMesh->mTangents         != nullptr) Elements |= CSurface::Tangent;
                    if (pMesh->mBitangents       != nullptr) Elements |= CSurface::Tangent;
                    if (pMesh->mTextureCoords[0] != nullptr) Elements |= CSurface::TexCoord0;
                            
                    // -----------------------------------------------------------------------------
                    // Create surface
                    // -----------------------------------------------------------------------------
                    CInternSurface& rNewSurface = static_cast<CInternSurface&>(AllocateSurface(NumberOfVertices, NumberOfIndices, Elements));
                            
                    // -----------------------------------------------------------------------------
                    // Link
                    // -----------------------------------------------------------------------------
                    rNewLOD.m_Surfaces[IndexOfSurface] = &rNewSurface;
                            
                    // -----------------------------------------------------------------------------
                    // Setup surface
                    // -----------------------------------------------------------------------------
                    for (unsigned int IndexOfFace = 0; IndexOfFace < NumberOfFaces; ++IndexOfFace)
                    {
                        aiFace CurrentFace = pMesh->mFaces[IndexOfFace];
                                
                        for (unsigned int IndexOfIndice = 0; IndexOfIndice < NumberOfIndicesPerFace; ++IndexOfIndice)
                        {
                            rNewSurface.m_pIndices[IndexOfFace * NumberOfIndicesPerFace + IndexOfIndice] = CurrentFace.mIndices[IndexOfIndice];
                        }
                    }
                            
                    Base::Float3 StartPosition(pVertexData[0].x, pVertexData[0].y, pVertexData[0].z);
                            
                    rNewModel.m_AABB.SetMin(StartPosition);
                    rNewModel.m_AABB.SetMax(StartPosition);
                            
                    for (unsigned int CurrentVertex = 0; CurrentVertex < NumberOfVertices; ++CurrentVertex)
                    {
                        Base::Float3 CurrentPosition(pVertexData[CurrentVertex].x, pVertexData[CurrentVertex].y, pVertexData[CurrentVertex].z);
                                
                        rNewModel.m_AABB.Extend(CurrentPosition);
                                
                        rNewSurface.m_pPositions[CurrentVertex][0] = pVertexData[CurrentVertex].x;
                        rNewSurface.m_pPositions[CurrentVertex][1] = pVertexData[CurrentVertex].y;
                        rNewSurface.m_pPositions[CurrentVertex][2] = pVertexData[CurrentVertex].z;
                                
                        if (rNewSurface.m_Elements & CSurface::Normal)
                        {
                            rNewSurface.m_pNormals[CurrentVertex][0] = pNormalData[CurrentVertex].x;
                            rNewSurface.m_pNormals[CurrentVertex][1] = pNormalData[CurrentVertex].y;
                            rNewSurface.m_pNormals[CurrentVertex][2] = pNormalData[CurrentVertex].z;
                        }
                                
                        if (rNewSurface.m_Elements & CSurface::Tangent)
                        {
                            assert(pTangentData != 0);
                                    
                            rNewSurface.m_pTangents[CurrentVertex][0] = pTangentData[CurrentVertex].x;
                            rNewSurface.m_pTangents[CurrentVertex][1] = pTangentData[CurrentVertex].y;
                            rNewSurface.m_pTangents[CurrentVertex][2] = pTangentData[CurrentVertex].z;
                        }
                                
                        if (rNewSurface.m_Elements & CSurface::Tangent)
                        {
                            assert(pBitangentData != 0);
                                    
                            rNewSurface.m_pBitangents[CurrentVertex][0] = pBitangentData[CurrentVertex].x;
                            rNewSurface.m_pBitangents[CurrentVertex][1] = pBitangentData[CurrentVertex].y;
                            rNewSurface.m_pBitangents[CurrentVertex][2] = pBitangentData[CurrentVertex].z;
                        }
                                
                        if (rNewSurface.m_Elements & CSurface::TexCoord0)
                        {
                            rNewSurface.m_pTexCoords[CurrentVertex][0] = pTextureData[CurrentVertex].x;
                            rNewSurface.m_pTexCoords[CurrentVertex][1] = pTextureData[CurrentVertex].y;
                        }
                    }
                            
                    // -----------------------------------------------------------------------------
                    // Create default material from file
                    // -----------------------------------------------------------------------------
                    unsigned int MaterialIndex = pScene->mMeshes[pNode->mMeshes[IndexOfSurface]]->mMaterialIndex;;
                            
                    aiMaterial* pMaterial = pScene->mMaterials[MaterialIndex];
                            
                    // -----------------------------------------------------------------------------
                    // Check if material has an engine material indicator
                    // -----------------------------------------------------------------------------
                    aiString    NativeMaterialExaminer;
                    std::string MaterialExaminer;
                            
                    if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &NativeMaterialExaminer) == AI_SUCCESS)
                    {
                        // -----------------------------------------------------------------------------
                        // Load material from file and link with surface
                        // -----------------------------------------------------------------------------
                        MaterialExaminer = std::string(NativeMaterialExaminer.data);
                                
                        if (MaterialExaminer.find(".mat") != std::string::npos)
                        {
                            SMaterialFileDescriptor MaterialFileDesc;

                            MaterialFileDesc.m_pFileName = MaterialExaminer.c_str();

                            rNewSurface.m_pDefaultMaterial = &MaterialManager::CreateMaterial(MaterialFileDesc);
                        }
                    }
                    else
                    {
                        // -----------------------------------------------------------------------------
                        // Create new material and load with model file settings
                        // -----------------------------------------------------------------------------
                        CMaterial& rNewMaterial = MaterialManager::CreateEmptyMaterial();
                                
                        // -----------------------------------------------------------------------------
                        // Link
                        // -----------------------------------------------------------------------------
                        rNewSurface.m_pDefaultMaterial = &rNewMaterial;
                                
                        // -----------------------------------------------------------------------------
                        // Get model specific attributes
                        // -----------------------------------------------------------------------------
                        aiString  TextureName;
                        aiColor4D DiffuseColor;
                                
                        float ColorR = 1.0f;
                        float ColorG = 1.0f;
                        float ColorB = 1.0f;
                                
                        const char* pColorMap = "";
                                
                        if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
                        {
                            ColorR = DiffuseColor.r;
                            ColorG = DiffuseColor.g;
                            ColorB = DiffuseColor.b;
                        }
                                
                        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &TextureName) == AI_SUCCESS)
                        {
                            pColorMap = TextureName.data;
                        }
                                
                        // -----------------------------------------------------------------------------
                        // Normal
                        // -----------------------------------------------------------------------------
                        const char* pNormalMap = "";
                                
                        if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &TextureName) == AI_SUCCESS)
                        {
                            pNormalMap = TextureName.data;
                        }
                                
                        // -----------------------------------------------------------------------------
                        // Setup material
                        // -----------------------------------------------------------------------------
                        rNewMaterial.m_Materialname    = "";
                        rNewMaterial.m_Color           = Base::Float3(ColorR, ColorG, ColorB);
                        rNewMaterial.m_Roughness       = 0.0f;
                        rNewMaterial.m_Reflectance     = 0.0f;
                        rNewMaterial.m_MetalMask       = 0.0f;
                        rNewMaterial.m_pColorMap       = 0;
                        rNewMaterial.m_pNormalMap      = 0;
                        rNewMaterial.m_pRoughnessMap   = 0;
                        rNewMaterial.m_pReflectanceMap = 0;
                        rNewMaterial.m_pMetalMaskMap   = 0;
                        rNewMaterial.m_pAOMap          = 0;
                        rNewMaterial.m_pBumpMap        = 0;

                        // -----------------------------------------------------------------------------
                        // Setup material textures
                        // -----------------------------------------------------------------------------
                        STextureDescriptor TextureDescriptor;

                        TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                        TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                        TextureDescriptor.m_NumberOfPixelsW  = 1;
                        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
                        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
                        TextureDescriptor.m_pPixels          = 0;
                        TextureDescriptor.m_pFileName        = 0;
                        TextureDescriptor.m_pIdentifier      = 0;

                        if (pColorMap)
                        {
                            TextureDescriptor.m_pFileName = pColorMap;

                            rNewMaterial.m_pColorMap = TextureManager::CreateTexture2D(TextureDescriptor);
                        }

                        if (pNormalMap)
                        {
                            TextureDescriptor.m_pFileName = pNormalMap;

                            rNewMaterial.m_pNormalMap = TextureManager::CreateTexture2D(TextureDescriptor);
                        }
                    }
                }
            }
        }

        return rNewModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateBox(float _Width, float _Height, float _Depth)
    {
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        float HalfWidth  = _Width  / 2.0f;
        float HalfHeight = _Height / 2.0f;
        float HalfDepth  = _Depth  / 2.0f;
        
        unsigned int NumberOfVertices = 8;
        unsigned int NumberOfIndices  = 36;
        
        // -----------------------------------------------------------------------------
        // Create data
        // -----------------------------------------------------------------------------
        CInternModel& rBoxModel = static_cast<CInternModel&>(AllocateModel(""));
        
        CInternLOD& rBoxLOD = static_cast<CInternLOD&>(AllocateLOD());
        
        CInternSurface& rBoxSurface = static_cast<CInternSurface&>(AllocateSurface(NumberOfVertices, NumberOfIndices, CSurface::Position));
        
        // -----------------------------------------------------------------------------
        // Link data
        // -----------------------------------------------------------------------------
        rBoxModel.m_NumberOfLODs = 1;
        rBoxModel.m_LODs[0]      = &rBoxLOD;
        
        rBoxLOD.m_NumberOfSurfaces = 1;
        rBoxLOD.m_Surfaces[0]      = &rBoxSurface;
        
        // -----------------------------------------------------------------------------
        // Create vertices's for a box
        // -----------------------------------------------------------------------------
        rBoxSurface.m_pPositions[0][0] = -HalfWidth;
        rBoxSurface.m_pPositions[0][1] = -HalfDepth;
        rBoxSurface.m_pPositions[0][2] = -HalfHeight;
        
        rBoxSurface.m_pPositions[1][0] = -HalfWidth;
        rBoxSurface.m_pPositions[1][1] = -HalfDepth;
        rBoxSurface.m_pPositions[1][2] =  HalfHeight;
        
        rBoxSurface.m_pPositions[2][0] =  HalfWidth;
        rBoxSurface.m_pPositions[2][1] = -HalfDepth;
        rBoxSurface.m_pPositions[2][2] =  HalfHeight;
        
        rBoxSurface.m_pPositions[3][0] =  HalfWidth;
        rBoxSurface.m_pPositions[3][1] = -HalfDepth;
        rBoxSurface.m_pPositions[3][2] = -HalfHeight;
        
        rBoxSurface.m_pPositions[4][0] = -HalfWidth;
        rBoxSurface.m_pPositions[4][1] =  HalfDepth;
        rBoxSurface.m_pPositions[4][2] = -HalfHeight;
        
        rBoxSurface.m_pPositions[5][0] = -HalfWidth;
        rBoxSurface.m_pPositions[5][1] =  HalfDepth;
        rBoxSurface.m_pPositions[5][2] =  HalfHeight;
        
        rBoxSurface.m_pPositions[6][0] =  HalfWidth;
        rBoxSurface.m_pPositions[6][1] =  HalfDepth;
        rBoxSurface.m_pPositions[6][2] =  HalfHeight;
        
        rBoxSurface.m_pPositions[7][0] =  HalfWidth;
        rBoxSurface.m_pPositions[7][1] =  HalfDepth;
        rBoxSurface.m_pPositions[7][2] = -HalfHeight;
        
        // -----------------------------------------------------------------------------
        // Create indices of box
        // -----------------------------------------------------------------------------
        rBoxSurface.m_pIndices[ 0] = 0; rBoxSurface.m_pIndices[ 1] = 1; rBoxSurface.m_pIndices[ 2] = 2;
        rBoxSurface.m_pIndices[ 3] = 2; rBoxSurface.m_pIndices[ 4] = 3; rBoxSurface.m_pIndices[ 5] = 0;
        
        rBoxSurface.m_pIndices[ 6] = 3; rBoxSurface.m_pIndices[ 7] = 2; rBoxSurface.m_pIndices[ 8] = 6;
        rBoxSurface.m_pIndices[ 9] = 6; rBoxSurface.m_pIndices[10] = 7; rBoxSurface.m_pIndices[11] = 3;
        
        rBoxSurface.m_pIndices[12] = 5; rBoxSurface.m_pIndices[13] = 4; rBoxSurface.m_pIndices[14] = 7;
        rBoxSurface.m_pIndices[15] = 7; rBoxSurface.m_pIndices[16] = 6; rBoxSurface.m_pIndices[17] = 5;
        
        rBoxSurface.m_pIndices[18] = 4; rBoxSurface.m_pIndices[19] = 5; rBoxSurface.m_pIndices[20] = 1;
        rBoxSurface.m_pIndices[21] = 1; rBoxSurface.m_pIndices[22] = 0; rBoxSurface.m_pIndices[23] = 4;
        
        rBoxSurface.m_pIndices[24] = 1; rBoxSurface.m_pIndices[25] = 5; rBoxSurface.m_pIndices[26] = 6;
        rBoxSurface.m_pIndices[27] = 6; rBoxSurface.m_pIndices[28] = 2; rBoxSurface.m_pIndices[29] = 1;
        
        rBoxSurface.m_pIndices[30] = 4; rBoxSurface.m_pIndices[31] = 0; rBoxSurface.m_pIndices[32] = 3;
        rBoxSurface.m_pIndices[33] = 3; rBoxSurface.m_pIndices[34] = 7; rBoxSurface.m_pIndices[35] = 4;

        return rBoxModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices)
    {
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        unsigned int Width  = _Slices;
        unsigned int Height = _Stacks;
        
        unsigned int NumberOfVertices = (Height - 2) * Width + 2;
        unsigned int NumberOfIndices  = ((Height - 2) * (Width - 1) * 2) * 3;
        
        // -----------------------------------------------------------------------------
        // Create data
        // -----------------------------------------------------------------------------
        CInternModel& rBoxModel = static_cast<CInternModel&>(AllocateModel(0));
        
        CInternLOD& rBoxLOD = static_cast<CInternLOD&>(AllocateLOD());
        
        CInternSurface& rBoxSurface = static_cast<CInternSurface&>(AllocateSurface(NumberOfVertices, NumberOfIndices, CSurface::Position));
        
        // -----------------------------------------------------------------------------
        // Link data
        // -----------------------------------------------------------------------------
        rBoxModel.m_NumberOfLODs = 1;
        rBoxModel.m_LODs[0]      = &rBoxLOD;
        
        rBoxLOD.m_NumberOfSurfaces = 1;
        rBoxLOD.m_Surfaces[0]      = &rBoxSurface;
        
        // -----------------------------------------------------------------------------
        // Create vertices's for a sphere
        // -----------------------------------------------------------------------------
        unsigned int IndexOfVertex = 0;
        
        for(unsigned int IndexOfStack = 1; IndexOfStack < Height - 1; ++ IndexOfStack)
        {
            for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width; ++ IndexOfSlice)
            {
                float THETA = static_cast<float>(IndexOfStack) / static_cast<float>(Height - 1) * Base::SConstants<float>::s_Pi;
                float PHI   = static_cast<float>(IndexOfSlice) / static_cast<float>(Width  - 1) * Base::SConstants<float>::s_Pi * 2.0f;
                
                rBoxSurface.m_pPositions[IndexOfVertex][0] =  Base::Sin(THETA) * Base::Cos(PHI) * _Radius;
                rBoxSurface.m_pPositions[IndexOfVertex][1] =  Base::Cos(THETA) * _Radius;
                rBoxSurface.m_pPositions[IndexOfVertex][2] = -Base::Sin(THETA) * Base::Sin(PHI) * _Radius;
                
                ++ IndexOfVertex;
            }
        }
        
        rBoxSurface.m_pPositions[IndexOfVertex][0] = 0.0f;
        rBoxSurface.m_pPositions[IndexOfVertex][1] = _Radius;
        rBoxSurface.m_pPositions[IndexOfVertex][2] = 0.0f;
        
        ++ IndexOfVertex;
        
        rBoxSurface.m_pPositions[IndexOfVertex][0] = 0.0f;
        rBoxSurface.m_pPositions[IndexOfVertex][1] = - _Radius;
        rBoxSurface.m_pPositions[IndexOfVertex][2] = 0.0f;
        
        assert((IndexOfVertex + 1) == NumberOfVertices);
        
        // -----------------------------------------------------------------------------
        // Create indices of sphere
        // -----------------------------------------------------------------------------
        unsigned int IndexOfIndex = 0;
        
        for(unsigned int IndexOfStack = 0; IndexOfStack < Height - 3; ++ IndexOfStack)
        {
            for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width - 1; ++ IndexOfSlice )
            {
                rBoxSurface.m_pIndices[IndexOfIndex ++] = (IndexOfStack    ) * Width + IndexOfSlice + 1;
                rBoxSurface.m_pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice + 1;
                rBoxSurface.m_pIndices[IndexOfIndex ++] = (IndexOfStack    ) * Width + IndexOfSlice;
                
                rBoxSurface.m_pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice + 1;
                rBoxSurface.m_pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice;
                rBoxSurface.m_pIndices[IndexOfIndex ++] = (IndexOfStack    ) * Width + IndexOfSlice;
            }
        }
        
        for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width - 1; ++ IndexOfSlice)
        {
            rBoxSurface.m_pIndices[IndexOfIndex ++] = IndexOfSlice + 1;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = IndexOfSlice;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = (Height - 2) * Width;
            
            rBoxSurface.m_pIndices[IndexOfIndex ++] = (Height - 3) * Width + IndexOfSlice;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = (Height - 3) * Width + IndexOfSlice + 1;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = (Height - 2) * Width + 1;
        }
        
        return rBoxModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateCone(float _Radius, float _Height, unsigned int _Slices)
    {        
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        assert(_Slices >= 3);
        
        unsigned int NumberOfVertices = 3 + _Slices;
        unsigned int NumberOfIndices  = _Slices * 6;

        // -----------------------------------------------------------------------------
        // Create data
        // -----------------------------------------------------------------------------
        CInternModel& rBoxModel = static_cast<CInternModel&>(AllocateModel(0));
        
        CInternLOD& rBoxLOD = static_cast<CInternLOD&>(AllocateLOD());
        
        CInternSurface& rBoxSurface = static_cast<CInternSurface&>(AllocateSurface(NumberOfVertices, NumberOfIndices, CSurface::Position));
        
        // -----------------------------------------------------------------------------
        // Link data
        // -----------------------------------------------------------------------------
        rBoxModel.m_NumberOfLODs = 1;
        rBoxModel.m_LODs[0]      = &rBoxLOD;
        
        rBoxLOD.m_NumberOfSurfaces = 1;
        rBoxLOD.m_Surfaces[0]      = &rBoxSurface;
        
        // -----------------------------------------------------------------------------
        // Create vertices's of a cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfVertex = 0;
        
        rBoxSurface.m_pPositions[IndexOfVertex ++] = Base::Float3(0.0f, 0.0f, 0.0f);
        rBoxSurface.m_pPositions[IndexOfVertex ++] = Base::Float3(0.0f, 0.0f, - _Height);
        
        for( unsigned int IndexOfSlice = 0; IndexOfSlice < _Slices + 1; ++IndexOfSlice )
        {
            assert( IndexOfVertex < NumberOfVertices);
            
            float PHI   = static_cast<float>(IndexOfSlice) / (_Slices) * Base::SConstants<float>::s_Pi * 2.0f;
            
            rBoxSurface.m_pPositions[IndexOfVertex][0] = Base::Cos(PHI) * _Radius;
            rBoxSurface.m_pPositions[IndexOfVertex][1] = Base::Sin(PHI) * _Radius;
            rBoxSurface.m_pPositions[IndexOfVertex][2] = - _Height;
            
            ++ IndexOfVertex;
        }
        
        // -----------------------------------------------------------------------------
        // This have to be done, to avoid numerical imprecisions (double rendering)
        // -----------------------------------------------------------------------------
        rBoxSurface.m_pPositions[IndexOfVertex - 1] = rBoxSurface.m_pPositions[2];
        
        // -----------------------------------------------------------------------------
        // Create indices for currently created cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfIndex = 0;
        
        for( unsigned int IndexOfSlice = 0; IndexOfSlice < _Slices; ++ IndexOfSlice )
        {
            rBoxSurface.m_pIndices[IndexOfIndex ++] = IndexOfSlice + 3;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = IndexOfSlice + 2;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = 0;
        }
        
        for( unsigned int IndexOfSlice = 0; IndexOfSlice < _Slices; ++ IndexOfSlice )
        {
            rBoxSurface.m_pIndices[IndexOfIndex ++] = IndexOfSlice + 2;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = IndexOfSlice + 3;
            rBoxSurface.m_pIndices[IndexOfIndex ++] = 1;
            
            assert(IndexOfSlice + 3 < NumberOfVertices);
        }
        
        assert(IndexOfIndex == NumberOfIndices);
        
        return rBoxModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CModel& CDtModelManager::CreateRectangle(float _X, float _Y, float _Width, float _Height)
    {
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        assert(_Width != 0.0f && _Height != 0.0f);
        
        unsigned int NumberOfVertices = 4;
        unsigned int NumberOfIndices  = 6;
        
        // -----------------------------------------------------------------------------
        // Create data
        // -----------------------------------------------------------------------------
        CInternModel& rBoxModel = static_cast<CInternModel&>(AllocateModel(0));
        
        CInternLOD& rBoxLOD = static_cast<CInternLOD&>(AllocateLOD());
        
        CInternSurface& rBoxSurface = static_cast<CInternSurface&>(AllocateSurface(NumberOfVertices, NumberOfIndices, CSurface::Position));
        
        // -----------------------------------------------------------------------------
        // Link data
        // -----------------------------------------------------------------------------
        rBoxModel.m_NumberOfLODs = 1;
        rBoxModel.m_LODs[0]      = &rBoxLOD;
        
        rBoxLOD.m_NumberOfSurfaces = 1;
        rBoxLOD.m_Surfaces[0]      = &rBoxSurface;
        
        // -----------------------------------------------------------------------------
        // Create vertices's of a cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfVertex = 0;
        
        rBoxSurface.m_pPositions[IndexOfVertex ++] = Base::Float3(_X         , _Y + _Height, 0.0f);
        rBoxSurface.m_pPositions[IndexOfVertex ++] = Base::Float3(_X + _Width, _Y +_Height , 0.0f);
        rBoxSurface.m_pPositions[IndexOfVertex ++] = Base::Float3(_X + _Width, _Y          , 0.0f);
        rBoxSurface.m_pPositions[IndexOfVertex ++] = Base::Float3(_X         , _Y          , 0.0f);
        
        assert(IndexOfVertex == NumberOfVertices);
        
        // -----------------------------------------------------------------------------
        // Create indices for currently created cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfIndex = 0;
        
        rBoxSurface.m_pIndices[IndexOfIndex ++] = 0;
        rBoxSurface.m_pIndices[IndexOfIndex ++] = 1;
        rBoxSurface.m_pIndices[IndexOfIndex ++] = 2;
        rBoxSurface.m_pIndices[IndexOfIndex ++] = 0;
        rBoxSurface.m_pIndices[IndexOfIndex ++] = 2;
        rBoxSurface.m_pIndices[IndexOfIndex ++] = 3;
        
        assert(IndexOfIndex == NumberOfIndices);
        
        return rBoxModel;
    }

    // -----------------------------------------------------------------------------

    void CDtModelManager::FreeModel(CModel& _rModel)
    {
        CInternModel& rInternModel = static_cast<CInternModel&>(_rModel);
        
        for (unsigned int IndexOfLOD = 0; IndexOfLOD < rInternModel.m_NumberOfLODs; ++IndexOfLOD)
        {
            CInternLOD& rInternLOD = static_cast<CInternLOD&>(*rInternModel.m_LODs[IndexOfLOD]);
            
            for (unsigned int IndexOfSurface = 0; IndexOfSurface < rInternLOD.m_NumberOfSurfaces; ++ IndexOfSurface)
            {
                CInternSurface& rInternSurface = static_cast<CInternSurface&>(*rInternLOD.m_Surfaces[IndexOfSurface]);
                
                if (rInternSurface.m_pPositions  != 0) Base::CMemory::Free(rInternSurface.m_pPositions);
                if (rInternSurface.m_pNormals    != 0) Base::CMemory::Free(rInternSurface.m_pNormals);
                if (rInternSurface.m_pTangents   != 0) Base::CMemory::Free(rInternSurface.m_pTangents);
                if (rInternSurface.m_pBitangents != 0) Base::CMemory::Free(rInternSurface.m_pBitangents);
                if (rInternSurface.m_pTexCoords  != 0) Base::CMemory::Free(rInternSurface.m_pTexCoords);
                
                rInternSurface.m_NumberOfVertices = 0;
                rInternSurface.m_NumberOfIndices  = 0;
                rInternSurface.m_pDefaultMaterial = 0;
                
                m_Surfaces.Free(&rInternSurface);
            }
            
            rInternLOD.m_NumberOfSurfaces = 0;
            
            m_LODs.Free(&rInternLOD);
        }
        
        rInternModel.m_NumberOfLODs = 0;
        rInternModel.m_Modelname.Clear();
        
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
        
        rNewModel.m_Modelname    = _pModelname != 0 ? _pModelname : "";
        rNewModel.m_NumberOfLODs = 0;
        
        for (unsigned int IndexOfLOD = 0; IndexOfLOD < CModel::s_NumberOfLODs; ++ IndexOfLOD)
        {
            rNewModel.m_LODs[IndexOfLOD] = 0;
        }
        
        // -----------------------------------------------------------------------------
        // Add model to hash table
        // -----------------------------------------------------------------------------
        if (Hash != 0)
        {
            m_ModelByID[Hash] = &rNewModel;
        }
        
        return rNewModel;
    }
    
    // -----------------------------------------------------------------------------
    
    CLOD& CDtModelManager::AllocateLOD()
    {
        // -----------------------------------------------------------------------------
        // Create new LOD
        // -----------------------------------------------------------------------------
        CInternLOD& rNewLOD = m_LODs.Allocate();
        
        rNewLOD.m_NumberOfSurfaces = 0;
        
        for (unsigned int IndexOfSurface = 0; IndexOfSurface < CLOD::s_NumberOfSurfaces; ++ IndexOfSurface)
        {
            rNewLOD.m_Surfaces[IndexOfSurface] = 0;
        }
        
        return rNewLOD;
    }
    
    // -----------------------------------------------------------------------------
    
    CSurface& CDtModelManager::AllocateSurface(unsigned int _NumberOfVertices, unsigned int _NumberOfIndices, unsigned int _Elements)
    {
        // -----------------------------------------------------------------------------
        // Create new surface
        // -----------------------------------------------------------------------------
        CInternSurface& rNewSurface = m_Surfaces.Allocate();
        
        rNewSurface.m_NumberOfIndices  = _NumberOfIndices;
        rNewSurface.m_NumberOfVertices = _NumberOfVertices;
        rNewSurface.m_Elements         = _Elements;
        rNewSurface.m_pDefaultMaterial = 0;
        
        // -----------------------------------------------------------------------------
        // Indices
        // -----------------------------------------------------------------------------
        assert(_NumberOfIndices > 0);
        
        rNewSurface.m_pIndices = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * _NumberOfIndices));
        
        // -----------------------------------------------------------------------------
        // Vertices
        // -----------------------------------------------------------------------------
        unsigned int NumberOfPositions      = _NumberOfVertices * ((_Elements & CSurface::Position)  == CSurface::Position);
        unsigned int NumberOfNormals        = _NumberOfVertices * ((_Elements & CSurface::Normal)    == CSurface::Normal);
        unsigned int NumberOfTagents        = _NumberOfVertices * ((_Elements & CSurface::Tangent)   == CSurface::Tangent);
        unsigned int NumberOfBitangents     = _NumberOfVertices * ((_Elements & CSurface::Tangent)   == CSurface::Tangent);
        unsigned int NumberOfTexCoords      = _NumberOfVertices * ((_Elements & CSurface::TexCoord0) == CSurface::TexCoord0);
        
        // -----------------------------------------------------------------------------
        // Position
        // -----------------------------------------------------------------------------
        assert(NumberOfPositions > 0);
        
        rNewSurface.m_pPositions = static_cast<Base::Float3*>(Base::CMemory::Allocate(sizeof(Base::Float3) * NumberOfPositions));
        
        // -----------------------------------------------------------------------------
        // Normals
        // -----------------------------------------------------------------------------
        rNewSurface.m_pNormals = 0;
        
        if (NumberOfNormals > 0)
        {
            rNewSurface.m_pNormals = static_cast<Base::Float3*>(Base::CMemory::Allocate(sizeof(Base::Float3) * NumberOfNormals));
        }
        
        // -----------------------------------------------------------------------------
        // Tangents
        // -----------------------------------------------------------------------------
        rNewSurface.m_pTangents = 0;
        
        if (NumberOfTagents > 0)
        {
            rNewSurface.m_pTangents = static_cast<Base::Float3*>(Base::CMemory::Allocate(sizeof(Base::Float3) * NumberOfTagents));
        }
        
        // -----------------------------------------------------------------------------
        // Bitangents
        // -----------------------------------------------------------------------------
        rNewSurface.m_pBitangents = 0;
        
        if (NumberOfBitangents > 0)
        {
            rNewSurface.m_pBitangents = static_cast<Base::Float3*>(Base::CMemory::Allocate(sizeof(Base::Float3) * NumberOfBitangents));
        }
        
        // -----------------------------------------------------------------------------
        // TexCoord 0
        // -----------------------------------------------------------------------------
        rNewSurface.m_pTexCoords = 0;
        
        if (NumberOfTexCoords > 0)
        {
            rNewSurface.m_pTexCoords = static_cast<Base::Float2*>(Base::CMemory::Allocate(sizeof(Base::Float2) * NumberOfTexCoords));
        }
        
        return rNewSurface;
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CDtModelManager::ConvertGenerationPresets(unsigned int _EngineFlag)
    {
        static const unsigned int s_AssimpGenerationPresets[] =
        {
            aiProcess_Triangulate,
            aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices,
            aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs,
            aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_SortByPType,
        };
        
        return s_AssimpGenerationPresets[_EngineFlag];
    };
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

    CModel& CreateModel(const SModelFileDescriptor& _rDescriptor)
    {
        return CDtModelManager::GetInstance().CreateModel(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    CModel& CreateModel(const SModelAssimpDescriptor& _rDescriptor)
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