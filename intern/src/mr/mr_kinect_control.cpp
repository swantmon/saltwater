
#include "mr/mr_precompiled.h"

#include "base/base_console.h"
#include "base/base_memory.h"

#include "data/data_texture_manager.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_kinect_control.h"

#define COLOR_WIDTH  1920
#define COLOR_HEIGHT 1080
#define DEPTH_WIDTH  512
#define DEPTH_HEIGHT 424

#define NOMINMAX
#include <windows.h>
#include <Kinect.h>
#include <NuiKinectFusionApi.h>

#pragma comment(lib,"Kinect20.lib")
#pragma comment(lib,"Kinect20.fusion.lib")

namespace
{
    void CheckResult(HRESULT Result, char* pMessage)
    {
        if (Result != S_OK)
        {
            BASE_CONSOLE_ERROR(pMessage);
            throw std::exception(pMessage);
        }
    }

    template<typename T>
    void SafeRelease(T*& rInterface)
    {
        if (rInterface != nullptr)
        {
            rInterface->Release();
            rInterface = nullptr;
        }
    }

    template<>
    void SafeRelease(NUI_FUSION_IMAGE_FRAME*& rFrame)
    {
        if (rFrame != nullptr)
        {
            NuiFusionReleaseImageFrame(rFrame);
            rFrame = nullptr;
        }
    }
}

namespace MR
{
    CKinectControl::CKinectControl()
        : m_pKinect          (nullptr)
        , m_pVolume          (nullptr)
        , m_pDepthFrameReader(nullptr)
        , m_pDepthImageFrame (nullptr)
        , m_pPointCloud      (nullptr)
        , m_pShadedSurface   (nullptr)
    {
    }

    // -----------------------------------------------------------------------------

    CKinectControl::~CKinectControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Start()
    {
        m_pTransform = new Matrix4();

        m_pTransform->M11 = 1; m_pTransform->M12 = 0; m_pTransform->M13 = 0; m_pTransform->M14 = 0;
        m_pTransform->M21 = 0; m_pTransform->M22 = 1; m_pTransform->M23 = 0; m_pTransform->M24 = 0;
        m_pTransform->M31 = 0; m_pTransform->M32 = 0; m_pTransform->M33 = 1; m_pTransform->M34 = 0;
        m_pTransform->M41 = 0; m_pTransform->M42 = 0; m_pTransform->M43 = 0; m_pTransform->M44 = 1;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize kinect
        ////////////////////////////////////////////////////////////////////////////////////////////

        CheckResult(GetDefaultKinectSensor(&m_pKinect), "Failed to get default kinect");

        // Initialize the Kinect and get the depth reader
        IDepthFrameSource* pDepthFrameSource = nullptr;

        CheckResult(m_pKinect->Open(), "failed to open kinect");

        CheckResult(m_pKinect->get_DepthFrameSource(&pDepthFrameSource), "Failed to get depth frame source");

        CheckResult(pDepthFrameSource->OpenReader(&m_pDepthFrameReader), "Failed to open depth frame reader");

        pDepthFrameSource->Release();

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Create reconstruction
        ////////////////////////////////////////////////////////////////////////////////////////////

        _NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE ProcessorType = NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_AMP;
        int Index = -1;
        WCHAR Description[MAX_PATH];
        WCHAR InstancePath[MAX_PATH];

        CheckResult(NuiFusionGetDeviceInfo(ProcessorType, Index, &Description[0], MAX_PATH, &InstancePath[0], MAX_PATH, nullptr), "Failed to get fusion device info");

        NUI_FUSION_RECONSTRUCTION_PARAMETERS ReconstructionParams = {};
        ReconstructionParams.voxelCountX = VoxelCountX;
        ReconstructionParams.voxelCountY = VoxelCountY;
        ReconstructionParams.voxelCountZ = VoxelCountZ;
        ReconstructionParams.voxelsPerMeter = 256;

        // Create the Kinect Fusion Reconstruction Volume
        CheckResult(NuiFusionCreateReconstruction(&ReconstructionParams, ProcessorType, -1, m_pTransform, &m_pVolume), "Failed to create reconstruction");

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Create reconstruction resources
        ////////////////////////////////////////////////////////////////////////////////////////////

        NUI_FUSION_CAMERA_PARAMETERS CameraParameters;

        CameraParameters.focalLengthX = NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_X;
        CameraParameters.focalLengthY = NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_Y;
        CameraParameters.principalPointX = NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_X;
        CameraParameters.principalPointY = NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_Y;

        // Frames generated from the depth input
        CheckResult(NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_FLOAT, NUI_DEPTH_RAW_WIDTH, NUI_DEPTH_RAW_HEIGHT, &CameraParameters, &m_pDepthImageFrame),
            "Failed to create depth image frame for kinect");

        // Create images to raycast the Reconstruction Volume
        CheckResult(NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_POINT_CLOUD, NUI_DEPTH_RAW_WIDTH, NUI_DEPTH_RAW_HEIGHT, &CameraParameters, &m_pPointCloud),
            "Failed to create point cloud for kinect");

        // Create images to raycast the Reconstruction Volume
        CheckResult(NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_COLOR, NUI_DEPTH_RAW_WIDTH, NUI_DEPTH_RAW_HEIGHT, &CameraParameters, &m_pShadedSurface),
            "Failed to create shaded surface for kinect");
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::ExportVolumeBlock(short* pVolumeBlock)
    {
        assert(m_pVolume != nullptr);

        HRESULT Result = m_pVolume->ExportVolumeBlock(0, 0, 0, VoxelCountX, VoxelCountY, VoxelCountZ, 1, VoxelCount * sizeof(short), pVolumeBlock);

        CheckResult(Result, "Failed to export volume block");
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Stop()
    {
        delete m_pTransform;
        SafeRelease(m_pVolume);
        SafeRelease(m_pDepthImageFrame);
        SafeRelease(m_pPointCloud);
        SafeRelease(m_pShadedSurface);
        SafeRelease(m_pDepthFrameReader);
        if (m_pKinect != nullptr)
        {
            m_pKinect->Close();
        }
        SafeRelease(m_pKinect);
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Update()
    {
        BOOLEAN available;
        m_pKinect->get_IsAvailable(&available);
        if (!available)
        {
            BASE_CONSOLE_INFO("Kinect not available");
        }

        IDepthFrame* pDepthFrame;

        m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

        SafeRelease(pDepthFrame);

        if (m_pVolume == nullptr)
        {
            BASE_CONSOLE_ERROR("Kinect volume is not initialized");
            return;
        }

        HRESULT Result = m_pVolume->ProcessFrame(m_pDepthImageFrame, NUI_FUSION_DEFAULT_ALIGN_ITERATION_COUNT, NUI_FUSION_DEFAULT_INTEGRATION_WEIGHT, nullptr, m_pTransform);
        if (Result == E_NUI_FUSION_TRACKING_ERROR)
        {
            BASE_CONSOLE_INFO("Kinect fusion tracking failed");
        }
        else
        {
            CheckResult(Result, "Failed to process kinect volume frame");
        }
        
        CheckResult(m_pVolume->CalculatePointCloud(m_pPointCloud, nullptr), "Failed to calculate point cloud");
    }
} // namespace MR