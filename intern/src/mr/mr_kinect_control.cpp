
#include "mr/mr_precompiled.h"

#include "base/base_console.h"
#include "base/base_memory.h"

#include "core/core_time.h"

#include "data/data_texture_manager.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_kinect_control.h"

#define NOMINMAX
#include <windows.h>
#include <Kinect.h>
#include <NuiKinectFusionApi.h>
#include <iostream>

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

    const int DepthImagePixelsCount = NUI_DEPTH_RAW_WIDTH * NUI_DEPTH_RAW_HEIGHT;
}

namespace MR
{
    CKinectControl::CKinectControl()
        : m_pKinect               (nullptr)
        , m_pVolume               (nullptr)
        , m_pDepthFrameReader     (nullptr)
        , m_pDepthImageFrame      (nullptr)
        , m_pPointCloud           (nullptr)
        , m_pShadedSurface        (nullptr)
        , m_pDepthImagePixelBuffer(nullptr)
        , m_pDepthDistortionMap   (nullptr)
        , m_pDepthDistortionLT    (nullptr)
    {
    }

    // -----------------------------------------------------------------------------

    CKinectControl::~CKinectControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Start(int VoxelCountX, int VoxelCountY, int VoxelCountZ, float VoxelsPerMeter)
    {
        m_VoxelCountX = VoxelCountX;
        m_VoxelCountY = VoxelCountY;
        m_VoxelCountZ = VoxelCountZ;
        m_VoxelsPerMeter = VoxelsPerMeter;
        m_VoxelCount = VoxelCountX * VoxelCountY * VoxelCountZ;

        m_VolumeExported = true;

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

        ICoordinateMapper* pMapper;

        CheckResult(m_pKinect->get_CoordinateMapper(&pMapper), "Failed to get coorindate mapper");

        CheckResult(pMapper->SubscribeCoordinateMappingChanged(&m_CoordinateMappingChangedEvent), "Fail to subscribe to CoordinateMappingChanged");

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
        ReconstructionParams.voxelCountX = m_VoxelCountX;
        ReconstructionParams.voxelCountY = m_VoxelCountY;
        ReconstructionParams.voxelCountZ = m_VoxelCountZ;
        ReconstructionParams.voxelsPerMeter = m_VoxelsPerMeter;

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

        m_pDepthImagePixelBuffer = new unsigned short[DepthImagePixelsCount];
        m_pDepthDistortionMap = new DepthSpacePoint[DepthImagePixelsCount];
        m_pDepthDistortionLT = new unsigned int[DepthImagePixelsCount];
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::ExportVolumeBlock(short* pVolumeBlock)
    {
        assert(m_pVolume != nullptr);

        HRESULT Result = m_pVolume->ExportVolumeBlock(0, 0, 0, m_VoxelCountX, m_VoxelCountY, m_VoxelCountZ, 1, m_VoxelCount * sizeof(short), pVolumeBlock);

        CheckResult(Result, "Failed to export volume block");

        m_VolumeExported = true;
    }

    // -----------------------------------------------------------------------------

    Base::Float4x4 CKinectControl::GetWorldToCameraMatrix()
    {
        Base::Float4x4 WorldToCamera;

        CheckResult(m_pVolume->GetCurrentWorldToCameraTransform(reinterpret_cast<Matrix4*>(&WorldToCamera)), "Failed to get current world to camera transform");

        return WorldToCamera;
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Stop()
    {
        delete[] m_pDepthImagePixelBuffer;
        delete[] m_pDepthDistortionMap;
        delete[] m_pDepthDistortionLT;
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
        if (WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)m_CoordinateMappingChangedEvent, 0))
        {
            ResetEvent((HANDLE)m_CoordinateMappingChangedEvent);
        }

        HRESULT Result;

        /*BOOLEAN available;
        m_pKinect->get_IsAvailable(&available);
        if (!available)
        {
            BASE_CONSOLE_ERROR("Kinect not available");
        }*/

        IDepthFrame* pDepthFrame;
        unsigned int BufferSize;
        unsigned short* pBuffer;

        //CheckResult(m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame), "Failed to acquire latest frame");
        
        if (m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame) != S_OK)
        {
            return;
        }

        CheckResult(pDepthFrame->AccessUnderlyingBuffer(&BufferSize, &pBuffer), "Failed to access underlying buffer");

        for (int i = 0; i < DepthImagePixelsCount; ++ i)
        {
            m_pDepthImagePixelBuffer[i] = pBuffer[i];
        }

        SafeRelease(pDepthFrame);

        if (m_pVolume == nullptr)
        {
            BASE_CONSOLE_ERROR("Kinect volume is not initialized");
            return;
        }

        m_pVolume->DepthToDepthFloatFrame(m_pDepthImagePixelBuffer, DepthImagePixelsCount * sizeof(unsigned short),
            m_pDepthImageFrame, NUI_FUSION_DEFAULT_MINIMUM_DEPTH, NUI_FUSION_DEFAULT_MAXIMUM_DEPTH, false);

        Result = m_pVolume->ProcessFrame(m_pDepthImageFrame, NUI_FUSION_DEFAULT_ALIGN_ITERATION_COUNT, NUI_FUSION_DEFAULT_INTEGRATION_WEIGHT, nullptr, m_pTransform);

        if (Result == E_NUI_FUSION_TRACKING_ERROR)
        {
            BASE_CONSOLE_INFO("Kinect fusion tracking failed");
            
            m_pTransform->M11 = 1; m_pTransform->M12 = 0; m_pTransform->M13 = 0; m_pTransform->M14 = 0;
            m_pTransform->M21 = 0; m_pTransform->M22 = 1; m_pTransform->M23 = 0; m_pTransform->M24 = 0;
            m_pTransform->M31 = 0; m_pTransform->M32 = 0; m_pTransform->M33 = 1; m_pTransform->M34 = 0;
            m_pTransform->M41 = 0; m_pTransform->M42 = 0; m_pTransform->M43 = 0; m_pTransform->M44 = 1;
            
            Matrix4 worldToVolumeTransform = *m_pTransform;

            float minDist = NUI_FUSION_DEFAULT_MINIMUM_DEPTH;
            worldToVolumeTransform.M43 -= (minDist * m_VoxelsPerMeter);

            m_pVolume->ResetReconstruction(m_pTransform, &worldToVolumeTransform);

            //m_pVolume->ResetReconstruction(m_pTransform, nullptr);

        }
        else
        {
            CheckResult(Result, "Failed to process kinect volume frame");
        }
        
        CheckResult(m_pVolume->GetCurrentWorldToCameraTransform(m_pTransform), "Failed to get world to camera transform");

        CheckResult(m_pVolume->CalculatePointCloud(m_pPointCloud, m_pTransform), "Failed to calculate point cloud");

        m_VolumeExported = false;
    }
} // namespace MR