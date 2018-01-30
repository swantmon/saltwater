
#include "mr/mr_precompiled.h"

#include "mr/mr_plane_detector.h"

#include "base/base_program_parameters.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include <cassert>
#include <sstream>

using namespace Gfx;

namespace
{
    Base::Int2 g_HistogramSize = Base::Int2(128, 128);

    const int g_MaxDetectablePlaneCount = 5;

    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    struct SHistogramMetaBuffer
    {
        Base::Float4x4 m_PoseMatrix;
        Base::Float4x4 m_InvPoseMatrix;
        Base::Int4 m_HistogramSize;
    };
}

namespace MR
{
    const int g_TileSize2D = 16;
    
    void CPlaneDetector::SetImages(Gfx::CTexturePtr _VertexMap, Gfx::CTexturePtr _NormalMap)
    {
        m_VertexMap = _VertexMap;
        m_NormalMap = _NormalMap;
    }

    // -----------------------------------------------------------------------------

    void CPlaneDetector::DetectPlanes(const Base::Float4x4& _PoseMatrix, Gfx::CTexturePtr _VertexMap, Gfx::CTexturePtr _NormalMap)
    {
        if (_VertexMap != nullptr && _NormalMap != nullptr)
        {
            m_VertexMap = _VertexMap;
            m_NormalMap = _NormalMap;
        }

        assert(m_VertexMap != nullptr && m_NormalMap != nullptr);
        assert(m_VertexMap->GetNumberOfPixelsU() == m_NormalMap->GetNumberOfPixelsU() &&
               m_VertexMap->GetNumberOfPixelsV() == m_NormalMap->GetNumberOfPixelsV());

        Performance::BeginDurationEvent("Plane Detection");

        ClearData();

        std::vector<Base::Float4> NewPlanes;

        Performance::BeginDurationEvent("Histogram Creation");
        CreateHistogram(_PoseMatrix);
        Performance::EndEvent();

        Performance::BeginDurationEvent("Plane Candidate Extraction");
        ExtractPlaneCandidates(NewPlanes);
        Performance::EndEvent();

        Performance::BeginDurationEvent("Plane Equation Calculation");
        FindPlaneEquations(NewPlanes);
        Performance::EndEvent();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CPlaneDetector::ExtractPlaneCandidates(Float4Vector& _rPlanes)
    {
        const int WorkGroupsX = DivUp(g_HistogramSize[0], g_TileSize2D);
        const int WorkGroupsY = DivUp(g_HistogramSize[1], g_TileSize2D);

        ContextManager::Barrier();
        
        ContextManager::SetShaderCS(m_PlaneCandidatesCSPtr);
        ContextManager::SetConstantBuffer(0, m_HistogramConstantBuffer);
        ContextManager::SetResourceBuffer(0, m_PlaneCountBuffer);
        ContextManager::SetResourceBuffer(1, m_PlaneBuffer);
        ContextManager::SetImageTexture(0, m_NormalHistogram);
        ContextManager::SetImageTexture(1, m_VertexMap);
        ContextManager::SetImageTexture(2, m_NormalMap);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        void* pBufferData = BufferManager::MapBufferRange(m_PlaneCountBuffer, CBuffer::EMap::Read, 0, sizeof(int32_t));
        int32_t PlaneCount = *static_cast<int32_t*>(pBufferData);
        BufferManager::UnmapBuffer(m_PlaneCountBuffer);

        pBufferData = BufferManager::MapBuffer(m_PlaneBuffer, CBuffer::EMap::Read);
        
        for (int32_t i = 0; i < PlaneCount; ++ i)
        {
            Base::Float4 Plane = static_cast<Base::Float4*>(pBufferData)[i];
            _rPlanes.push_back(Plane);
        }

        BufferManager::UnmapBuffer(m_PlaneBuffer);
    }

    // -----------------------------------------------------------------------------

    void CPlaneDetector::FindPlaneEquations(Float4Vector& _rPlanes)
    {
        const int WorkGroupsX = DivUp(m_VertexMap->GetNumberOfPixelsU(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_VertexMap->GetNumberOfPixelsV(), g_TileSize2D);

        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_PlaneEquationCSPtr);
        ContextManager::SetConstantBuffer(0, m_HistogramConstantBuffer);
        ContextManager::SetResourceBuffer(0, m_PlaneCountBuffer);
        ContextManager::SetResourceBuffer(1, m_PlaneBuffer);
        ContextManager::SetImageTexture(0, m_NormalHistogram);
        ContextManager::SetImageTexture(1, m_VertexMap);
        ContextManager::SetImageTexture(2, m_NormalMap);

        for (int i = 0; i < _rPlanes.size(); ++ i)
        {
            // We just reuse the Atomic Counter as an Index
            BufferManager::UploadBufferData(m_PlaneCountBuffer, &i, 0, sizeof(int32_t));

            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CPlaneDetector::CreateHistogram(const Base::Float4x4& _PoseMatrix)
    {
        //////////////////////////////////////////////////////////////////////////////////////
        // Create a 2D-Histogram based on the inclination and the azimuth of the normals
        // "Histogram of Oriented Normal Vectors for Object Recognition with a Depth Sensor" (HONV)
        // We use the lowest resolution of the pyramid
        //////////////////////////////////////////////////////////////////////////////////////
        
        const int WorkGroupsX = DivUp(m_VertexMap->GetNumberOfPixelsU(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_VertexMap->GetNumberOfPixelsV(), g_TileSize2D);

        ContextManager::Barrier();

        SHistogramMetaBuffer BufferData;
        BufferData.m_PoseMatrix = _PoseMatrix;
        BufferData.m_InvPoseMatrix = _PoseMatrix.GetInverted();
        BufferData.m_HistogramSize = Base::Int4(g_HistogramSize[0], g_HistogramSize[1], 0, 0);

        BufferManager::UploadBufferData(m_HistogramConstantBuffer, &BufferData);

        ContextManager::SetShaderCS(m_HistogramCreationCSPtr);
        ContextManager::SetConstantBuffer(0, m_HistogramConstantBuffer);
        ContextManager::SetImageTexture(0, m_NormalHistogram);
        ContextManager::SetImageTexture(1, m_VertexMap);
        ContextManager::SetImageTexture(2, m_NormalMap);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CPlaneDetector::ClearData()
    {
        TextureManager::ClearTexture(m_NormalHistogram);

        Base::Int2 Counter = Base::Int2(0, g_MaxDetectablePlaneCount);  // Just set counter to 0
        BufferManager::UploadBufferData(m_PlaneCountBuffer, &Counter, 0, sizeof(Counter));
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CPlaneDetector::GetVertexMap()
    {
        return m_VertexMap;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CPlaneDetector::GetNormalMap()
    {
        return m_NormalMap;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CPlaneDetector::GetNormalHistogram()
    {
        return m_NormalHistogram;
    }

    // -----------------------------------------------------------------------------

    CPlaneDetector::CPlaneDetector()
    {
        m_MaxDetectablePlaneCount = Base::CProgramParameters::GetInstance().GetInt("mr:plane_detection:max_plane_count", g_MaxDetectablePlaneCount);

        //////////////////////////////////////////////////////////////////////////
        // Create Shaders
        //////////////////////////////////////////////////////////////////////////

        std::stringstream DefineStream;

        DefineStream
            << "#define TILE_SIZE2D " << g_TileSize2D << " \n"
            << "#define MAP_TEXTURE_FORMAT " << Base::CProgramParameters::GetInstance().GetStdString("mr:slam:map_format", "rgba16f") << " \n"
            << "#define MAX_DETECTABLE_PLANE_COUNT " << m_MaxDetectablePlaneCount << " \n";

        std::string DefineString = DefineStream.str();

        m_HistogramCreationCSPtr = ShaderManager::CompileCS("scalable_kinect_fusion\\plane_detection\\cs_histogram_creation.glsl", "main", DefineString.c_str());
        m_PlaneCandidatesCSPtr   = ShaderManager::CompileCS("scalable_kinect_fusion\\plane_detection\\cs_plane_candidates.glsl"  , "main", DefineString.c_str());
        m_PlaneEquationCSPtr     = ShaderManager::CompileCS("scalable_kinect_fusion\\plane_detection\\cs_plane_equation.glsl"    , "main", DefineString.c_str());

        //////////////////////////////////////////////////////////////////////////
        // Create Buffers
        //////////////////////////////////////////////////////////////////////////

        SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Usage = CBuffer::EUsage::GPURead;
        BufferDesc.m_Binding = CBuffer::ConstantBuffer;
        BufferDesc.m_Access = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(SHistogramMetaBuffer);

        m_HistogramConstantBuffer = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_Usage = CBuffer::EUsage::GPUToCPU;
        BufferDesc.m_Binding = CBuffer::ResourceBuffer;
        BufferDesc.m_Access = CBuffer::EAccess::CPUReadWrite;
        BufferDesc.m_NumberOfBytes = sizeof(Base::Float4);

        m_PlaneCountBuffer = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_NumberOfBytes = sizeof(Base::Float4) * g_MaxDetectablePlaneCount;

        m_PlaneBuffer = BufferManager::CreateBuffer(BufferDesc);

        //////////////////////////////////////////////////////////////////////////
        // Create Textures
        //////////////////////////////////////////////////////////////////////////

        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = g_HistogramSize[0];
        TextureDescriptor.m_NumberOfPixelsV = g_HistogramSize[1];
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::CPUWrite;
        TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R32_INT;

        m_NormalHistogram = TextureManager::CreateTexture2D(TextureDescriptor);
    }

    // -----------------------------------------------------------------------------

    CPlaneDetector::~CPlaneDetector()
    {

    }

} // namespace MR