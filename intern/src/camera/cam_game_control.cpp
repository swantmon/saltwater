
#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_camera_interface.h"

namespace Cam
{
    CGameControl::CGameControl()
        : CControl           (CControl::GameControl)
        , m_pMainCameraEntity(nullptr)
    {
        m_Position[0] = 0.0f;
        m_Position[1] = 0.0f;
        m_Position[2] = 1.0f;
    }
    
    // -----------------------------------------------------------------------------
    
    CGameControl::~CGameControl()
    {

        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::SetProjectionMatrix(const Base::Float3x3& _rProjectionMatrix)
    {
        Gfx::Cam::InjectCameraMatrix(_rProjectionMatrix);
    }

    // -----------------------------------------------------------------------------

    void CGameControl::SetMarkerTransformation(const Base::Float3x3& _rRotationMatrix, const Base::Float3& _rTranslation)
    {
        m_RotationMatrix[0][0] = _rRotationMatrix[0][0];
        m_RotationMatrix[0][1] = _rRotationMatrix[0][1];
        m_RotationMatrix[0][2] = _rRotationMatrix[0][2];

        m_RotationMatrix[1][0] = -_rRotationMatrix[1][0];
        m_RotationMatrix[1][1] = -_rRotationMatrix[1][1];
        m_RotationMatrix[1][2] = -_rRotationMatrix[1][2];

        m_RotationMatrix[2][0] = -_rRotationMatrix[2][0];
        m_RotationMatrix[2][1] = -_rRotationMatrix[2][1];
        m_RotationMatrix[2][2] = -_rRotationMatrix[2][2];

        m_Position[0] = -_rTranslation[0];
        m_Position[1] =  _rTranslation[1];
        m_Position[2] =  _rTranslation[2];

        m_Position = m_RotationMatrix.GetInverted() * m_Position;
    }

    // -----------------------------------------------------------------------------

    void CGameControl::SetEntity(Dt::CEntity& _rEntity)
    {
        m_pMainCameraEntity = &_rEntity;
    }

    // -----------------------------------------------------------------------------

    Dt::CEntity* CGameControl::GetEntity()
    {
        return m_pMainCameraEntity;
    }

    // -----------------------------------------------------------------------------

    const Dt::CEntity* CGameControl::GetEntity() const
    {
        return m_pMainCameraEntity;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternOnEvent(const Base::CInputEvent& _rEvent)
    {
        BASE_UNUSED(_rEvent);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternUpdate()
    {
        assert(m_pMainCameraEntity != 0);

        if (m_pMainCameraEntity != 0)
        {
            Dt::CTransformationFacet* pTransformationFacet = m_pMainCameraEntity->GetTransformationFacet();
            Dt::CCameraActorFacet*    pCameraFacet         = static_cast<Dt::CCameraActorFacet*>(m_pMainCameraEntity->GetDetailFacet(Dt::SFacetCategory::Data));

            assert(pTransformationFacet != nullptr);

            // -----------------------------------------------------------------------------
            // Position
            // -----------------------------------------------------------------------------
            m_Position.Set(m_pMainCameraEntity->GetWorldPosition());

            // -----------------------------------------------------------------------------
            // Rotation
            // -----------------------------------------------------------------------------
            Base::Float3& rRotationInDegree = pTransformationFacet->GetRotation();

            m_RotationMatrix.SetRotation(rRotationInDegree[0], rRotationInDegree[1], rRotationInDegree[2]);

            // -----------------------------------------------------------------------------
            // Projection
            // -----------------------------------------------------------------------------
            if (pCameraFacet->GetProjectionType() == Dt::CCameraActorFacet::Perspective)
            {
                // TODO: Get aspect ratio from ???
                Gfx::Cam::SetFieldOfView(pCameraFacet->GetFoV(), 1280.0f / 720.0f, pCameraFacet->GetNear(), pCameraFacet->GetFar());
            }
            else if (pCameraFacet->GetProjectionType() == Dt::CCameraActorFacet::Orthographic)
            {
                Gfx::Cam::SetOrthographic(pCameraFacet->GetSize(), pCameraFacet->GetSize(), pCameraFacet->GetNear(), pCameraFacet->GetFar());
            }

            // -----------------------------------------------------------------------------
            // Camera mode + variables
            // -----------------------------------------------------------------------------
            Gfx::Cam::SetAutoCameraMode();
            
            if (pCameraFacet->GetCameraMode() == Dt::CCameraActorFacet::Manual)
            {
                Gfx::Cam::SetManualCameraMode();

                Gfx::Cam::SetShutterSpeed(pCameraFacet->GetShutterSpeed());

                Gfx::Cam::SetAperture(pCameraFacet->GetAperture());

                Gfx::Cam::SetISO(pCameraFacet->GetISO());

                Gfx::Cam::SetEC(pCameraFacet->GetEC());
            }

            // -----------------------------------------------------------------------------
            // Other
            // -----------------------------------------------------------------------------
            Gfx::Cam::SetBackgroundColor(pCameraFacet->GetBackgroundColor());
            
            Gfx::Cam::SetCullingMask(pCameraFacet->GetCullingMask());

            Gfx::Cam::SetViewportRect(pCameraFacet->GetViewportRect());

            Gfx::Cam::SetDepth(pCameraFacet->GetDepth());
        }

        Gfx::Cam::SetPosition(m_Position);
        Gfx::Cam::SetRotationMatrix(m_RotationMatrix);
        Gfx::Cam::Update();
    }
} // namespace Cam