
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_render_state.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_view_manager.h"

namespace Gfx
{
namespace Cam
{
    void SetFieldOfView(float _FOVY, float _Near, float _Far)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        Base::Int2 ScreenSize = Main::GetActiveWindowSize();

        float Width  = static_cast<float>(ScreenSize[0]);
        float Height = static_cast<float>(ScreenSize[1]);

        float AspectRatio = Width / Height;

        return rCamera.SetFieldOfView(_FOVY, AspectRatio, _Near, _Far);
    }

    // -----------------------------------------------------------------------------

    void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.SetOrthographic(_Left, _Right, _Bottom, _Top, _Near, _Far);
    }

    // -----------------------------------------------------------------------------

    void SetProjection(const Base::Float4x4& _rProjectionMatrix)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.SetProjection(_rProjectionMatrix);
    }

    // -----------------------------------------------------------------------------

    void SetBackgroundColor(Base::Float3& _rBackgroundColor)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.SetBackgroundColor(_rBackgroundColor);
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& GetBackgroundColor()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetBackgroundColor();
    }

    // -----------------------------------------------------------------------------

    void SetCullingMask(unsigned int _CullingMask)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetCullingMask(_CullingMask);
    }

    // -----------------------------------------------------------------------------

    unsigned int GetCullingMask()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetCullingMask();
    }

    // -----------------------------------------------------------------------------

    void SetViewportRect(Base::AABB2Float& _rViewportRect)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetViewportRect(_rViewportRect);
    }

    // -----------------------------------------------------------------------------

    const Base::AABB2Float& GetViewportRect()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetViewportRect();
    }

    // -----------------------------------------------------------------------------

    void SetDepth(float _Depth)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetDepth(_Depth);
    }

    // -----------------------------------------------------------------------------

    float GetDepth()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetDepth();
    }

    // -----------------------------------------------------------------------------

    void SetAutoCameraMode()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetCameraMode(Gfx::CCamera::Auto);
    }

    // -----------------------------------------------------------------------------

    void SetManualCameraMode()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetCameraMode(Gfx::CCamera::Manual);
    }

    // -----------------------------------------------------------------------------

    void SetShutterSpeed(float _ShutterSpeed)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetShutterSpeed(_ShutterSpeed);
    }

    // -----------------------------------------------------------------------------

    float GetShutterSpeed()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetShutterSpeed();
    }

    // -----------------------------------------------------------------------------

    void SetAperture(float _Aperture)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetAperture(_Aperture);
    }

    // -----------------------------------------------------------------------------

    float GetAperture()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetAperture();
    }

    // -----------------------------------------------------------------------------

    void SetISO(float _ISO)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetISO(_ISO);
    }

    // -----------------------------------------------------------------------------

    float GetISO()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetISO();
    }

    // -----------------------------------------------------------------------------

    void SetEC(float _EC)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetEC(_EC);
    }

    // -----------------------------------------------------------------------------

    float GetEC()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetEC();
    }

    // -----------------------------------------------------------------------------

    void SetRotationMatrix()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetRotationMatrix();
    }

    // -----------------------------------------------------------------------------

    void SetRotationMatrix(const Base::Float3x3& _rMatrix)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetRotationMatrix(_rMatrix);
    }

    // -----------------------------------------------------------------------------

    const Base::Float3x3& GetRotationMatrix()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        return rView.GetRotationMatrix();
    }

    // -----------------------------------------------------------------------------

    void SetPosition(float _X, float _Y, float _Z)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetPosition(_X, _Y, _Z);
    }

    // -----------------------------------------------------------------------------

    void SetPosition(const Base::Float3& _rPosition)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetPosition(_rPosition);
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& GetPosition()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        return rView.GetPosition();
    }

    // -----------------------------------------------------------------------------

    void SetViewMatrix(const Base::Float4x4& _rViewMatrix)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetViewMatrix(_rViewMatrix);
    }

    // -----------------------------------------------------------------------------

    const Base::Float4x4& GetViewMatrix()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        return rView.GetViewMatrix();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.Update();
    }
} // namespace Cam
} // namespace Gfx