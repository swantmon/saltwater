
#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_pool.h"
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
#include "mr/mr_webcam_control.h"

#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include <assert.h>
#include <unordered_map>
#include <vector>

using namespace MR;

namespace
{
    std::string g_PathToAssets = "../assets/";
} // namespace

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

        void Clear();

        void Update();

        bool IsActive();

        CControl* GetActiveControl();

    private:

        struct CInternMarker
        {
        public:
            unsigned int                           m_UID;
            unsigned int                           m_NativeID;
            Dt::CARControllerPluginFacet::SMarker* m_pDataInfos;
            ARPattHandle*                          m_pPatternHandle;
        };

        struct CInternTrackedMarker
        {
        public:
            unsigned int   m_UID;
            unsigned int   m_FrameCounter;
            Base::Float3x3 m_RotationToCamera;
            Base::Float3   m_TranslationToCamera;
        };

    private:

        typedef Base::CPool<CInternMarker, 8>                    CMarkers;

        typedef std::vector<Dt::CEntity*>                        CEntityVector;
        typedef std::vector<CInternTrackedMarker>                CTrackedMarker;
        typedef std::unordered_map<unsigned int, CInternMarker*> CMarkerByIDs;

    private:

        ARHandle*	                  m_pNativeTrackingHandle;
        AR3DHandle*	                  m_pNativeTracking3DHandle;
        ARParamLT*	                  m_pNativeParameterLT;
        CEntityVector                 m_DirtyEntities;
        CTrackedMarker                m_TrackedMarker;
        CMarkerByIDs                  m_MarkerByIDs;
        CMarkers                      m_Markers;
        CControl*                     m_pActiveControl;
        CControl*                     m_pControls[CControl::NumberOfControls];
        Dt::CARControllerPluginFacet* m_pControllerPlugin;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        void UpdatePlugin(Dt::CEntity& _rEntity);
        void UpdatePluginARController(Dt::CEntity& _rEntity);
        void UpdatePluginARTrackedObject(Dt::CEntity& _rEntity);

        void CreatePluginARController(Dt::CEntity& _rEntity);
        void CreatePluginARTrackedObject(Dt::CEntity& _rEntity);

        void SetupActiveControl();
        void SetupCameraEntity();
        void SetupTrackerManager();
        void SetupMarkerManager();

        void UpdateActiveControl();
        void UpdateTrackerManager();
        void UpdateCameraEntity();
        void UpdateTrackedEntities();
    };
} // namespace

namespace
{
    CMRControlManager::CMRControlManager()
        : m_pNativeTrackingHandle  (0)
        , m_pNativeTracking3DHandle(0)
        , m_pNativeParameterLT     (0)
        , m_TrackedMarker          ()
        , m_DirtyEntities          ()
        , m_Markers                ()
        , m_MarkerByIDs            ()
        , m_pActiveControl         (nullptr)
        , m_pControllerPlugin      (nullptr)
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
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Clear()
    {
        // -----------------------------------------------------------------------------
        // Delete controls
        // -----------------------------------------------------------------------------
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

        // -----------------------------------------------------------------------------
        // Delete entities
        // -----------------------------------------------------------------------------
        m_DirtyEntities.clear();

        // -----------------------------------------------------------------------------
        // Delete tracked marker informations and IDs
        // -----------------------------------------------------------------------------
        m_TrackedMarker.clear();
        m_MarkerByIDs  .clear();

        // -----------------------------------------------------------------------------
        // Delete markers
        // -----------------------------------------------------------------------------
        CMarkers::CIterator CurrentMarker = m_Markers.Begin();
        CMarkers::CIterator EndOfMarkers  = m_Markers.End();

        for (; CurrentMarker != EndOfMarkers; ++CurrentMarker)
        {
            arPattDeleteHandle(CurrentMarker->m_pPatternHandle);
        }

        m_Markers.Clear();

        // -----------------------------------------------------------------------------
        // Delete native part
        // -----------------------------------------------------------------------------
        arPattDetach(m_pNativeTrackingHandle);
        ar3DDeleteHandle(&m_pNativeTracking3DHandle);
        arDeleteHandle(m_pNativeTrackingHandle);
        arParamLTFree(&m_pNativeParameterLT);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        // -----------------------------------------------------------------------------
        // New and dirty entities
        // -----------------------------------------------------------------------------
        CEntityVector::iterator CurrentDirtyEntity = m_DirtyEntities.begin();
        CEntityVector::iterator EndOfDirtyEntities = m_DirtyEntities.end();

        for (; CurrentDirtyEntity != EndOfDirtyEntities; ++CurrentDirtyEntity)
        {
            UpdatePlugin(**CurrentDirtyEntity);
        }

        m_DirtyEntities.clear();

        // -----------------------------------------------------------------------------
        // Default behavior of the manager
        // -----------------------------------------------------------------------------
        UpdateActiveControl();

        UpdateTrackerManager();

        UpdateCameraEntity();

        UpdateTrackedEntities();
    }

    // -----------------------------------------------------------------------------

    bool CMRControlManager::IsActive()
    {
        return (m_pControllerPlugin != nullptr && m_pActiveControl != nullptr && m_pActiveControl->IsStarted());
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
        Dt::CARControllerPluginFacet* pControllerPlugin = static_cast<Dt::CARControllerPluginFacet*>(_rEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        assert(pControllerPlugin != nullptr);

        m_pControllerPlugin = pControllerPlugin;

        SetupActiveControl();

        SetupCameraEntity();

        SetupTrackerManager();

        SetupMarkerManager();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdatePluginARTrackedObject(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::CreatePluginARController(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::CreatePluginARTrackedObject(Dt::CEntity& _rEntity)
    {
        BASE_UNUSED(_rEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::SetupActiveControl()
    {
        assert (m_pControllerPlugin != nullptr);

        // -----------------------------------------------------------------------------
        // Create control
        // -----------------------------------------------------------------------------
        switch (m_pControllerPlugin->GetDeviceType())
        {
        case Dt::CARControllerPluginFacet::Webcam:
        {
            m_pActiveControl = m_pControls[CControl::Webcam];

            assert(m_pActiveControl != nullptr);

            // -----------------------------------------------------------------------------
            // Setup device depending stuff
            // -----------------------------------------------------------------------------
            CWebcamControl* pWebcamControl = static_cast<CWebcamControl*>(m_pActiveControl);

            pWebcamControl->SetDeviceNumber(m_pControllerPlugin->GetDeviceNumber());
        }
        break;
        case Dt::CARControllerPluginFacet::Kinect:
        {
            m_pActiveControl = m_pControls[CControl::Kinect];

            assert(m_pActiveControl != nullptr);
        }
        break;
        };

        // -----------------------------------------------------------------------------
        // Update general stuff
        // -----------------------------------------------------------------------------
        m_pActiveControl->SetConvertedFrame(m_pControllerPlugin->GetOutputBackground());
        m_pActiveControl->SetCubemap(m_pControllerPlugin->GetOutputCubemap());

        m_pActiveControl->Start(m_pControllerPlugin->GetCameraParameterFile());
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::SetupCameraEntity()
    {
        assert(m_pControllerPlugin != nullptr && m_pActiveControl != nullptr);

        // -----------------------------------------------------------------------------
        // Set camera
        // Note: Camera will be automatically set to RAW projection input
        // -----------------------------------------------------------------------------
        Dt::CEntity* pCameraEntity = m_pControllerPlugin->GetCameraEntity();

        assert(pCameraEntity != nullptr);

        Dt::CCameraActorFacet* pCameraActorFacet = static_cast<Dt::CCameraActorFacet*>(pCameraEntity->GetDetailFacet(Dt::SFacetCategory::Data));

        pCameraActorFacet->SetProjectionType(Dt::CCameraActorFacet::RAW);
        pCameraActorFacet->SetProjectionMatrix(m_pActiveControl->GetProjectionMatrix());
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::SetupTrackerManager()
    {
        assert(m_pActiveControl != nullptr);

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
        m_pNativeParameterLT = arParamLTCreate(&NativeParams, AR_PARAM_LT_DEFAULT_OFFSET);

        assert(m_pNativeParameterLT != 0);

        // -----------------------------------------------------------------------------
        // Create AR handle with parameters
        // -----------------------------------------------------------------------------
        m_pNativeTrackingHandle = arCreateHandle(m_pNativeParameterLT);

        assert(m_pNativeTrackingHandle != 0);

        // -----------------------------------------------------------------------------
        // Set AR pixel format
        // -----------------------------------------------------------------------------
        Error = arSetPixelFormat(m_pNativeTrackingHandle, OriginalPixelFormat);

        assert(Error >= 0);

        // -----------------------------------------------------------------------------
        // Create 3D AR handle
        // -----------------------------------------------------------------------------
        m_pNativeTracking3DHandle = ar3DCreateHandle(&NativeParams);

        assert(m_pNativeTracking3DHandle != 0);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::SetupMarkerManager()
    {
        assert(m_pControllerPlugin != nullptr);

        unsigned int NumberOfMarker = m_pControllerPlugin->GetNumberOfMarker();

        for (unsigned int IndexOfMarker = 0; IndexOfMarker < NumberOfMarker; ++ IndexOfMarker)
        {
            Dt::CARControllerPluginFacet::SMarker& rCurrentMarker = m_pControllerPlugin->GetMarker(IndexOfMarker);

            // -----------------------------------------------------------------------------
            // Create marker and save to collection
            // -----------------------------------------------------------------------------
#pragma warning( disable : 4706 4996 )
            auto LoadPatternFromBuffer = [&](ARPattHandle *pattHandle, const char *buffer) 
            {
                char   *bufCopy;
                int     patno;
                int     h, i1, i2, i3;
                int     i, j, l, m;
                char   *buffPtr;
                const char *delims = " \t\n\r";

                if (!pattHandle) {
                    ARLOGe("Error: NULL pattHandle.\n");
                    return (-1);
                }
                if (!buffer) {
                    ARLOGe("Error: can't load pattern from NULL buffer.\n");
                    return (-1);
                }

                for (i = 0; i < pattHandle->patt_num_max; i++) {
                    if (pattHandle->pattf[i] == 0) break;
                }
                if (i == pattHandle->patt_num_max) return -1;
                patno = i;

                if (!(bufCopy = _strdup(buffer))) { // Make a mutable copy.
                    ARLOGe("Error: out of memory.\n");
                    return (-1);
                }
                buffPtr = strtok(bufCopy, delims);

                for (h = 0; h<4; h++) {
                    l = 0;
                    for (i3 = 0; i3 < 3; i3++) { // Three colours B G R
                        for (i2 = 0; i2 < pattHandle->pattSize; i2++) { // Rows
                            for (i1 = 0; i1 < pattHandle->pattSize; i1++) { // Columns

                                /* Switch file scanning to buffer reading */

                                /* if( fscanf(fp, "%d", &j) != 1 ) {
                                ARLOGe("Pattern Data read error!!\n");
                                return -1;
                                }
                                */

                                if (buffPtr == NULL) {
                                    ARLOGe("Pattern Data read error!!\n");
                                    free(bufCopy);
                                    return -1;
                                }

                                j = atoi(buffPtr);
                                buffPtr = strtok(NULL, delims);

                                j = 255 - j;
                                pattHandle->patt[patno * 4 + h][(i2*pattHandle->pattSize + i1) * 3 + i3] = j;
                                if (i3 == 0) pattHandle->pattBW[patno * 4 + h][i2*pattHandle->pattSize + i1] = j;
                                else          pattHandle->pattBW[patno * 4 + h][i2*pattHandle->pattSize + i1] += j;
                                if (i3 == 2) pattHandle->pattBW[patno * 4 + h][i2*pattHandle->pattSize + i1] /= 3;
                                l += j;
                            }
                        }
                    }
                    l /= (pattHandle->pattSize*pattHandle->pattSize * 3);

                    m = 0;
                    for (i = 0; i < pattHandle->pattSize*pattHandle->pattSize * 3; i++) {
                        pattHandle->patt[patno * 4 + h][i] -= l;
                        m += (pattHandle->patt[patno * 4 + h][i] * pattHandle->patt[patno * 4 + h][i]);
                    }
                    pattHandle->pattpow[patno * 4 + h] = sqrt((ARdouble)m);
                    if (pattHandle->pattpow[patno * 4 + h] == 0.0) pattHandle->pattpow[patno * 4 + h] = 0.0000001;

                    m = 0;
                    for (i = 0; i < pattHandle->pattSize*pattHandle->pattSize; i++) {
                        pattHandle->pattBW[patno * 4 + h][i] -= l;
                        m += (pattHandle->pattBW[patno * 4 + h][i] * pattHandle->pattBW[patno * 4 + h][i]);
                    }
                    pattHandle->pattpowBW[patno * 4 + h] = sqrt((ARdouble)m);
                    if (pattHandle->pattpowBW[patno * 4 + h] == 0.0) pattHandle->pattpowBW[patno * 4 + h] = 0.0000001;
                }

                free(bufCopy);

                pattHandle->pattf[patno] = 1;
                pattHandle->patt_num++;

                return(patno);
            };

            auto LoadPattern = [&](ARPattHandle *pattHandle, const char *filename)
            {
                FILE   *fp;
                int     patno;
                size_t  ret;

                /* Old variables */
                /*
                int     h, i1, i2, i3;
                int     i, j, l, m;
                */

                /* New variables */
                long pos = 0;
                char* bytes = NULL;

                /* Open file */
                fp = fopen(filename, "rb");
                if (fp == NULL) {
                    ARLOGe("Error opening pattern file '%s' for reading.\n", filename);
                    return (-1);
                }

                /* Determine number of bytes in file */
                fseek(fp, 0L, SEEK_END);
                pos = ftell(fp);
                fseek(fp, 0L, SEEK_SET);

                //ARLOGd("Pattern file is %ld bytes\n", pos);

                /* Allocate buffer */
                bytes = (char *)malloc(pos + 1);
                if (!bytes) {
                    ARLOGe("Out of memory!!\n");
                    fclose(fp);
                    return (-1);
                }

                /* Read pattern into buffer and close file */
                ret = fread(bytes, pos, 1, fp);
                fclose(fp);
                if (ret < 1) {
                    ARLOGe("Error reading pattern file '%s'.\n", filename);
                    free(bytes);
                    return (-1);
                }

                /* Terminate string */
                bytes[pos] = '\0';

                /* Load pattern from buffer */
                patno = LoadPatternFromBuffer(pattHandle, bytes);

                /* Free allocated buffer */
                free(bytes);

                return(patno);
            };
#pragma warning( error : 4706 4996 )

            // -----------------------------------------------------------------------------
            // Build path to texture in file system
            // -----------------------------------------------------------------------------
            std::string PathToPattern;
        
            // -----------------------------------------------------------------------------
            // Create hash and check if marker already available
            // -----------------------------------------------------------------------------
            const char* pPatternFile = rCurrentMarker.m_PatternFile.GetConst();
        
            assert(pPatternFile != 0);

		    PathToPattern = g_PathToAssets + pPatternFile;
        
            // -----------------------------------------------------------------------------
            // Setup basic marker informations and after that create handle
            // -----------------------------------------------------------------------------
            CInternMarker& rNewMarker = m_Markers.Allocate();

            rNewMarker.m_UID        = rCurrentMarker.m_UID;
            rNewMarker.m_pDataInfos = &rCurrentMarker;
            
            rNewMarker.m_pPatternHandle = arPattCreateHandle();
            
            assert(rNewMarker.m_pPatternHandle != 0);

    #ifdef __APPLE__
            rNewMarker.m_NativeID = arPattLoad(rNewMarker.m_pPatternHandle, PathToPattern.c_str());
#else
            rNewMarker.m_NativeID = LoadPattern(rNewMarker.m_pPatternHandle, PathToPattern.c_str());
    #endif  
                        
            // -----------------------------------------------------------------------------
            // Set hash to map to reuse this kind of marker
            // -----------------------------------------------------------------------------
            m_MarkerByIDs[rNewMarker.m_NativeID] = &rNewMarker;

            // -----------------------------------------------------------------------------
            // Attach marker
            // -----------------------------------------------------------------------------
            int Error = 0;

            Error = arPattAttach(m_pNativeTrackingHandle, rNewMarker.m_pPatternHandle);

            assert(Error == 0);
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdateActiveControl()
    {
        if (IsActive() == false) return;

        m_pActiveControl->Update();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdateTrackerManager()
    {
        if (IsActive() == false) return;
        
        // -----------------------------------------------------------------------------
        // Prepare collections
        // -----------------------------------------------------------------------------        
        m_TrackedMarker.clear();

        // -----------------------------------------------------------------------------
        // Get image stream for tracking and detect marker in image
        // -----------------------------------------------------------------------------
        ARUint8* pImagedata;
        
        pImagedata = static_cast<ARUint8*>(m_pActiveControl->GetOriginalFrame()->GetPixels());
        
        assert(pImagedata != 0);

        arDetectMarker(m_pNativeTrackingHandle, pImagedata);
        
        // -----------------------------------------------------------------------------
        // Get number of markers found in last detection
        // -----------------------------------------------------------------------------
        unsigned int NumberOfMarkers;
        
        NumberOfMarkers = static_cast<unsigned int>(arGetMarkerNum(m_pNativeTrackingHandle));

        if (NumberOfMarkers == 0)
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Check for pattern visibility
        // -----------------------------------------------------------------------------
        static ARdouble PatternTransformation[3][4];
        
        ARMarkerInfo* pMarkerInfos;
        
        pMarkerInfos = arGetMarker(m_pNativeTrackingHandle);

        for (unsigned int IndexOfMarker = 0; IndexOfMarker < NumberOfMarkers; ++ IndexOfMarker)
        {
            ARMarkerInfo& rCurrentMarkerInfo = pMarkerInfos[IndexOfMarker];

            if (rCurrentMarkerInfo.cf >= 0.7)
            {
                // -----------------------------------------------------------------------------
                // Found a marker
                // -----------------------------------------------------------------------------
                CInternMarker* pMarker = m_MarkerByIDs.at(rCurrentMarkerInfo.id);
                
                // -----------------------------------------------------------------------------
                // Get transformation of this pattern
                // -----------------------------------------------------------------------------
                arGetTransMatSquare(m_pNativeTracking3DHandle, &(rCurrentMarkerInfo), pMarker->m_pDataInfos->m_WidthInMeter * 1000.0f, PatternTransformation);
                
                // -----------------------------------------------------------------------------
                // Create / Edit marker infos
                // -----------------------------------------------------------------------------
                CInternTrackedMarker NewMarkerInfo;

                NewMarkerInfo.m_UID          = pMarker->m_pDataInfos->m_UID;
                NewMarkerInfo.m_FrameCounter = 0;
                
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
                
                m_TrackedMarker.push_back(NewMarkerInfo);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdateCameraEntity()
    {
        if (IsActive() == false) return;

        // -----------------------------------------------------------------------------
        // Take the first detected marker as origin
        // -----------------------------------------------------------------------------
        CTrackedMarker::iterator CurrentOfMarkerInfo = m_TrackedMarker.begin();
        CTrackedMarker::iterator EndOfMarkerInfos    = m_TrackedMarker.end();

        for (; CurrentOfMarkerInfo != EndOfMarkerInfos; ++ CurrentOfMarkerInfo)
        {
            CInternTrackedMarker& rMarkerInfo = *CurrentOfMarkerInfo;

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
            Dt::CEntity* pCameraEntity = m_pControllerPlugin->GetCameraEntity();

            assert(pCameraEntity != nullptr);

            pCameraEntity->SetWorldPosition(Position);

            Dt::CTransformationFacet* pTransformationFacet = pCameraEntity->GetTransformationFacet();

            assert(pTransformationFacet != nullptr);

            Base::Float3 Rotation;

            RotationMatrix.GetRotation(Rotation);

            pTransformationFacet->SetRotation(Rotation * -1.0f);

            break;
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdateTrackedEntities()
    {
        if (IsActive() == false) return;
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

    void Clear()
    {
        CMRControlManager::GetInstance().Clear();
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
