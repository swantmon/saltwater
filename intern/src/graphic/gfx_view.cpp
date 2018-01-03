
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_view.h"

namespace Gfx
{
    CView::CView()
        : m_RotationMatrix          (Base::Float3x3::s_Identity)
        , m_ViewMatrix              (Base::Float4x4::s_Identity)
        , m_Position                (0.0f, 0.0f, 0.0f)
        , m_View                    (0.0f, 0.0f, 0.0f)
        , m_Right                   (0.0f, 0.0f, 0.0f)
        , m_Up                      (0.0f, 0.0f, 0.0f)
        , m_pFirstCamera            (nullptr)
    {
    }

    // --------------------------------------------------------------------------------

    CView::~CView()
    {
    }

    // --------------------------------------------------------------------------------

    void CView::SetPosition(float _AxisX, float _AxisY, float _AxisZ)
    {
        m_Position[0] = _AxisX;
        m_Position[1] = _AxisY;
        m_Position[2] = _AxisZ;
    }

    // --------------------------------------------------------------------------------

    void CView::SetPosition(const Base::Float3& _rPosition)
    {
        m_Position = _rPosition;
    }

    // --------------------------------------------------------------------------------

    const Base::Float3& CView::GetPosition() const
    {
        return m_Position;
    }

    // --------------------------------------------------------------------------------

    const Base::Float3& CView::GetViewDirection() const
    {
        return m_View;
    }

    // --------------------------------------------------------------------------------

    const Base::Float3& CView::GetRightDirection() const
    {
        return m_Right;
    }

    // --------------------------------------------------------------------------------

    const Base::Float3& CView::GetUpDirection() const
    {
        return m_Up;
    }

    // --------------------------------------------------------------------------------

    void CView::SetRotationMatrix()
    {
        m_RotationMatrix.SetIdentity();
    }

    // --------------------------------------------------------------------------------

    void CView::SetRotationMatrix(const Base::Float3x3& _rMatrix)
    {
        m_RotationMatrix = _rMatrix;
    }

    // --------------------------------------------------------------------------------

    const Base::Float3x3& CView::GetRotationMatrix() const
    {
        return m_RotationMatrix;
    }

    // -----------------------------------------------------------------------------

    void CView::SetViewMatrix(const Base::Float4x4& _rViewMatrix)
    {
        _rViewMatrix.GetRotation(m_RotationMatrix);

        _rViewMatrix.GetTranslation(m_Position);
    }

    // --------------------------------------------------------------------------------

    const Base::Float4x4& CView::GetViewMatrix() const
    {
        return m_ViewMatrix;
    }

    // --------------------------------------------------------------------------------

    void CView::Update()
    {
        CCamera* pCamera;

        // --------------------------------------------------------------------------------
        // Extract the axes of the camera out of the rotation matrix.
        // --------------------------------------------------------------------------------
        m_Right =  m_RotationMatrix[0];
        m_Up    =  m_RotationMatrix[1];
        m_View  =  m_RotationMatrix[2] * Base::Float3(-1.0f);

        // --------------------------- -----------------------------------------------------
        // Get the right handed view space coordinate system.
        // --------------------------------------------------------------------------------
        Base::Float3 XAxis(m_RotationMatrix[0]);
        Base::Float3 YAxis(m_RotationMatrix[1]);
        Base::Float3 ZAxis(m_RotationMatrix[2]);

        // --------------------------------------------------------------------------------
        // Create the view matrix.
        // --------------------------------------------------------------------------------
        m_ViewMatrix[0][0] = XAxis[0]; m_ViewMatrix[0][1] = XAxis[1]; m_ViewMatrix[0][2] = XAxis[2]; m_ViewMatrix[0][3] = -(XAxis.DotProduct(m_Position));
        m_ViewMatrix[1][0] = YAxis[0]; m_ViewMatrix[1][1] = YAxis[1]; m_ViewMatrix[1][2] = YAxis[2]; m_ViewMatrix[1][3] = -(YAxis.DotProduct(m_Position));
        m_ViewMatrix[2][0] = ZAxis[0]; m_ViewMatrix[2][1] = ZAxis[1]; m_ViewMatrix[2][2] = ZAxis[2]; m_ViewMatrix[2][3] = -(ZAxis.DotProduct(m_Position));
        m_ViewMatrix[3][0] = 0.0f    ; m_ViewMatrix[3][1] = 0.0f    ; m_ViewMatrix[3][2] = 0.0f    ; m_ViewMatrix[3][3] = 1.0f;
        
        // --------------------------------------------------------------------------------
        // Update all the cameras attached to the current view.
        // --------------------------------------------------------------------------------
        for (pCamera = m_pFirstCamera; pCamera != nullptr; pCamera = pCamera->m_pSibling)
        {
            pCamera->Update();
        }
    }
} // namespace Gfx
