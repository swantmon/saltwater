
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "plugin/slam/mr_plane_extractor.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <gl/glew.h>

using namespace MR;
using namespace Gfx;

namespace
{
    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

} // namespace

namespace MR
{
    // -----------------------------------------------------------------------------

    void CPlaneExtractor::UpdatePlane(int _PlaneID)
    {
        auto& rPlaneMap = m_pReconstructor->GetPlanes();

        auto Iter = rPlaneMap.find(_PlaneID);

        if (Iter != rPlaneMap.end())
        {
            const auto& rPlane = *Iter;
        }
    }

    // -----------------------------------------------------------------------------

    CPlaneExtractor::CPlaneExtractor(MR::CSLAMReconstructor* _pReconstructor)
        : m_pReconstructor(_pReconstructor)
    {
        assert(_pReconstructor != nullptr);
    }

    // -----------------------------------------------------------------------------

    CPlaneExtractor::~CPlaneExtractor()
    {

    }

} // namespace MR
