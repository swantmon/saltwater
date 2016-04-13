
#pragma once

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

        void SetFieldOfView(float _FOVY, float _Aspect, float _Near, float _Far);
        void SetPerspective(float _Width, float _Height, float _Near, float _Far);
        void SetPerspective(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
        void SetOrthographic(float _Width, float _Height, float _Near, float _Far);
        void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
        
        void InjectCameraMatrix(const Base::Float3x3& _rCameraMatrix);

    public:

        CViewPtr GetView();

    public:

        float GetWidth() const;
        float GetHeight() const;

        float GetLeft() const;
        float GetRight() const;

        float GetTop() const;
        float GetBottom() const;

        float GetNear() const;
        float GetFar() const;

        float GetRadius() const;

        ECameraMode GetMode() const;
        float GetShuttertime() const;
        float GetAperture() const;
        float GetISO() const;
        float GetExposureCompensation() const;

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

        ECameraMode      m_Mode;

        Base::Float3     m_ObjectSpaceFrustum[8];
        Base::Float3     m_WorldSpaceFrustum [8];

        Base::AABB3Float m_WorldAABB;

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
