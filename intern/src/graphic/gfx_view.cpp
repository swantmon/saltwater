
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_view.h"

namespace Gfx
{
    CView::CView()
        : m_RotationMatrix(1.0f)
        , m_ViewMatrix    (1.0f)
        , m_Position      (0.0f, 0.0f, 0.0f)
        , m_View          (0.0f, 0.0f, 0.0f)
        , m_Right         (0.0f, 0.0f, 0.0f)
        , m_Up            (0.0f, 0.0f, 0.0f)
        , m_pFirstCamera  (nullptr)
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

    void CView::SetPosition(const glm::vec3& _rPosition)
    {
        m_Position = _rPosition;
    }

    // --------------------------------------------------------------------------------

    const glm::vec3& CView::GetPosition() const
    {
        return m_Position;
    }

    // --------------------------------------------------------------------------------

    const glm::vec3& CView::GetViewDirection() const
    {
        return m_View;
    }

    // --------------------------------------------------------------------------------

    const glm::vec3& CView::GetRightDirection() const
    {
        return m_Right;
    }

    // --------------------------------------------------------------------------------

    const glm::vec3& CView::GetUpDirection() const
    {
        return m_Up;
    }

    // --------------------------------------------------------------------------------

    void CView::SetRotationMatrix(const glm::mat3& _rMatrix)
    {
        m_RotationMatrix = _rMatrix;
    }

    // --------------------------------------------------------------------------------

    const glm::mat3& CView::GetRotationMatrix() const
    {
        return m_RotationMatrix;
    }

    // -----------------------------------------------------------------------------

    void CView::SetViewMatrix(const glm::mat4& _rViewMatrix)
    {
        m_RotationMatrix[0] = _rViewMatrix[0];
        m_RotationMatrix[1] = _rViewMatrix[1];
        m_RotationMatrix[2] = _rViewMatrix[2];

        m_Position = _rViewMatrix[3];

        m_Position = glm::transpose(m_RotationMatrix) * m_Position;

        m_Position = m_Position * glm::vec3(-1.0f);
    }

    // --------------------------------------------------------------------------------

    const glm::mat4& CView::GetViewMatrix() const
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
        m_View  =  m_RotationMatrix[2] * glm::vec3(-1.0f);

        // --------------------------- -----------------------------------------------------
        // Get the right handed view space coordinate system.
        // --------------------------------------------------------------------------------
        glm::vec3 XAxis(m_RotationMatrix[0]);
        glm::vec3 YAxis(m_RotationMatrix[1]);
        glm::vec3 ZAxis(m_RotationMatrix[2]);

        // --------------------------------------------------------------------------------
        // Create the view matrix.
        // --------------------------------------------------------------------------------
        m_ViewMatrix[0][0] = XAxis[0]; m_ViewMatrix[1][0] = XAxis[1]; m_ViewMatrix[2][0] = XAxis[2]; m_ViewMatrix[3][0] = -glm::dot(XAxis, m_Position);
        m_ViewMatrix[0][1] = YAxis[0]; m_ViewMatrix[1][1] = YAxis[1]; m_ViewMatrix[2][1] = YAxis[2]; m_ViewMatrix[3][1] = -glm::dot(YAxis, m_Position);
        m_ViewMatrix[0][2] = ZAxis[0]; m_ViewMatrix[1][2] = ZAxis[1]; m_ViewMatrix[2][2] = ZAxis[2]; m_ViewMatrix[3][2] = -glm::dot(ZAxis, m_Position);
        m_ViewMatrix[0][3] = 0.0f    ; m_ViewMatrix[1][3] = 0.0f    ; m_ViewMatrix[2][3] = 0.0f    ; m_ViewMatrix[3][3] = 1.0f;
        
        // --------------------------------------------------------------------------------
        // Update all the cameras attached to the current view.
        // --------------------------------------------------------------------------------
        for (pCamera = m_pFirstCamera; pCamera != nullptr; pCamera = pCamera->m_pSibling)
        {
            pCamera->Update();
        }
    }
} // namespace Gfx
