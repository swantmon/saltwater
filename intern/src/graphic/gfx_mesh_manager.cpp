
#include "graphic/gfx_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_model.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_input_layout.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_shader_manager.h"

#include <unordered_map>
#include <functional>

using namespace Gfx;
using namespace Gfx::MeshManager;

// -----------------------------------------------------------------------------
// Here we define the some vertex shader:
// -----------------------------------------------------------------------------
namespace
{
    // -----------------------------------------------------------------------------
    // Define shader combinations
    // -----------------------------------------------------------------------------
    static const unsigned int s_NumberOfVertexShader = 3;

    // -----------------------------------------------------------------------------
    // Define all vertex shader needed inside this renderer
    // -----------------------------------------------------------------------------
    const Base::Char* g_pShaderFilenameVS[] =
    {
        "vs_m_pn.glsl",
        "vs_m_pnx0.glsl",
        "vs_m_pntbx0.glsl",
    };

    const Base::Char* g_pShaderNamesVS[] =
    {
        "VSShaderPN",
        "VSShaderPNX0",
        "VSShaderPNTBX0",
    };

    const Base::Char* g_pMVPShaderFilenameVS[] =
    {
        "vs_mvp_pn.glsl",
        "vs_mvp_pnx0.glsl",
        "vs_mvp_pntbx0.glsl",
    };

    const Base::Char* g_pMVPShaderNamesVS[] =
    {
        "VSMVPShaderPN",
        "VSMVPShaderPNX0",
        "VSMVPShaderPNTBX0",
    };

    // -----------------------------------------------------------------------------
    // Define input layouts depending on vertex shader
    // -----------------------------------------------------------------------------
    struct SInputElementDescriptorSetting
    {
        unsigned int m_Offset;
        unsigned int m_NumberOfElements;
    };

    const SInputElementDescriptorSetting g_InputLayoutDescriptor[] =
    {
        { 0, 2 },
        { 2, 3 },
        { 5, 5 },
    };

    const Gfx::SInputElementDescriptor g_InputLayouts[] =
    {
        // VSShaderBlankPN
        { "POSITION", 0, Gfx::CInputLayout::Float3Format, 0,  0, 24, Gfx::CInputLayout::PerVertex, 0 },
        { "NORMAL"  , 0, Gfx::CInputLayout::Float3Format, 0, 12, 24, Gfx::CInputLayout::PerVertex, 0 },
        // VSShaderDiffusePNX0
        { "POSITION", 0, Gfx::CInputLayout::Float3Format, 0,  0, 32, Gfx::CInputLayout::PerVertex, 0 },
        { "NORMAL"  , 0, Gfx::CInputLayout::Float3Format, 0, 12, 32, Gfx::CInputLayout::PerVertex, 0 },
        { "TEXCOORD", 0, Gfx::CInputLayout::Float2Format, 0, 24, 32, Gfx::CInputLayout::PerVertex, 0 },
        // VSShaderBumpedPNX0
        { "POSITION" , 0, Gfx::CInputLayout::Float3Format, 0,  0, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "NORMAL"   , 0, Gfx::CInputLayout::Float3Format, 0, 12, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "TANGENT"  , 0, Gfx::CInputLayout::Float3Format, 0, 24, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "BITANGENT", 0, Gfx::CInputLayout::Float3Format, 0, 36, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "TEXCOORD" , 0, Gfx::CInputLayout::Float2Format, 0, 48, 56, Gfx::CInputLayout::PerVertex, 0 },
    };

    // -----------------------------------------------------------------------------
    // Define shader combinations
    // -----------------------------------------------------------------------------
    const Gfx::CSurface::SSurfaceKey g_SurfaceCombinations[s_NumberOfVertexShader] =
    {
        // -----------------------------------------------------------------------------
        // 01. Attribute: HasPosition
        // 02. Attribute: HasNormal
        // 04. Attribute: HasTangent
        // 05. Attribute: HasBitangent
        // 06. Attribute: HasTexCoords
        // -----------------------------------------------------------------------------

        // 01  , 02   , 03   , 04   , 05
        { true, true, false, false, false },
        { true, true, false, false, true },
        { true, true, true,  true,  true },
    };
}

namespace
{
    class CGfxMeshManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxMeshManager)
        
    public:

        CGfxMeshManager();
        ~CGfxMeshManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();
        
        CMeshPtr CreateMesh(const SMeshDescriptor& _rDescriptor);
        CMeshPtr CreateBox(float _Width, float _Height, float _Depth);
        CMeshPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
        CMeshPtr CreateSphereIsometric(float _Radius, unsigned int _Refinement);
        CMeshPtr CreateCone(float _Radius, float _Height, unsigned int _Slices);
        CMeshPtr CreateRectangle(float _AxisX, float _AxisY, float _Width, float _Height);

    private:
        
        class CInternModel : public CMesh
        {
        private:
            
            friend class CGfxMeshManager;
        };
        
        class CInternLOD : public CLOD
        {
        private:
            
            friend class CGfxMeshManager;
        };
        
        class CInternSurface : public CSurface
        {
        private:
            
            friend class CGfxMeshManager;
        };
        
    private:
        
        typedef Base::CManagedPool<CInternModel   , 64  , 1> CModels;
        typedef Base::CManagedPool<CInternLOD     , 256 , 1> CLODs;
        typedef Base::CManagedPool<CInternSurface , 1024, 1> CSurfaces;
        
        typedef std::unordered_map<unsigned int, CInternModel*> CModelByIDs;
        typedef CModelByIDs::iterator                           CModelByIDPair;
        
    private:
        
        CModels    m_Meshes;
        CLODs      m_LODs;
        CSurfaces  m_Surfaces;
        
        CModelByIDs m_ModelByID;

    private:

        void SetVertexShaderOfSurface(CInternSurface& _rSurface);
    };
} // namespace

namespace
{
    CGfxMeshManager::CGfxMeshManager()
        : m_Meshes   ()
        , m_LODs     ()
        , m_Surfaces ()
        , m_ModelByID()
    {
        m_ModelByID.reserve(64);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxMeshManager::~CGfxMeshManager()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMeshManager::OnStart()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMeshManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshManager::Clear()
    {
        m_Meshes   .Clear();
        m_LODs     .Clear();
        m_Surfaces .Clear();

        m_ModelByID.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMeshPtr CGfxMeshManager::CreateMesh(const Gfx::SMeshDescriptor& _rDescriptor)
    {
        Dt::CMesh& rDataModel = *_rDescriptor.m_pMesh;
        
        // -----------------------------------------------------------------------------
        // Check existing model
        // -----------------------------------------------------------------------------
        unsigned int Hash = 0;
        
        if (rDataModel.GetMeshname().length() > 0)
        {
            Hash = Base::CRC32(rDataModel.GetMeshname().c_str(), static_cast<unsigned int>(rDataModel.GetMeshname().length()));
            
            if (m_ModelByID.find(Hash) != m_ModelByID.end())
            {
                return CMeshPtr(m_ModelByID.at(Hash));
            }
        }
        
        // -----------------------------------------------------------------------------
        // Create model
        // -----------------------------------------------------------------------------
        CModels::CPtr ModelPtr = m_Meshes.Allocate();
        
        CInternModel& rModel = *ModelPtr;
        
        rModel.m_NumberOfLODs = rDataModel.GetNumberOfLODs();
        
        // -----------------------------------------------------------------------------
        // For every LOD we have to create everything (data, material, informations,
        // ...)
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfLOD = 0; IndexOfLOD < rDataModel.GetNumberOfLODs(); ++IndexOfLOD)
        {
            Dt::CLOD& rCurrentLOD = *rDataModel.GetLOD(IndexOfLOD);
            
            CLODs::CPtr LODPtr = m_LODs.Allocate();
            
            CInternLOD& rLOD = *LODPtr;
            
            rLOD.m_NumberOfSurfaces = rCurrentLOD.GetNumberOfSurfaces();
            
            // -----------------------------------------------------------------------------
            // For every surface in model create material and surface informations
            // -----------------------------------------------------------------------------
            for (unsigned int IndexOfSurface = 0; IndexOfSurface < rCurrentLOD.GetNumberOfSurfaces(); ++IndexOfSurface)
            {
                // -----------------------------------------------------------------------------
                // Create surface depending on data
                // -----------------------------------------------------------------------------
                Dt::CSurface& rCurrentSurface = *rCurrentLOD.GetSurface(IndexOfSurface);
                
                CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
                
                CInternSurface& rSurface = *SurfacePtr;
                
                // -----------------------------------------------------------------------------
                // Surface attributes
                // -----------------------------------------------------------------------------
                rSurface.m_SurfaceKey.m_HasPosition  = true;
                rSurface.m_SurfaceKey.m_HasNormal    = ((rCurrentSurface.GetElements() & Dt::CSurface::Normal)     == Dt::CSurface::Normal);
                rSurface.m_SurfaceKey.m_HasTangent   = ((rCurrentSurface.GetElements() & Dt::CSurface::Tangent)    == Dt::CSurface::Tangent);
                rSurface.m_SurfaceKey.m_HasBitangent = ((rCurrentSurface.GetElements() & Dt::CSurface::Tangent)    == Dt::CSurface::Tangent);
                rSurface.m_SurfaceKey.m_HasTexCoords = ((rCurrentSurface.GetElements() & Dt::CSurface::TexCoord0)  == Dt::CSurface::TexCoord0);

                // -----------------------------------------------------------------------------
                // Set vertex shader
                // -----------------------------------------------------------------------------
                SetVertexShaderOfSurface(rSurface);

                // -----------------------------------------------------------------------------
                // Prepare mesh data for data alignment
                // -----------------------------------------------------------------------------
                unsigned int NumberOfVertices           = rCurrentSurface.GetNumberOfVertices() * rSurface.m_SurfaceKey.m_HasPosition;
                unsigned int NumberOfNormals            = rCurrentSurface.GetNumberOfVertices() * rSurface.m_SurfaceKey.m_HasNormal;
                unsigned int NumberOfTagents            = rCurrentSurface.GetNumberOfVertices() * rSurface.m_SurfaceKey.m_HasTangent;
                unsigned int NumberOfBitangents         = rCurrentSurface.GetNumberOfVertices() * rSurface.m_SurfaceKey.m_HasBitangent;
                unsigned int NumberOfTexCoords          = rCurrentSurface.GetNumberOfVertices() * (rSurface.m_SurfaceKey.m_HasTexCoords >= 1);
                unsigned int NumberOfVerticeElements    = 3 * rSurface.m_SurfaceKey.m_HasPosition;
                unsigned int NumberOfNormalElements     = 3 * rSurface.m_SurfaceKey.m_HasNormal;
                unsigned int NumberOfTagentsElements    = 3 * rSurface.m_SurfaceKey.m_HasTangent;
                unsigned int NumberOfBitangentsElements = 3 * rSurface.m_SurfaceKey.m_HasBitangent;
                unsigned int NumberOfTexCoordElements   = 2 * (rSurface.m_SurfaceKey.m_HasTexCoords >= 1);
                unsigned int NumberOfVertexElements     = NumberOfVertices * NumberOfVerticeElements + NumberOfNormals * NumberOfNormalElements + NumberOfTagents * NumberOfTagentsElements + NumberOfBitangents * NumberOfBitangentsElements + NumberOfTexCoords * NumberOfTexCoordElements;
                unsigned int NumberOfElementsPerVertex  = NumberOfVerticeElements + NumberOfNormalElements + NumberOfTagentsElements + NumberOfBitangentsElements + NumberOfTexCoordElements;
                unsigned int NumberOfIndices            = rCurrentSurface.GetNumberOfIndices();
                
                // -----------------------------------------------------------------------------
                // Prepare upload data of vertices
                // -----------------------------------------------------------------------------
                unsigned int* pUploadIndexData = static_cast<unsigned int* >(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
                float* pUploadVertexData = static_cast<float* >(Base::CMemory::Allocate(sizeof(float) * NumberOfVertexElements));

                // -----------------------------------------------------------------------------
                // Get data from data model
                // -----------------------------------------------------------------------------
                const glm::vec3* pPosition   = rCurrentSurface.GetPositions();
                const glm::vec3* pNormals    = rCurrentSurface.GetNormals();
                const glm::vec3* pTangents   = rCurrentSurface.GetTangents();
                const glm::vec3* pBitangents = rCurrentSurface.GetBitangents();
                const glm::vec2* pTexCoords  = rCurrentSurface.GetTexCoords();
                const unsigned int* pIndices    = rCurrentSurface.GetIndices();

                // -----------------------------------------------------------------------------
                // Prepare AABB
                // -----------------------------------------------------------------------------
                glm::vec3 StartPosition(pPosition[0][0], pPosition[0][1], pPosition[0][2]);

                rModel.m_AABB.SetMin(StartPosition);
                rModel.m_AABB.SetMax(StartPosition);

                // -----------------------------------------------------------------------------
                // Copy indices
                // -----------------------------------------------------------------------------
                Base::CMemory::Copy(pUploadIndexData, pIndices, sizeof(unsigned int) * NumberOfIndices);
                
                // -----------------------------------------------------------------------------
                // Iterate throw every vertex and put it into upload data
                // -----------------------------------------------------------------------------
                for (unsigned int CurrentVertex = 0; CurrentVertex < NumberOfVertices; ++CurrentVertex)
                {
                    unsigned int CurrentAlignIndex = CurrentVertex * NumberOfElementsPerVertex;
                    
                    glm::vec3 CurrentPosition(pPosition[CurrentVertex][0], pPosition[CurrentVertex][1], pPosition[CurrentVertex][2]);
                    
                    rModel.m_AABB.Extend(CurrentPosition);
                    
                    pUploadVertexData[CurrentAlignIndex + 0] = pPosition[CurrentVertex][0];
                    pUploadVertexData[CurrentAlignIndex + 1] = pPosition[CurrentVertex][1];
                    pUploadVertexData[CurrentAlignIndex + 2] = pPosition[CurrentVertex][2];
                    
                    CurrentAlignIndex += 3;
                    
                    if (rSurface.m_SurfaceKey.m_HasNormal)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = pNormals[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = pNormals[CurrentVertex][1];
                        pUploadVertexData[CurrentAlignIndex + 2] = pNormals[CurrentVertex][2];
                        
                        CurrentAlignIndex += 3;
                    }
                    
                    if (rSurface.m_SurfaceKey.m_HasTangent)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = pTangents[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = pTangents[CurrentVertex][1];
                        pUploadVertexData[CurrentAlignIndex + 2] = pTangents[CurrentVertex][2];
                        
                        CurrentAlignIndex += 3;
                    }
                    
                    if (rSurface.m_SurfaceKey.m_HasBitangent)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = pBitangents[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = pBitangents[CurrentVertex][1];
                        pUploadVertexData[CurrentAlignIndex + 2] = pBitangents[CurrentVertex][2];
                        
                        CurrentAlignIndex += 3;
                    }
                    
                    if (rSurface.m_SurfaceKey.m_HasTexCoords >= 1)
                    {
                        pUploadVertexData[CurrentAlignIndex + 0] = pTexCoords[CurrentVertex][0];
                        pUploadVertexData[CurrentAlignIndex + 1] = pTexCoords[CurrentVertex][1];
                        
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
                
                rSurface.m_VertexBufferPtr = BufferManager::CreateBuffer(VertexBufferDesc);
                
                // -----------------------------------------------------------------------------
                
                SBufferDescriptor IndexBufferDesc;
                
                IndexBufferDesc.m_Stride        = 0;
                IndexBufferDesc.m_Usage         = CBuffer::GPURead;
                IndexBufferDesc.m_Binding       = CBuffer::IndexBuffer;
                IndexBufferDesc.m_Access        = CBuffer::CPUWrite;
                IndexBufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
                IndexBufferDesc.m_pBytes        = pUploadIndexData;
                IndexBufferDesc.m_pClassKey     = 0;
                
                rSurface.m_IndexBufferPtr = BufferManager::CreateBuffer(IndexBufferDesc);
                
                // -----------------------------------------------------------------------------
                // Set last data information of the surface
                // -----------------------------------------------------------------------------
                rSurface.m_NumberOfVertices = NumberOfVertices;
                rSurface.m_NumberOfIndices  = NumberOfIndices;
                
                // -----------------------------------------------------------------------------
                // Delete allocated memory
                // -----------------------------------------------------------------------------
                Base::CMemory::Free(pUploadIndexData);
                Base::CMemory::Free(pUploadVertexData);
                
                // -----------------------------------------------------------------------------
                // Materials
                // -----------------------------------------------------------------------------
                if (rCurrentSurface.GetMaterial())
                {
                    unsigned int MaterialHash = rCurrentSurface.GetMaterial()->GetHash();
                    
                    rSurface.m_MaterialPtr = Gfx::MaterialManager::GetMaterialByHash(MaterialHash);
                }
                else
                {
                    rSurface.m_MaterialPtr = Gfx::MaterialManager::GetDefaultMaterial();
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
        
        return CMeshPtr(ModelPtr);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CGfxMeshManager::CreateBox(float _Width, float _Height, float _Depth)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CModels::CPtr ModelPtr = m_Meshes.Allocate();

        CInternModel& rModel = *ModelPtr;

        CLODs::CPtr LODPtr = m_LODs.Allocate();

        CInternLOD& rLOD = *LODPtr;

        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();

        CInternSurface& rSurface = *SurfacePtr;

        rSurface.m_MaterialPtr = nullptr;

        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_NumberOfSurfaces = 1;
        rLOD.m_Surfaces[0] = SurfacePtr;

        // -----------------------------------------------------------------------------
        // Prepare surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key          = 0;
        rSurface.m_SurfaceKey.m_HasPosition  = true;
        rSurface.m_SurfaceKey.m_HasNormal    = true;
        rSurface.m_SurfaceKey.m_HasTangent   = false;
        rSurface.m_SurfaceKey.m_HasBitangent = false;
        rSurface.m_SurfaceKey.m_HasTexCoords = false;

        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        float HalfWidth  = _Width  / 2.0f;
        float HalfHeight = _Height / 2.0f;
        float HalfDepth  = _Depth  / 2.0f;

        unsigned int NumberOfVertices = 8;
        unsigned int NumberOfIndices = 36;

        glm::vec3* pVertices = static_cast<glm::vec3*>(Base::CMemory::Allocate(sizeof(glm::vec3) * NumberOfVertices * 2));
        unsigned int* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));

        assert(pVertices);
        assert(pIndices);

        // -----------------------------------------------------------------------------
        // Create vertices's for a box
        // -----------------------------------------------------------------------------
        pVertices[0][0] = -HalfWidth;
        pVertices[0][1] = -HalfDepth;
        pVertices[0][2] = -HalfHeight;

        pVertices[1][0] = -HalfWidth;
        pVertices[1][1] = -HalfDepth;
        pVertices[1][2] = -HalfHeight;

        pVertices[2][0] = -HalfWidth;
        pVertices[2][1] = -HalfDepth;
        pVertices[2][2] =  HalfHeight;

        pVertices[3][0] = -HalfWidth;
        pVertices[3][1] = -HalfDepth;
        pVertices[3][2] =  HalfHeight;

        pVertices[4][0] =  HalfWidth;
        pVertices[4][1] = -HalfDepth;
        pVertices[4][2] =  HalfHeight;

        pVertices[5][0] =  HalfWidth;
        pVertices[5][1] = -HalfDepth;
        pVertices[5][2] =  HalfHeight;

        pVertices[6][0] =  HalfWidth;
        pVertices[6][1] = -HalfDepth;
        pVertices[6][2] = -HalfHeight;

        pVertices[7][0] =  HalfWidth;
        pVertices[7][1] = -HalfDepth;
        pVertices[7][2] = -HalfHeight;

        pVertices[8][0] = -HalfWidth;
        pVertices[8][1] =  HalfDepth;
        pVertices[8][2] = -HalfHeight;

        pVertices[9][0] = -HalfWidth;
        pVertices[9][1] =  HalfDepth;
        pVertices[9][2] = -HalfHeight;

        pVertices[10][0] = -HalfWidth;
        pVertices[10][1] =  HalfDepth;
        pVertices[10][2] =  HalfHeight;

        pVertices[11][0] = -HalfWidth;
        pVertices[11][1] =  HalfDepth;
        pVertices[11][2] =  HalfHeight;

        pVertices[12][0] = HalfWidth;
        pVertices[12][1] = HalfDepth;
        pVertices[12][2] = HalfHeight;

        pVertices[13][0] = HalfWidth;
        pVertices[13][1] = HalfDepth;
        pVertices[13][2] = HalfHeight;

        pVertices[14][0] =  HalfWidth;
        pVertices[14][1] =  HalfDepth;
        pVertices[14][2] = -HalfHeight;

        pVertices[15][0] =  HalfWidth;
        pVertices[15][1] =  HalfDepth;
        pVertices[15][2] = -HalfHeight;

        // -----------------------------------------------------------------------------
        // Create indices of box
        // -----------------------------------------------------------------------------
        pIndices[0] = 4; pIndices[1] = 0; pIndices[2] = 3;
        pIndices[3] = 3; pIndices[4] = 7; pIndices[5] = 4;

        pIndices[6] = 1; pIndices[7]  = 0; pIndices[8]  = 4;
        pIndices[9] = 4; pIndices[10] = 5; pIndices[11] = 1;

        pIndices[12] = 3; pIndices[13] = 2; pIndices[14] = 6;
        pIndices[15] = 6; pIndices[16] = 7; pIndices[17] = 3;

        pIndices[18] = 1; pIndices[19] = 5; pIndices[20] = 6;
        pIndices[21] = 6; pIndices[22] = 2; pIndices[23] = 1;

        pIndices[24] = 4; pIndices[25] = 7; pIndices[26] = 6;
        pIndices[27] = 6; pIndices[28] = 5; pIndices[29] = 4;

        pIndices[30] = 0; pIndices[31] = 1; pIndices[32] = 3;
        pIndices[33] = 3; pIndices[34] = 1; pIndices[35] = 2;

        
        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(glm::vec3) * NumberOfVertices * 2;
        BufferDesc.m_pBytes        = pVertices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_IndexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = NumberOfIndices;

        // -----------------------------------------------------------------------------
        // Vertex Shader
        // -----------------------------------------------------------------------------
        SetVertexShaderOfSurface(rSurface);

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        rSurface.m_MaterialPtr = Gfx::MaterialManager::GetDefaultMaterial();
        
        // -----------------------------------------------------------------------------
        // Remove allocated memory after uploading to buffer
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pVertices);
        Base::CMemory::Free(pIndices);
        
        return CMeshPtr(ModelPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CMeshPtr CGfxMeshManager::CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CModels::CPtr MeshPtr = m_Meshes.Allocate();
        
        CInternModel& rModel = *MeshPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_NumberOfSurfaces = 1;
        rLOD.m_Surfaces[0] = SurfacePtr;

        // -----------------------------------------------------------------------------
        // Prepare surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key          = 0;
        rSurface.m_SurfaceKey.m_HasPosition  = true;
        rSurface.m_SurfaceKey.m_HasNormal    = true;
        rSurface.m_SurfaceKey.m_HasTangent   = false;
        rSurface.m_SurfaceKey.m_HasBitangent = false;
        rSurface.m_SurfaceKey.m_HasTexCoords = false;
        
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        unsigned int Width  = _Slices;
        unsigned int Height = _Stacks;
        
        unsigned int NumberOfVertices = (Height - 2) * Width + 2;
        unsigned int NumberOfIndices  = ((Height - 2) * (Width - 1) * 2) * 3;
        
        glm::vec3* pVertices = static_cast<glm::vec3*>(Base::CMemory::Allocate(sizeof(glm::vec3) * NumberOfVertices * 2));
        unsigned int* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
        
        assert(pVertices);
        assert(pIndices);
        
        // -----------------------------------------------------------------------------
        // Create vertices's for a sphere
        // -----------------------------------------------------------------------------
        unsigned int IndexOfVertex = 0;
        
        for(unsigned int IndexOfStack = 1; IndexOfStack < Height - 1; ++ IndexOfStack)
        {
            for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width; ++ IndexOfSlice)
            {
                float THETA = static_cast<float>(IndexOfStack) / static_cast<float>(Height - 1) * glm::pi<float>();
                float PHI   = static_cast<float>(IndexOfSlice) / static_cast<float>(Width  - 1) * glm::pi<float>() * 2.0f;
                
                pVertices[IndexOfVertex][0] =  glm::sin(THETA) * glm::cos(PHI) * _Radius;
                pVertices[IndexOfVertex][1] =  glm::cos(THETA) * _Radius;
                pVertices[IndexOfVertex][2] = -glm::sin(THETA) * glm::sin(PHI) * _Radius;

                ++IndexOfVertex;

                pVertices[IndexOfVertex][0] =  glm::sin(THETA) * glm::cos(PHI) * _Radius;
                pVertices[IndexOfVertex][1] =  glm::cos(THETA) * _Radius;
                pVertices[IndexOfVertex][2] = -glm::sin(THETA) * glm::sin(PHI) * _Radius;
                
                ++ IndexOfVertex;
            }
        }
        
        pVertices[IndexOfVertex][0] = 0.0f;
        pVertices[IndexOfVertex][1] = _Radius;
        pVertices[IndexOfVertex][2] = 0.0f;

        ++IndexOfVertex;

        pVertices[IndexOfVertex][0] = 0.0f;
        pVertices[IndexOfVertex][1] = _Radius;
        pVertices[IndexOfVertex][2] = 0.0f;
        
        ++ IndexOfVertex;
        
        pVertices[IndexOfVertex][0] = 0.0f;
        pVertices[IndexOfVertex][1] = - _Radius;
        pVertices[IndexOfVertex][2] = 0.0f;

        ++IndexOfVertex;

        pVertices[IndexOfVertex][0] = 0.0f;
        pVertices[IndexOfVertex][1] = -_Radius;
        pVertices[IndexOfVertex][2] = 0.0f;
        
        assert((IndexOfVertex + 1) == NumberOfVertices * 2);
        
        // -----------------------------------------------------------------------------
        // Create indices of sphere
        // -----------------------------------------------------------------------------
        unsigned int IndexOfIndex = 0;
        
        for(unsigned int IndexOfStack = 0; IndexOfStack < Height - 3; ++ IndexOfStack)
        {
            for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width - 1; ++ IndexOfSlice )
            {
                pIndices[IndexOfIndex ++] = (IndexOfStack)     * Width + IndexOfSlice;
                pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice + 1;
                pIndices[IndexOfIndex ++] = (IndexOfStack)     * Width + IndexOfSlice + 1;
                
                pIndices[IndexOfIndex ++] = (IndexOfStack)     * Width + IndexOfSlice;
                pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice;
                pIndices[IndexOfIndex ++] = (IndexOfStack + 1) * Width + IndexOfSlice + 1;
            }
        }
        
        for(unsigned int IndexOfSlice = 0; IndexOfSlice < Width - 1; ++ IndexOfSlice)
        {
            pIndices[IndexOfIndex ++] = (Height - 2) * Width;
            pIndices[IndexOfIndex ++] = IndexOfSlice;
            pIndices[IndexOfIndex ++] = IndexOfSlice + 1;
            
            pIndices[IndexOfIndex ++] = (Height - 2) * Width + 1;
            pIndices[IndexOfIndex ++] = (Height - 3) * Width + IndexOfSlice + 1;
            pIndices[IndexOfIndex ++] = (Height - 3) * Width + IndexOfSlice;
        }
        
        assert(IndexOfIndex == NumberOfIndices);
        
        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------        
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(glm::vec3) * NumberOfVertices * 2;
        BufferDesc.m_pBytes        = pVertices;
        BufferDesc.m_pClassKey     = 0;

        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_IndexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = NumberOfIndices;

        // -----------------------------------------------------------------------------
        // Vertex Shader
        // -----------------------------------------------------------------------------
        SetVertexShaderOfSurface(rSurface);

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        rSurface.m_MaterialPtr = Gfx::MaterialManager::GetDefaultMaterial();

        // -----------------------------------------------------------------------------
        // Remove allocated memory after uploading to buffer
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pVertices);
        Base::CMemory::Free(pIndices);
        
        return CMeshPtr(MeshPtr);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CGfxMeshManager::CreateSphereIsometric(float _Radius, unsigned int _Refinement)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CModels::CPtr MeshPtr = m_Meshes.Allocate();
        
        CInternModel& rModel = *MeshPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_NumberOfSurfaces = 1;
        rLOD.m_Surfaces[0] = SurfacePtr;

        // -----------------------------------------------------------------------------
        // Prepare surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key          = 0;
        rSurface.m_SurfaceKey.m_HasPosition  = true;
        rSurface.m_SurfaceKey.m_HasNormal    = true;
        rSurface.m_SurfaceKey.m_HasTangent   = false;
        rSurface.m_SurfaceKey.m_HasBitangent = false;
        rSurface.m_SurfaceKey.m_HasTexCoords = false;
        
        // -----------------------------------------------------------------------------
        // Calculate data
        // -----------------------------------------------------------------------------
        std::vector<glm::vec3> LightVertices;
        std::vector<glm::uvec3>  LightTriangles;

        // Create a simple tetrahedron
        LightVertices.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));
        LightVertices.push_back(glm::vec3(1.0f, -1.0f, 1.0f));
        LightVertices.push_back(glm::vec3(-1.0f, 1.0f, 1.0f));
        LightVertices.push_back(glm::vec3(1.0f, 1.0f, -1.0f));

        LightTriangles.push_back(glm::uvec3(0, 3, 2));
        LightTriangles.push_back(glm::uvec3(0, 1, 3));
        LightTriangles.push_back(glm::uvec3(1, 0, 2));
        LightTriangles.push_back(glm::uvec3(1, 2, 3));

        // Split the faces of the tetrahedron
        for (unsigned int RefinementStep = 0; RefinementStep < _Refinement; ++RefinementStep)
        {
            std::vector<glm::vec3> Vertices;
            std::vector<glm::uvec3>  Triangles;

            unsigned int NextVertexIndex = 0;

            for (unsigned int IndexOfTriangle = 0; IndexOfTriangle < LightTriangles.size(); ++IndexOfTriangle)
            {
                glm::vec3 NewVertices[6];

                for (unsigned int IndexOfVertex = 0; IndexOfVertex < 3; ++IndexOfVertex)
                {
                    NewVertices[IndexOfVertex] = LightVertices[LightTriangles[IndexOfTriangle][IndexOfVertex]];
                }

                glm::vec3 StartVertex, EndVertex;

                for (unsigned int IndexOfVertex = 0; IndexOfVertex < 3; ++IndexOfVertex)
                {
                    StartVertex  = NewVertices[IndexOfVertex];
                    EndVertex    = NewVertices[(IndexOfVertex + 1) % 3];

                    NewVertices[IndexOfVertex + 3] = (StartVertex + EndVertex) / 2.0f;
                }

                for (unsigned int IndexOfVertex = 0; IndexOfVertex < 6; ++IndexOfVertex)
                {
                    Vertices.push_back(NewVertices[IndexOfVertex]);
                }

                Triangles.push_back(glm::uvec3(NextVertexIndex + 0, NextVertexIndex + 3, NextVertexIndex + 5));
                Triangles.push_back(glm::uvec3(NextVertexIndex + 3, NextVertexIndex + 1, NextVertexIndex + 4));
                Triangles.push_back(glm::uvec3(NextVertexIndex + 3, NextVertexIndex + 4, NextVertexIndex + 5));
                Triangles.push_back(glm::uvec3(NextVertexIndex + 5, NextVertexIndex + 4, NextVertexIndex + 2));

                NextVertexIndex += 6;
            }

            LightVertices  = std::move(Vertices);
            LightTriangles = std::move(Triangles);
        }

        // We want a unit sphere
        std::vector<glm::vec3> VerticesNormal;

        for (unsigned int IndexOfVertex = 0; IndexOfVertex < LightVertices.size(); ++IndexOfVertex)
        {
            glm::vec3 Vertex = glm::normalize(LightVertices[IndexOfVertex]);

            VerticesNormal.push_back(Vertex * _Radius);
            VerticesNormal.push_back(Vertex);
        }

        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------        
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(VerticesNormal[0]) * static_cast<unsigned int>(VerticesNormal.size());
        BufferDesc.m_pBytes        = VerticesNormal.data();
        BufferDesc.m_pClassKey     = 0;

        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = static_cast<unsigned int>(VerticesNormal.size()) / 2;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(LightTriangles[0]) * static_cast<unsigned int>(LightTriangles.size());
        BufferDesc.m_pBytes        = LightTriangles.data();
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_IndexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = static_cast<unsigned int>(LightTriangles.size()) * 3;

        // -----------------------------------------------------------------------------
        // Vertex Shader
        // -----------------------------------------------------------------------------
        SetVertexShaderOfSurface(rSurface);

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        rSurface.m_MaterialPtr = Gfx::MaterialManager::GetDefaultMaterial();

        return CMeshPtr(MeshPtr);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CGfxMeshManager::CreateCone(float _Radius, float _Height, unsigned int _Slices)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CModels::CPtr ModelPtr = m_Meshes.Allocate();

        CInternModel& rModel = *ModelPtr;

        CLODs::CPtr LODPtr = m_LODs.Allocate();

        CInternLOD& rLOD = *LODPtr;

        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();

        CInternSurface& rSurface = *SurfacePtr;

        rSurface.m_MaterialPtr = nullptr;

        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_NumberOfSurfaces = 1;
        rLOD.m_Surfaces[0] = SurfacePtr;

        // -----------------------------------------------------------------------------
        // Prepare surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key          = 0;
        rSurface.m_SurfaceKey.m_HasPosition  = true;
        rSurface.m_SurfaceKey.m_HasNormal    = true;
        rSurface.m_SurfaceKey.m_HasTangent   = false;
        rSurface.m_SurfaceKey.m_HasBitangent = false;
        rSurface.m_SurfaceKey.m_HasTexCoords = false;

        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        assert(_Slices >= 3);

        unsigned int NumberOfVertices = 3 + _Slices;
        unsigned int NumberOfIndices = _Slices * 6;

        glm::vec3* pVertices = static_cast<glm::vec3*>(Base::CMemory::Allocate(sizeof(glm::vec3) * NumberOfVertices * 2));
        unsigned int* pIndices = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));

        assert(pVertices);
        assert(pIndices);

        // -----------------------------------------------------------------------------
        // Create vertices's of a cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfVertex = 0;

        pVertices[IndexOfVertex++] = glm::vec3(0.0f, 0.0f,  _Height / 2.0f);
        pVertices[IndexOfVertex++] = glm::vec3(0.0f, 0.0f,  _Height / 2.0f);
        pVertices[IndexOfVertex++] = glm::vec3(0.0f, 0.0f, -_Height / 2.0f);
        pVertices[IndexOfVertex++] = glm::vec3(0.0f, 0.0f, -_Height / 2.0f);

        for (unsigned int IndexOfSlice = 0; IndexOfSlice < _Slices + 1; ++IndexOfSlice)
        {
            assert(IndexOfVertex < NumberOfVertices);

            float PHI = static_cast<float>(IndexOfSlice) / (_Slices)* glm::pi<float>() * 2.0f;

            pVertices[IndexOfVertex][0] = glm::cos(PHI) * _Radius;
            pVertices[IndexOfVertex][1] = glm::sin(PHI) * _Radius;
            pVertices[IndexOfVertex][2] = -_Height / 2.0f;

            ++IndexOfVertex;

            pVertices[IndexOfVertex][0] = glm::cos(PHI) * _Radius;
            pVertices[IndexOfVertex][1] = glm::sin(PHI) * _Radius;
            pVertices[IndexOfVertex][2] = -_Height / 2.0f;

            ++IndexOfVertex;
        }

        // -----------------------------------------------------------------------------
        // This have to be done, to avoid numerical imprecisions (double rendering)
        // -----------------------------------------------------------------------------
        pVertices[IndexOfVertex - 1] = pVertices[2];

        // -----------------------------------------------------------------------------
        // Create indices for currently created cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfIndex = 0;

        for (unsigned int IndexOfSlice = 0; IndexOfSlice < _Slices; ++IndexOfSlice)
        {
            pIndices[IndexOfIndex++] = IndexOfSlice + 3;
            pIndices[IndexOfIndex++] = IndexOfSlice + 2;
            pIndices[IndexOfIndex++] = 0;
        }

        for (unsigned int IndexOfSlice = 0; IndexOfSlice < _Slices; ++IndexOfSlice)
        {
            pIndices[IndexOfIndex++] = IndexOfSlice + 2;
            pIndices[IndexOfIndex++] = IndexOfSlice + 3;
            pIndices[IndexOfIndex++] = 1;

            assert(IndexOfSlice + 3 < NumberOfVertices);
        }

        assert(IndexOfIndex == NumberOfIndices);

        
        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(glm::vec3) * NumberOfVertices * 2;
        BufferDesc.m_pBytes        = pVertices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_IndexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = NumberOfIndices;

        // -----------------------------------------------------------------------------
        // Vertex Shader
        // -----------------------------------------------------------------------------
        SetVertexShaderOfSurface(rSurface);

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        rSurface.m_MaterialPtr = Gfx::MaterialManager::GetDefaultMaterial();
        
        // -----------------------------------------------------------------------------
        // Remove allocated memory after uploading to buffer
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pVertices);
        Base::CMemory::Free(pIndices);
        
        return CMeshPtr(ModelPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CMeshPtr CGfxMeshManager::CreateRectangle(float _AxisX, float _AxisY, float _Width, float _Height)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CModels::CPtr ModelPtr = m_Meshes.Allocate();
        
        CInternModel& rModel = *ModelPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_NumberOfSurfaces = 1;
        rLOD.m_Surfaces[0] = SurfacePtr;

        // -----------------------------------------------------------------------------
        // Prepare surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key          = 0;
        rSurface.m_SurfaceKey.m_HasPosition  = true;
        rSurface.m_SurfaceKey.m_HasNormal    = false;
        rSurface.m_SurfaceKey.m_HasTangent   = false;
        rSurface.m_SurfaceKey.m_HasBitangent = false;
        rSurface.m_SurfaceKey.m_HasTexCoords = false;
        
        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        assert(_Width != 0.0f && _Height != 0.0f);
        
        unsigned int NumberOfVertices = 4;
        unsigned int NumberOfIndices  = 6;
        
        glm::vec2* pVertices = static_cast<glm::vec2*>(Base::CMemory::Allocate(sizeof(glm::vec2) * NumberOfVertices));
        unsigned int* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
        
        // -----------------------------------------------------------------------------
        // Create vertices's of a cone.
        // -----------------------------------------------------------------------------
        unsigned int IndexOfVertex = 0;
        
        pVertices[IndexOfVertex ++] = glm::vec2(_AxisX         , _AxisY + _Height);
        pVertices[IndexOfVertex ++] = glm::vec2(_AxisX + _Width, _AxisY +_Height);
        pVertices[IndexOfVertex ++] = glm::vec2(_AxisX + _Width, _AxisY);
        pVertices[IndexOfVertex ++] = glm::vec2(_AxisX         , _AxisY);
        
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
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(glm::vec2) * NumberOfVertices;
        BufferDesc.m_pBytes        = pVertices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = 0;
        
        rSurface.m_IndexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = NumberOfIndices;
        
        // -----------------------------------------------------------------------------
        // Remove allocated memory after uploading to buffer
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pVertices);
        Base::CMemory::Free(pIndices);
        
        return CMeshPtr(ModelPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshManager::SetVertexShaderOfSurface(CInternSurface& _rSurface)
    {
        unsigned int ShaderLinkIndex = 0;

        // -----------------------------------------------------------------------------
        // Try to find the right shader for that surface
        // If no shader was found we use a blank shader with pink color.
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfShader = 0; IndexOfShader < s_NumberOfVertexShader; ++IndexOfShader)
        {
            Gfx::CSurface::SSurfaceKey::BSurfaceID TempMostReliableKey = g_SurfaceCombinations[IndexOfShader].m_Key & _rSurface.m_SurfaceKey.m_Key;

            if (g_SurfaceCombinations[IndexOfShader].m_Key == TempMostReliableKey)
            {
                ShaderLinkIndex = IndexOfShader;
            }
        }

        // -----------------------------------------------------------------------------
        // Now we get the index of the real vertex and pixel shader
        // -----------------------------------------------------------------------------
        const unsigned int VSIndex = ShaderLinkIndex;

        // -----------------------------------------------------------------------------
        // Compile shader
        // -----------------------------------------------------------------------------
        Gfx::CShaderPtr VSShader = Gfx::ShaderManager::CompileVS(g_pShaderFilenameVS[VSIndex], g_pShaderNamesVS[VSIndex]);

        Gfx::CShaderPtr VSMVPShader = Gfx::ShaderManager::CompileVS(g_pMVPShaderFilenameVS[VSIndex], g_pMVPShaderNamesVS[VSIndex]);

        assert(VSShader != 0 && VSMVPShader != 0);

        // -----------------------------------------------------------------------------
        // Set input layout if vertex shader has no input layout
        // -----------------------------------------------------------------------------
        if (VSShader->GetInputLayout() == nullptr)
        {
            SInputElementDescriptorSetting InputLayoutDesc = g_InputLayoutDescriptor[VSIndex];

            Gfx::SInputElementDescriptor* pDescriptor = static_cast<Gfx::SInputElementDescriptor*>(Base::CMemory::Allocate(sizeof(Gfx::SInputElementDescriptor) * InputLayoutDesc.m_NumberOfElements));

            unsigned int IndexOfRenderInputDesc = InputLayoutDesc.m_Offset;

            for (unsigned int IndexOfElement = 0; IndexOfElement < InputLayoutDesc.m_NumberOfElements; ++IndexOfElement)
            {
                pDescriptor[IndexOfElement] = g_InputLayouts[IndexOfRenderInputDesc];

                ++IndexOfRenderInputDesc;
            }

            Gfx::ShaderManager::CreateInputLayout(pDescriptor, InputLayoutDesc.m_NumberOfElements, VSShader);

            Base::CMemory::Free(pDescriptor);
        }

        if (VSMVPShader->GetInputLayout() == nullptr)
        {
            SInputElementDescriptorSetting InputLayoutDesc = g_InputLayoutDescriptor[VSIndex];

            Gfx::SInputElementDescriptor* pDescriptor = static_cast<Gfx::SInputElementDescriptor*>(Base::CMemory::Allocate(sizeof(Gfx::SInputElementDescriptor) * InputLayoutDesc.m_NumberOfElements));

            unsigned int IndexOfRenderInputDesc = InputLayoutDesc.m_Offset;

            for (unsigned int IndexOfElement = 0; IndexOfElement < InputLayoutDesc.m_NumberOfElements; ++IndexOfElement)
            {
                pDescriptor[IndexOfElement] = g_InputLayouts[IndexOfRenderInputDesc];

                ++IndexOfRenderInputDesc;
            }

            Gfx::ShaderManager::CreateInputLayout(pDescriptor, InputLayoutDesc.m_NumberOfElements, VSMVPShader);

            Base::CMemory::Free(pDescriptor);
        }


        _rSurface.m_VertexShaderPtr = VSShader;

        _rSurface.m_MVPVertexShaderPtr = VSMVPShader;
    }
} // namespace

namespace Gfx
{
namespace MeshManager
{
    void OnStart()
    {
        CGfxMeshManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxMeshManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CGfxMeshManager::GetInstance().Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMeshPtr CreateMesh(const SMeshDescriptor& _rDescriptor)
    {
        return CGfxMeshManager::GetInstance().CreateMesh(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CreateBox(float _Width, float _Height, float _Depth)
    {
        return CGfxMeshManager::GetInstance().CreateBox(_Width, _Height, _Depth);
    }
    
    // -----------------------------------------------------------------------------
    
    CMeshPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices)
    {
        return CGfxMeshManager::GetInstance().CreateSphere(_Radius, _Stacks, _Slices);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CreateSphereIsometric(float _Radius, unsigned int _Refinement)
    {
        return CGfxMeshManager::GetInstance().CreateSphereIsometric(_Radius, _Refinement);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CreateCone(float _Radius, float _Height, unsigned int _Slices)
    {
        return CGfxMeshManager::GetInstance().CreateCone(_Radius, _Height, _Slices);
    }
    
    // -----------------------------------------------------------------------------
    
    CMeshPtr CreateRectangle(float _AxisX, float _AxisY, float _Width, float _Height)
    {
        return CGfxMeshManager::GetInstance().CreateRectangle(_AxisX, _AxisY, _Width, _Height);
    }
} // namespace MeshManager
} // namespace Gfx