
#include "mr/mr_precompiled.h"

#include "mr/mr_plane_detector.h"

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
    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }
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

    void CPlaneDetector::DetectPlanes(Gfx::CTexturePtr _VertexMap, Gfx::CTexturePtr _NormalMap)
    {
        if (_VertexMap != nullptr && _NormalMap != nullptr)
        {
            m_VertexMap = _VertexMap;
            m_NormalMap = _NormalMap;
        }

        assert(m_VertexMap != nullptr && m_NormalMap != nullptr);

        TextureManager::ClearTexture(m_NormalHistogram);

        //////////////////////////////////////////////////////////////////////////////////////
        // Create a 2D-Histogram based on the inclination and the azimuth of the normals
        // "Histogram of Oriented Normal Vectors for Object Recognition with a Depth Sensor" (HONV)
        // We use the lowest resolution of the pyramid
        //////////////////////////////////////////////////////////////////////////////////////
        
        const int WorkGroupsX = DivUp(m_VertexMap->GetNumberOfPixelsU(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_VertexMap->GetNumberOfPixelsV(), g_TileSize2D);

        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_NormalHistogramCSPtr);
        //ContextManager::SetConstantBuffer(0, m_HONVMetadataBuffer);
        //ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);
        ContextManager::SetImageTexture(0, m_NormalHistogram);
        ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_VertexMap));
        ContextManager::SetImageTexture(2, static_cast<CTexturePtr>(m_NormalMap));
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
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
        std::stringstream Stream;

        std::string DefineString = Stream.str();

        m_NormalHistogramCSPtr = ShaderManager::CompileCS("scalable_kinect_fusion\\plane_detection\\cs_histogram.glsl", "main", DefineString.c_str());
    }

    // -----------------------------------------------------------------------------

    CPlaneDetector::~CPlaneDetector()
    {

    }

} // namespace MR