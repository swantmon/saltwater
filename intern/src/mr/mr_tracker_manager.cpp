
#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "mr/mr_marker_manager.h"
#include "mr/mr_tracker_manager.h"
#include "mr/mr_control_manager.h"

#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include "opencv2/opencv.hpp"

#include <vector>

using namespace MR;

namespace
{
    class CMRTrackerManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRTrackerManager);

    public:

        CMRTrackerManager();
        ~CMRTrackerManager();

    public:

        void OnStart();
        void OnExit();

        void Clear();

        void Update();

    public:

        void RegisterMarker(CMarkerPtr _MarkerPtr);

        unsigned int PollMarker(CMarkerInfo* _pMarkerInfo);
        
    private:

        typedef std::vector<CMarkerInfo> CMarkerInfos;

    private:

        ARHandle*	 m_pARHandle;
        AR3DHandle*	 m_pAR3DHandle;
        ARParamLT*	 m_pNativeParamLookupTable;
        int          m_DistortionF;
        int          m_ContributionF;
        bool         m_IsActive;
        CMarkerInfos m_MarkerInfos;
        unsigned int m_IndexOfMarkerInfo;
        unsigned int m_NumberOfRegisteredMarker;
    };
} // namespace

namespace
{
    CMRTrackerManager::CMRTrackerManager()
        : m_pARHandle               (0)
        , m_pAR3DHandle             (0)
        , m_pNativeParamLookupTable (0)
        , m_DistortionF             (0)
        , m_ContributionF           (0)
        , m_IsActive                (false)
        , m_MarkerInfos             ()
        , m_IndexOfMarkerInfo       (0)
        , m_NumberOfRegisteredMarker(0)
    {

    }

    // -----------------------------------------------------------------------------

    CMRTrackerManager::~CMRTrackerManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRTrackerManager::OnStart()
    {
        int Error;

        // -----------------------------------------------------------------------------
        // Get camera parameters and save to native structure
        // -----------------------------------------------------------------------------
        ARParam NativeParams;
        
        SDeviceParameter& rCameraParameters = ControlManager::GetActiveControl().GetCameraParameters();

        NativeParams.xsize = rCameraParameters.m_FrameWidth;
        NativeParams.ysize = rCameraParameters.m_FrameHeight;

        NativeParams.mat[0][0] = rCameraParameters.m_ProjectionMatrix[0][0];
        NativeParams.mat[0][1] = rCameraParameters.m_ProjectionMatrix[0][1];
        NativeParams.mat[0][2] = rCameraParameters.m_ProjectionMatrix[0][2];
        NativeParams.mat[0][3] = rCameraParameters.m_ProjectionMatrix[0][3];

        NativeParams.mat[1][0] = rCameraParameters.m_ProjectionMatrix[1][0];
        NativeParams.mat[1][1] = rCameraParameters.m_ProjectionMatrix[1][1];
        NativeParams.mat[1][2] = rCameraParameters.m_ProjectionMatrix[1][2];
        NativeParams.mat[1][3] = rCameraParameters.m_ProjectionMatrix[1][3];

        NativeParams.mat[2][0] = rCameraParameters.m_ProjectionMatrix[2][0];
        NativeParams.mat[2][1] = rCameraParameters.m_ProjectionMatrix[2][1];
        NativeParams.mat[2][2] = rCameraParameters.m_ProjectionMatrix[2][2];
        NativeParams.mat[2][3] = rCameraParameters.m_ProjectionMatrix[2][3];

        NativeParams.dist_factor[0] = rCameraParameters.m_DistortionFactor[0];
        NativeParams.dist_factor[1] = rCameraParameters.m_DistortionFactor[1];
        NativeParams.dist_factor[2] = rCameraParameters.m_DistortionFactor[2];
        NativeParams.dist_factor[3] = rCameraParameters.m_DistortionFactor[3];
        NativeParams.dist_factor[4] = rCameraParameters.m_DistortionFactor[4];
        NativeParams.dist_factor[5] = rCameraParameters.m_DistortionFactor[5];
        NativeParams.dist_factor[6] = rCameraParameters.m_DistortionFactor[6];
        NativeParams.dist_factor[7] = rCameraParameters.m_DistortionFactor[7];
        NativeParams.dist_factor[8] = rCameraParameters.m_DistortionFactor[8];

        NativeParams.dist_function_version = rCameraParameters.m_DistortionFunctionVersion;

        AR_PIXEL_FORMAT OriginalPixelFormat = static_cast<AR_PIXEL_FORMAT>(rCameraParameters.m_PixelFormat);
        
        // -----------------------------------------------------------------------------
        // Create parameter lookup table
        // -----------------------------------------------------------------------------
        m_pNativeParamLookupTable = arParamLTCreate(&NativeParams, AR_PARAM_LT_DEFAULT_OFFSET);
        
        assert(m_pNativeParamLookupTable != 0);
        
        // -----------------------------------------------------------------------------
        // Create AR handle with parameters
        // -----------------------------------------------------------------------------
        m_pARHandle = arCreateHandle(m_pNativeParamLookupTable);

        assert(m_pARHandle != 0);
        
        // -----------------------------------------------------------------------------
        // Set AR pixel format
        // -----------------------------------------------------------------------------
        Error = arSetPixelFormat(m_pARHandle, OriginalPixelFormat);
        
        assert(Error >= 0);
        
        // -----------------------------------------------------------------------------
        // Create 3D AR handle
        // -----------------------------------------------------------------------------
        m_pAR3DHandle = ar3DCreateHandle(&NativeParams);

        assert(m_pAR3DHandle != 0);
        
        m_IsActive = true;
    }

    // -----------------------------------------------------------------------------

    void CMRTrackerManager::OnExit()
    {
        Clear();

        arPattDetach(m_pARHandle);
        ar3DDeleteHandle(&m_pAR3DHandle);
        arDeleteHandle(m_pARHandle);
        arParamLTFree(&m_pNativeParamLookupTable);

        m_IsActive = false;
    }

    // -----------------------------------------------------------------------------

    void CMRTrackerManager::Clear()
    {
        m_MarkerInfos.clear();
    }

    // -----------------------------------------------------------------------------

    void CMRTrackerManager::Update()
    {
        if (m_IsActive == false) return;
        
        // -----------------------------------------------------------------------------
        // Prepare collections
        // -----------------------------------------------------------------------------
        m_IndexOfMarkerInfo = 0;
        
        m_MarkerInfos.clear();

        // -----------------------------------------------------------------------------
        // Get image stream for tracking and detect marker in image
        // -----------------------------------------------------------------------------
        ARUint8* pImagedata;
        
        pImagedata = static_cast<ARUint8*>(ControlManager::GetActiveControl().GetOriginalFrame()->GetPixels());
        
        assert(pImagedata != 0);

        arDetectMarker(m_pARHandle, pImagedata);
        
        // -----------------------------------------------------------------------------
        // Get number of markers found in last detection
        // -----------------------------------------------------------------------------
        unsigned int NumberOfMarkers;
        
        NumberOfMarkers = static_cast<unsigned int>(arGetMarkerNum(m_pARHandle));

        if (NumberOfMarkers == 0)
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Check for pattern visibility
        // -----------------------------------------------------------------------------
        static ARdouble PatternTransformation[3][4];
        
        ARMarkerInfo* pMarkerInfos;
        
        pMarkerInfos = arGetMarker(m_pARHandle);

        for (unsigned int IndexOfMarker = 0; IndexOfMarker < NumberOfMarkers; ++ IndexOfMarker)
        {
            ARMarkerInfo& rCurrentMarkerInfo = pMarkerInfos[IndexOfMarker];

            if (rCurrentMarkerInfo.cf >= 0.7)
            {
                // -----------------------------------------------------------------------------
                // Found a marker
                // -----------------------------------------------------------------------------
                CMarkerPtr MarkerPtr = MarkerManager::GetMarkerByID(rCurrentMarkerInfo.id);
                
                // -----------------------------------------------------------------------------
                // Get transformation of this pattern
                // -----------------------------------------------------------------------------
                arGetTransMatSquare(m_pAR3DHandle, &(rCurrentMarkerInfo), MarkerPtr->m_WidthInMeter * 1000.0f, PatternTransformation);

                // -----------------------------------------------------------------------------
                // Edit Marker
                // -----------------------------------------------------------------------------
                if (MarkerPtr->m_IsFound == false)
                {
                    ++ MarkerPtr->m_AppearCounter;
                    
                    MarkerPtr->m_IsFound = true;
                }
                
                // -----------------------------------------------------------------------------
                // Create / Edit marker infos
                // -----------------------------------------------------------------------------
                CMarkerInfo NewMarkerInfo;

                NewMarkerInfo.m_UserID                 = MarkerPtr->m_UserID;
                NewMarkerInfo.m_FrameCounter           = 0;
                
                NewMarkerInfo.m_TranslationToCamera[0] = static_cast<float>(PatternTransformation[0][3]) / 1000.0f;
                NewMarkerInfo.m_TranslationToCamera[1] = static_cast<float>(PatternTransformation[1][3]) / 1000.0f;
                NewMarkerInfo.m_TranslationToCamera[2] = static_cast<float>(PatternTransformation[2][3]) / 1000.0f;
                
                NewMarkerInfo.m_RotationToCamera[0][0] = static_cast<float>(PatternTransformation[0][0]);
                NewMarkerInfo.m_RotationToCamera[0][1] = static_cast<float>(PatternTransformation[0][1]);
                NewMarkerInfo.m_RotationToCamera[0][2] = static_cast<float>(PatternTransformation[0][2]);

                NewMarkerInfo.m_RotationToCamera[1][0] = static_cast<float>(PatternTransformation[1][0]);
                NewMarkerInfo.m_RotationToCamera[1][1] = static_cast<float>(PatternTransformation[1][1]);
                NewMarkerInfo.m_RotationToCamera[1][2] = static_cast<float>(PatternTransformation[1][2]);

                NewMarkerInfo.m_RotationToCamera[2][0] = static_cast<float>(PatternTransformation[2][0]);
                NewMarkerInfo.m_RotationToCamera[2][1] = static_cast<float>(PatternTransformation[2][1]);
                NewMarkerInfo.m_RotationToCamera[2][2] = static_cast<float>(PatternTransformation[2][2]);
                
                m_MarkerInfos.push_back(NewMarkerInfo);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMRTrackerManager::RegisterMarker(CMarkerPtr _MarkerPtr)
    {
        int Error = 0;

        ARPattHandle* pNativeARPatternHandle = static_cast<ARPattHandle*>(_MarkerPtr->m_pHandle);

        Error = arPattAttach(m_pARHandle, pNativeARPatternHandle);

        assert(Error == 0);

        _MarkerPtr->m_IsRegistered = true;
        
        ++ m_NumberOfRegisteredMarker;
        
        m_MarkerInfos.reserve(m_NumberOfRegisteredMarker);
    }

    // -----------------------------------------------------------------------------

    unsigned int CMRTrackerManager::PollMarker(CMarkerInfo* _pMarkerInfo)
    {
        if (m_IndexOfMarkerInfo == m_MarkerInfos.size())
        {
            _pMarkerInfo = 0;

            return 0;
        }

        *_pMarkerInfo = m_MarkerInfos[m_IndexOfMarkerInfo];

        ++ m_IndexOfMarkerInfo;

        return m_IndexOfMarkerInfo <= m_MarkerInfos.size();
    }
} // namespace

namespace MR
{
namespace TrackerManager
{
    void OnStart()
    {
        CMRTrackerManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRTrackerManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CMRTrackerManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRTrackerManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void RegisterMarker(CMarkerPtr _MarkerPtr)
    {
        CMRTrackerManager::GetInstance().RegisterMarker(_MarkerPtr);
    }

    // -----------------------------------------------------------------------------

    unsigned int PollMarker(CMarkerInfo* _pMarkerInfo)
    {
        return CMRTrackerManager::GetInstance().PollMarker(_pMarkerInfo);
    }
} // namespace TrackerManager
} // namespace MR
