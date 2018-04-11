
#pragma once

#include "base/base_aabb2.h"
#include "base/base_aabb3.h"
#include "base/base_include_glm.h"
#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_view.h"

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

        struct SFace
        {
            enum Enum
            {
                Near = 0,
                Far = 4,
                Left = 0,
                Right = 2,
                Bottom = 0,
                Top = 1
            };
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

        void SetProjectionMatrix(const glm::mat4& _rProjectionMatrix, float _Near, float _Far);

    public:

        void SetAspectRatio(float _Aspect);
        float GetAspectRatio();

        void SetBackgroundColor(glm::vec3& _rBackgroundColor);
        glm::vec3& GetBackgroundColor();
        const glm::vec3& GetBackgroundColor() const;

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

        float GetFOVY() const;

        const glm::mat4& GetProjectionMatrix() const;
        const glm::mat4& GetViewProjectionMatrix() const;

        const glm::vec3* GetWorldSpaceFrustum() const;

        const Base::AABB3Float& GetWorldAABB() const;

    public:

        void Update();

    public:

        void SetBackgroundTexture2D(CTexturePtr _Texture2DPtr);
        CTexturePtr GetBackgroundTexture2D();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    protected:

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewProjectionMatrix;

        unsigned int m_CullingMask;
        float        m_Depth;
        float        m_Left;
        float        m_Right;
        float        m_Bottom;
        float        m_Top;
        float        m_Near;
        float        m_Far;
        float        m_Radius;
        float        m_ShutterSpeed;
        float        m_Aperture;
        float        m_ISO;
        float        m_EC;
        float        m_Size;

        ECameraMode m_Mode;

        glm::vec3 m_BackgroundColor;
        glm::vec3 m_ObjectSpaceFrustum[8];
        glm::vec3 m_WorldSpaceFrustum [8];

        Base::AABB3Float m_WorldAABB;
        Base::AABB2Float m_ViewportRect;

        CCamera* m_pSibling;

        CViewPtr m_ViewPtr;

        CTexturePtr m_BackgroundTexture2DPtr;
        Base::U64   m_TimeStamp;

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
