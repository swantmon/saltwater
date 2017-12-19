
#include "camera/cam_precompiled.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_actor_type.h"
#include "data/data_camera_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_camera_interface.h"

namespace Cam
{
    CGameControl::CGameControl()
        : CControl           (CControl::GameControl)
        , m_pMainCameraEntity(nullptr)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CGameControl::~CGameControl()
    {

        
    }

    // -----------------------------------------------------------------------------

    void CGameControl::SetEntity(Dt::CEntity& _rEntity)
    {
        m_pMainCameraEntity = &_rEntity;

        OnDirtyEntity(m_pMainCameraEntity);
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

    void CGameControl::InternOnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (m_pMainCameraEntity != _pEntity) return;

        if (m_pMainCameraEntity->GetDirtyFlags() & Dt::CEntity::DirtyMove)
        {
            Dt::CTransformationFacet* pTransformationFacet = m_pMainCameraEntity->GetTransformationFacet();

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
        }

        if (m_pMainCameraEntity->GetDirtyFlags() & Dt::CEntity::DirtyDetail)
        {
            Dt::CCameraActorFacet*    pCameraFacet = static_cast<Dt::CCameraActorFacet*>(m_pMainCameraEntity->GetDetailFacet(Dt::SFacetCategory::Data));

            assert(pCameraFacet != nullptr);

            // -----------------------------------------------------------------------------
            // Projection
            // -----------------------------------------------------------------------------
            if (pCameraFacet->GetProjectionType() == Dt::CCameraActorFacet::Perspective)
            {
                Gfx::Cam::SetFieldOfView(pCameraFacet->GetFoV(), pCameraFacet->GetNear(), pCameraFacet->GetFar());
            }
            else if (pCameraFacet->GetProjectionType() == Dt::CCameraActorFacet::Orthographic)
            {
                float Left   = -pCameraFacet->GetSize() / 2.0f;
                float Right  =  pCameraFacet->GetSize() / 2.0f;
                float Bottom = -pCameraFacet->GetSize() / 2.0f;
                float Top    =  pCameraFacet->GetSize() / 2.0f;
                
                Gfx::Cam::SetOrthographic(Left, Right, Bottom, Top, pCameraFacet->GetNear(), pCameraFacet->GetFar());
            }
            else if (pCameraFacet->GetProjectionType() == Dt::CCameraActorFacet::RAW)
            {
                Base::Float4x4 ProjectionMatrix(Base::Float3x3::s_Identity);

                float Near = pCameraFacet->GetNear();
                float Far  = pCameraFacet->GetFar();

                ProjectionMatrix.SetRHPerspective(Near, Far, pCameraFacet->GetProjectionMatrix());

                // -----------------------------------------------------------------------------
                // Decompose left, right, top, bottom, near and far from projection
                // matrix:
                // Near = ProjectionMatrix[2][3] / (ProjectionMatrix[2][2] - 1);
                // Far  = ProjectionMatrix[2][3] / (ProjectionMatrix[2][2] + 1);
                // -----------------------------------------------------------------------------
                float Bottom = Near * (ProjectionMatrix[1][2] - 1.0f) / ProjectionMatrix[1][1];
                float Top    = Near * (ProjectionMatrix[1][2] + 1.0f) / ProjectionMatrix[1][1];
                float Left   = Near * (ProjectionMatrix[0][2] - 1.0f) / ProjectionMatrix[0][0];
                float Right  = Near * (ProjectionMatrix[0][2] + 1.0f) / ProjectionMatrix[0][0];

                Gfx::Cam::SetPerspective(Left, Right, Bottom, Top, Near, Far);
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
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternUpdate()
    {
        static float Test = 0.0f;

        Test += 1.0f;

        Gfx::Cam::SetPosition(Base::Float3::s_Zero);
        Gfx::Cam::SetRotationMatrix(Base::Float3x3().SetRotationX(Base::DegreesToRadians(-90.0f - Test)));

        Gfx::Cam::Update();
    }
} // namespace Cam