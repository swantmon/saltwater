
#include "graphic/gfx_camera.h"
#include "graphic/gfx_render_state.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_view_manager.h"

namespace Gfx
{
namespace Cam
{
    void SetPosition(const Base::Float3& _rPosition)
    {
        CCamera&      rCamera      = *ViewManager::GetMainCamera();
        CView&        rView        = *rCamera.GetView();

        rView.SetPosition(_rPosition);
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& GetPosition()
    {
        CCamera&      rCamera      = *ViewManager::GetMainCamera();
        CView&        rView        = *rCamera.GetView();

        return rView.GetPosition();
    }

    // -----------------------------------------------------------------------------

    void SetRotation(const Base::Float3x3& _rMatrix)
    {
        CCamera&      rCamera      = *ViewManager::GetMainCamera();
        CView&        rView        = *rCamera.GetView();

        rView.SetRotationMatrix(_rMatrix);
    }

    // -----------------------------------------------------------------------------

    const Base::Float3x3& GetRotation()
    {
        CCamera&      rCamera      = *ViewManager::GetMainCamera();
        CView&        rView        = *rCamera.GetView();

        return rView.GetRotationMatrix();
    }
    
    // -----------------------------------------------------------------------------
    
    void InjectCameraMatrix(const Base::Float3x3& _rCameraMatrix)
    {
        CCamera& rCamera = *ViewManager::GetMainCamera();
        
        rCamera.InjectCameraMatrix(_rCameraMatrix);
        
        rCamera.Update();
    }
    
    // -----------------------------------------------------------------------------

    void Update()
    {
        CCamera&      rCamera      = *ViewManager::GetMainCamera();
        CView&        rView        = *rCamera.GetView();

        rView.Update();
    }
} // namespace Cam
} // namespace Gfx