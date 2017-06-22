//
//  mr_scalable_slam_reconstructor.h
//  graphic
//
//  Created by Tobias Schwandt on 12/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_uncopyable.h"
#include "base/base_matrix4x4.h"
#include "base/base_vector2.h"

#include "mr/mr_slam_reconstruction_settings.h"

#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_3d.h"

#include <array>
#include <map>
#include <vector>

namespace MR
{
    class IRGBDCameraControl;

	struct IndexCompare
	{
		bool operator()(const Base::Int3& rLeft, const Base::Int3& rRight)
		{
			if (rLeft[0] < rRight[0])
			{
				return true;
			}
			if (rLeft[0] > rRight[0])
			{
				return false;
			}
			if (rLeft[1] < rRight[1])
			{
				return true;
			}
			if (rLeft[1] > rRight[1])
			{
				return false;
			}
			if (rLeft[2] < rRight[2])
			{
				return true;
			}
			if (rLeft[2] > rRight[2])
			{
				return false;
			}
			return false;
		}
	};

    class CScalableSLAMReconstructor : private Base::CUncopyable
    {
	public:

		struct SRootGrid
		{
			Gfx::CTexture3DPtr m_TSDFVolumePtr;
			Gfx::CTexture3DPtr m_ColorVolumePtr;
			Base::Int3 m_Offset;
			bool m_IsVisible;
		};

		typedef std::map<Base::Int3, SRootGrid, IndexCompare> CRootGridMap;

    public:

        CScalableSLAMReconstructor(const SReconstructionSettings* pReconstructionSettings = nullptr);
        ~CScalableSLAMReconstructor();

    public:

        void Update();
        void ResetReconstruction(const SReconstructionSettings* pReconstructionSettings = nullptr);

        void PauseIntegration(bool _Paused);
        void PauseTracking(bool _Paused);
        bool IsTrackingLost() const;
        Base::Float4x4 GetPoseMatrix() const;

		CRootGridMap& GetRootGrids();

        void GetReconstructionSettings(SReconstructionSettings* pReconstructionSettings);

    private:

        void Start();
        void Exit();

		Base::Float4 GetHessianNormalForm(const Base::Float3& rA, const Base::Float3& rB, const Base::Float3& rC);
		float GetPointPlaneDistance(const Base::Float3& rPoint, const Base::Float4& rPlane);
		bool RootGridVisible(const Base::Int3& rKey);
		void UpdateRootrids();

		void SetupData();

        void SetupShaders();
        void SetupTextures();
        void SetupBuffers();

        void CreateReferencePyramid();
        void Integrate();
        void Raycast();
        void CreateRaycastPyramid();

        void PerformTracking();

		void UpdateFrustum();

        void DetermineSummands(int PyramidLevel, const Base::Float4x4& rIncPoseMatrix);
        void ReduceSum(int PyramidLevel);
        bool CalculatePoseMatrix(Base::Float4x4& rIncPoseMatrix);

    private:

        static int GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize);

	private:

		static const int g_FrustumCorners = 8;

    private:

        SReconstructionSettings m_ReconstructionSettings;

        Gfx::CBufferPtr m_IntrinsicsConstantBufferPtr;
        Gfx::CBufferPtr m_TrackingDataConstantBufferPtr;
        Gfx::CBufferPtr m_RaycastPyramidConstantBufferPtr;
        Gfx::CBufferPtr m_ICPSummationConstantBufferPtr;
        Gfx::CBufferPtr m_IncPoseMatrixConstantBufferPtr;
        Gfx::CBufferPtr m_BilateralFilterConstantBufferPtr;
		Gfx::CBufferPtr m_IntegrationConstantBufferPtr;

        Gfx::CShaderPtr m_ClearVolumeCSPtr;
        Gfx::CShaderPtr m_BilateralFilterCSPtr;
        Gfx::CShaderPtr m_VertexMapCSPtr;
        Gfx::CShaderPtr m_NormalMapCSPtr;
        Gfx::CShaderPtr m_DownSampleDepthCSPtr;
        Gfx::CShaderPtr m_IntegrationCSPtr;
        Gfx::CShaderPtr m_RaycastCSPtr;
        Gfx::CShaderPtr m_RaycastPyramidCSPtr;
        Gfx::CShaderPtr m_DetermineSummandsCSPtr;
        Gfx::CShaderPtr m_ReduceSumCSPtr;

        Gfx::CTexture2DPtr m_RawDepthBufferPtr;
        Gfx::CTexture2DPtr m_RawCameraFramePtr;
        std::vector<Gfx::CTexture2DPtr> m_SmoothDepthBufferPtr;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceVertexMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceNormalMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_RaycastVertexMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_RaycastNormalMapPtr;

		CRootGridMap m_RootGrids;

        Gfx::CBufferPtr m_ICPResourceBufferPtr;

        std::unique_ptr<MR::IRGBDCameraControl> m_pRGBDCameraControl;

        Base::Float4x4 m_PoseMatrix;
        
        std::vector<unsigned short> m_DepthPixels;
        std::vector<Base::Byte4> m_CameraPixels;

        int m_IntegratedFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;

        bool m_IsIntegrationPaused;
        bool m_IsTrackingPaused;

		std::vector<float> m_GridSizes;

		std::array<Base::Float3, 8> m_FrustumPoints;
		std::array<Base::Float4, 6> m_FrustumPlanes;
    };
} // namespace MR
