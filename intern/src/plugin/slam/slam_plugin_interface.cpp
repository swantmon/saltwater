
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/engine.h"

#include "plugin/slam/slam_plugin_interface.h"

#include "plugin/slam/gfx_reconstruction_renderer.h"

#include <iostream>

CORE_PLUGIN_INFO(SLAM::CPluginInterface, "SLAM", "1.0", "This plugin provides Simultaneous Localization and Mapping based on a TSDF.")

namespace SLAM
{
    void CPluginInterface::InitializeReconstructor()
    {
        Gfx::ReconstructionRenderer::GetReconstructor().Start();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::TerminateReconstructor()
    {
        Gfx::ReconstructionRenderer::GetReconstructor().Exit();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnNewFrame(Gfx::CTexturePtr DepthBuffer, Gfx::CTexturePtr ColorBuffer, const glm::mat4* pTransform)
    {
        Gfx::ReconstructionRenderer::GetReconstructor().OnNewFrame(DepthBuffer, ColorBuffer, pTransform);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::SetImageSizesAndIntrinsicData(glm::vec4 _ImageSizes, glm::vec4 _Intrinsics)
    {
        glm::ivec2 DepthSize = glm::ivec2(_ImageSizes.x, _ImageSizes.y);
        glm::ivec2 ColorSize = glm::ivec2(glm::max(_ImageSizes.z, 1.0f), glm::max(_ImageSizes.w, 1.0f));

        glm::vec2 FocalLength = glm::vec2(_Intrinsics.x, _Intrinsics.y);
        glm::vec2 FocalPoint = glm::vec2(_Intrinsics.z, _Intrinsics.w);
        
        Gfx::ReconstructionRenderer::GetReconstructor().SetImageSizes(DepthSize, ColorSize);
        Gfx::ReconstructionRenderer::GetReconstructor().SetIntrinsics(FocalLength, FocalPoint);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ResetReconstruction()
    {
        Gfx::ReconstructionRenderer::GetReconstructor().ResetReconstruction();
    }

    // -----------------------------------------------------------------------------

    glm::mat4 CPluginInterface::GetPoseMatrix()
    {
        return Gfx::ReconstructionRenderer::GetReconstructor().GetPoseMatrix();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        Gfx::ReconstructionRenderer::OnStart();
        Gfx::ReconstructionRenderer::OnSetupShader();
        Gfx::ReconstructionRenderer::OnSetupKernels();
        Gfx::ReconstructionRenderer::OnSetupRenderTargets();
        Gfx::ReconstructionRenderer::OnSetupStates();
        Gfx::ReconstructionRenderer::OnSetupTextures();
        Gfx::ReconstructionRenderer::OnSetupBuffers();
        Gfx::ReconstructionRenderer::OnSetupResources();
        Gfx::ReconstructionRenderer::OnSetupModels();
        Gfx::ReconstructionRenderer::OnSetupEnd();

        Engine::RegisterEventHandler(Engine::Gfx_OnUpdate, Gfx::ReconstructionRenderer::Update);
        Engine::RegisterEventHandler(Engine::Gfx_OnRenderGBuffer, Gfx::ReconstructionRenderer::Render);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        Gfx::ReconstructionRenderer::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("SLAM plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("SLAM plugin resumed!");
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void InitializeReconstructor()
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).InitializeReconstructor();
}

extern "C" CORE_PLUGIN_API_EXPORT void TerminateReconstructor()
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).TerminateReconstructor();
}

extern "C" CORE_PLUGIN_API_EXPORT void OnNewDepthFrame(Gfx::CTexturePtr DepthFrame, Gfx::CTexturePtr ColorBuffer, const glm::mat4* pTransform)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).OnNewFrame(DepthFrame, ColorBuffer, pTransform);
}

extern "C" CORE_PLUGIN_API_EXPORT void SetImageSizesAndIntrinsicData(glm::vec4 _ImageSizes, glm::vec4 _Intrinsics)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).SetImageSizesAndIntrinsicData(_ImageSizes, _Intrinsics);
}

extern "C" CORE_PLUGIN_API_EXPORT void ResetReconstruction()
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).ResetReconstruction();
}

extern "C" CORE_PLUGIN_API_EXPORT void GetPoseMatrix(glm::mat4& _PoseMatrix)
{
    _PoseMatrix = static_cast<SLAM::CPluginInterface&>(GetInstance()).GetPoseMatrix();
}