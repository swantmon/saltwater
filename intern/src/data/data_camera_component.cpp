
#include "data/data_precompiled.h"

#include "data/data_camera_component.h"

#include <string>

namespace Dt
{
    CCameraComponent::CCameraComponent()
        : m_CullingMask     (0)
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
        , m_ProjectionMatrix(glm::mat3(1.0f))
        , m_ViewportRect    ()
        , m_ClearFlag       (Skybox)
        , m_ProjectionType  (Perspective)
        , m_CameraMode      (Auto)
    {
    }

    // -----------------------------------------------------------------------------

    CCameraComponent::~CCameraComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetClearFlag(EClearFlag _ClearFlag)
    {
        m_ClearFlag = _ClearFlag;
    }

    // -----------------------------------------------------------------------------

    CCameraComponent::EClearFlag CCameraComponent::GetClearFlag() const
    {
        return m_ClearFlag;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetBackgroundColor(const glm::vec3& _rBackgroundColor)
    {
        m_BackgroundColor = _rBackgroundColor;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CCameraComponent::GetBackgroundColor()
    {
        return m_BackgroundColor;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CCameraComponent::GetBackgroundColor() const
    {
        return m_BackgroundColor;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetBackgroundTexture(Gfx::CTexturePtr _BackgroundTexturePtr)
    {
        m_pBackgroundTexture = _BackgroundTexturePtr;
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CCameraComponent::GetBackgroundTexture()
    {
        return m_pBackgroundTexture;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetCullingMask(unsigned int _CullingMask)
    {
        m_CullingMask = _CullingMask;
    }

    // -----------------------------------------------------------------------------

    unsigned int CCameraComponent::GetCullingMask() const
    {
        return m_CullingMask;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetProjectionType(EProjectionType  _ProjectionType)
    {
        m_ProjectionType = _ProjectionType;
    }

    // -----------------------------------------------------------------------------

    CCameraComponent::EProjectionType CCameraComponent::GetProjectionType() const
    {
        return m_ProjectionType;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetSize(float _Size)
    {
        m_Size = _Size;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetSize() const
    {
        return m_Size;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetFoV(float _FoV)
    {
        m_FoV = _FoV;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetFoV() const
    {
        return m_FoV;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetProjectionMatrix(const glm::mat4& _rProjection)
    {
        m_ProjectionMatrix = _rProjection;
    }

    // -----------------------------------------------------------------------------

    glm::mat4& CCameraComponent::GetProjectionMatrix()
    {
        return m_ProjectionMatrix;
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& CCameraComponent::GetProjectionMatrix() const
    {
        return m_ProjectionMatrix;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetNear(float _Near)
    {
        m_Near = _Near;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetNear() const
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetFar(float _Far)
    {
        m_Far = _Far;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetFar() const
    {
        return m_Far;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetViewportRect(const Base::AABB2Float& _rViewportRect)
    {
        m_ViewportRect = _rViewportRect;
    }

    // -----------------------------------------------------------------------------

    Base::AABB2Float& CCameraComponent::GetViewportRect()
    {
        return m_ViewportRect;
    }

    // -----------------------------------------------------------------------------

    const Base::AABB2Float& CCameraComponent::GetViewportRect() const
    {
        return m_ViewportRect;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetDepth(float _Depth)
    {
        m_Depth = _Depth;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetDepth() const
    {
        return m_Depth;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetCameraMode(ECameraMode _CameraMode)
    {
        m_CameraMode = _CameraMode;
    }

    // -----------------------------------------------------------------------------

    CCameraComponent::ECameraMode CCameraComponent::GetCameraMode() const
    {
        return m_CameraMode;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetShutterSpeed(float _ShutterSpeed)
    {
        m_ShutterSpeed = _ShutterSpeed;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetShutterSpeed() const
    {
        return m_ShutterSpeed;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetAperture(float _Aperture)
    {
        m_Aperture = _Aperture;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetAperture() const
    {
        return m_Aperture;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetISO(float _ISO)
    {
        m_ISO = _ISO;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetISO() const
    {
        return m_ISO;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetEC(float _EC)
    {
        m_EC = _EC;
    }

    // -----------------------------------------------------------------------------

    float CCameraComponent::GetEC() const
    {
        return m_EC;
    }
} // namespace Dt