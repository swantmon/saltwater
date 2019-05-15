
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
    class CPlaneColorizer
    {
    public:

        CPlaneColorizer(MR::CSLAMReconstructor* _pReconstructor);
        ~CPlaneColorizer();

        void UpdatePlane(int _PlaneID);
        void UpdatePlane(CSLAMReconstructor::SPlane& _rPlane);

        void ColorizeAllPlanes();

    private:

        void ColorizePlane(CSLAMReconstructor::SPlane& _rPlane);

        enum EPLANETYPE
        {
            FLOOR,
            WALL,
            CEILING
        };

    private:

        MR::CSLAMReconstructor* m_pReconstructor;

        Gfx::CTexturePtr m_DummyTexturePtr;

        int m_PlaneTextureSize;
    };
} // namespace Scpt