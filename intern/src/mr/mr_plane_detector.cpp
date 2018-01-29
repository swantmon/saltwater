
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

namespace MR
{

    void CPlaneDetector::SetDepthImage(Gfx::CTexturePtr _DepthImage)
    {
        m_DepthImage = _DepthImage;
    }

    // -----------------------------------------------------------------------------

    void CPlaneDetector::DetectPlanes(Gfx::CTexturePtr _DepthImage)
    {
        if (_DepthImage != nullptr)
        {
            m_DepthImage = _DepthImage;
        }
    }

    // -----------------------------------------------------------------------------

    CPlaneDetector::CPlaneDetector()
    {

    }

    // -----------------------------------------------------------------------------

    CPlaneDetector::~CPlaneDetector()
    {

    }

} // namespace MR