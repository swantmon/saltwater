
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_camera.h"

namespace Gfx
{
    CCamera::CCamera()
        : m_ProjectionMatrix      ()
        , m_ViewProjectionMatrix  ()
        , m_CullingMask           (0)
        , m_Depth                 (0)
        , m_Left                  (0.0f)
        , m_Right                 (0.0f)
        , m_Bottom                (0.0f)
        , m_Top                   (0.0f)
        , m_Near                  (0.0f)
        , m_Far                   (0.0f)
        , m_Radius                (0.0f)
        , m_Mode                  (Auto)
        , m_ShutterSpeed          (0.0f)
        , m_Aperture              (0.0f)
        , m_ISO                   (0.0f)
        , m_EC                    (0.0f)
        , m_Size                  (0.0f)
        , m_BackgroundColor       (0.0f)
        , m_WorldAABB             ()
        , m_ViewportRect          ()
        , m_pSibling              (nullptr)
        , m_ViewPtr               (nullptr)
        , m_BackgroundTexture2DPtr(0)
        , m_TimeStamp             (static_cast<Base::U64>(-1))
    {
    }

    // --------------------------------------------------------------------------------

    CCamera::~CCamera()
    {
        m_BackgroundTexture2DPtr = 0;
    }

    // --------------------------------------------------------------------------------

    void CCamera::SetFieldOfView(float _FOVY, float _Aspect, float _Near, float _Far)
    {
        float Left;
        float Right;
        float Bottom;
        float Top;

        Bottom = -glm::tan(glm::radians(_FOVY) / 2.0f) * glm::max(_Near, 0.000001f);
        Top    = -Bottom;
        Left   =  _Aspect * Bottom;
        Right  =  _Aspect * Top;

        SetPerspective(Left, Right, Bottom, Top, _Near, _Far);
    }

    // --------------------------------------------------------------------------------

    void CCamera::SetPerspective(float _Width, float _Height, float _Near, float _Far)
    {
        float HalfWidth;
        float HalfHeight;

        HalfWidth  = _Width  / 2.0f;
        HalfHeight = _Height / 2.0f;

        SetPerspective(-HalfWidth, HalfWidth, -HalfHeight, HalfHeight, _Near, _Far);
    }

    // --------------------------------------------------------------------------------

    void CCamera::SetPerspective(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far)
    {
        float Scale;

        // --------------------------------------------------------------------------------
        // Save the dimensions.
        // --------------------------------------------------------------------------------
        m_Left   = _Left;
		m_Right  = _Right;
		m_Bottom = _Bottom;
        m_Top    = _Top;
        m_Near   = _Near;
        m_Far    = _Far;

        // --------------------------------------------------------------------------------
        // Compute the view frustum without the eye point in object space coordinates. By
        // default the view directions is along the negative-z-axis.       
        // 
        //      z |
        //        |    / y up
        //        |   / 
        //        |  /
        //        | /
        //        |/_________ 
        //        |          
        //        |         x right
        //        |
        //        |
        //   view |
        //        
        // --------------------------------------------------------------------------------
        Scale = _Far / _Near;

        m_ObjectSpaceFrustum[SFace::Near | SFace::Left  | SFace::Bottom] = glm::vec3(_Left         , _Bottom        , -_Near);
        m_ObjectSpaceFrustum[SFace::Near | SFace::Left  | SFace::Top   ] = glm::vec3(_Left         , _Top           , -_Near);
        m_ObjectSpaceFrustum[SFace::Near | SFace::Right | SFace::Bottom] = glm::vec3(_Right        , _Bottom        , -_Near);
        m_ObjectSpaceFrustum[SFace::Near | SFace::Right | SFace::Top   ] = glm::vec3(_Right        , _Top           , -_Near);
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Bottom] = glm::vec3(_Left  * Scale, _Bottom * Scale, -_Far );
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Top   ] = glm::vec3(_Left  * Scale, _Top    * Scale, -_Far );
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Right | SFace::Bottom] = glm::vec3(_Right * Scale, _Bottom * Scale, -_Far );
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Right | SFace::Top   ] = glm::vec3(_Right * Scale, _Top    * Scale, -_Far );

        // --------------------------------------------------------------------------------
        // Get the distance from the eye point to the furthest point of the camera.
        // --------------------------------------------------------------------------------
        m_Radius = static_cast<float>(m_ObjectSpaceFrustum[SFace::Far | SFace::Right | SFace::Top].length());

        // --------------------------------------------------------------------------------
        // Compute the projection matrix.
        // --------------------------------------------------------------------------------
        m_ProjectionMatrix = glm::frustumRH(_Left, _Right, _Bottom, _Top, _Near, _Far);
    }
    
    // --------------------------------------------------------------------------------

    void CCamera::SetOrthographic(float _Width, float _Height, float _Near, float _Far)
    {
        float HalfWidth;
        float HalfHeight;

        HalfWidth  = _Width  / 2.0f;
        HalfHeight = _Height / 2.0f;

        SetOrthographic(-HalfWidth, HalfWidth, -HalfHeight, HalfHeight, _Near, _Far);
    }

    // --------------------------------------------------------------------------------

    void CCamera::SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far)
    {
        // --------------------------------------------------------------------------------
        // Save the dimensions.
        // --------------------------------------------------------------------------------
        m_Left   = _Left;		
		m_Right  = _Right;
		m_Bottom = _Bottom;
        m_Top    = _Top;
        m_Near   = _Near;
        m_Far    = _Far;

        // --------------------------------------------------------------------------------
        // Compute the cubic view frustum in object space coordinates.
        // --------------------------------------------------------------------------------
        m_ObjectSpaceFrustum[SFace::Near | SFace::Left  | SFace::Bottom] = glm::vec3(_Left , _Bottom, -_Near);
        m_ObjectSpaceFrustum[SFace::Near | SFace::Left  | SFace::Top   ] = glm::vec3(_Left , _Top   , -_Near);
        m_ObjectSpaceFrustum[SFace::Near | SFace::Right | SFace::Bottom] = glm::vec3(_Right, _Bottom, -_Near);
        m_ObjectSpaceFrustum[SFace::Near | SFace::Right | SFace::Top   ] = glm::vec3(_Right, _Top   , -_Near);
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Bottom] = glm::vec3(_Left , _Bottom, -_Far );
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Top   ] = glm::vec3(_Left , _Top   , -_Far );
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Right | SFace::Bottom] = glm::vec3(_Right, _Bottom, -_Far );
        m_ObjectSpaceFrustum[SFace::Far  | SFace::Right | SFace::Top   ] = glm::vec3(_Right, _Top   , -_Far );

        // --------------------------------------------------------------------------------
        // Compute the projection matrix.
        // --------------------------------------------------------------------------------
        m_ProjectionMatrix = glm::orthoRH(_Left, _Right, _Bottom, _Top, _Near, _Far);
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetAspectRatio(float _Aspect)
    {
        float Left;
        float Right;

        Left  = _Aspect * m_Bottom;
        Right = _Aspect * m_Top;

        SetPerspective(Left, Right, m_Bottom, m_Top, m_Near, m_Far);
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetAspectRatio()
    {
        return m_Left / m_Bottom;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetBackgroundColor(glm::vec3& _rBackgroundColor)
    {
        m_BackgroundColor = _rBackgroundColor;
    }

    // -----------------------------------------------------------------------------
    glm::vec3& CCamera::GetBackgroundColor()
    {
        return m_BackgroundColor;
    }

    // -----------------------------------------------------------------------------
    const glm::vec3& CCamera::GetBackgroundColor() const
    {
        return m_BackgroundColor;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetCullingMask(unsigned int _CullingMask)
    {
        m_CullingMask = _CullingMask;
    }

    // -----------------------------------------------------------------------------
    unsigned int CCamera::GetCullingMask() const
    {
        return m_CullingMask;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetViewportRect(Base::AABB2Float& _rViewportRect)
    {
        m_ViewportRect = _rViewportRect;
    }

    // -----------------------------------------------------------------------------

    Base::AABB2Float& CCamera::GetViewportRect()
    {
        return m_ViewportRect;
    }

    // -----------------------------------------------------------------------------

    const Base::AABB2Float& CCamera::GetViewportRect() const
    {
        return m_ViewportRect;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetDepth(float _Depth)
    {
        m_Depth = _Depth;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetDepth() const
    {
        return m_Depth;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetCameraMode(ECameraMode _CameraMode)
    {
        m_Mode = _CameraMode;
    }

    // -----------------------------------------------------------------------------

    CCamera::ECameraMode CCamera::GetCameraMode() const
    {
        return m_Mode;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetShutterSpeed(float _ShutterSpeed)
    {
        m_ShutterSpeed = _ShutterSpeed;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetShutterSpeed() const
    {
        return m_ShutterSpeed;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetAperture(float _Aperture)
    {
        m_Aperture = _Aperture;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetAperture() const
    {
        return m_Aperture;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetISO(float _ISO)
    {
        m_ISO = _ISO;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetISO() const
    {
        return m_ISO;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetEC(float _EC)
    {
        m_EC = _EC;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetEC() const
    {
        return m_EC;
    }

    // -----------------------------------------------------------------------------

    CViewPtr CCamera::GetView()
    {
        return m_ViewPtr;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetWidth() const
    {
        return m_Left - m_Right;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetHeight() const
    {
        return m_Top - m_Bottom;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetLeft() const
    {
        return m_Left;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetRight() const
    {
        return m_Right;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetTop() const
    {
        return m_Top;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetBottom() const
    {
        return m_Bottom;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetRadius() const
    {
        return m_Radius;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetNear() const
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetFar() const
    {
        return m_Far;
    }

    // -----------------------------------------------------------------------------

    float CCamera::GetFOVY() const
    {
        return glm::atan((m_Top - m_Bottom) / m_Near / 2.0f) * 2.0f;
    }

    // --------------------------------------------------------------------------------

    const glm::mat4& CCamera::GetProjectionMatrix() const
    {
        return m_ProjectionMatrix;
    }

    // --------------------------------------------------------------------------------

    const glm::mat4& CCamera::GetViewProjectionMatrix() const
    {
        return m_ViewProjectionMatrix;
    }

    // --------------------------------------------------------------------------------

    const glm::vec3* CCamera::GetWorldSpaceFrustum() const
    {
        return m_WorldSpaceFrustum;
    }

    // --------------------------------------------------------------------------------

    const Base::AABB3Float& CCamera::GetWorldAABB() const
    {
        return m_WorldAABB;
    }

    // --------------------------------------------------------------------------------

    void CCamera::UpdateFrustum()
    {
        // --------------------------------------------------------------------------------
        // Compute the model of the camera in world space.
        // --------------------------------------------------------------------------------
        m_WorldSpaceFrustum[SFace::Near | SFace::Left  | SFace::Bottom] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Near | SFace::Left  | SFace::Bottom];
        m_WorldSpaceFrustum[SFace::Near | SFace::Left  | SFace::Top   ] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Near | SFace::Left  | SFace::Top   ];
        m_WorldSpaceFrustum[SFace::Near | SFace::Right | SFace::Bottom] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Near | SFace::Right | SFace::Bottom];
        m_WorldSpaceFrustum[SFace::Near | SFace::Right | SFace::Top   ] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Near | SFace::Right | SFace::Top   ];

        m_WorldSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Bottom] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Bottom];
        m_WorldSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Top   ] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Far  | SFace::Left  | SFace::Top   ];
        m_WorldSpaceFrustum[SFace::Far  | SFace::Right | SFace::Bottom] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Far  | SFace::Right | SFace::Bottom];
        m_WorldSpaceFrustum[SFace::Far  | SFace::Right | SFace::Top   ] = m_ViewPtr->m_Position + m_ViewPtr->m_RotationMatrix * m_ObjectSpaceFrustum[SFace::Far  | SFace::Right | SFace::Top   ];
    }

	// --------------------------------------------------------------------------------

    void CCamera::Update()
    {
        // --------------------------------------------------------------------------------
        // Get the coordinates of the frustum vertices's in world space.
        // --------------------------------------------------------------------------------
        UpdateFrustum();

        // --------------------------------------------------------------------------------
        // Compute the product of view and projection matrix.
        // --------------------------------------------------------------------------------
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewPtr->m_ViewMatrix;

		// --------------------------------------------------------------------------------
		// Determine the lower and higher extrema of camera's world position
		// --------------------------------------------------------------------------------
		glm::vec3& rMin = m_WorldAABB.GetMin();
        glm::vec3& rMax = m_WorldAABB.GetMax();

        rMin = m_WorldSpaceFrustum[0];
        rMax = m_WorldSpaceFrustum[0];

		for (int IndexOfVertex = 1; IndexOfVertex < 8; ++ IndexOfVertex)
		{
			if (m_WorldSpaceFrustum[IndexOfVertex][0] < rMin[0])
			{
                rMin[0] = m_WorldSpaceFrustum[IndexOfVertex][0];
			}
            else if (m_WorldSpaceFrustum[IndexOfVertex][0] > rMax[0])
            {
                rMax[0] = m_WorldSpaceFrustum[IndexOfVertex][0];
            }

            if (m_WorldSpaceFrustum[IndexOfVertex][1] < rMin[1])
            {
                rMin[1] = m_WorldSpaceFrustum[IndexOfVertex][1];
            }
            else if (m_WorldSpaceFrustum[IndexOfVertex][1] > rMax[1])
            {
                rMax[1] = m_WorldSpaceFrustum[IndexOfVertex][1];
            }

            if (m_WorldSpaceFrustum[IndexOfVertex][2] < rMin[2])
            {
                rMin[2] = m_WorldSpaceFrustum[IndexOfVertex][2];
            }
            else if (m_WorldSpaceFrustum[IndexOfVertex][2] > rMax[2])
            {
                rMax[2] = m_WorldSpaceFrustum[IndexOfVertex][2];
            }
		}
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetBackgroundTexture2D(CTexturePtr _Texture2DPtr)
    {
        m_BackgroundTexture2DPtr = _Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CCamera::GetBackgroundTexture2D()
    {
        return m_BackgroundTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CCamera::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CCamera::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx
