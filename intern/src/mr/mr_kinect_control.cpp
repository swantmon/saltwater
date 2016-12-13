
#include "mr/mr_precompiled.h"

#include "base/base_console.h"
#include "base/base_memory.h"

#include "data/data_texture_manager.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_kinect_control.h"

#include <Kinect.h>
#include <NuiKinectFusionApi.h>

#define COLOR_WIDTH  1920
#define COLOR_HEIGHT 1080
#define DEPTH_WIDTH  512
#define DEPTH_HEIGHT 424

namespace
{
    std::string g_PathToAssets = "../assets/";
} // namespace

namespace MR
{
    CKinectControl::CKinectControl()
    {
    }

    // -----------------------------------------------------------------------------

    CKinectControl::~CKinectControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Start()
    {
		HRESULT hr = S_OK;

		_NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE ProcessorType = NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_AMP;
		int Index;
		PWSTR Description;
		PWSTR InstancePath;
		unsigned int DescriptionSize;
		unsigned int InstancePathSize;

		NuiFusionGetDeviceInfo(ProcessorType, Index, &Description[0], DescriptionSize, &InstancePath[0], InstancePathSize, nullptr);

		NUI_FUSION_RECONSTRUCTION_PARAMETERS ReconstructionParams = {};
		ReconstructionParams.voxelCountX = 256;
		ReconstructionParams.voxelCountY = 256;
		ReconstructionParams.voxelCountZ = 256;
		ReconstructionParams.voxelsPerMeter = 256;

		// Create the Kinect Fusion Reconstruction Volume
		hr = NuiFusionCreateReconstruction(
			&ReconstructionParams,
			m_processorType, m_deviceIndex,
			&m_worldToCameraTransform,
			&m_pVolume);

		if (FAILED(hr))
		{
			if (E_NUI_GPU_FAIL == hr)
			{
				WCHAR buf[MAX_PATH];
				swprintf_s(buf, ARRAYSIZE(buf), L"Device %d not able to run Kinect Fusion, or error initializing.", m_deviceIndex);
				SetStatusMessage(buf);
			}
			else if (E_NUI_GPU_OUTOFMEMORY == hr)
			{
				WCHAR buf[MAX_PATH];
				swprintf_s(buf, ARRAYSIZE(buf), L"Device %d out of memory error initializing reconstruction - try a smaller reconstruction volume.", m_deviceIndex);
				SetStatusMessage(buf);
			}
			else if (NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_CPU != m_processorType)
			{
				WCHAR buf[MAX_PATH];
				swprintf_s(buf, ARRAYSIZE(buf), L"Failed to initialize Kinect Fusion reconstruction volume on device %d.", m_deviceIndex);
				SetStatusMessage(buf);
			}
			else
			{
				SetStatusMessage(L"Failed to initialize Kinect Fusion reconstruction volume on CPU.");
			}

			return hr;
		}

		// Save the default world to volume transformation to be optionally used in ResetReconstruction
		hr = m_pVolume->GetCurrentWorldToVolumeTransform(&m_defaultWorldToVolumeTransform);
		if (FAILED(hr))
		{
			SetStatusMessage(L"Failed in call to GetCurrentWorldToVolumeTransform.");
			return hr;
		}

		if (m_bTranslateResetPoseByMinDepthThreshold)
		{
			// This call will set the world-volume transformation
			hr = ResetReconstruction();
			if (FAILED(hr))
			{
				return hr;
			}
		}

		// Frames generated from the depth input
		hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_FLOAT, m_cDepthWidth, m_cDepthHeight, &m_cameraParameters, &m_pDepthFloatImage);
		if (FAILED(hr))
		{
			SetStatusMessage(L"Failed to initialize Kinect Fusion image.");
			return hr;
		}

		// Create images to raycast the Reconstruction Volume
		hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_POINT_CLOUD, m_cDepthWidth, m_cDepthHeight, &m_cameraParameters, &m_pPointCloud);
		if (FAILED(hr))
		{
			SetStatusMessage(L"Failed to initialize Kinect Fusion image.");
			return hr;
		}

		// Create images to raycast the Reconstruction Volume
		hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_COLOR, m_cDepthWidth, m_cDepthHeight, &m_cameraParameters, &m_pShadedSurface);
		if (FAILED(hr))
		{
			SetStatusMessage(L"Failed to initialize Kinect Fusion image.");
			return hr;
		}

		_ASSERT(m_pDepthImagePixelBuffer == nullptr);
		m_pDepthImagePixelBuffer = new(std::nothrow) UINT16[m_cDepthImagePixels];
		if (nullptr == m_pDepthImagePixelBuffer)
		{
			SetStatusMessage(L"Failed to initialize Kinect Fusion depth image pixel buffer.");
			return hr;
		}

		_ASSERT(m_pDepthDistortionMap == nullptr);
		m_pDepthDistortionMap = new(std::nothrow) DepthSpacePoint[m_cDepthImagePixels];
		if (nullptr == m_pDepthDistortionMap)
		{
			SetStatusMessage(L"Failed to initialize Kinect Fusion depth image distortion buffer.");
			return E_OUTOFMEMORY;
		}

		SAFE_DELETE_ARRAY(m_pDepthDistortionLT);
		m_pDepthDistortionLT = new(std::nothrow) UINT[m_cDepthImagePixels];

		if (nullptr == m_pDepthDistortionLT)
		{
			SetStatusMessage(L"Failed to initialize Kinect Fusion depth image distortion Lookup Table.");
			return E_OUTOFMEMORY;
		}

		// If we have valid parameters, let's go ahead and use them.
		if (m_cameraParameters.focalLengthX != 0)
		{
			SetupUndistortion();
		}

		m_fStartTime = m_timer.AbsoluteTime();

		// Set an introductory message
		SetStatusMessage(L"Click ‘Reset Reconstruction' to clear!");

		return hr;
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Stop()
    {
       
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Update()
    {
        
    }
} // namespace MR