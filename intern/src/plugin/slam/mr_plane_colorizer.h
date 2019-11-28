
#pragma once

#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/core/core_program_parameters.h"

#include "engine/engine.h"

#include "engine/core/core_plugin_manager.h"

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

        void UpdatePlane(const std::string& _ID);
        void UpdatePlane(CSLAMReconstructor::SPlane& _rPlane);

        void ColorizeAllPlanes();

    private:

        void ColorizePlane(CSLAMReconstructor::SPlane& _rPlane, bool _WholeExtent);

        enum EPLANETYPE
        {
            FLOOR,
            WALL,
            CEILING
        };

        void SetupShaders();
        void SetupBuffers();
        void SetupStates();
        void SetupMeshes();

    private:

        MR::CSLAMReconstructor* m_pReconstructor;

        int m_PlaneTextureSize;
        float m_CameraOffset;
        float m_MaxRaycastLength;

        Gfx::CTexturePtr m_DummyTexturePtr;

        Gfx::CShaderPtr m_PlaneColorizationVSPtr;
        Gfx::CShaderPtr m_PlaneColorizationFSPtr;

        Gfx::CShaderPtr m_ExtentColorizationVSPtr;
        Gfx::CShaderPtr m_ExtentColorizationFSPtr;

        Gfx::CBufferPtr m_ConstantBufferPtr;
        
        Gfx::CMeshPtr m_ExtentMeshPtr;

        Gfx::CInputLayoutPtr m_PlaneMeshLayoutPtr;

        Gfx::CViewPortSetPtr m_ViewPortSetPtr;

        Gfx::CTargetSetPtr m_TargetSetPtr;

        using InpaintWithPixMixFunc = void(*)(const glm::ivec2&, const std::vector<glm::u8vec4>&, std::vector<glm::u8vec4>&);
        InpaintWithPixMixFunc InpaintWithPixMix;
    };
} // namespace Scpt