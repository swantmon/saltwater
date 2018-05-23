
#pragma once

#include "base/base_uncopyable.h"
#include "base/base_include_glm.h"

#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_texture.h"

#include "plugin/slam/mr_icp_tracker.h"
#include "plugin/slam/mr_slam_reconstruction_settings.h"

#include <memory>
#include <vector>

namespace MR
{
    class IRGBDCameraControl;

    class CSLAMReconstructor : private Base::CUncopyable
	{
    public:

        CSLAMReconstructor(const SReconstructionSettings* pReconstructionSettings = nullptr);
        ~CSLAMReconstructor();

    public:

        void Update();
        void ResetReconstruction(const SReconstructionSettings* pReconstructionSettings = nullptr);

        void PauseIntegration(bool _Paused);
        void PauseTracking(bool _Paused);
        bool IsTrackingLost() const;
        glm::mat4 GetPoseMatrix() const;
        Gfx::CTexturePtr GetTSDFVolume();
        Gfx::CTexturePtr GetColorVolume();

        void GetReconstructionSettings(SReconstructionSettings* pReconstructionSettings);

    private:

        void Start();
        void Exit();

        void SetupShaders();
        void SetupTextures();
        void SetupBuffers();

        void CreateReferencePyramid();
        void Integrate();
        void Raycast();
        void CreateRaycastPyramid();
        
        void ClearVolume();

    private:

        static int GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize);

    private:

        SReconstructionSettings m_ReconstructionSettings;

        Gfx::CBufferPtr m_IntrinsicsConstantBufferPtr;
        Gfx::CBufferPtr m_TrackingDataConstantBufferPtr;
        Gfx::CBufferPtr m_RaycastPyramidConstantBufferPtr;
        Gfx::CBufferPtr m_BilateralFilterConstantBufferPtr;

        Gfx::CShaderPtr m_ClearVolumeCSPtr;
        Gfx::CShaderPtr m_BilateralFilterCSPtr;
        Gfx::CShaderPtr m_VertexMapCSPtr;
        Gfx::CShaderPtr m_NormalMapCSPtr;
        Gfx::CShaderPtr m_DownSampleDepthCSPtr;
        Gfx::CShaderPtr m_IntegrationCSPtr;
        Gfx::CShaderPtr m_RaycastCSPtr;
        Gfx::CShaderPtr m_RaycastPyramidCSPtr;

        Gfx::CTexturePtr m_RawDepthBufferPtr;
        Gfx::CTexturePtr m_RawCameraFramePtr;
        std::vector<Gfx::CTexturePtr> m_SmoothDepthBufferPtr;
        std::vector<Gfx::CTexturePtr> m_ReferenceVertexMapPtr;
        std::vector<Gfx::CTexturePtr> m_ReferenceNormalMapPtr;
        std::vector<Gfx::CTexturePtr> m_RaycastVertexMapPtr;
        std::vector<Gfx::CTexturePtr> m_RaycastNormalMapPtr;

        Gfx::CTexturePtr m_TSDFVolumePtr;
        Gfx::CTexturePtr m_ColorVolumePtr;
        
        std::unique_ptr<MR::IRGBDCameraControl> m_pRGBDCameraControl;

        glm::mat4 m_PoseMatrix;
        
        std::vector<unsigned short> m_DepthPixels;
        std::vector<char> m_CameraPixels;

        int m_IntegratedFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;

        bool m_IsIntegrationPaused;
        bool m_IsTrackingPaused;

        std::unique_ptr<CICPTracker> m_pTracker;
    };
} // namespace MR
