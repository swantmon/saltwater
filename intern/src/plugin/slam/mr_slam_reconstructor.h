
#pragma once

#include "base/base_uncopyable.h"
#include "base/base_include_glm.h"

#include "plugin/slam/mr_slam_reconstruction_settings.h"
#include "plugin/slam/mr_icp_tracker.h"

#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_view_port_set.h"

#include <array>
#include <map>
#include <memory>
#include <vector>

namespace MR
{
    class IRGBDCameraControl;

    struct IndexCompare
    {
        bool operator()(const glm::ivec3& rLeft, const glm::ivec3& rRight) const
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

    class CSLAMReconstructor : private Base::CUncopyable
    {
    public:

        struct SPlane
        {
            glm::mat4 m_Transform;
            glm::vec4 m_Extent;

            Gfx::CMeshPtr m_Mesh;
        };

        struct SPlaneVertex
        {
            glm::vec3 m_Position;
            glm::vec2 m_UV;
        };

        using CPlaneVertices = std::vector<SPlaneVertex>;
        using CPlaneIndices = std::vector<uint32_t>;

        struct SIndirectParameters
        {
            uint32_t m_Count;
            uint32_t m_InstanceCount;
            uint32_t m_FirstIndex;
            uint32_t m_BaseIndex;
        };

        struct SIndexedParameters
        {
            uint32_t m_IndexCount;
            uint32_t m_InstanceCount;
            uint32_t m_FirstIndex;
            uint32_t m_BaseVertex;
            uint32_t m_BaseInstance;
        };

        struct SComputeParameters
        {
            uint32_t m_WorkGroupsX;
            uint32_t m_WorkGroupsY;
            uint32_t m_WorkGroupsZ;
        };

        struct SIndirectBuffers
        {
            SIndirectParameters m_Draw;
            SIndexedParameters m_Indexed;
            SComputeParameters m_ComputeDiv;
            SComputeParameters m_Compute;

            static const int s_DrawOffset = 0;
            static const int s_IndexedOffset = sizeof(SIndirectParameters);
            static const int s_ComputeDivOffset = s_IndexedOffset + sizeof(SIndexedParameters);
            static const int s_ComputeOffset = s_ComputeDivOffset + sizeof(SComputeParameters);
        };

        struct SRootVolume
        {
            glm::ivec3 m_Offset;
            bool m_IsVisible;
            int m_PoolIndex;
            Gfx::CBufferPtr m_Level1QueuePtr;
            Gfx::CBufferPtr m_Level2QueuePtr;
            Gfx::CBufferPtr m_IndirectLevel1Buffer;
            Gfx::CBufferPtr m_IndirectLevel2Buffer;
        };

        struct SRaycastConstantBuffer
        {
            glm::vec3 m_AABBMin;
            int m_MinWeight;
            glm::vec3 m_AABBMax;
            int m_VolumeTextureWidth;
        };

        struct SSLAMVolume
        {
            Gfx::CBufferPtr m_RootVolumePositionBufferPtr;// 3D Buffer with indices to m_RootVolumePoolPtr
            Gfx::CBufferPtr m_RootVolumePoolPtr;          // Individual Volumes
            Gfx::CBufferPtr m_RootGridPoolPtr;            // Highest level grids (16x16x16)
            Gfx::CBufferPtr m_Level1PoolPtr;              // Internal Grid       ( 8x 8x 8)
            Gfx::CBufferPtr m_TSDFPoolPtr;                // TSDF Data           ( 8x 8x 8)
            Gfx::CBufferPtr m_PoolItemCountBufferPtr;
            Gfx::CBufferPtr m_AABBBufferPtr;
            glm::ivec3 m_MinOffset;                         // AABB of the whole reconstruction
            glm::ivec3 m_MaxOffset;
            int m_RootVolumeTotalWidth;
            int m_RootGridPoolSize;
            int m_Level1PoolSize;
            int m_TSDFPoolSize;
        };

        typedef std::map<glm::ivec3, SRootVolume, IndexCompare> CRootVolumeMap;
        typedef std::vector<SRootVolume*> CRootVolumeVector;

    public:

        CSLAMReconstructor(const SReconstructionSettings* pReconstructionSettings = nullptr);
        ~CSLAMReconstructor();

    public:

        void ResetReconstruction(const SReconstructionSettings* pReconstructionSettings = nullptr);

        void AddPlane(glm::mat4 _Transform, glm::vec4 _Extent, int _ID);
        void UpdatePlane(glm::mat4 _Transform, glm::vec4 _Extent, int _ID);

        void AddPlaneWithMesh(glm::mat4 _Transform, glm::vec4 _Extent, const CPlaneVertices& _rVertices, const CPlaneIndices& _rIndices, int _ID);
        void UpdatePlaneWithMesh(glm::mat4 _Transform, glm::vec4 _Extent, const CPlaneVertices& _rVertices, const CPlaneIndices& _rIndices, int _ID);

        void RemovePlane(int _ID);

        const std::map<int, SPlane>& GetPlanes() const;

        void PauseIntegration(bool _Paused);
        void PauseTracking(bool _Paused);
        bool IsTrackingLost() const;
        glm::mat4 GetPoseMatrix() const;

        CRootVolumeMap& GetRootVolumeMap();
        CRootVolumeVector& GetRootVolumeVector();

        SSLAMVolume& GetVolume();

        const std::vector<float>& GetVolumeSizes() const;

        void GetReconstructionSettings(SReconstructionSettings* pReconstructionSettings);

        Gfx::CTexturePtr GetVertexMap();
        Gfx::CTexturePtr GetNormalMap();
        Gfx::CTexturePtr GetColorMap();

        glm::ivec2 GetDepthImageSize();
        glm::vec4 GetDepthIntrinsics();

        float GetReconstructionSize();

        void CreatePool();

        void SetImageSizes(glm::ivec2 _DepthFrameSize, glm::ivec2 _ColorFrameSize);
        void SetIntrinsics(glm::vec2 _FocalLength, glm::vec2 _FocalPoint);
        void SetDepthBounds(float _Min, float _Max);

        void GetImageSizes(glm::ivec2& _rDepthFrameSize, glm::ivec2& _rColorFrameSize);
        void GetIntrinsics(glm::vec2& _rFocalLength, glm::vec2& _rFocalPoint);
        void GetDepthBounds(float& _rMin, float& _rMax);

        void OnNewFrame(Gfx::CTexturePtr DepthBuffer, Gfx::CTexturePtr ColorBuffer, const glm::mat4* pTransform);
        
        void Start();
        void Exit();

        bool IsInitialized();

        Gfx::CTexturePtr CreatePlaneTexture(const Base::AABB3Float& _rAABB);

    private:

        glm::vec4 GetHessianNormalForm(const glm::vec3& rA, const glm::vec3& rB, const glm::vec3& rC);
        float GetPointPlaneDistance(const glm::vec3& rPoint, const glm::vec4& rPlane);
        bool RootGridInFrustum(const glm::ivec3& rKey);

        void UpdateRootgrids();
        void CreateIntegrationQueues(std::vector<uint32_t>& rVolumeQueue);
        void IntegrateHierarchies(std::vector<uint32_t>& rVolumeQueue);
        
        void ClearBuffer(Gfx::CBufferPtr BufferPtr);
        void ClearBuffer(Gfx::CBufferPtr BufferPtr, size_t Size);
        void ClearPool();

        void ClearMarkerStatistics();

        void SetupData();
        void SetupMeshes();
        void SetupRenderStates();

        void SetupShaders();
        void SetupTextures();
        void SetupBuffers(bool _CreatePool = true);

        void CreateReferencePyramid();
        void RasterizeRootVolumes();
        void GatherVolumeCounters(unsigned int Count, Gfx::CBufferPtr CounterBufferPtr, Gfx::CBufferPtr QueueBuffer, Gfx::CBufferPtr IndirectBufferPtr);
        
        void Raycast();
        void CreateRaycastPyramid();
        
        void UpdateFrustum();
                        
    private:

        SReconstructionSettings m_ReconstructionSettings;

        Gfx::CBufferPtr m_IntrinsicsConstantBufferPtr;
        Gfx::CBufferPtr m_TrackingDataConstantBufferPtr;
        Gfx::CBufferPtr m_RaycastPyramidConstantBufferPtr;
        Gfx::CBufferPtr m_BilateralFilterConstantBufferPtr;

        Gfx::CBufferPtr m_IndexedIndirectBufferPtr;
        
        Gfx::CBufferPtr m_RootVolumeInstanceBufferPtr;
        Gfx::CBufferPtr m_VolumeQueueBufferPtr;

        Gfx::CBufferPtr m_PointRasterizationBufferPtr;

        Gfx::CBufferPtr m_PlaneExtractionBufferPtr;

        SSLAMVolume m_VolumeBuffers;

        Gfx::CBufferPtr m_VolumeIndexBufferPtr;
        
        Gfx::CShaderPtr m_BilateralFilterCSPtr;
        Gfx::CShaderPtr m_VertexMapCSPtr;
        Gfx::CShaderPtr m_NormalMapCSPtr;
        Gfx::CShaderPtr m_DownSampleDepthCSPtr;
        Gfx::CShaderPtr m_IntegrateRootGridCSPtr;
        Gfx::CShaderPtr m_IntegrateLevel1GridCSPtr;
        Gfx::CShaderPtr m_IntegrateTSDFCSPtr;
        Gfx::CShaderPtr m_RaycastCSPtr;
        Gfx::CShaderPtr m_RaycastPyramidCSPtr;
        Gfx::CShaderPtr m_VolumeCountersCSPtr;
        Gfx::CShaderPtr m_RasterizeRootVolumeVSPtr;
        Gfx::CShaderPtr m_RasterizeRootVolumeFSPtr;
        
        Gfx::CShaderPtr m_PlaneCSPtr;

        Gfx::CShaderPtr m_PointCloudVSPtr;
        Gfx::CShaderPtr m_PointCloudGSPtr;
        Gfx::CShaderPtr m_PointCloudFSPtr;
        Gfx::CShaderPtr m_GatherVoxelsCSPtr;

        Gfx::CShaderPtr m_ClearAtomicCountersCSPtr;

        Gfx::CShaderPtr m_FillIndirectBufferCSPtr;
        
        Gfx::CMeshPtr m_CubeMeshPtr;
        
        Gfx::CInputLayoutPtr m_CubeInputLayoutPtr;
        Gfx::CTargetSetPtr m_EmptyTargetSetPtr;
        Gfx::CViewPortSetPtr m_DepthViewPortSetPtr;

        Gfx::CTexturePtr m_FullVolumePtr;
        Gfx::CTargetSetPtr m_EmptyFullVolumePtr;
        Gfx::CViewPortSetPtr m_FullVolumeViewPort;

        Gfx::CTexturePtr m_RawDepthBufferPtr;
        Gfx::CTexturePtr m_RawCameraFramePtr;
        Gfx::CTexturePtr m_RawVertexMapPtr;
        std::vector<Gfx::CTexturePtr> m_SmoothDepthBufferPtr;
        std::vector<Gfx::CTexturePtr> m_ReferenceVertexMapPtr;
        std::vector<Gfx::CTexturePtr> m_ReferenceNormalMapPtr;
        std::vector<Gfx::CTexturePtr> m_RaycastVertexMapPtr;
        std::vector<Gfx::CTexturePtr> m_RaycastNormalMapPtr;
        
        Gfx::CBufferPtr m_AtomicCounterBufferPtr;

        CRootVolumeMap m_RootVolumeMap;
        CRootVolumeVector m_RootVolumeVector;

        glm::ivec2 m_DepthFrameSize;
        glm::ivec2 m_ColorFrameSize;
        glm::vec2 m_FocalLength;
        glm::vec2 m_FocalPoint;
        glm::vec2 m_DepthBounds;

        glm::mat4 m_PoseMatrix;
        
        std::vector<unsigned short> m_DepthPixels;
        std::vector<char> m_CameraPixels;

        int m_VolumeDepthThreshold;

        unsigned int m_RootGridPoolSize;
        unsigned int m_Level1GridPoolSize;
        unsigned long long m_TSDFPoolSize;

        bool m_PoolFull;
        
        int m_RootVolumePoolItemCount;

        int m_IntegratedFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;

        bool m_IsIntegrationPaused;
        bool m_IsTrackingPaused;

        bool m_IsTrackingNeeded;

        int m_MinWeight;

        std::vector<float> m_VolumeSizes;
        
        std::array<glm::vec3, 8> m_FrustumPoints;
        std::array<glm::vec4, 6> m_FrustumPlanes;
        
        bool m_UseConservativeRasterization;

        std::unique_ptr<CICPTracker> m_pTracker;

        std::vector<char> m_ClearVector;

        float m_ReconstructionSize;

        bool m_CreateNormalsFromTSDF;
        bool m_RaycastBackSides;

        bool m_IsInizialized;

        std::map<int, SPlane> m_Planes;
    };
} // namespace MR
