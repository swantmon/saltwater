//
//  mr_slam_reconstructor.h
//  graphic
//
//  Created by Tobias Schwandt on 12/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_uncopyable.h"
#include "base/base_matrix4x4.h"
#include "base/base_vector2.h"

#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_3d.h"

#include <vector>

namespace MR
{
    class IRGBDCameraControl;

    class CSLAMReconstructor : private Base::CUncopyable
    {        
    public:

        struct SReconstructionSettings
        {
            const static int MAX_PYRAMIDLEVELS = 8;

            float m_VolumeSize;
            int m_VolumeResolution;
            float m_TruncatedDistance;
            int m_MaxIntegrationWeight;
            int m_PyramidLevelCount;
            int m_PyramidLevelIterations[MAX_PYRAMIDLEVELS];
            Base::Int2 m_DepthThreshold;

            SReconstructionSettings()
                : m_VolumeSize(1.0f)
                , m_VolumeResolution(256)
                , m_TruncatedDistance(30.0f)
                , m_MaxIntegrationWeight(200)
                , m_PyramidLevelCount(3)
                , m_DepthThreshold(500, 8000)
            {
                m_PyramidLevelIterations[0] = 10;
                m_PyramidLevelIterations[1] = 5;
                m_PyramidLevelIterations[2] = 4;
            }
        };

    public:

        CSLAMReconstructor(const SReconstructionSettings* pReconstructionSettings = nullptr);
        ~CSLAMReconstructor();

    public:

        void Update();
        void ResetReconstruction(const SReconstructionSettings* pReconstructionSettings = nullptr);

        bool IsTrackingLost() const;
        Base::Float4x4 GetPoseMatrix() const;
        Gfx::CTexture3DPtr GetVolume();

        void GetReconstructionData(SReconstructionSettings& rReconstructionSettings);

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

        void PerformTracking();

        void DetermineSummands(int PyramidLevel, const Base::Float4x4& rIncPoseMatrix);
        void ReduceSum(int PyramidLevel);
        bool CalculatePoseMatrix(Base::Float4x4& rIncPoseMatrix);
        void ClearVolume();

    private:

        static int GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize);

    private:

        SReconstructionSettings m_ReconstructionSettings;

        Gfx::CBufferPtr m_IntrinsicsConstantBufferPtr;
        Gfx::CBufferPtr m_TrackingDataConstantBufferPtr;
        Gfx::CBufferPtr m_RaycastPyramidConstantBufferPtr;
        Gfx::CBufferPtr m_ICPSummationConstantBufferPtr;
        Gfx::CBufferPtr m_IncPoseMatrixConstantBufferPtr;
        Gfx::CBufferPtr m_BilateralFilterConstantBufferPtr;

        Gfx::CShaderPtr m_ClearVolumeCSPtr;
        Gfx::CShaderPtr m_MirrorDepthCSPtr;
        Gfx::CShaderPtr m_BilateralFilterCSPtr;
        Gfx::CShaderPtr m_VertexMapCSPtr;
        Gfx::CShaderPtr m_NormalMapCSPtr;
        Gfx::CShaderPtr m_DownSampleDepthCSPtr;
        Gfx::CShaderPtr m_VolumeIntegrationCSPtr;
        Gfx::CShaderPtr m_RaycastCSPtr;
        Gfx::CShaderPtr m_RaycastPyramidCSPtr;
        Gfx::CShaderPtr m_DetermineSummandsCSPtr;
        Gfx::CShaderPtr m_ReduceSumCSPtr;

        Gfx::CTexture2DPtr m_RawDepthBufferPtr;
        std::vector<Gfx::CTexture2DPtr> m_SmoothDepthBufferPtr;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceVertexMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceNormalMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_RaycastVertexMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_RaycastNormalMapPtr;

        Gfx::CTexture3DPtr m_VolumePtr;

        Gfx::CBufferPtr m_ICPResourceBufferPtr;

        std::unique_ptr<MR::IRGBDCameraControl> m_pRGBDCameraControl;

        Base::Float4x4 m_PoseMatrix;
        
        std::vector<unsigned short> m_DepthPixels;

        int m_IntegratedDepthFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;
    };
} // namespace MR
