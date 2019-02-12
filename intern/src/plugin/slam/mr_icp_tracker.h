
#pragma once

#include "base/base_include_glm.h"

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_texture.h"

#include "plugin/slam/mr_slam_reconstruction_settings.h"

#include <vector>

namespace MR
{
    class CICPTracker
    {
    public:

        typedef std::vector<Gfx::CTexturePtr> CTextureVector;

        glm::mat4 Track(
            const glm::mat4& _rPoseMatrix,
            const CTextureVector& _rReferenceVertexMap,
            const CTextureVector& _rReferenceNormalMap,
            const CTextureVector& _rRaycastVertexMap,
            const CTextureVector& _rRaycastNormalMap,
            const Gfx::CBufferPtr& _rIntrinsicsBuffer
        );

        bool IsTrackingLost()
        {
            return m_IsTrackingLost;
        }

    public:

        CICPTracker(int _Width, int _Height, const MR::SReconstructionSettings& _Settings);
       ~CICPTracker();

    private:

        void CICPTracker::DetermineSummands(int PyramidLevel, const glm::mat4& rIncPoseMatrix,
            const CICPTracker::CTextureVector& _rReferenceVertexMap,
            const CICPTracker::CTextureVector& _rReferenceNormalMap,
            const CICPTracker::CTextureVector& _rRaycastVertexMap,
            const CICPTracker::CTextureVector& _rRaycastNormalMap
        );
        void CICPTracker::ReduceSum(int PyramidLevel);
        bool CICPTracker::CalculatePoseMatrix(glm::mat4& rIncPoseMatrix);

    private:

        int m_Width;
        int m_Height;
        int m_PyramidLevelCount;

        bool m_UseShuffleIntrinsics;

        bool m_IsTrackingLost;

        Gfx::CBufferPtr m_ICPSummationConstantBufferPtr;
        Gfx::CBufferPtr m_IncPoseMatrixConstantBufferPtr;
        Gfx::CBufferPtr m_ICPResourceBufferPtr;
        Gfx::CBufferPtr m_TrackingDataBufferPtr;

        Gfx::CShaderPtr m_DetermineSummandsCSPtr;
        Gfx::CShaderPtr m_ReduceSumCSPtr[3];

        glm::int3 m_Iterations;

    };
} // namespace MR