
#include "mr/mr_precompiled.h"

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "mr/mr_marker_manager.h"
#include "mr/mr_session_manager.h"

#include "arcore_c_api.h"

#include <vector>

using namespace MR;

namespace 
{
    class CMRMarkerManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRMarkerManager)

    public:

        CMRMarkerManager();
        ~CMRMarkerManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

        const CMarker* AcquireNewMarker(float _X, float _Y);
        void ReleaseMarker(const CMarker* _pMarker);

    private:

        class CInternMarker : public CMarker
        {
        public:

            ArAnchor* m_pAnchor;

        private:

            friend class CMRMarkerManager;
        };


    private:

        typedef std::vector<CInternMarker> CTrackedObjects;

    private:

        CTrackedObjects m_TrackedObjects;
    };
} // namespace 

namespace 
{
    CMRMarkerManager::CMRMarkerManager()
        : m_TrackedObjects( )
    {
    }

    // -----------------------------------------------------------------------------

    CMRMarkerManager::~CMRMarkerManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRMarkerManager::OnStart()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CMRMarkerManager::OnExit()
    {
        m_TrackedObjects.clear();
    }

    // -----------------------------------------------------------------------------

    void CMRMarkerManager::Update()
    {
#ifdef PLATFORM_ANDROID
        const CSession& rActiveSession = MR::SessionManager::GetSession();

        if (rActiveSession.GetSessionState() != CSession::Success) return;

        ArSession* pARSession = static_cast<ArSession*>(rActiveSession.GetSession());

        for (auto& rObject : m_TrackedObjects)
        {
            ArTrackingState TrackingState = AR_TRACKING_STATE_STOPPED;

            ArAnchor_getTrackingState(pARSession, rObject.m_pAnchor, &TrackingState);

            switch (TrackingState)
            {
            case AR_TRACKING_STATE_PAUSED:
                rObject.m_TrackingState = CMarker::Paused;
                break;
            case AR_TRACKING_STATE_STOPPED:
                rObject.m_TrackingState = CMarker::Stopped;
                break;
            case AR_TRACKING_STATE_TRACKING:
                rObject.m_TrackingState = CMarker::Tracking;
                break;
            }

            if (TrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArPose* pARPose = 0;

            ArPose_create(pARSession, 0, &pARPose);

            ArAnchor_getPose(pARSession, rObject.m_pAnchor, pARPose);

            ArPose_getMatrix(pARSession, pARPose, glm::value_ptr(rObject.m_ModelMatrix));

            ArPose_destroy(pARPose);
        }
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------

    const CMarker* CMRMarkerManager::AcquireNewMarker(float _X, float _Y)
    {
#if PLATFORM_ANDROID
        CInternMarker* pReturnMarker = nullptr;

        const CSession& rActiveSession = MR::SessionManager::GetSession();

        if (rActiveSession.GetSessionState() != CSession::Success) return nullptr;

        ArSession* pARSession = static_cast<ArSession*>(rActiveSession.GetSession());
        ArFrame*   pARFrame = static_cast<ArFrame*>(rActiveSession.GetFrame());

        if (pARSession != nullptr && pARFrame != nullptr)
        {
            ArHitResultList* pHitResultList = 0;

            ArHitResultList_create(pARSession, &pHitResultList);

            assert(pHitResultList);

            ArFrame_hitTest(pARSession, pARFrame, _X, _Y, pHitResultList);

            int NumberOfHits = 0;

            ArHitResultList_getSize(pARSession, pHitResultList, &NumberOfHits);

            // -----------------------------------------------------------------------------
            // The hitTest method sorts the resulting list by distance from the camera,
            // increasing.  The first hit result will usually be the most relevant when
            // responding to user input
            // -----------------------------------------------------------------------------
            ArHitResult* pHitResult = nullptr;

            for (int IndexOfHit = 0; IndexOfHit < NumberOfHits; ++IndexOfHit)
            {
                ArHitResult* pEstimatedHitResult = nullptr;

                ArHitResult_create(pARSession, &pEstimatedHitResult);

                ArHitResultList_getItem(pARSession, pHitResultList, IndexOfHit, pEstimatedHitResult);

                if (pEstimatedHitResult == nullptr)
                {
                    return nullptr;
                }

                // -----------------------------------------------------------------------------
                // Get trackables
                // -----------------------------------------------------------------------------
                ArTrackable* pTrackable = nullptr;

                ArHitResult_acquireTrackable(pARSession, pEstimatedHitResult, &pTrackable);

                ArTrackableType TrackableType = AR_TRACKABLE_NOT_VALID;

                ArTrackable_getType(pARSession, pTrackable, &TrackableType);

                switch (TrackableType)
                {
                case AR_TRACKABLE_PLANE:
                {
                    ArPose* pPose = nullptr;

                    ArPose_create(pARSession, nullptr, &pPose);

                    ArHitResult_getHitPose(pARSession, pEstimatedHitResult, pPose);

                    int32_t IsPoseInPolygon = 0;

                    ArPlane* pPlane = ArAsPlane(pTrackable);

                    ArPlane_isPoseInPolygon(pARSession, pPlane, pPose, &IsPoseInPolygon);

                    ArTrackable_release(pTrackable);

                    ArPose_destroy(pPose);

                    if (!IsPoseInPolygon)
                    {
                        continue;
                    }

                    pHitResult = pEstimatedHitResult;
                } break;

                case AR_TRACKABLE_POINT:
                {
                    ArPoint* pPoint = ArAsPoint(pTrackable);

                    ArPointOrientationMode OrientationMode;

                    ArPoint_getOrientationMode(pARSession, pPoint, &OrientationMode);

                    if (OrientationMode == AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL)
                    {
                        pHitResult = pEstimatedHitResult;
                    }
                } break;

                default:
                {
                    ArTrackable_release(pTrackable);

                    BASE_CONSOLE_INFO("Undefined trackable type found.")
                }
                };
            }

            if (pHitResult != nullptr)
            {
                ArAnchor* pAnchor = nullptr;

                if (ArHitResult_acquireNewAnchor(pARSession, pHitResult, &pAnchor) != AR_SUCCESS)
                {
                    return nullptr;
                }

                ArTrackingState TrackingState = AR_TRACKING_STATE_STOPPED;

                ArAnchor_getTrackingState(pARSession, pAnchor, &TrackingState);

                if (TrackingState != AR_TRACKING_STATE_TRACKING)
                {
                    ArAnchor_release(pAnchor);

                    return nullptr;
                }

                CInternMarker NewMarker;

                NewMarker.m_pAnchor = pAnchor;

                m_TrackedObjects.emplace_back(NewMarker);

                pReturnMarker = &m_TrackedObjects.back();

                ArHitResult_destroy(pHitResult);
            }

            ArHitResultList_destroy(pHitResultList);

            pHitResultList = nullptr;
        }

        return pReturnMarker;
#else
        BASE_UNUSED(_X);
        BASE_UNUSED(_Y);

        return nullptr;
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------

    void CMRMarkerManager::ReleaseMarker(const CMarker* _pMarker)
    {
#if PLATFORM_ANDROID
        auto MarkerIter = std::find_if(m_TrackedObjects.begin(), m_TrackedObjects.end(), [&](CInternMarker& _rObject) { return &_rObject == _pMarker; });

        if (MarkerIter == m_TrackedObjects.end()) return;

        const CInternMarker* pInternMarker = static_cast<const CInternMarker*>(_pMarker);

        ArAnchor_release(pInternMarker->m_pAnchor);

        m_TrackedObjects.erase(MarkerIter);
#else
        BASE_UNUSED(_pMarker);
#endif // PLATFORM_ANDROID
    }
} // namespace 

namespace MR
{
namespace MarkerManager
{
    void OnStart()
    {
        CMRMarkerManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRMarkerManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRMarkerManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    const CMarker* AcquireNewMarker(float _X, float _Y)
    {
        return CMRMarkerManager::GetInstance().AcquireNewMarker(_X, _Y);
    }

    // -----------------------------------------------------------------------------

    void ReleaseMarker(const CMarker* _pMarker)
    {
        CMRMarkerManager::GetInstance().ReleaseMarker(_pMarker);
    }
} // namespace MarkerManager
} // namespace MR