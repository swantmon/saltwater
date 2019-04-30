
#pragma once

#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/core/core_program_parameters.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_selection.h"
#include "engine/graphic/gfx_selection_renderer.h"
#include "engine/graphic/gfx_view_manager.h"

#include "plugin/slam/mr_slam_reconstructor.h"

namespace MR
{
    class CPlaneExtractor
    {
    public:

        CPlaneExtractor(MR::CSLAMReconstructor* _pReconstructor);
        ~CPlaneExtractor();

        void UpdatePlane(int _PlaneID);

    private:

        enum EPLANETYPE
        {
            FLOOR,
            WALL,
            CEILING
        };

    private:

        MR::CSLAMReconstructor* m_pReconstructor;
    };
} // namespace Scpt