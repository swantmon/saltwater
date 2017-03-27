//
//  mr_slam_reconstructor.h
//  graphic
//
//  Created by Tobias Schwandt on 12/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture_3d.h"

#include <vector>

namespace MR
{
    class IRGBDCameraControl;

    class CSLAMReconstructor : private Base::CUncopyable
    {
    public:

        struct ReconstructionSettings
        {
            const static int MAX_PYRAMIDLEVELS = 8;

            float m_VolumeSize;
            int m_VolumeResolution;
            float m_TruncatedDistance;
            int m_MaxIntegrationWeight;
            int m_PyramidLevelCount;
            int m_PyramidLevelIterations[MAX_PYRAMIDLEVELS];
            Base::Int2 m_DepthThreshold;

            ReconstructionSettings()
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

        CSLAMReconstructor(const ReconstructionSettings* pReconstructionSettings = nullptr);
        ~CSLAMReconstructor();

    public:

        void Update();
        void ResetReconstruction(const ReconstructionSettings* pReconstructionSettings = nullptr);

        bool IsTrackingLost() const;
        Base::Float4x4 GetPoseMatrix() const;
        Gfx::CTexture3DPtr GetVolume();

        void GetReconstructionData(ReconstructionSettings& rReconstructionSettings);

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

        ReconstructionSettings m_ReconstructionSettings;

        Gfx::CBufferPtr m_IntrinsicsConstantBuffer;
        Gfx::CBufferPtr m_TrackingDataConstantBuffer;
        Gfx::CBufferPtr m_RaycastPyramidConstantBuffer;
        Gfx::CBufferPtr m_ICPSummationConstantBuffer;
        Gfx::CBufferPtr m_IncPoseMatrixConstantBuffer;
        Gfx::CBufferPtr m_BilateralFilterConstantBuffer;

        Gfx::CShaderPtr m_CSClearVolume;
        Gfx::CShaderPtr m_CSMirrorDepth;
        Gfx::CShaderPtr m_CSBilateralFilter;
        Gfx::CShaderPtr m_CSVertexMap;
        Gfx::CShaderPtr m_CSNormalMap;
        Gfx::CShaderPtr m_CSDownSampleDepth;
        Gfx::CShaderPtr m_CSVolumeIntegration;
        Gfx::CShaderPtr m_CSRaycast;
        Gfx::CShaderPtr m_CSRaycastPyramid;
        Gfx::CShaderPtr m_CSDetermineSummands;
        Gfx::CShaderPtr m_CSReduceSum;

        Gfx::CTexture2DPtr m_RawDepthBuffer;
        std::vector<Gfx::CTexture2DPtr> m_SmoothDepthBuffer;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceVertexMap;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceNormalMap;
        std::vector<Gfx::CTexture2DPtr> m_RaycastVertexMap;
        std::vector<Gfx::CTexture2DPtr> m_RaycastNormalMap;

        Gfx::CTexture3DPtr m_Volume;

        Gfx::CBufferPtr m_ICPBuffer;

        std::unique_ptr<MR::IRGBDCameraControl> m_pRGBDCameraControl;

        Base::Float4x4 m_PoseMatrix;
        
        std::vector<unsigned short> m_DepthPixels;

        int m_IntegratedDepthFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;
    };
} // namespace MR
