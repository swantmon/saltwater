//
//  mr_slam_reconstructor.h
//  graphic
//
//  Created by Tobias Schwandt on 12/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_shader.h"

#include <gl/glew.h>

namespace MR
{
    class IRGBDCameraControl;

    class CSLAMReconstructor : private Base::CUncopyable
    {
    public:
        CSLAMReconstructor();
        ~CSLAMReconstructor();

    public:

        void Start();
        void Exit();

        void Update();
        void ResetReconstruction();

        bool IsTrackingLost();
        Base::Float4x4 GetPoseMatrix();
        GLuint GetVolume();

    private:

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
        
        int GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize);
        void ClearVolume();

    private:

        GLuint m_DrawCallConstantBuffer;
        GLuint m_IntrinsicsConstantBuffer;
        GLuint m_TrackingDataConstantBuffer;
        GLuint m_RaycastPyramidConstantBuffer;
        GLuint m_ICPSummationConstantBuffer;
        GLuint m_IncPoseMatrixConstantBuffer;
        GLuint m_BilateralFilterConstantBuffer;

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

        GLuint m_RawDepthBuffer;
        GLuint *m_SmoothDepthBuffer;
        GLuint *m_ReferenceVertexMap;
        GLuint *m_ReferenceNormalMap;
        GLuint *m_RaycastVertexMap;
        GLuint *m_RaycastNormalMap;

        GLuint m_Volume;

        GLuint m_ICPBuffer;

        std::unique_ptr<MR::IRGBDCameraControl> m_pRGBDCameraControl;

        Base::Float4x4 m_PoseMatrix;
        
        std::vector<unsigned short> m_DepthPixels;

        int m_IntegratedDepthFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;
    };
} // namespace MR
