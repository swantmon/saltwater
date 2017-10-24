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

		struct SRootVolume
		{
            Base::Int3 m_Offset;
            bool m_IsVisible;
            int m_PoolIndex;
            Gfx::CBufferPtr m_Level1QueuePtr;
            Gfx::CBufferPtr m_Level2QueuePtr;
            int m_Level1QueueSize;
            int m_Level2QueueSize;
            Gfx::CBufferPtr m_IndirectLevel1Buffer;
            Gfx::CBufferPtr m_IndirectLevel2Buffer;
		};

        struct SScalableVolume
        {
            Gfx::CBufferPtr m_RootVolumePositionBufferPtr;// 3D Buffer with indices to m_RootVolumePoolPtr
            Gfx::CBufferPtr m_RootVolumePoolPtr;          // Individual Volumes
            Gfx::CBufferPtr m_RootGridPoolPtr;            // Highest level grids (16x16x16)
            Gfx::CBufferPtr m_Level1PoolPtr;              // Internal Grid       ( 8x 8x 8)
            Gfx::CBufferPtr m_TSDFPoolPtr;                // TSDF Data           ( 8x 8x 8)
            Gfx::CBufferPtr m_PoolItemCountBufferPtr;
            Base::Int3 m_MinOffset;                         // AABB of the whole reconstruction
            Base::Int3 m_MaxOffset;
            int m_RootVolumeTotalWidth;
        };

        typedef std::map<Base::Int3, SRootVolume, IndexCompare> CRootVolumeMap;
        typedef std::vector<SRootVolume*> CRootVolumeVector;

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

		CRootVolumeMap& GetRootVolumeMap();
        CRootVolumeVector& GetRootVolumeVector();

        SScalableVolume& GetVolume();

        const std::vector<float>& GetVolumeSizes() const;

        void GetReconstructionSettings(SReconstructionSettings* pReconstructionSettings);

        Gfx::CTexture2DPtr GetVertexMap();
        
    private:

        void Start();
        void Exit();

		Base::Float4 GetHessianNormalForm(const Base::Float3& rA, const Base::Float3& rB, const Base::Float3& rC);
		float GetPointPlaneDistance(const Base::Float3& rPoint, const Base::Float4& rPlane);
		bool RootGridInFrustum(const Base::Int3& rKey);

		void UpdateRootrids();
        void CreateIntegrationQueues(std::vector<uint32_t>& rVolumeQueue);
        void IntegrateHierarchies(std::vector<uint32_t>& rVolumeQueue);

        void RasterizeRootGrid(SRootVolume& rRootGrid);
        void RasterizeRootGridReverse(SRootVolume& rRootGrid);
        void RasterizeLevel1Grid(SRootVolume& rRootGrid);

        void RasterizeFullVolumeReverse(SRootVolume& rRootGrid);

        void ClearBuffer(Gfx::CBufferPtr BufferPtr, size_t Size);
        void ClearPool();

		void SetupData();
        void SetupMeshes();
        void SetupRenderStates();

        void SetupShaders();
        void SetupTextures();
        void SetupBuffers();

        void CreateReferencePyramid();
        void RasterizeRootVolumes();
        void GatherVolumeCounters(unsigned int Count, Gfx::CBufferPtr CounterBufferPtr, Gfx::CBufferPtr QueueBuffer, Gfx::CBufferPtr IndirectBufferPtr);
        void GatherGridCounters(unsigned int Count, Gfx::CBufferPtr CounterBufferPtr, Gfx::CBufferPtr QueueBuffer, Gfx::CBufferPtr IndirectBufferPtr);

        void Raycast();
        void CreateRaycastPyramid();

        void PerformTracking();

		void UpdateFrustum();

        void DetermineSummands(int PyramidLevel, const Base::Float4x4& rIncPoseMatrix);
        void ReduceSum(int PyramidLevel);
        bool CalculatePoseMatrix(Base::Float4x4& rIncPoseMatrix);

        Gfx::CMeshPtr CreateGridMesh(int Width);
        
    private:

        static int DivUp(int TotalShaderCount, int WorkGroupSize);

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
        Gfx::CBufferPtr m_PositionConstantBufferPtr;
        Gfx::CBufferPtr m_HierarchyConstantBufferPtr;

        Gfx::CBufferPtr m_IndexedIndirectBufferPtr;
        
        Gfx::CBufferPtr m_RootVolumeInstanceBufferPtr;
        Gfx::CBufferPtr m_VolumeAtomicCounterBufferPtr;
        Gfx::CBufferPtr m_VolumeQueueBufferPtr;

        Gfx::CBufferPtr m_GridRasterizationBufferPtr;
        Gfx::CBufferPtr m_PointRasterizationBufferPtr;

        SScalableVolume m_VolumeBuffers;

        Gfx::CBufferPtr m_VolumeIndexBufferPtr;

        Gfx::CShaderPtr m_ClearVolumeCSPtr;
        Gfx::CShaderPtr m_BilateralFilterCSPtr;
        Gfx::CShaderPtr m_VertexMapCSPtr;
        Gfx::CShaderPtr m_NormalMapCSPtr;
        Gfx::CShaderPtr m_DownSampleDepthCSPtr;
        Gfx::CShaderPtr m_IntegrateRootGridCSPtr;
        Gfx::CShaderPtr m_IntegrateLevel1GridCSPtr;
        Gfx::CShaderPtr m_IntegrateTSDFCSPtr;
        Gfx::CShaderPtr m_RaycastCSPtr;
        Gfx::CShaderPtr m_RaycastPyramidCSPtr;
        Gfx::CShaderPtr m_DetermineSummandsCSPtr;
        Gfx::CShaderPtr m_ReduceSumCSPtr;
		Gfx::CShaderPtr m_RootgridDepthCSPtr;
        Gfx::CShaderPtr m_VolumeCountersCSPtr;
        Gfx::CShaderPtr m_RasterizeRootVolumeVSPtr;
        Gfx::CShaderPtr m_RasterizeRootVolumeFSPtr;
        Gfx::CShaderPtr m_GridCountersCSPtr;

        Gfx::CShaderPtr m_RasterizeRootGridVSPtr;
        Gfx::CShaderPtr m_RasterizeRootGridFSPtr;
        Gfx::CShaderPtr m_RasterizeLevel1GridVSPtr;
        Gfx::CShaderPtr m_RasterizeLevel1GridFSPtr;

        Gfx::CShaderPtr m_PointsRootGridVSPtr;
        Gfx::CShaderPtr m_PointsRootGridGSPtr;
        Gfx::CShaderPtr m_PointsRootGridFSPtr;
        Gfx::CShaderPtr m_PointsRootGridCSPtr;
        Gfx::CShaderPtr m_PointsFullCSPtr;

        Gfx::CShaderPtr m_ClearAtomicCountersCSPtr;

        Gfx::CShaderPtr m_FillIndirectBufferCSPtr;

        Gfx::CMeshPtr m_CubeMeshPtr;
        Gfx::CMeshPtr m_Grid8MeshPtr;
        Gfx::CMeshPtr m_Grid16MeshPtr;
        
        Gfx::CInputLayoutPtr m_CubeInputLayoutPtr;
        Gfx::CTargetSetPtr m_EmptyTargetSetPtr;
        Gfx::CViewPortSetPtr m_DepthViewPortSetPtr;

        Gfx::CTexture3DPtr m_RootGridVolumePtr;
        Gfx::CTargetSetPtr m_RootGridVolumeTargetSetPtr;
        Gfx::CViewPortSetPtr m_RootGridViewPort;

        Gfx::CTexture3DPtr m_FullVolumePtr;
        Gfx::CTargetSetPtr m_FullVolumeTargetSetPtr;
        Gfx::CViewPortSetPtr m_FullVolumeViewPort;

        Gfx::CTexture2DPtr m_RawDepthBufferPtr;
        Gfx::CTexture2DPtr m_RawCameraFramePtr;
        Gfx::CTexture2DPtr m_RawVertexMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_SmoothDepthBufferPtr;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceVertexMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_ReferenceNormalMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_RaycastVertexMapPtr;
        std::vector<Gfx::CTexture2DPtr> m_RaycastNormalMapPtr;
        
        Gfx::CBufferPtr m_AtomicCounterBufferPtr;

		CRootVolumeMap m_RootVolumeMap;
        CRootVolumeVector m_RootVolumeVector;

        Gfx::CBufferPtr m_ICPResourceBufferPtr;

        std::unique_ptr<MR::IRGBDCameraControl> m_pRGBDCameraControl;

        Base::Float4x4 m_PoseMatrix;
        
        std::vector<unsigned short> m_DepthPixels;
        std::vector<Base::Byte4> m_CameraPixels;

        int m_RootVolumePoolItemCount;

        int m_IntegratedFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;

        bool m_IsIntegrationPaused;
        bool m_IsTrackingPaused;

		std::vector<float> m_VolumeSizes;
        
		std::array<Base::Float3, 8> m_FrustumPoints;
		std::array<Base::Float4, 6> m_FrustumPlanes;
        
        bool m_IsConservativeRasterizationAvailable;
    };
} // namespace MR
