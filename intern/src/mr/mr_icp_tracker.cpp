
#include "mr/mr_precompiled.h"

#include "mr/mr_icp_tracker.h"

#include "base/base_program_parameters.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_texture_manager.h"

#include <cassert>
#include <sstream>

using namespace Gfx;

namespace
{
    const int g_ICPValueCount = 27;
    const int g_TileSize2D = 16;

    const float g_EpsilonDistance = 0.1f;
    const float g_EpsilonAngle = 0.75f;
    
    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    struct SIncBuffer
    {
        glm::mat4 m_PoseMatrix;
        glm::mat4 m_InvPoseMatrix;
        int m_PyramidLevel;
        float Padding[3];
    };

    struct STrackingData
    {
        glm::mat4 m_PoseMatrix;
        glm::mat4 m_InvPoseMatrix;
    };
}

namespace MR
{
    glm::mat4 CICPTracker::Track(
        const glm::mat4& _rPoseMatrix,
        const CICPTracker::CTextureVector& _rReferenceVertexMap,
        const CICPTracker::CTextureVector& _rReferenceNormalMap,
        const CICPTracker::CTextureVector& _rRaycastVertexMap,
        const CICPTracker::CTextureVector& _rRaycastNormalMap,
        const Gfx::CBufferPtr& _rIntrinsicsBuffer
    )
    {
        STrackingData TrackingData;
        TrackingData.m_PoseMatrix = _rPoseMatrix;
        TrackingData.m_InvPoseMatrix = glm::inverse(_rPoseMatrix);
        Gfx::BufferManager::UploadBufferData(m_TrackingDataBufferPtr, &TrackingData);

        std::string Markers[] =
        {
            "Reduce sum0",
            "Reduce sum1",
            "Reduce sum2",
            "Determine summands0",
            "Determine summands1",
            "Determine summands2"
        };

        ContextManager::SetConstantBuffer(0, _rIntrinsicsBuffer);

        glm::mat4 IncPoseMatrix = _rPoseMatrix;

        for (int PyramidLevel = m_PyramidLevelCount - 1; PyramidLevel >= 0; --PyramidLevel)
        {
            for (int Iteration = 0; Iteration < m_Iterations[PyramidLevel]; ++Iteration)
            {
                Performance::BeginEvent(Markers[3 + PyramidLevel].c_str());
                DetermineSummands(PyramidLevel, IncPoseMatrix,
                    _rReferenceVertexMap,
                    _rReferenceNormalMap,
                    _rRaycastVertexMap,
                    _rRaycastNormalMap
                );
                Performance::EndEvent();

                Performance::BeginEvent(Markers[PyramidLevel].c_str());
                ReduceSum(PyramidLevel);
                Performance::EndEvent();

                m_IsTrackingLost = !CalculatePoseMatrix(IncPoseMatrix);
                if (m_IsTrackingLost)
                {
                    return glm::mat4(1.0f);
                }
            }
        }
        return IncPoseMatrix;
    }

    // -----------------------------------------------------------------------------

    void CICPTracker::DetermineSummands(int PyramidLevel, const glm::mat4& rIncPoseMatrix,
        const CICPTracker::CTextureVector& _rReferenceVertexMap,
        const CICPTracker::CTextureVector& _rReferenceNormalMap,
        const CICPTracker::CTextureVector& _rRaycastVertexMap,
        const CICPTracker::CTextureVector& _rRaycastNormalMap
    )
    {
        const int WorkGroupsX = DivUp(m_Width >> PyramidLevel, g_TileSize2D);
        const int WorkGroupsY = DivUp(m_Height >> PyramidLevel, g_TileSize2D);

        SIncBuffer TrackingData;
        TrackingData.m_PoseMatrix = rIncPoseMatrix;
        TrackingData.m_InvPoseMatrix = glm::inverse(rIncPoseMatrix);
        TrackingData.m_PyramidLevel = PyramidLevel;

        BufferManager::UploadBufferData(m_IncPoseMatrixConstantBufferPtr, &TrackingData);

        ContextManager::SetShaderCS(m_DetermineSummandsCSPtr);
        ContextManager::SetResourceBuffer(0, m_ICPResourceBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataBufferPtr);
        ContextManager::SetConstantBuffer(2, m_IncPoseMatrixConstantBufferPtr);

        ContextManager::SetImageTexture(0, _rReferenceVertexMap[PyramidLevel]);
        ContextManager::SetImageTexture(1, _rReferenceNormalMap[PyramidLevel]);
        ContextManager::SetImageTexture(2, _rRaycastVertexMap[PyramidLevel]);
        ContextManager::SetImageTexture(3, _rRaycastNormalMap[PyramidLevel]);

        ContextManager::Barrier();

        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CICPTracker::ReduceSum(int PyramidLevel)
    {
        const int SummandsX = DivUp(m_Width >> PyramidLevel, g_TileSize2D);
        const int SummandsY = DivUp(m_Height >> PyramidLevel, g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = glm::log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);

        glm::ivec2 BufferData;
        BufferData[0] = Summands;
        BufferData[1] = SummandsPOT;

        BufferManager::UploadBufferData(m_ICPSummationConstantBufferPtr, &BufferData);

        ContextManager::SetShaderCS(m_ReduceSumCSPtr[m_UseShuffleIntrinsics ? PyramidLevel : 0]);

        ContextManager::SetResourceBuffer(0, m_ICPResourceBufferPtr);
        ContextManager::SetConstantBuffer(2, m_ICPSummationConstantBufferPtr);

        ContextManager::Barrier();

        ContextManager::Dispatch(1, g_ICPValueCount, 1);
    }

    // -----------------------------------------------------------------------------

    bool CICPTracker::CalculatePoseMatrix(glm::mat4& rIncPoseMatrix)
    {
        typedef float Scalar;

        Scalar A[36];
        Scalar b[6];

        float ICPValues[g_ICPValueCount];
        void* pICPBuffer = BufferManager::MapBufferRange(m_ICPResourceBufferPtr, CBuffer::EMap::Read, 0, sizeof(float) * g_ICPValueCount);
        memcpy(ICPValues, pICPBuffer, sizeof(ICPValues[0]) * g_ICPValueCount);
        BufferManager::UnmapBuffer(m_ICPResourceBufferPtr);

        int ValueIndex = 0;
        for (int i = 0; i < 6; ++i)
        {
            for (int j = i; j < 7; ++j)
            {
                float Value = ICPValues[ValueIndex++];

                if (j == 6)
                {
                    b[i] = static_cast<Scalar>(Value);
                }
                else
                {
                    A[j * 6 + i] = A[i * 6 + j] = static_cast<Scalar>(Value);
                }
            }
        }

        Scalar L[36];

        for (int i = 0; i < 6; ++i)
        {
            for (int j = 0; j <= i; ++j)
            {
                Scalar Sum = 0.0;
                for (int k = 0; k < j; ++k)
                {
                    Sum += L[k * 6 + i] * L[k * 6 + j];
                }
                L[j * 6 + i] = i == j ? sqrt(A[i * 6 + i] - Sum) : ((1.0f / L[j * 6 + j]) * (A[j * 6 + i] - Sum));
            }
        }

        const Scalar Det = L[0] * L[0] * L[7] * L[7] * L[14] * L[14] * L[21] * L[21] * L[28] * L[28] * L[35] * L[35];

        if (std::isnan(Det) || abs(Det) < 1e-5)
        {
            return false;
        }

        Scalar y[6];

        y[0] = b[0] / L[0];
        y[1] = (b[1] - L[1] * y[0]) / L[7];
        y[2] = (b[2] - L[2] * y[0] - L[8] * y[1]) / L[14];
        y[3] = (b[3] - L[3] * y[0] - L[9] * y[1] - L[15] * y[2]) / L[21];
        y[4] = (b[4] - L[4] * y[0] - L[10] * y[1] - L[16] * y[2] - L[22] * y[3]) / L[28];
        y[5] = (b[5] - L[5] * y[0] - L[11] * y[1] - L[17] * y[2] - L[23] * y[3] - L[29] * y[4]) / L[35];

        Scalar x[6];

        x[5] = y[5] / L[35];
        x[4] = (y[4] - L[29] * x[5]) / L[28];
        x[3] = (y[3] - L[23] * x[5] - L[22] * x[4]) / L[21];
        x[2] = (y[2] - L[17] * x[5] - L[16] * x[4] - L[15] * x[3]) / L[14];
        x[1] = (y[1] - L[11] * x[5] - L[10] * x[4] - L[9] * x[3] - L[8] * x[2]) / L[7];
        x[0] = (y[0] - L[5] * x[5] - L[4] * x[4] - L[3] * x[3] - L[2] * x[2] - L[1] * x[1]) / L[0];

        glm::mat4 Rotation = glm::eulerAngleXYZ(static_cast<float>(x[0]), static_cast<float>(x[1]), static_cast<float>(x[2]));
        glm::mat4 Translation = glm::translate(glm::vec3(static_cast<float>(x[3]), static_cast<float>(x[4]), static_cast<float>(x[5])));

        rIncPoseMatrix = Translation * Rotation * rIncPoseMatrix;

        return true;
    }

    // -----------------------------------------------------------------------------

    CICPTracker::CICPTracker(int _Width, int _Height, const MR::SReconstructionSettings& _Settings)
        : m_Width(_Width)
        , m_Height(_Height)
    {
        m_PyramidLevelCount = _Settings.m_PyramidLevelCount;
        m_Iterations = _Settings.m_PyramidLevelIterations;

        m_IsTrackingLost = true;

        m_UseShuffleIntrinsics = false;

        const bool EnableShuffleIntrinsics = Base::CProgramParameters::GetInstance().Get("mr:slam:shuffle_intrinsics_enable", true);

        if (EnableShuffleIntrinsics)
        {
            m_UseShuffleIntrinsics = Main::IsExtensionAvailable("GL_NV_shader_thread_shuffle");

            if (!m_UseShuffleIntrinsics)
            {
                BASE_CONSOLE_INFO("Shuffle intrinsics are not available. Will use fallback method");
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Create buffers
        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = CBuffer::EUsage::GPURead;
        BufferDesc.m_Binding = CBuffer::ConstantBuffer;
        BufferDesc.m_Access = CBuffer::CPUWrite;
        BufferDesc.m_pClassKey = 0;

        BufferDesc.m_NumberOfBytes = 16;
        m_ICPSummationConstantBufferPtr = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_NumberOfBytes = sizeof(SIncBuffer);
        m_IncPoseMatrixConstantBufferPtr = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_NumberOfBytes = sizeof(STrackingData);
        m_TrackingDataBufferPtr = BufferManager::CreateBuffer(BufferDesc);

        const int ICPRowCount = DivUp(_Width, g_TileSize2D) * DivUp(m_Height, g_TileSize2D);

        BufferDesc.m_Usage = CBuffer::GPUToCPU;
        BufferDesc.m_Binding = CBuffer::ResourceBuffer;
        BufferDesc.m_Access = CBuffer::CPURead;
        BufferDesc.m_NumberOfBytes = sizeof(float) * ICPRowCount * g_ICPValueCount;
        BufferDesc.m_pBytes = nullptr;
        m_ICPResourceBufferPtr = BufferManager::CreateBuffer(BufferDesc);
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Create shaders
        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        const int SummandsX = DivUp(m_Width, g_TileSize2D);
        const int SummandsY = DivUp(m_Height, g_TileSize2D);

        const int Summands = SummandsX * SummandsY;

        const std::string InternalFormatString = Base::CProgramParameters::GetInstance().Get("mr:slam:map_format", "rgba16f");

        std::stringstream DefineStream;

        DefineStream
            << "#define PYRAMID_LEVELS " << _Settings.m_PyramidLevelCount << " \n"
            << "#define DEPTH_IMAGE_WIDTH " << _Width << " \n"
            << "#define DEPTH_IMAGE_HEIGHT " << _Height << " \n"
            << "#define TILE_SIZE2D " << g_TileSize2D << " \n"
            << "#define TRUNCATED_DISTANCE " << _Settings.m_TruncatedDistance / 1000.0f << " \n"
            << "#define EPSILON_DISTANCE " << g_EpsilonDistance << " \n"
            << "#define EPSILON_ANGLE " << g_EpsilonAngle << " \n"
            << "#define ICP_VALUE_COUNT " << g_ICPValueCount << " \n"
            << "#define ICP_SUMMAND_COUNT " << Summands << " \n"
            << "#define MAP_TEXTURE_FORMAT " << InternalFormatString << " \n";
        if (m_UseShuffleIntrinsics)
        {
            DefineStream << "#define USE_SHUFFLE_INTRINSICS\n";
        }

        std::string DefineString = DefineStream.str();

        m_DetermineSummandsCSPtr = ShaderManager::CompileCS("slam\\tracking\\cs_determine_summands.glsl", "main", DefineString.c_str());

        if (m_UseShuffleIntrinsics)
        {
            for (int i = 0; i < 3; ++i)
            {
                const int ReductionSummandsX = DivUp(m_Width >> i, g_TileSize2D);
                const int ReductionSummandsY = DivUp(m_Height >> i, g_TileSize2D);

                const int ReductionSummands = ReductionSummandsX * ReductionSummandsY;
                const float ReductionSummandsLog2 = glm::log2(static_cast<float>(ReductionSummands));
                const int ReductionSummandsPOT = 1 << (static_cast<int>(ReductionSummandsLog2) + 1);

                std::stringstream TempStream;
                TempStream << DefineString << "#define REDUCTION_SHADER_COUNT " << ReductionSummandsPOT / 2 << " \n";

                m_ReduceSumCSPtr[i] = ShaderManager::CompileCS("slam\\tracking\\cs_reduce_sum.glsl", "main", TempStream.str().c_str());
            }
        }
        else
        {
            const int ReductionSummandsX = DivUp(m_Width, g_TileSize2D);
            const int ReductionSummandsY = DivUp(m_Height, g_TileSize2D);

            const int ReductionSummands = ReductionSummandsX * ReductionSummandsY;
            const float ReductionSummandsLog2 = glm::log2(static_cast<float>(ReductionSummands));
            const int ReductionSummandsPOT = 1 << (static_cast<int>(ReductionSummandsLog2) + 1);

            DefineStream << "#define REDUCTION_SHADER_COUNT " << ReductionSummandsPOT / 2 << " \n";

            m_ReduceSumCSPtr[0] = ShaderManager::CompileCS("slam\\tracking\\cs_reduce_sum.glsl", "main", DefineStream.str().c_str());
        }
    }

    // -----------------------------------------------------------------------------

    CICPTracker::~CICPTracker()
    {

    }

} // namespace MR