
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

    const int g_MaxDetectablePlanes = 5;

    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    struct SHistogramMetaBuffer
    {
        Base::Float4x4 m_PoseMatrix;
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

        Performance::BeginDurationEvent("Histogram Creation");
        CreateHistogram(_PoseMatrix);
        Performance::EndEvent();

        Performance::BeginDurationEvent("Plane Extraction");
        ExtractPlanes();
        Performance::EndEvent();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CPlaneDetector::ExtractPlanes()
    {
        const int WorkGroupsX = DivUp(g_HistogramSize[0], g_TileSize2D);
        const int WorkGroupsY = DivUp(g_HistogramSize[1], g_TileSize2D);

        ContextManager::Barrier();
        
        ContextManager::SetShaderCS(m_PlaneExtractionCSPtr);
        ContextManager::SetImageTexture(0, m_NormalHistogram);
        ContextManager::SetImageTexture(1, m_VertexMap);
        ContextManager::SetImageTexture(2, m_NormalMap);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
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

        int Counter = 0;
        BufferManager::UploadBufferData(m_PlaneBuffer, &Counter, 0, sizeof(int32_t)); // Just set counter to 0
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
        //////////////////////////////////////////////////////////////////////////
        // Create Shaders
        //////////////////////////////////////////////////////////////////////////

        std::stringstream DefineStream;

        DefineStream
            << "#define TILE_SIZE2D " << g_TileSize2D << " \n"
            << "#define MAP_TEXTURE_FORMAT " << Base::CProgramParameters::GetInstance().GetStdString("mr:slam:map_format", "rgba16f") << " \n";

        std::string DefineString = DefineStream.str();

        m_HistogramCreationCSPtr = ShaderManager::CompileCS("scalable_kinect_fusion\\plane_detection\\cs_histogram_creation.glsl", "main", DefineString.c_str());
        m_PlaneExtractionCSPtr =   ShaderManager::CompileCS("scalable_kinect_fusion\\plane_detection\\cs_plane_extraction.glsl"  , "main", DefineString.c_str());

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
        BufferDesc.m_NumberOfBytes = (g_MaxDetectablePlanes + 1) * sizeof(Base::Float4); // + 1 because of some meta data

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