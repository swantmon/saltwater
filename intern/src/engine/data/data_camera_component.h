
#pragma once

#include "engine/engine_config.h"

#include "base/base_aabb2.h"
#include "base/base_include_glm.h"

#include "engine/data/data_component.h"

#include "engine/graphic/gfx_texture.h"

namespace Dt
{
    class CTexture2D;
} // namespace Dt

namespace Dt
{
    class ENGINE_API CCameraComponent : public CComponent<CCameraComponent>
    {
    public:

        enum EClearFlag
        {
            Skybox,
            Texture,
            SolidColor,
            DepthOnly,
            DontClear,
        };

        enum EProjectionType
        {
            Perspective,
            Orthographic,
            External,
        };

        enum ECameraMode
        {
            Auto,
            Manual,
        };

    public:

        void SetClearFlag(EClearFlag _ClearFlag);
        EClearFlag GetClearFlag() const;

        void SetBackgroundColor(const glm::vec3& _rBackgroundColor);
        glm::vec3& GetBackgroundColor();
        const glm::vec3& GetBackgroundColor() const;

        void SetBackgroundTexture(Gfx::CTexturePtr _BackgroundTexturePtr);
        Gfx::CTexturePtr GetBackgroundTexture();

        void SetCullingMask(unsigned int _CullingMask);
        unsigned int GetCullingMask() const;

        void SetProjectionType(EProjectionType  _ProjectionType);
        EProjectionType GetProjectionType() const;

        void SetSize(float _Size);
        float GetSize() const;

        void SetFoV(float _FoV);
        float GetFoV() const;

        void SetProjectionMatrix(const glm::mat4& _rProjection);
        glm::mat4& GetProjectionMatrix();
        const glm::mat4& GetProjectionMatrix() const;

        void SetNear(float _Near);
        float GetNear() const;

        void SetFar(float _Far);
        float GetFar() const;

        void SetViewportRect(const Base::AABB2Float& _rViewportRect);
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

        CCameraComponent();
        ~CCameraComponent();

    private:

        unsigned int     m_CullingMask;                     //< Culling camera against some objects in map (0 = everything)
        float            m_Depth;                           //< Depth of the depth buffer (maybe needed for some effects)
        float            m_ShutterSpeed;                    //< Shutter speed of the camera in seconds
        float            m_Aperture;                        //< Aperture of the camera
        float            m_ISO;                             //< ISO of the camera
        float            m_EC;                              //< Exposure compensation of the camera (value between -5 and 5)
        float            m_Size;                            //< Size of the camera if orthographic is active
        float            m_FoV;                             //< Field of view even projection is active
        float            m_Near;                            //< Near field of the camera
        float            m_Far;                             //< Far field of the camera
        Gfx::CTexturePtr m_pBackgroundTexture;              //< Background texture
        glm::vec3        m_BackgroundColor;                 //< Default background color of the camera (depending on clear flag)
        glm::mat4        m_ProjectionMatrix;                //< RAW projection matrix even RAW is active
        Base::AABB2Float m_ViewportRect;                    //< View port this camera should render
        EClearFlag       m_ClearFlag;                       //< Clear flag of the render target (@see EClearFlag)
        EProjectionType  m_ProjectionType;                  //< Camera can be orthographic or projection
        ECameraMode      m_CameraMode;                      //< Setup the camera as automatic or manual behavior for light sensitivity (default is auto with sunny16 setup)
    };
} // namespace Dt