
#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "data/data_actor_facet.h"
#include "data/data_plugin_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_kinect_control.h"
#include "mr/mr_marker_info.h"
#include "mr/mr_marker_manager.h"
#include "mr/mr_webcam_control.h"

#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include <assert.h>

using namespace MR;

namespace
{
    class CMRControlManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRControlManager);
        
    public:

        CMRControlManager();
       ~CMRControlManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

        bool IsActive();

        CControl* GetActiveControl();

    private:

        typedef std::vector<Dt::CEntity*> CEntityVector;
        typedef std::vector<CMarkerInfo>  CMarkerInfos;

    private:

        ARHandle*	                  m_pARHandle;
        AR3DHandle*	                  m_pAR3DHandle;
        ARParamLT*	                  m_pNativeParamLookupTable;
        bool                          m_IsActive;
        CMarkerInfos                  m_MarkerInfos;
        CEntityVector                 m_DirtyEntities;
        CControl*                     m_pActiveControl;
        CControl*                     m_pControls[CControl::NumberOfControls];
        Dt::CARControllerPluginFacet* m_pController;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdatePlugin(Dt::CEntity& _rEntity);
        void UpdatePluginARController(Dt::CEntity& _rEntity);
        void UpdatePluginARTrackedObject(Dt::CEntity& _rEntity);

        void CreatePluginARController(Dt::CEntity& _rEntity);
        void CreatePluginARTrackedObject(Dt::CEntity& _rEntity);

        void TrackMarker();
        void UpdateTrackerEntities();
    };
} // namespace

namespace
{
    CMRControlManager::CMRControlManager()
        : m_pARHandle               (0)
        , m_pAR3DHandle             (0)
        , m_pNativeParamLookupTable (0)
        , m_IsActive                (false)
        , m_MarkerInfos             ()
        , m_DirtyEntities           ()
        , m_pActiveControl          (nullptr)
        , m_pController             (nullptr)
    {
        m_DirtyEntities.reserve(8);
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));

        m_pControls[CControl::Webcam] = Base::CMemory::NewObject<CWebcamControl>();
        m_pControls[CControl::Kinect] = Base::CMemory::NewObject<CKinectControl>();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnExit()
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->Stop();
        }

        if (m_pControls[CControl::Webcam] != nullptr)
        {
            Base::CMemory::Free(m_pControls[CControl::Webcam]);
        }

        if (m_pControls[CControl::Kinect] != nullptr)
        {
            Base::CMemory::Free(m_pControls[CControl::Kinect]);
        }

        arPattDetach(m_pARHandle);
        ar3DDeleteHandle(&m_pAR3DHandle);
        arDeleteHandle(m_pARHandle);
        arParamLTFree(&m_pNativeParamLookupTable);

        m_IsActive = false;

        m_MarkerInfos  .clear();
        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->Update();
        }

        TrackMarker();

        UpdateTrackerEntities();

        // -----------------------------------------------------------------------------

        CEntityVector::iterator CurrentDirtyEntity = m_DirtyEntities.begin();
        CEntityVector::iterator EndOfDirtyEntities = m_DirtyEntities.end();

        for (; CurrentDirtyEntity != EndOfDirtyEntities; ++CurrentDirtyEntity)
        {
            UpdatePlugin(**CurrentDirtyEntity);
        }

        m_DirtyEntities.clear();
    }

    // -----------------------------------------------------------------------------

    bool CMRControlManager::IsActive()
    {
        return (m_pActiveControl != nullptr && m_pActiveControl->IsStarted());
    }

    // -----------------------------------------------------------------------------
    
    CControl* CMRControlManager::GetActiveControl()
    {
        return m_pActiveControl;
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (_pEntity->GetCategory() != Dt::SEntityCategory::Plugin) return;

        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create a light
            // -----------------------------------------------------------------------------
            switch (_pEntity->GetType())
            {
            case Dt::SPluginType::ARControlManager: CreatePluginARController(*_pEntity); break;
            case Dt::SPluginType::ARTrackedObject:  CreatePluginARTrackedObject(*_pEntity); break;
            }
        }

        m_DirtyEntities.push_back(_pEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdatePlugin(Dt::CEntity& _rEntity)
    {
        // -----------------------------------------------------------------------------
        // Update light if it should be dirty
        // -----------------------------------------------------------------------------
        switch (_rEntity.GetType())
        {
        case Dt::SPluginType::ARControlManager: UpdatePluginARController(_rEntity); break;
        case Dt::SPluginType::ARTrackedObject:  UpdatePluginARTrackedObject(_rEntity); break;
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdatePluginARController(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdatePluginARTrackedObject(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::CreatePluginARController(Dt::CEntity& _rEntity)
    {
        Dt::CARControllerPluginFacet* pControllerPlugin = static_cast<Dt::CARControllerPluginFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pControllerPlugin != nullptr);

        m_pController = pControllerPlugin;

        // -----------------------------------------------------------------------------
        // Create control
        // -----------------------------------------------------------------------------
        SControlDescription ControlDesc;

        ControlDesc.m_pOutputBackground    = pControllerPlugin->GetOutputBackground();
        ControlDesc.m_pOutputCubemap       = pControllerPlugin->GetOutputCubemap();
        ControlDesc.m_pCameraParameterFile = pControllerPlugin->GetCameraParameterFile();
        ControlDesc.m_DeviceNumber         = pControllerPlugin->GetDeviceNumber();

        switch (pControllerPlugin->GetDeviceType())
        {
        case Dt::CARControllerPluginFacet::Webcam:
            m_pActiveControl = m_pControls[CControl::Webcam];
            break;
        case Dt::CARControllerPluginFacet::Kinect:
            m_pActiveControl = m_pControls[CControl::Kinect];
            break;
        };

        assert(m_pActiveControl != nullptr);

        m_pActiveControl->Start(ControlDesc);

        // -----------------------------------------------------------------------------
        // Set camera
        // Note: Camera will be automatically set to RAW projection input
        // -----------------------------------------------------------------------------
        Dt::CEntity* pCameraEntity = pControllerPlugin->GetCameraEntity();

        assert(pCameraEntity != nullptr);

        Dt::CCameraActorFacet* pCameraActorFacet = static_cast<Dt::CCameraActorFacet*>(pCameraEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        pCameraActorFacet->SetProjectionType  (Dt::CCameraActorFacet::RAW);
        pCameraActorFacet->SetProjectionMatrix(m_pActiveControl->GetProjectionMatrix());

        // -----------------------------------------------------------------------------
        // Create tracker
        // -----------------------------------------------------------------------------
        int Error;

        // -----------------------------------------------------------------------------
        // Get camera parameters and save to native structure
        // -----------------------------------------------------------------------------
        ARParam NativeParams;
        
        SDeviceParameter& rCameraParameters = m_pActiveControl->GetCameraParameters();

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

        // -----------------------------------------------------------------------------
        // Create marker
        // -----------------------------------------------------------------------------
        unsigned int NumberOfMarker = pControllerPlugin->GetNumberOfMarker();

        for (unsigned int IndexOfMarker = 0; IndexOfMarker < NumberOfMarker; ++ IndexOfMarker)
        {
            Dt::CARControllerPluginFacet::SMarker& rCurrentMarker = pControllerPlugin->GetMarker(IndexOfMarker);

            SMarkerDescription MarkerDescription;

            MarkerDescription.m_UserID       = rCurrentMarker.m_UID;
            MarkerDescription.m_Type         = SMarkerDescription::Square;
            MarkerDescription.m_pPatternFile = rCurrentMarker.m_PatternFile.GetConst();
            MarkerDescription.m_WidthInMeter = rCurrentMarker.m_WidthInMeter;

            MR::CMarkerPtr MarkerPtr = MR::MarkerManager::CreateMarker(MarkerDescription);


            int Error = 0;

            ARPattHandle* pNativeARPatternHandle = static_cast<ARPattHandle*>(MarkerPtr->m_pHandle);

            Error = arPattAttach(m_pARHandle, pNativeARPatternHandle);

            assert(Error == 0);

            MarkerPtr->m_IsRegistered = true;
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::CreatePluginARTrackedObject(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::TrackMarker()
    {
        if (m_IsActive == false) return;
        
        // -----------------------------------------------------------------------------
        // Prepare collections
        // -----------------------------------------------------------------------------        
        m_MarkerInfos.clear();

        // -----------------------------------------------------------------------------
        // Get image stream for tracking and detect marker in image
        // -----------------------------------------------------------------------------
        ARUint8* pImagedata;
        
        pImagedata = static_cast<ARUint8*>(m_pActiveControl->GetOriginalFrame()->GetPixels());
        
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

    void CMRControlManager::UpdateTrackerEntities()
    {
        // -----------------------------------------------------------------------------
        // Detect marker
        // -----------------------------------------------------------------------------
        MR::CMarkerInfo MarkerInfo;

        CMarkerInfos::iterator CurrentOfMarkerInfo = m_MarkerInfos.begin();
        CMarkerInfos::iterator EndOfMarkerInfos    = m_MarkerInfos.end();

        for (; CurrentOfMarkerInfo != EndOfMarkerInfos; ++ CurrentOfMarkerInfo)
        {
            CMarkerInfo& rMarkerInfo = *CurrentOfMarkerInfo;

            Base::Float3x3 RotationMatrix(Base::Float3x3::s_Identity);
            Base::Float3   Position;

            RotationMatrix[0][0] = rMarkerInfo.m_RotationToCamera[0][0];
            RotationMatrix[0][1] = rMarkerInfo.m_RotationToCamera[0][1];
            RotationMatrix[0][2] = rMarkerInfo.m_RotationToCamera[0][2];

            RotationMatrix[1][0] = -rMarkerInfo.m_RotationToCamera[1][0];
            RotationMatrix[1][1] = -rMarkerInfo.m_RotationToCamera[1][1];
            RotationMatrix[1][2] = -rMarkerInfo.m_RotationToCamera[1][2];

            RotationMatrix[2][0] = -rMarkerInfo.m_RotationToCamera[2][0];
            RotationMatrix[2][1] = -rMarkerInfo.m_RotationToCamera[2][1];
            RotationMatrix[2][2] = -rMarkerInfo.m_RotationToCamera[2][2];

            Position[0] = -rMarkerInfo.m_TranslationToCamera[0];
            Position[1] =  rMarkerInfo.m_TranslationToCamera[1];
            Position[2] =  rMarkerInfo.m_TranslationToCamera[2];

            Position = RotationMatrix.GetInverted() * Position;

            // -----------------------------------------------------------------------------
            // Marker Found: Now search for entity with AR facet
            // -----------------------------------------------------------------------------
            Dt::CEntity* pCameraEntity = m_pController->GetCameraEntity();

            assert(pCameraEntity != nullptr);

            pCameraEntity->SetWorldPosition(Position);

            Dt::CTransformationFacet* pTransformationFacet = pCameraEntity->GetTransformationFacet();

            assert(pTransformationFacet != nullptr);

            Base::Float3 Rotation;

            RotationMatrix.GetRotation(Rotation);

            pTransformationFacet->SetRotation(Rotation * -1.0f);
        }
    }

} // namespace

namespace MR
{
namespace ControlManager
{
    void OnStart()
    {
        CMRControlManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRControlManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRControlManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    bool IsActive()
    {
        return CMRControlManager::GetInstance().IsActive();
    }

    // -----------------------------------------------------------------------------

    CControl* GetActiveControl()
    {
        return CMRControlManager::GetInstance().GetActiveControl();
    }
} // namespace ControlManager
} // namespace MR
