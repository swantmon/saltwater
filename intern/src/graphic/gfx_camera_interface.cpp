
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_render_state.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_view_manager.h"

#include <assert.h>

namespace Gfx
{
namespace Cam
{
    void SetFieldOfView(float _FOVY, float _Near, float _Far)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        glm::ivec2 ScreenSize = Main::GetActiveWindowSize();

        assert(ScreenSize[0] > 0 && ScreenSize[1] > 0);

        float Width  = static_cast<float>(ScreenSize[0]);
        float Height = static_cast<float>(ScreenSize[1]);

        float AspectRatio = Width / Height;

        return rCamera.SetFieldOfView(_FOVY, AspectRatio, _Near, _Far);
    }

    // -----------------------------------------------------------------------------

    void SetPerspective(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.SetPerspective(_Left, _Right, _Bottom, _Top, _Near, _Far);
    }

    // -----------------------------------------------------------------------------

    void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.SetOrthographic(_Left, _Right, _Bottom, _Top, _Near, _Far);
    }

    // -----------------------------------------------------------------------------

    void SetProjectionMatrix(const glm::mat4& _rProjectionMatrix, float _Near, float _Far)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetProjectionMatrix(_rProjectionMatrix, _Near, _Far);
    }

    // -----------------------------------------------------------------------------

    void SetBackgroundColor(glm::vec3& _rBackgroundColor)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.SetBackgroundColor(_rBackgroundColor);
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& GetBackgroundColor()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetBackgroundColor();
    }

    // -----------------------------------------------------------------------------

    void SetCullingMask(unsigned int _CullingMask)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetCullingMask(_CullingMask);
    }

    // -----------------------------------------------------------------------------

    unsigned int GetCullingMask()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetCullingMask();
    }

    // -----------------------------------------------------------------------------

    void SetViewportRect(Base::AABB2Float& _rViewportRect)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetViewportRect(_rViewportRect);
    }

    // -----------------------------------------------------------------------------

    const Base::AABB2Float& GetViewportRect()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetViewportRect();
    }

    // -----------------------------------------------------------------------------

    void SetDepth(float _Depth)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetDepth(_Depth);
    }

    // -----------------------------------------------------------------------------

    float GetDepth()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetDepth();
    }

    // -----------------------------------------------------------------------------

    void SetAutoCameraMode()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetCameraMode(Gfx::CCamera::Auto);
    }

    // -----------------------------------------------------------------------------

    void SetManualCameraMode()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetCameraMode(Gfx::CCamera::Manual);
    }

    // -----------------------------------------------------------------------------

    void SetShutterSpeed(float _ShutterSpeed)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetShutterSpeed(_ShutterSpeed);
    }

    // -----------------------------------------------------------------------------

    float GetShutterSpeed()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetShutterSpeed();
    }

    // -----------------------------------------------------------------------------

    void SetAperture(float _Aperture)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetAperture(_Aperture);
    }

    // -----------------------------------------------------------------------------

    float GetAperture()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetAperture();
    }

    // -----------------------------------------------------------------------------

    void SetISO(float _ISO)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetISO(_ISO);
    }

    // -----------------------------------------------------------------------------

    float GetISO()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetISO();
    }

    // -----------------------------------------------------------------------------

    void SetEC(float _EC)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        rCamera.SetEC(_EC);
    }

    // -----------------------------------------------------------------------------

    float GetEC()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetEC();
    }

    // -----------------------------------------------------------------------------

    void SetRotationMatrix(const glm::mat3& _rMatrix)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());
        assert(ViewManager::GetMainCamera()->GetView() != nullptr && ViewManager::GetMainCamera()->GetView().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetRotationMatrix(_rMatrix);
    }

    // -----------------------------------------------------------------------------

    const glm::mat3& GetRotationMatrix()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());
        assert(ViewManager::GetMainCamera()->GetView() != nullptr && ViewManager::GetMainCamera()->GetView().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        return rView.GetRotationMatrix();
    }

    // -----------------------------------------------------------------------------

    void SetPosition(float _AxisX, float _AxisY, float _AxisZ)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());
        assert(ViewManager::GetMainCamera()->GetView() != nullptr && ViewManager::GetMainCamera()->GetView().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetPosition(_AxisX, _AxisY, _AxisZ);
    }

    // -----------------------------------------------------------------------------

    void SetPosition(const glm::vec3& _rPosition)
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());
        assert(ViewManager::GetMainCamera()->GetView() != nullptr && ViewManager::GetMainCamera()->GetView().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.SetPosition(_rPosition);
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& GetPosition()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());
        assert(ViewManager::GetMainCamera()->GetView() != nullptr && ViewManager::GetMainCamera()->GetView().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        return rView.GetPosition();
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& GetViewMatrix()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());
        assert(ViewManager::GetMainCamera()->GetView() != nullptr && ViewManager::GetMainCamera()->GetView().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        return rView.GetViewMatrix();
    }

    // -----------------------------------------------------------------------------

    const glm::mat4& GetProjectionMatrix()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();

        return rCamera.GetProjectionMatrix();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
        assert(ViewManager::GetMainCamera() != nullptr && ViewManager::GetMainCamera().IsValid());
        assert(ViewManager::GetMainCamera()->GetView() != nullptr && ViewManager::GetMainCamera()->GetView().IsValid());

        CCamera& rCamera = *ViewManager::GetMainCamera();
        CView&   rView   = *rCamera.GetView();

        rView.Update();
    }
} // namespace Cam
} // namespace Gfx