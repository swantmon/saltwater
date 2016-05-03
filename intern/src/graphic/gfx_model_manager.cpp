
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_math_constants.h"
#include "base/base_math_operations.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include "data/data_entity.h"
#include "data/data_model.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_input_layout.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_model_manager.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_shader_manager.h"

#include <unordered_map>
#include <functional>

using namespace Gfx;
using namespace Gfx::ModelManager;

namespace
{
    class CGfxModelManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxModelManager)
        
    public:

        CGfxModelManager();
        ~CGfxModelManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();
        
        CModelPtr CreateModel(const SModelDescriptor& _rDescriptor);
        
        CModelPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
        CModelPtr CreateRectangle(float _X, float _Y, float _Width, float _Height);
        
    private:
        
        class CInternModel : public CModel
        {
        private:
            
            friend class CGfxModelManager;
        };
        
        class CInternLOD : public CLOD
        {
        private:
            
            friend class CGfxModelManager;
        };
        
        class CInternSurface : public CSurface
        {
        private:
            
            friend class CGfxModelManager;
        };
        
    private:
        
        typedef Base::CManagedPool<CInternModel   , 64  , 1> CModels;
        typedef Base::CManagedPool<CInternLOD     , 256 , 1> CLODs;
        typedef Base::CManagedPool<CInternSurface , 1024, 1> CSurfaces;
        
        typedef std::unordered_map<unsigned int, CInternModel*> CModelByIDs;
        typedef CModelByIDs::iterator                           CModelByIDPair;
        
    private:
        
        CModels    m_Models;
        CLODs      m_LODs;
        CSurfaces  m_Surfaces;
        
        CModelByIDs m_ModelByID;
    };
} // namespace

namespace
{
    CGfxModelManager::CGfxModelManager()
        : m_Models   ()
        , m_LODs     ()
        , m_Surfaces ()
        , m_ModelByID()
    {
        m_ModelByID.reserve(64);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxModelManager::~CGfxModelManager()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxModelManager::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxModelManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxModelManager::Clear()
    {
        m_Models   .Clear();
        m_LODs     .Clear();
        m_Surfaces .Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CModelPtr CGfxModelManager::CreateModel(const Gfx::SModelDescriptor& _rDescriptor)
    {
        const Dt::CModel& rDataModel = *_rDescriptor.m_pModel;
        
        // -----------------------------------------------------------------------------
        // Check existing model
        // -----------------------------------------------------------------------------
        unsigned int Hash = 0;
        
        if (rDataModel.m_Modelname != "")
        {
            Hash = Base::CRC32(rDataModel.m_Modelname.GetConst(), static_cast<unsigned int>(rDataModel.m_Modelname.GetLength()));
            
            if (m_ModelByID.find(Hash) != m_ModelByID.end())
            {
                return CModelPtr(m_ModelByID.at(Hash));
            }
        }
        
        // -----------------------------------------------------------------------------
        // Create model
        // -----------------------------------------------------------------------------
        CModels::CPtr ModelPtr = m_Models.Allocate();
        
        CInternModel& rModel = *ModelPtr;
        
        rModel.m_NumberOfLODs = rDataModel.m_NumberOfLODs;
        
        // -----------------------------------------------------------------------------
        // For every LOD we have to create everything (data, material, informations,
        // ...)
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfLOD = 0; IndexOfLOD < rDataModel.m_NumberOfLODs; ++IndexOfLOD)
        {
            const Dt::CLOD& rCurrentLOD = *rDataModel.m_LODs[IndexOfLOD];
            
            CLODs::CPtr LODPtr = m_LODs.Allocate();
            
            CInternLOD& rLOD = *LODPtr;
            
            rLOD.m_NumberOfSurfaces = rCurrentLOD.m_NumberOfSurfaces;
            
            // -----------------------------------------------------------------------------
            // For every surface in model create material and surface informations
            // -----------------------------------------------------------------------------
            for (unsigned int IndexOfSurface = 0; IndexOfSurface < rCurrentLOD.m_NumberOfSurfaces; ++IndexOfSurface)
            {
                // -----------------------------------------------------------------------------
                // Create surface depending on data
                // -----------------------------------------------------------------------------
                const Dt::CSurface& rCurrentSurface = *rCurrentLOD.m_Surfaces[IndexOfSurface];
                
                CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
                
                CInternSurface& rSurface = *SurfacePtr;
                
                // -----------------------------------------------------------------------------
                // Surface attributes
                // -----------------------------------------------------------------------------
                rSurface.m_SurfaceKey.m_HasPosition  = true;
                rSurface.m_SurfaceKey.m_HasNormal    = ((rCurrentSurface.m_Elements & Dt::CSurface::Normal)     == Dt::CSurface::Normal);
                rSurface.m_SurfaceKey.m_HasTangent   = ((rCurrentSurface.m_Elements & Dt::CSurface::Tangent)    == Dt::CSurface::Tangent);
                rSurface.m_SurfaceKey.m_HasBitangent = ((rCurrentSurface.m_Elements & Dt::CSurface::Tangent)    == Dt::CSurface::Tangent);
                rSurface.m_SurfaceKey.m_HasTexCoords = ((rCurrentSurface.m_Elements & Dt::CSurface::TexCoord0)  == Dt::CSurface::TexCoord0);
                
                // -----------------------------------------------------------------------------
                // Prepare mesh data for data alignment
                // -----------------------------------------------------------------------------
                unsigned int NumberOfVertices           = rCurrentSurface.m_NumberOfVertices * rSurface.m_SurfaceKey.m_HasPosition;
                unsigned int NumberOfNormals            = rCurrentSurface.m_NumberOfVertices * rSurface.m_SurfaceKey.m_HasNormal;
                unsigned int NumberOfTagents            = rCurrentSurface.m_NumberOfVertices * rSurface.m_SurfaceKey.m_HasTangent;
                unsigned int NumberOfBitangents         = rCurrentSurface.m_NumberOfVertices * rSurface.m_SurfaceKey.m_HasBitangent;
                unsigned int NumberOfTexCoords          = rCurrentSurface.m_NumberOfVertices * (rSurface.m_SurfaceKey.m_HasTexCoords >= 1);
                unsigned int NumberOfVerticeElements    = 3 * rSurface.m_SurfaceKey.m_HasPosition;
                unsigned int NumberOfNormalElements     = 3 * rSurface.m_SurfaceKey.m_HasNormal;
                unsigned int NumberOfTagentsElements    = 3 * rSurface.m_SurfaceKey.m_HasTangent;
                unsigned int NumberOfBitangentsElements = 3 * rSurface.m_SurfaceKey.m_HasBitangent;
                unsigned int NumberOfTexCoordElements   = 2 * (rSurface.m_SurfaceKey.m_HasTexCoords >= 1);
                unsigned int NumberOfVertexElements     = NumberOfVertices * NumberOfVerticeElements + NumberOfNormals * NumberOfNormalElements + NumberOfTagents * NumberOfTagentsElements + NumberOfBitangents * NumberOfBitangentsElements + NumberOfTexCoords * NumberOfTexCoordElements;
                unsigned int NumberOfElementsPerVertex  = NumberOfVerticeElements + NumberOfNormalElements + NumberOfTagentsElements + NumberOfBitangentsElements + NumberOfTexCoordElements;
                unsigned int NumberOfIndices            = rCurrentSurface.m_NumberOfIndices;
                
                // -----------------------------------------------------------------------------
                // Prepare upload data of vertices
                // -----------------------------------------------------------------------------
                float* pUploadVertexData = static_cast<float* >(Base::CMemory::Allocate(sizeof(float) * NumberOfVertexElements));
                
                // -----------------------------------------------------------------------------
                // Prepare AABB
                // -----------------------------------------------------------------------------
                Base::Float3 StartPosition(rCurrentSurface.m_pPositions[0][0], rCurrentSurface.m_pPositions[0][1], rCurrentSurface.m_pPositions[0][2]);
                
                rModel.m_AABB.SetMin(StartPosition);
                rModel.m_AABB.SetMax(StartPosition);
                
                // -----------------------------------------------------------------------------
                // Iterate throw every vertex and put it into upload data
                // -----------------------------------------------------------------------------
                for (unsigned int CurrentVertex = 0; CurrentVertex < NumberOfVertices; ++CurrentVertex)
                {
                    unsigned int CurrentAlignIndex = CurrentVertex * NumberOfElementsPerVertex;
                    
                    Base::Float3 CurrentPosition(rCurrentSurface.m_pPositions[CurrentVertex][0], rCurrentSurface.m_pPositions[CurrentVertex][1], rCurrentSurface.m_pPositions[CurrentVertex][2]);
                    
                    rModel.m_AABB.Extend(CurrentPosition);
                    
                    pUploadVertexData[CurrentAlignIndex + 0] = rCurrentSurface.m_pPositions[CurrentVertex][0];
                    pUploadVertexData[CurrentAlignIndex + 1] = rCurrentSurface.m_pPositions[CurrentVertex][1];
                    pUploadVertexData[CurrentAlignIndex + 2] = rCurrentSurface.m_pPositions[CurrentVertex][2];
                    
                    CurrentAlignIndex += 3;
                    
                    if (rSurface.m_SurfaceKey.m_HasNormal)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = rCurrentSurface.m_pNormals[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = rCurrentSurface.m_pNormals[CurrentVertex][1];
                        pUploadVertexData[CurrentAlignIndex + 2] = rCurrentSurface.m_pNormals[CurrentVertex][2];
                        
                        CurrentAlignIndex += 3;
                    }
                    
                    if (rSurface.m_SurfaceKey.m_HasTangent)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = rCurrentSurface.m_pTangents[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = rCurrentSurface.m_pTangents[CurrentVertex][1];
                        pUploadVertexData[CurrentAlignIndex + 2] = rCurrentSurface.m_pTangents[CurrentVertex][2];
                        
                        CurrentAlignIndex += 3;
                    }
                    
                    if (rSurface.m_SurfaceKey.m_HasBitangent)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = rCurrentSurface.m_pBitangents[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = rCurrentSurface.m_pBitangents[CurrentVertex][1];
                        pUploadVertexData[CurrentAlignIndex + 2] = rCurrentSurface.m_pBitangents[CurrentVertex][2];
                        
                        CurrentAlignIndex += 3;
                    }
                    
                    if (rSurface.m_SurfaceKey.m_HasTexCoords >= 1)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = rCurrentSurface.m_pTexCoords[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = rCurrentSurface.m_pTexCoords[CurrentVertex][1];
                        
                        CurrentAlignIndex += 2;
                    }
                }
                
                // -----------------------------------------------------------------------------
                // Create buffer with vertices's and indices (setup surface data)
                // -----------------------------------------------------------------------------
                SBufferDescriptor VertexBufferDesc;
                
                VertexBufferDesc.m_Stride        = 0;
                VertexBufferDesc.m_Usage         = CBuffer::GPURead;
                VertexBufferDesc.m_Binding       = CBuffer::VertexBuffer;
                VertexBufferDesc.m_Access        = CBuffer::CPUWrite;
                VertexBufferDesc.m_NumberOfBytes = sizeof(float) * NumberOfVertexElements;
                VertexBufferDesc.m_pBytes        = pUploadVertexData;
                VertexBufferDesc.m_pClassKey     = 0;
                
                CBufferPtr PositionBuffer = BufferManager::CreateBuffer(VertexBufferDesc);
                
                rSurface.m_VertexBuffer = BufferManager::CreateVertexBufferSet(PositionBuffer);
                
                // -----------------------------------------------------------------------------
                
                SBufferDescriptor IndexBufferDesc;
                
                IndexBufferDesc.m_Stride        = 0;
                IndexBufferDesc.m_Usage         = CBuffer::GPURead;
                IndexBufferDesc.m_Binding       = CBuffer::IndexBuffer;
                IndexBufferDesc.m_Access        = CBuffer::CPUWrite;
                IndexBufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
                IndexBufferDesc.m_pBytes        = rCurrentSurface.m_pIndices;
                IndexBufferDesc.m_pClassKey     = 0;
                
                rSurface.m_IndexBuffer = BufferManager::CreateBuffer(IndexBufferDesc);
                
                // -----------------------------------------------------------------------------
                // Set last data information of the surface
                // -----------------------------------------------------------------------------
                rSurface.m_NumberOfVertices = NumberOfVertices;
                rSurface.m_NumberOfIndices  = NumberOfIndices;
                
                // -----------------------------------------------------------------------------
                // Delete allocated memory
                // -----------------------------------------------------------------------------
                Base::CMemory::Free(pUploadVertexData);
                
                // -----------------------------------------------------------------------------
                // Materials
                // -----------------------------------------------------------------------------
                if (rCurrentSurface.m_pDefaultMaterial)
                {
                    Gfx::SMaterialDescriptor MaterialDesc;
                    
                    MaterialDesc.m_ID        = rSurface.m_SurfaceKey.m_Key;
                    MaterialDesc.m_pMaterial = rCurrentSurface.m_pDefaultMaterial;
                    
                    rSurface.m_MaterialPtr = Gfx::MaterialManager::CreateMaterial(MaterialDesc);
                }
                
                // -----------------------------------------------------------------------------
                // Assign surface to current LOD
                // -----------------------------------------------------------------------------
                rLOD.m_Surfaces[IndexOfSurface] = SurfacePtr;
            }
            
            // -----------------------------------------------------------------------------
            // Assign this LOD to our model
            // -----------------------------------------------------------------------------
            rModel.m_LODs[IndexOfLOD] = LODPtr;
        }
        
        // -----------------------------------------------------------------------------
        // Put this new model to hash list
        // -----------------------------------------------------------------------------
        if (Hash != 0)
        {
            m_ModelByID[Hash] = &rModel;
        }
        
        return CModelPtr(ModelPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CModelPtr CGfxModelManager::CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CModels::CPtr ModelPtr = m_Models.Allocate();
        
        CInternModel& rModel = *ModelPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_LODs[0] = LODPtr;
        
        rLOD.m_Surfaces[0] = SurfacePtr;
        
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        unsigned int Width  = _Slices;
        unsigned int Height = _Stacks;
        
        unsigned int NumberOfVertices = (Height - 2) * Width + 2;
        unsigned int NumberOfIndices  = ((Height - 2) * (Width - 1) * 2) * 3;
        
        Base::Float3* pVertices = static_cast<Base::Float3*>(Base::CMemory::Allocate(sizeof(Base::Float3) * NumberOfVertices));
        unsigned int* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
        
        assert(pVertices);
        assert(pVertices);
        
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
                
                pVertices[IndexOfVertex][0] =  Base::Sin(THETA) * Base::Cos(PHI) * _Radius;
                pVertices[IndexOfVertex][1] =  Base::Cos(THETA) * _Radius;
                pVertices[IndexOfVertex][2] = -Base::Sin(THETA) * Base::Sin(PHI) * _Radius;
                
                ++ IndexOfVertex;
            }
        }
        
        pVertices[IndexOfVertex][0] = 0.0f;
        pVertices[IndexOfVertex][1] = _Radius;
        pVertices[IndexOfVertex][2] = 0.0f;
        
        ++ IndexOfVertex;
        
        pVertices[IndexOfVertex][0] = 0.0f;
        pVertices[IndexOfVertex][1] = - _Radius;
        pVertices[IndexOfVertex][2] = 0.0f;
        
        assert((IndexOfVertex + 1) == NumberOfVertices);
        
        // -----------------------------------------------------------------------------
        // Create indices of sphere
        // -----------------------------------------------------------------------------
        unsigned int IndexOfIndex = 0;
        
        for(unsigned int IndexOfStack = 0; IndexOfStack < Height - 3; ++ IndexOfStack)
        {
            for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width - 1; ++ IndexOfSlice )
            {
                pIndices[IndexOfIndex ++] = (IndexOfStack    ) * Width + IndexOfSlice + 1;
                pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice + 1;
                pIndices[IndexOfIndex ++] = (IndexOfStack    ) * Width + IndexOfSlice;
                
                pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice + 1;
                pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice;
                pIndices[IndexOfIndex ++] = (IndexOfStack    ) * Width + IndexOfSlice;
            }
        }
        
        for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width - 1; ++ IndexOfSlice)
        {
            pIndices[IndexOfIndex ++] = IndexOfSlice + 1;
            pIndices[IndexOfIndex ++] = IndexOfSlice;
            pIndices[IndexOfIndex ++] = (Height - 2) * Width;
            
            pIndices[IndexOfIndex ++] = (Height - 3) * Width + IndexOfSlice;
            pIndices[IndexOfIndex ++] = (Height - 3) * Width + IndexOfSlice + 1;
            pIndices[IndexOfIndex ++] = (Height - 2) * Width + 1;
        }
        
        assert(IndexOfIndex == NumberOfIndices);
        
        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key = 0;
        rSurface.m_SurfaceKey.m_HasPosition = 1;
        
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(Base::Float3) * NumberOfVertices;
        BufferDesc.m_pBytes        = pVertices;
        BufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ConePositionBuffer = BufferManager::CreateBuffer(BufferDesc);
        
        rSurface.m_VertexBuffer     = BufferManager::CreateVertexBufferSet(ConePositionBuffer);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_IndexBuffer     = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = NumberOfIndices;
        
        // -----------------------------------------------------------------------------
        // Remove allocated memory after uploading to buffer
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pVertices);
        Base::CMemory::Free(pIndices);
        
        return CModelPtr(ModelPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CModelPtr CGfxModelManager::CreateRectangle(float _X, float _Y, float _Width, float _Height)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CModels::CPtr ModelPtr = m_Models.Allocate();
        
        CInternModel& rModel = *ModelPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_LODs[0] = LODPtr;
        
        rLOD.m_Surfaces[0] = SurfacePtr;
        
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        assert(_Width != 0.0f && _Height != 0.0f);
        
        unsigned int NumberOfVertices = 4;
        unsigned int NumberOfIndices  = 6;
        
        Base::Float2* pVertices = static_cast<Base::Float2*>(Base::CMemory::Allocate(sizeof(Base::Float2) * NumberOfVertices));
        unsigned int* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
        
        // -----------------------------------------------------------------------------
        // Create vertices's of a cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfVertex = 0;
        
        pVertices[IndexOfVertex ++] = Base::Float2(_X         , _Y + _Height);
        pVertices[IndexOfVertex ++] = Base::Float2(_X + _Width, _Y +_Height);
        pVertices[IndexOfVertex ++] = Base::Float2(_X + _Width, _Y);
        pVertices[IndexOfVertex ++] = Base::Float2(_X         , _Y);
        
        assert(IndexOfVertex == NumberOfVertices);
        
        // -----------------------------------------------------------------------------
        // Create indices for currently created cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfIndex = 0;
        
        pIndices[IndexOfIndex ++] = 0;
        pIndices[IndexOfIndex ++] = 1;
        pIndices[IndexOfIndex ++] = 2;
        pIndices[IndexOfIndex ++] = 0;
        pIndices[IndexOfIndex ++] = 2;
        pIndices[IndexOfIndex ++] = 3;
        
        assert(IndexOfIndex == NumberOfIndices);
        
        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key = 0;
        rSurface.m_SurfaceKey.m_HasPosition = 1;
        
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(Base::Float2) * NumberOfVertices;
        BufferDesc.m_pBytes        = pVertices;
        BufferDesc.m_pClassKey     = 0;
        
        CBufferPtr RectanglePositionBuffer = BufferManager::CreateBuffer(BufferDesc);
        
        rSurface.m_VertexBuffer     = BufferManager::CreateVertexBufferSet(RectanglePositionBuffer);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_IndexBuffer     = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = NumberOfIndices;
        
        // -----------------------------------------------------------------------------
        // Remove allocated memory after uploading to buffer
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pVertices);
        Base::CMemory::Free(pIndices);
        
        return CModelPtr(ModelPtr);
    }
} // namespace

namespace Gfx
{
namespace ModelManager
{
    void OnStart()
    {
        CGfxModelManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxModelManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CGfxModelManager::GetInstance().Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CModelPtr CreateModel(const SModelDescriptor& _rDescriptor)
    {
        return CGfxModelManager::GetInstance().CreateModel(_rDescriptor);
    }
    
    // -----------------------------------------------------------------------------
    
    CModelPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices)
    {
        return CGfxModelManager::GetInstance().CreateSphere(_Radius, _Stacks, _Slices);
    }
    
    // -----------------------------------------------------------------------------
    
    CModelPtr CreateRectangle(float _X, float _Y, float _Width, float _Height)
    {
        return CGfxModelManager::GetInstance().CreateRectangle(_X, _Y, _Width, _Height);
    }
} // namespace ModelManager
} // namespace Gfx