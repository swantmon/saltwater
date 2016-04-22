
#pragma once

#include "base/base_aabb2.h"
#include "base/base_aabb3.h"
#include "base/base_managed_pool.h"
#include "base/base_matrix4x4.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include "graphic/gfx_view.h"

namespace Gfx
{
    class CCamera : public Base::CManagedPoolItemBase
    {
    public:

        enum ECameraMode
        {
            Auto,
            Manual,
        };

    public:
        
        CCamera();
        ~CCamera();
        
    public:

        void SetFieldOfView(float _FOVY, float _Near, float _Far);
        void SetFieldOfView(float _FOVY, float _Aspect, float _Near, float _Far);
        void SetPerspective(float _Width, float _Height, float _Near, float _Far);
        void SetPerspective(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
        void SetOrthographic(float _Width, float _Height, float _Near, float _Far);
        void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
        
        void SetProjection(const Base::Float4x4& _rProjectionMatrix);




        // TODO: remove this function because it is only for MR
        void InjectCameraMatrix(const Base::Float3x3& _rCameraMatrix);





    public:

        void SetAspectRatio(float _Aspect);
        float GetAspectRatio();

        void SetBackgroundColor(Base::Float3& _rBackgroundColor);
        Base::Float3& GetBackgroundColor();
        const Base::Float3& GetBackgroundColor() const;

        void SetCullingMask(unsigned int _CullingMask);
        unsigned int GetCullingMask() const;

        void SetViewportRect(Base::AABB2Float& _rViewportRect);
        Base::AABB2Float& GetViewportRect();
        const Base::AABB2Float& GetViewportRect() const;

        void SetDepth(float _Depth);
        float GetDepth() const;

        void SetCameraMode(ECameraMode _CameraMode);
        ECameraMode GetCameraMode() const;

        void SetShutterSpeed(float _ShutterSpeed);
        float GetShutterSpeed() const;

        void SetAperture(float _Aperture);
        float GetAperture() const;

        void SetISO(float _ISO);
        float GetISO() const;

        void SetEC(float _EC);
        float GetEC() const;

    public:

        CViewPtr GetView();

    public:

        float GetWidth() const;
        float GetHeight() const;

        float GetLeft() const;
        float GetRight() const;

        float GetTop() const;
        float GetBottom() const;

        float GetRadius() const;

        float GetNear() const;
        float GetFar() const;

        const Base::Float4x4& GetProjectionMatrix() const;
        const Base::Float4x4& GetViewProjectionMatrix() const;

        const Base::Float3* GetWorldSpaceFrustum() const;

        const Base::AABB3Float& GetWorldAABB() const;

    public:

        void Update();

    public:

        struct SFace
        {
            enum Enum
            {
                Near   = 0,
                Far    = 4,
                Left   = 0,
                Right  = 2,
                Bottom = 0,
                Top    = 1
            };
        };

    protected:

        Base::Float4x4   m_ProjectionMatrix;
        Base::Float4x4   m_ViewProjectionMatrix;

        unsigned int     m_CullingMask;
        float            m_Depth;
        float            m_Left;
        float            m_Right;
        float            m_Bottom;
        float            m_Top;
        float            m_Near;
        float            m_Far;
        float            m_Radius;
        float            m_ShutterSpeed;
        float            m_Aperture;
        float            m_ISO;
        float            m_EC;
        float            m_Size;

        ECameraMode      m_Mode;

        Base::Float3     m_BackgroundColor;
        Base::Float3     m_ObjectSpaceFrustum[8];
        Base::Float3     m_WorldSpaceFrustum [8];

        Base::AABB3Float m_WorldAABB;
        Base::AABB2Float m_ViewportRect;

        CCamera*         m_pSibling;

        CViewPtr         m_ViewPtr;

    protected:

        void UpdateFrustum();

    private:

        friend class CView;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CCamera> CCameraPtr;
} // namespace Gfx
