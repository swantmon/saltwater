
#include "data/data_precompiled.h"

#include "data/data_camera_actor_facet.h"

namespace Dt
{
    CCameraActorFacet::CCameraActorFacet()
        : m_IsMainCamera    (false)
        , m_CullingMask     (0)
        , m_Depth           (-1)
        , m_ShutterSpeed    (1.0f / 125.0f)
        , m_Aperture        (16.0f)
        , m_ISO             (100.0f)
        , m_EC              (0.0f)
        , m_Size            (5.0f)
        , m_FoV             (60.0f)
        , m_Near            (0.3f)
        , m_Far             (1000.0f)
        , m_BackgroundColor (glm::vec3(1.0f))
        , m_pTexture2D      (nullptr)
        , m_ProjectionMatrix(glm::mat3(1.0f))
        , m_ViewportRect    ()
        , m_ClearFlag       (Skybox)
        , m_ProjectionType  (Perspective)
        , m_CameraMode      (Auto)
    {
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet::~CCameraActorFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetMainCamera(bool _Flag)
    {
        m_IsMainCamera = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CCameraActorFacet::IsMainCamera() const
    {
        return m_IsMainCamera;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetClearFlag(EClearFlag _ClearFlag)
    {
        m_ClearFlag = _ClearFlag;
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet::EClearFlag CCameraActorFacet::GetClearFlag() const
    {
        return m_ClearFlag;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetTexture(Dt::CTexture2D* _pTexture2D)
    {
        m_pTexture2D = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CCameraActorFacet::GetTexture()
    {
        return m_pTexture2D;
    }

    // -----------------------------------------------------------------------------

    bool CCameraActorFacet::GetHasTexture() const
    {
        return m_pTexture2D != 0;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetBackgroundColor(glm::vec3& _rBackgroundColor)
    {
        m_BackgroundColor = _rBackgroundColor;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CCameraActorFacet::GetBackgroundColor()
    {
        return m_BackgroundColor;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CCameraActorFacet::GetBackgroundColor() const
    {
        return m_BackgroundColor;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetCullingMask(unsigned int _CullingMask)
    {
        m_CullingMask = _CullingMask;
    }

    // -----------------------------------------------------------------------------

    unsigned int CCameraActorFacet::GetCullingMask() const
    {
        return m_CullingMask;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetProjectionType(EProjectionType  _ProjectionType)
    {
        m_ProjectionType = _ProjectionType;
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet::EProjectionType CCameraActorFacet::GetProjectionType() const
    {
        return m_ProjectionType;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetSize(float _Size)
    {
        m_Size = _Size;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetSize() const
    {
        return m_Size;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetFoV(float _FoV)
    {
        m_FoV = _FoV;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetFoV() const
    {
        return m_FoV;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetProjectionMatrix(const glm::mat3& _rProjection)
    {
        m_ProjectionMatrix = _rProjection;
    }

    // -----------------------------------------------------------------------------

    glm::mat3& CCameraActorFacet::GetProjectionMatrix()
    {
        return m_ProjectionMatrix;
    }

    // -----------------------------------------------------------------------------

    const glm::mat3& CCameraActorFacet::GetProjectionMatrix() const
    {
        return m_ProjectionMatrix;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetNear(float _Near)
    {
        m_Near = _Near;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetNear() const
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetFar(float _Far)
    {
        m_Far = _Far;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetFar() const
    {
        return m_Far;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetViewportRect(const Base::AABB2Float& _rViewportRect)
    {
        m_ViewportRect = _rViewportRect;
    }

    // -----------------------------------------------------------------------------

    Base::AABB2Float& CCameraActorFacet::GetViewportRect()
    {
        return m_ViewportRect;
    }

    // -----------------------------------------------------------------------------

    const Base::AABB2Float& CCameraActorFacet::GetViewportRect() const
    {
        return m_ViewportRect;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetDepth(float _Depth)
    {
        m_Depth = _Depth;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetDepth() const
    {
        return m_Depth;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetCameraMode(ECameraMode _CameraMode)
    {
        m_CameraMode = _CameraMode;
    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet::ECameraMode CCameraActorFacet::GetCameraMode() const
    {
        return m_CameraMode;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetShutterSpeed(float _ShutterSpeed)
    {
        m_ShutterSpeed = _ShutterSpeed;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetShutterSpeed() const
    {
        return m_ShutterSpeed;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetAperture(float _Aperture)
    {
        m_Aperture = _Aperture;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetAperture() const
    {
        return m_Aperture;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetISO(float _ISO)
    {
        m_ISO = _ISO;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetISO() const
    {
        return m_ISO;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetEC(float _EC)
    {
        m_EC = _EC;
    }

    // -----------------------------------------------------------------------------

    float CCameraActorFacet::GetEC() const
    {
        return m_EC;
    }
} // namespace Dt