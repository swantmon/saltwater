
#include "engine/engine_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_asset_importer.h"
#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_mesh_component.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_input_layout.h"
#include "engine/graphic/gfx_material_manager.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_shader_manager.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <array>
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
        "geometry/vs_m_pn.glsl",
        "geometry/vs_m_pnx0.glsl",
        "geometry/vs_m_pntbx0.glsl",
    };

    const Base::Char* g_pShaderNamesVS[] =
    {
        "VSShaderPN",
        "VSShaderPNX0",
        "VSShaderPNTBX0",
    };

    const Base::Char* g_pMVPShaderFilenameVS[] =
    {
        "geometry/vs_mvp_pn.glsl",
        "geometry/vs_mvp_pnx0.glsl",
        "geometry/vs_mvp_pntbx0.glsl",
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
        
        CMeshPtr CreateMeshFromFile(const std::string& _rPathToFile, int _GenFlag, int _MeshIndex);
        CMeshPtr CreateMesh(const void* _rVertices, int _NumberOfVertices, int _SizeOfVertex, const uint32_t* _rIndices, int _NumberOfIndices);
        CMeshPtr CreateBox(float _Width, float _Height, float _Depth);
        CMeshPtr CreateSphere(float _Radius, unsigned int _Stacks, unsigned int _Slices);
        CMeshPtr CreateSphereIsometric(float _Radius, unsigned int _Refinement);
        CMeshPtr CreateCone(float _Radius, float _Height, unsigned int _Slices);
        CMeshPtr CreateRectangle(float _AxisX, float _AxisY, float _Width, float _Height);

    private:

        class CInternMesh : public CMesh
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
        
        using CMeshes = Base::CManagedPool<CInternMesh   , 64  , 1>;
        using CLODs = Base::CManagedPool<CInternLOD    , 256 , 1>;
        using CSurfaces = Base::CManagedPool<CInternSurface, 1024, 1>;
        
        using CMeshByHash = std::unordered_map<Base::BHash, CInternMesh*>;
        
    private:
        
        CMeshes    m_Meshes;
        CLODs      m_LODs;
        CSurfaces  m_Surfaces;
        
        CMeshByHash m_ModelByHash;

        Dt::CComponentManager::CComponentDelegate::HandleType m_OnDirtyComponentDelegate;

    private:

        void SetVertexShaderOfSurface(CInternSurface& _rSurface);

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        void FillMeshFromFile(CInternMesh* _pMesh, const std::string& _rFilename, int _GenFlag, int _MeshIndex);

        void FillMeshFromAssimp(CInternMesh* _pMesh, const aiScene* _pScene, int _MeshIndex);
    };
} // namespace

namespace
{
    CGfxMeshManager::CGfxMeshManager()
        : m_Meshes     ()
        , m_LODs       ()
        , m_Surfaces   ()
        , m_ModelByHash()
    {
        m_ModelByHash.reserve(64);
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxMeshManager::~CGfxMeshManager()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMeshManager::OnStart()
    {
        m_OnDirtyComponentDelegate = Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(std::bind(&CGfxMeshManager::OnDirtyComponent, this, std::placeholders::_1));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMeshManager::OnExit()
    {
        m_Meshes.Clear();
        m_LODs.Clear();
        m_Surfaces.Clear();

        m_ModelByHash.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMeshPtr CGfxMeshManager::CreateMeshFromFile(const std::string& _rPathToFile, int _GenFlag, int _MeshIndex)
    {
        if (_rPathToFile.length() == 0) return nullptr;

        // -----------------------------------------------------------------------------
        // Check existing model
        // -----------------------------------------------------------------------------
        unsigned int Hash = 0;
        
        Hash = Base::CRC32(Hash, _rPathToFile.c_str(), static_cast<unsigned int>(_rPathToFile.length()));

        Hash = Base::CRC32(Hash, &_GenFlag, sizeof(_GenFlag));

        Hash = Base::CRC32(Hash, &_MeshIndex, sizeof(_MeshIndex));

        if (m_ModelByHash.find(Hash) != m_ModelByHash.end())
        {
            return m_ModelByHash.at(Hash);
        }

        // -----------------------------------------------------------------------------
        // Entities with model data
        // -----------------------------------------------------------------------------
        auto MeshPtr = m_Meshes.Allocate();

        FillMeshFromFile(MeshPtr, _rPathToFile, _GenFlag, _MeshIndex);
                
        // -----------------------------------------------------------------------------
        // Put this new model to hash list
        // -----------------------------------------------------------------------------
        m_ModelByHash[Hash] = MeshPtr;
        
        return CMeshPtr(MeshPtr);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CGfxMeshManager::CreateMesh(const void* _rVertices, int _NumberOfVertices, int _SizeOfVertex, const uint32_t* _rIndices, int _NumberOfIndices)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CMeshes::CPtr ModelPtr = m_Meshes.Allocate();

        CInternMesh& rModel = *ModelPtr;

        CLODs::CPtr LODPtr = m_LODs.Allocate();

        CInternLOD& rLOD = *LODPtr;

        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();

        CInternSurface& rSurface = *SurfacePtr;

        rSurface.m_MaterialPtr = nullptr;

        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_Surface = SurfacePtr;

        // -----------------------------------------------------------------------------
        // Prepare surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key = 0;
        rSurface.m_SurfaceKey.m_HasPosition = true;
        rSurface.m_SurfaceKey.m_HasNormal = true;
        rSurface.m_SurfaceKey.m_HasTangent = false;
        rSurface.m_SurfaceKey.m_HasBitangent = false;
        rSurface.m_SurfaceKey.m_HasTexCoords = true;

        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------
        SBufferDescriptor BufferDesc;

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = CBuffer::GPURead;
        BufferDesc.m_Binding = CBuffer::VertexBuffer;
        BufferDesc.m_Access = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = _NumberOfVertices * _SizeOfVertex;
        BufferDesc.m_pBytes = const_cast<void*>(_rVertices);
        BufferDesc.m_pClassKey = nullptr;

        rSurface.m_VertexBufferPtr = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = static_cast<unsigned>(_NumberOfVertices);

        // -----------------------------------------------------------------------------

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = CBuffer::GPURead;
        BufferDesc.m_Binding = CBuffer::IndexBuffer;
        BufferDesc.m_Access = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = _NumberOfIndices * sizeof(_rIndices[0]);
        BufferDesc.m_pBytes = const_cast<uint32_t*>(_rIndices);
        BufferDesc.m_pClassKey = nullptr;

        rSurface.m_IndexBufferPtr = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfIndices = static_cast<unsigned>(_NumberOfIndices);

        // -----------------------------------------------------------------------------
        // Vertex Shader
        // -----------------------------------------------------------------------------
        SetVertexShaderOfSurface(rSurface);

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        rSurface.m_MaterialPtr = Gfx::MaterialManager::GetDefaultMaterial();

        return CMeshPtr(ModelPtr);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CGfxMeshManager::CreateBox(float _Width, float _Height, float _Depth)
    {
        // -----------------------------------------------------------------------------
        // Create model with LOD, surface and materials
        // -----------------------------------------------------------------------------
        CMeshes::CPtr ModelPtr = m_Meshes.Allocate();

        CInternMesh& rModel = *ModelPtr;

        CLODs::CPtr LODPtr = m_LODs.Allocate();

        CInternLOD& rLOD = *LODPtr;

        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();

        CInternSurface& rSurface = *SurfacePtr;

        rSurface.m_MaterialPtr = nullptr;

        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_Surface = SurfacePtr;

        // -----------------------------------------------------------------------------
        // Prepare surface
        // -----------------------------------------------------------------------------
        rSurface.m_SurfaceKey.m_Key          = 0;
        rSurface.m_SurfaceKey.m_HasPosition  = true;
        rSurface.m_SurfaceKey.m_HasNormal    = true;
        rSurface.m_SurfaceKey.m_HasTangent   = false;
        rSurface.m_SurfaceKey.m_HasBitangent = false;
        rSurface.m_SurfaceKey.m_HasTexCoords = true;

        // -----------------------------------------------------------------------------
        // Calculate Data
        // -----------------------------------------------------------------------------
        float HalfWidth  = _Width  / 2.0f;
        float HalfHeight = _Height / 2.0f;
        float HalfDepth  = _Depth  / 2.0f;

        unsigned int NumberOfVertices = 36;
        unsigned int NumberOfIndices = 36;

        unsigned int NumberOfBytes = (2 * sizeof(glm::vec3) + sizeof(glm::vec2)) * NumberOfVertices;

        auto* pVertices = static_cast<float*>(Base::CMemory::Allocate(NumberOfBytes));
        auto* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));

        assert(pVertices);
        assert(pIndices);

        std::array<glm::vec3, 8> Vertices;
        std::array<glm::vec2, 4> UVs;
        std::array<glm::vec3, 6> Normals;
        std::array<std::array<glm::ivec3, 3>, 12> Triangles;

        Vertices[0] = glm::vec3(-HalfWidth, -HalfHeight,  HalfDepth);
        Vertices[1] = glm::vec3( HalfWidth, -HalfHeight,  HalfDepth);
        Vertices[2] = glm::vec3(-HalfWidth,  HalfHeight,  HalfDepth);
        Vertices[3] = glm::vec3( HalfWidth,  HalfHeight,  HalfDepth);
        Vertices[4] = glm::vec3(-HalfWidth,  HalfHeight, -HalfDepth);
        Vertices[5] = glm::vec3( HalfWidth,  HalfHeight, -HalfDepth);
        Vertices[6] = glm::vec3(-HalfWidth, -HalfHeight, -HalfDepth);
        Vertices[7] = glm::vec3( HalfWidth, -HalfHeight, -HalfDepth);

        UVs[0] = glm::vec2(0.0f, 0.0f);
        UVs[1] = glm::vec2(1.0f, 0.0f);
        UVs[2] = glm::vec2(0.0f, 1.0f);
        UVs[3] = glm::vec2(1.0f, 1.0f);

        Normals[0] = glm::vec3( 0.0f,  0.0f,  1.0f);
        Normals[1] = glm::vec3( 0.0f,  1.0f,  0.0f);
        Normals[2] = glm::vec3( 0.0f,  0.0f, -1.0f);
        Normals[3] = glm::vec3( 0.0f, -1.0f,  0.0f);
        Normals[4] = glm::vec3( 1.0f,  0.0f,  0.0f);
        Normals[5] = glm::vec3(-1.0f,  0.0f,  0.0f);

        Triangles[0] = { glm::ivec3(0, 0, 0), glm::ivec3(1, 1, 0), glm::ivec3(2, 2, 0) };
        Triangles[1] = { glm::ivec3(2, 2, 0), glm::ivec3(1, 1, 0), glm::ivec3(3, 3, 0) };
        Triangles[2] = { glm::ivec3(2, 0, 1), glm::ivec3(3, 1, 1), glm::ivec3(4, 2, 1) };
        Triangles[3] = { glm::ivec3(4, 2, 1), glm::ivec3(3, 1, 1), glm::ivec3(5, 3, 1) };
        Triangles[4] = { glm::ivec3(4, 3, 2), glm::ivec3(5, 2, 2), glm::ivec3(6, 1, 2) };
        Triangles[5] = { glm::ivec3(6, 1, 2), glm::ivec3(5, 2, 2), glm::ivec3(7, 0, 2) };
        Triangles[6] = { glm::ivec3(6, 0, 3), glm::ivec3(7, 1, 3), glm::ivec3(0, 2, 3) };
        Triangles[7] = { glm::ivec3(0, 2, 3), glm::ivec3(7, 1, 3), glm::ivec3(1, 3, 3) };
        Triangles[8] = { glm::ivec3(1, 0, 4), glm::ivec3(7, 1, 4), glm::ivec3(3, 2, 4) };
        Triangles[9] = { glm::ivec3(3, 2, 4), glm::ivec3(7, 1, 4), glm::ivec3(5, 3, 4) };
        Triangles[10] = { glm::ivec3(6, 0, 5), glm::ivec3(0, 1, 5), glm::ivec3(4, 2, 5) };
        Triangles[11] = { glm::ivec3(4, 2, 5), glm::ivec3(0, 1, 5), glm::ivec3(2, 3, 5) };

        unsigned int IndexOfVertex = 0;

        for (auto Triangle : Triangles)
        {
            for (auto Vertex : Triangle)
            {
                pVertices[IndexOfVertex + 0] = Vertices[Vertex[0]][0];
                pVertices[IndexOfVertex + 1] = Vertices[Vertex[0]][1];
                pVertices[IndexOfVertex + 2] = Vertices[Vertex[0]][2];

                IndexOfVertex += 3;

                pVertices[IndexOfVertex + 0] = Normals[Vertex[2]][0];
                pVertices[IndexOfVertex + 1] = Normals[Vertex[2]][1];
                pVertices[IndexOfVertex + 2] = Normals[Vertex[2]][2];

                IndexOfVertex += 3;

                pVertices[IndexOfVertex + 0] = UVs[Vertex[1]][0];
                pVertices[IndexOfVertex + 1] = UVs[Vertex[1]][1];

                IndexOfVertex += 2;
            }
        }

        for (int i = 0; i < static_cast<int>(NumberOfIndices); ++i)
        {
            pIndices[i] = i;
        }

        // -----------------------------------------------------------------------------
        // Create buffer on graphic device and setup surface
        // -----------------------------------------------------------------------------
        SBufferDescriptor BufferDesc;
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = NumberOfBytes;
        BufferDesc.m_pBytes        = pVertices;
        BufferDesc.m_pClassKey     = nullptr;
        
        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = nullptr;
        
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
        CMeshes::CPtr MeshPtr = m_Meshes.Allocate();
        
        CInternMesh& rModel = *MeshPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_Surface = SurfacePtr;

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
        
        auto* pVertices = static_cast<glm::vec3*>(Base::CMemory::Allocate(sizeof(glm::vec3) * NumberOfVertices * 2));
        auto* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
        
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
        BufferDesc.m_pClassKey     = nullptr;

        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = nullptr;
        
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
        CMeshes::CPtr MeshPtr = m_Meshes.Allocate();
        
        CInternMesh& rModel = *MeshPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_Surface = SurfacePtr;

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
        LightVertices.emplace_back(-1.0f, -1.0f, -1.0f);
        LightVertices.emplace_back(1.0f, -1.0f, 1.0f);
        LightVertices.emplace_back(-1.0f, 1.0f, 1.0f);
        LightVertices.emplace_back(1.0f, 1.0f, -1.0f);

        LightTriangles.emplace_back(0, 3, 2);
        LightTriangles.emplace_back(0, 1, 3);
        LightTriangles.emplace_back(1, 0, 2);
        LightTriangles.emplace_back(1, 2, 3);

        // Split the faces of the tetrahedron
        for (unsigned int RefinementStep = 0; RefinementStep < _Refinement; ++RefinementStep)
        {
            std::vector<glm::vec3> Vertices;
            std::vector<glm::uvec3>  Triangles;

            unsigned int NextVertexIndex = 0;

            for (auto & LightTriangle : LightTriangles)
            {
                glm::vec3 NewVertices[6];

                for (unsigned int IndexOfVertex = 0; IndexOfVertex < 3; ++IndexOfVertex)
                {
                    NewVertices[IndexOfVertex] = LightVertices[LightTriangle[IndexOfVertex]];
                }

                glm::vec3 StartVertex, EndVertex;

                for (unsigned int IndexOfVertex = 0; IndexOfVertex < 3; ++IndexOfVertex)
                {
                    StartVertex  = NewVertices[IndexOfVertex];
                    EndVertex    = NewVertices[(IndexOfVertex + 1) % 3];

                    NewVertices[IndexOfVertex + 3] = (StartVertex + EndVertex) / 2.0f;
                }

                for (auto NewVertice : NewVertices)
                {
                    Vertices.push_back(NewVertice);
                }

                Triangles.emplace_back(NextVertexIndex + 0, NextVertexIndex + 3, NextVertexIndex + 5);
                Triangles.emplace_back(NextVertexIndex + 3, NextVertexIndex + 1, NextVertexIndex + 4);
                Triangles.emplace_back(NextVertexIndex + 3, NextVertexIndex + 4, NextVertexIndex + 5);
                Triangles.emplace_back(NextVertexIndex + 5, NextVertexIndex + 4, NextVertexIndex + 2);

                NextVertexIndex += 6;
            }

            LightVertices  = std::move(Vertices);
            LightTriangles = std::move(Triangles);
        }

        // We want a unit sphere
        std::vector<glm::vec3> VerticesNormal;

        for (auto LightVertice : LightVertices)
        {
            glm::vec3 Vertex = glm::normalize(LightVertice);

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
        BufferDesc.m_pClassKey     = nullptr;

        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = static_cast<unsigned int>(VerticesNormal.size()) / 2;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(LightTriangles[0]) * static_cast<unsigned int>(LightTriangles.size());
        BufferDesc.m_pBytes        = LightTriangles.data();
        BufferDesc.m_pClassKey     = nullptr;
        
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
        CMeshes::CPtr ModelPtr = m_Meshes.Allocate();

        CInternMesh& rModel = *ModelPtr;

        CLODs::CPtr LODPtr = m_LODs.Allocate();

        CInternLOD& rLOD = *LODPtr;

        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();

        CInternSurface& rSurface = *SurfacePtr;

        rSurface.m_MaterialPtr = nullptr;

        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_Surface = SurfacePtr;

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

        auto* pVertices = static_cast<glm::vec3*>(Base::CMemory::Allocate(sizeof(glm::vec3) * NumberOfVertices * 2));
        auto* pIndices = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));

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
            assert(IndexOfVertex < NumberOfVertices * 2);

            float PHI = static_cast<float>(IndexOfSlice) / (_Slices)* glm::pi<float>() * 2.0f;

            pVertices[IndexOfVertex][0] = -glm::cos(PHI) * _Radius;
            pVertices[IndexOfVertex][1] =  glm::sin(PHI) * _Radius;
            pVertices[IndexOfVertex][2] = -_Height / 2.0f;

            ++IndexOfVertex;

            pVertices[IndexOfVertex][0] = -glm::cos(PHI) * _Radius;
            pVertices[IndexOfVertex][1] =  glm::sin(PHI) * _Radius;
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
        BufferDesc.m_pClassKey     = nullptr;
        
        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = nullptr;
        
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
        CMeshes::CPtr ModelPtr = m_Meshes.Allocate();
        
        CInternMesh& rModel = *ModelPtr;
        
        CLODs::CPtr LODPtr = m_LODs.Allocate();
        
        CInternLOD& rLOD = *LODPtr;
        
        CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();
        
        CInternSurface& rSurface = *SurfacePtr;
        
        rSurface.m_MaterialPtr = nullptr;
        
        rModel.m_NumberOfLODs = 1;
        rModel.m_LODs[0] = LODPtr;

        rLOD.m_Surface = SurfacePtr;

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
        
        auto* pVertices = static_cast<glm::vec2*>(Base::CMemory::Allocate(sizeof(glm::vec2) * NumberOfVertices));
        auto* pIndices  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
        
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
        BufferDesc.m_pClassKey     = nullptr;
        
        rSurface.m_VertexBufferPtr  = BufferManager::CreateBuffer(BufferDesc);
        rSurface.m_NumberOfVertices = NumberOfVertices;
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
        BufferDesc.m_pBytes        = pIndices;
        BufferDesc.m_pClassKey     = nullptr;
        
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

        assert(VSShader != nullptr && VSMVPShader != nullptr);

        // -----------------------------------------------------------------------------
        // Set input layout if vertex shader has no input layout
        // -----------------------------------------------------------------------------
        if (VSShader->GetInputLayout() == nullptr)
        {
            SInputElementDescriptorSetting InputLayoutDesc = g_InputLayoutDescriptor[VSIndex];

            auto* pDescriptor = static_cast<Gfx::SInputElementDescriptor*>(Base::CMemory::Allocate(sizeof(Gfx::SInputElementDescriptor) * InputLayoutDesc.m_NumberOfElements));

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

            auto* pDescriptor = static_cast<Gfx::SInputElementDescriptor*>(Base::CMemory::Allocate(sizeof(Gfx::SInputElementDescriptor) * InputLayoutDesc.m_NumberOfElements));

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

    // -----------------------------------------------------------------------------

    void CGfxMeshManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeInfo() != Base::CTypeInfo::Get<Dt::CMeshComponent>()) return;

        auto* pMeshComponent = static_cast<Dt::CMeshComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pMeshComponent->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new mesh
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CMeshComponent::DirtyCreate) != 0)
        {
            switch (pMeshComponent->GetMeshType())
            {
            case Dt::CMeshComponent::Asset:
                pMeshComponent->SetFacet(Dt::CMeshComponent::Graphic, 
                    CreateMeshFromFile(
                        pMeshComponent->GetFilename().c_str(),
                        pMeshComponent->GetGeneratorFlag(), 
                        pMeshComponent->GetMeshIndex()
                    )
                );
                break;
            case Dt::CMeshComponent::Box:
                pMeshComponent->SetFacet(Dt::CMeshComponent::Graphic, CreateBox(1.0f, 1.0f, 1.0f));
                break;
            case Dt::CMeshComponent::Sphere:
                pMeshComponent->SetFacet(Dt::CMeshComponent::Graphic, CreateSphere(1.0f, 24, 24));
                break;
            case Dt::CMeshComponent::IsometricSphere:
                pMeshComponent->SetFacet(Dt::CMeshComponent::Graphic, CreateSphereIsometric(1.0f, 8));
                break;
            case Dt::CMeshComponent::Cone:
                pMeshComponent->SetFacet(Dt::CMeshComponent::Graphic, CreateCone(1.0f, 1.0f, 24));
                break;
            case Dt::CMeshComponent::Rectangle:
                pMeshComponent->SetFacet(Dt::CMeshComponent::Graphic, CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f));
                break;
            default:
                ENGINE_CONSOLE_ERROR("The selected predefined mesh is currently not supported!");
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshManager::FillMeshFromFile(CInternMesh* _pMesh, const std::string& _rPathToFile, int _GenFlag, int _MeshIndex)
    {
        // -----------------------------------------------------------------------------
        // Build path to texture in file system and load model
        // -----------------------------------------------------------------------------
        std::string PathToModel = Core::AssetManager::GetPathToAssets() + "/" + _rPathToFile;

        auto Importer = Core::AssetImporter::AllocateAssimpImporter(PathToModel, _GenFlag);

        if (Importer == nullptr) return;

        const auto* pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (!pImporter) return;

        const aiScene* pScene = pImporter->GetScene();

        if (pScene == nullptr) return;

        FillMeshFromAssimp(_pMesh, pScene, _MeshIndex);

        // -----------------------------------------------------------------------------
        // Release importer
        // -----------------------------------------------------------------------------
        Core::AssetImporter::ReleaseImporter(Importer);
    }

    // -----------------------------------------------------------------------------

    void CGfxMeshManager::FillMeshFromAssimp(CInternMesh* _pMesh, const aiScene* _pScene, int _MeshIndex)
    {
        // -----------------------------------------------------------------------------
        // Fill mesh data from assimp into the component
        // -----------------------------------------------------------------------------
        auto FillMeshInComponent = [&](CInternMesh* _pMesh, const aiMesh* _pAssimpMesh)
        {
            // -----------------------------------------------------------------------------
            // Setup model
            // -----------------------------------------------------------------------------
            _pMesh->m_NumberOfLODs = 1;

            for (unsigned int IndexOfLOD = 0; IndexOfLOD < _pMesh->m_NumberOfLODs; IndexOfLOD++)
            {
                // -----------------------------------------------------------------------------
                // Create LOD
                // -----------------------------------------------------------------------------
                CLODs::CPtr LODPtr = m_LODs.Allocate();

                CInternLOD& rInternLOD = *LODPtr;

                // -----------------------------------------------------------------------------
                // Link
                // -----------------------------------------------------------------------------
                _pMesh->m_LODs[IndexOfLOD] = LODPtr;

                // -----------------------------------------------------------------------------
                // Create surface
                // -----------------------------------------------------------------------------
                CSurfaces::CPtr SurfacePtr = m_Surfaces.Allocate();

                CInternSurface& rSurface = *SurfacePtr;

                rSurface.m_SurfaceKey.m_HasPosition = true;

                assert(_pAssimpMesh->mVertices != nullptr);

                rSurface.m_SurfaceKey.m_HasNormal = (_pAssimpMesh->mNormals != nullptr);
                rSurface.m_SurfaceKey.m_HasTangent = (_pAssimpMesh->mTangents != nullptr);
                rSurface.m_SurfaceKey.m_HasBitangent = (_pAssimpMesh->mBitangents != nullptr);
                rSurface.m_SurfaceKey.m_HasTexCoords = (_pAssimpMesh->mTextureCoords[0] != nullptr);

                // -----------------------------------------------------------------------------
                // Set vertex shader
                // -----------------------------------------------------------------------------
                SetVertexShaderOfSurface(rSurface);

                // -----------------------------------------------------------------------------
                // Link
                // -----------------------------------------------------------------------------
                rInternLOD.m_Surface = &rSurface;

                // -----------------------------------------------------------------------------
                // Data
                // -----------------------------------------------------------------------------
                unsigned int NumberOfVertices = _pAssimpMesh->mNumVertices;
                unsigned int NumberOfFaces = _pAssimpMesh->mNumFaces;
                unsigned int NumberOfIndicesPerFace = _pAssimpMesh->mFaces->mNumIndices;
                unsigned int NumberOfIndices = NumberOfFaces * NumberOfIndicesPerFace;
                unsigned int NumberOfNormals = NumberOfVertices * rSurface.m_SurfaceKey.m_HasNormal;
                unsigned int NumberOfTagents = NumberOfVertices * rSurface.m_SurfaceKey.m_HasTangent;
                unsigned int NumberOfBitangents = NumberOfVertices * rSurface.m_SurfaceKey.m_HasBitangent;
                unsigned int NumberOfTexCoords = NumberOfVertices * (rSurface.m_SurfaceKey.m_HasTexCoords >= 1);
                unsigned int NumberOfVerticeElements = 3 * rSurface.m_SurfaceKey.m_HasPosition;
                unsigned int NumberOfNormalElements = 3 * rSurface.m_SurfaceKey.m_HasNormal;
                unsigned int NumberOfTagentsElements = 3 * rSurface.m_SurfaceKey.m_HasTangent;
                unsigned int NumberOfBitangentsElements = 3 * rSurface.m_SurfaceKey.m_HasBitangent;
                unsigned int NumberOfTexCoordElements = 2 * (rSurface.m_SurfaceKey.m_HasTexCoords >= 1);
                unsigned int NumberOfVertexElements = NumberOfVertices * NumberOfVerticeElements + NumberOfNormals * NumberOfNormalElements + NumberOfTagents * NumberOfTagentsElements + NumberOfBitangents * NumberOfBitangentsElements + NumberOfTexCoords * NumberOfTexCoordElements;

                assert(NumberOfIndicesPerFace == 3);

                auto* pUploadIndexData  = static_cast<unsigned int*>(Base::CMemory::Allocate(sizeof(unsigned int) * NumberOfIndices));
                auto* pUploadVertexData = static_cast<float*>(Base::CMemory::Allocate(sizeof(float) * NumberOfVertexElements));

                // -----------------------------------------------------------------------------
                // Get data from file
                // -----------------------------------------------------------------------------
                aiVector3D* pVertexData = _pAssimpMesh->mVertices;
                aiVector3D* pNormalData = _pAssimpMesh->mNormals;
                aiVector3D* pTangentData = _pAssimpMesh->mTangents;
                aiVector3D* pBitangentData = _pAssimpMesh->mBitangents;
                aiVector3D* pTextureData = _pAssimpMesh->mTextureCoords[0];

                // -----------------------------------------------------------------------------
                // Setup surface
                // -----------------------------------------------------------------------------
                for (unsigned int IndexOfFace = 0; IndexOfFace < NumberOfFaces; ++IndexOfFace)
                {
                    aiFace CurrentFace = _pAssimpMesh->mFaces[IndexOfFace];

                    for (unsigned int IndexOfIndice = 0; IndexOfIndice < NumberOfIndicesPerFace; ++IndexOfIndice)
                    {
                        pUploadIndexData[IndexOfFace * NumberOfIndicesPerFace + IndexOfIndice] = CurrentFace.mIndices[IndexOfIndice];
                    }
                }

                unsigned int VertexDataIndex = 0;

                for (unsigned int CurrentVertex = 0; CurrentVertex < NumberOfVertices; ++CurrentVertex)
                {
                    pUploadVertexData[VertexDataIndex + 0] = pVertexData[CurrentVertex].x;
                    pUploadVertexData[VertexDataIndex + 1] = pVertexData[CurrentVertex].y;
                    pUploadVertexData[VertexDataIndex + 2] = pVertexData[CurrentVertex].z;

                    VertexDataIndex += 3;

                    if (rSurface.m_SurfaceKey.m_HasNormal)
                    {
                        pUploadVertexData[VertexDataIndex + 0] = pNormalData[CurrentVertex].x;
                        pUploadVertexData[VertexDataIndex + 1] = pNormalData[CurrentVertex].y;
                        pUploadVertexData[VertexDataIndex + 2] = pNormalData[CurrentVertex].z;

                        VertexDataIndex += 3;
                    }

                    if (rSurface.m_SurfaceKey.m_HasTangent)
                    {
                        assert(pTangentData != nullptr);

                        pUploadVertexData[VertexDataIndex + 0] = pTangentData[CurrentVertex].x;
                        pUploadVertexData[VertexDataIndex + 1] = pTangentData[CurrentVertex].y;
                        pUploadVertexData[VertexDataIndex + 2] = pTangentData[CurrentVertex].z;

                        VertexDataIndex += 3;
                    }

                    if (rSurface.m_SurfaceKey.m_HasBitangent)
                    {
                        assert(pBitangentData != nullptr);

                        pUploadVertexData[VertexDataIndex + 0] = pBitangentData[CurrentVertex].x;
                        pUploadVertexData[VertexDataIndex + 1] = pBitangentData[CurrentVertex].y;
                        pUploadVertexData[VertexDataIndex + 2] = pBitangentData[CurrentVertex].z;

                        VertexDataIndex += 3;
                    }

                    if (rSurface.m_SurfaceKey.m_HasTexCoords)
                    {
                        pUploadVertexData[VertexDataIndex + 0] = pTextureData[CurrentVertex].x;
                        pUploadVertexData[VertexDataIndex + 1] = pTextureData[CurrentVertex].y;

                        VertexDataIndex += 2;
                    }
                }

                assert(VertexDataIndex == NumberOfVertexElements);

                // -----------------------------------------------------------------------------
                // Create buffer with vertices's and indices (setup surface data)
                // -----------------------------------------------------------------------------
                SBufferDescriptor VertexBufferDesc;

                VertexBufferDesc.m_Stride = 0;
                VertexBufferDesc.m_Usage = CBuffer::GPURead;
                VertexBufferDesc.m_Binding = CBuffer::VertexBuffer;
                VertexBufferDesc.m_Access = CBuffer::CPUWrite;
                VertexBufferDesc.m_NumberOfBytes = sizeof(float) * NumberOfVertexElements;
                VertexBufferDesc.m_pBytes = pUploadVertexData;
                VertexBufferDesc.m_pClassKey = nullptr;

                rSurface.m_VertexBufferPtr = BufferManager::CreateBuffer(VertexBufferDesc);

                // -----------------------------------------------------------------------------

                SBufferDescriptor IndexBufferDesc;

                IndexBufferDesc.m_Stride = 0;
                IndexBufferDesc.m_Usage = CBuffer::GPURead;
                IndexBufferDesc.m_Binding = CBuffer::IndexBuffer;
                IndexBufferDesc.m_Access = CBuffer::CPUWrite;
                IndexBufferDesc.m_NumberOfBytes = sizeof(unsigned int) * NumberOfIndices;
                IndexBufferDesc.m_pBytes = pUploadIndexData;
                IndexBufferDesc.m_pClassKey = nullptr;

                rSurface.m_IndexBufferPtr = BufferManager::CreateBuffer(IndexBufferDesc);

                // -----------------------------------------------------------------------------
                // Set last data information of the surface
                // -----------------------------------------------------------------------------
                rSurface.m_NumberOfVertices = NumberOfVertices;
                rSurface.m_NumberOfIndices = NumberOfIndices;

                // -----------------------------------------------------------------------------
                // Delete allocated memory
                // -----------------------------------------------------------------------------
                Base::CMemory::Free(pUploadIndexData);
                Base::CMemory::Free(pUploadVertexData);

                // -----------------------------------------------------------------------------
                // Load default material from material manager
                // -----------------------------------------------------------------------------
                rSurface.m_MaterialPtr = MaterialManager::GetDefaultMaterial();
            }
        };

        // -----------------------------------------------------------------------------
        // Only single meshes are currently supported!
        // Question: Do we need multiple surfaces?
        // -----------------------------------------------------------------------------
        unsigned int NumberOfMeshes = _pScene->mNumMeshes;

        if (NumberOfMeshes > static_cast<unsigned int>(_MeshIndex))
        {
            FillMeshInComponent(_pMesh, _pScene->mMeshes[_MeshIndex]);
        }
        else
        {
            ENGINE_CONSOLE_ERROR("The given mesh index to load mesh is higher as the number of available meshes in file.")
        }
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
    
    CMeshPtr CreateMeshFromFile(const std::string& _rPathToFile, int _GenFlag, int _MeshIndex)
    {
        return CGfxMeshManager::GetInstance().CreateMeshFromFile(_rPathToFile, _GenFlag, _MeshIndex);
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CreateMesh(const void* _rVertices, int _NumberOfVertices, int _SizeOfVertex, const uint32_t* _rIndices, int _NumberOfIndices)
    {
        return CGfxMeshManager::GetInstance().CreateMesh(_rVertices, _NumberOfVertices, _SizeOfVertex, _rIndices, _NumberOfIndices);
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