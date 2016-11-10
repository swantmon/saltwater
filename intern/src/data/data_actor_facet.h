//
//  data_actor_facet.h
//  data
//
//  Created by Tobias Schwandt on 23/03/16.
//  Copyright © 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb2.h"
#include "base/base_matrix3x3.h"

#include "data/data_lod.h"

namespace Dt
{
    class CMesh;
    class CMaterial;
} // namespace Dt

namespace Dt
{
    struct SActorType
    {
        enum Enum
        {
            Node,
            Model,
            AR,
            Camera,
            NumberOfTypes,
            UndefinedType = -1,
        };
    };
} // namespace Dt

namespace Dt
{
    class CModelActorFacet
    {
    public:

        void SetModel(CMesh* _pModel);
        CMesh* GetModel();

        void SetMaterial(unsigned int _Surface, CMaterial* _pMaterial);
        CMaterial* GetMaterial(unsigned int _Surface);

    public:

        CModelActorFacet();
        ~CModelActorFacet();

    private:

        CMesh*     m_pModel;
        CMaterial* m_pMaterial[CLOD::s_NumberOfSurfaces];
    };
} // namespace Dt

namespace Dt
{
    class CARActorFacet
    {
    public:

        void SetModel(CMesh* _pModel);
        CMesh* GetModel();

        void SetMaterial(unsigned int _Surface, CMaterial* _pMaterial);
        CMaterial* GetMaterial(unsigned int _Surface);

    public:

        CARActorFacet();
        ~CARActorFacet();

    private:

        CMesh*     m_pModel;
        CMaterial* m_pMaterial[CLOD::s_NumberOfSurfaces];
    };
} // namespace Dt

namespace Dt
{
    class CCameraActorFacet
    {
    public:

        enum EClearFlag
        {
            Skybox,
            SolidColor,
            DepthOnly,
            DontClear,
        };

        enum EProjectionType
        {
            Perspective,
            Orthographic,
            RAW,
        };

        enum ECameraMode
        {
            Auto,
            Manual,
        };

    public:

        void SetMainCamera(bool _Flag);
        bool IsMainCamera() const;

        void SetClearFlag(EClearFlag _ClearFlag);
        EClearFlag GetClearFlag() const;

        void SetBackgroundColor(Base::Float3& _rBackgroundColor);
        Base::Float3& GetBackgroundColor();
        const Base::Float3& GetBackgroundColor() const;

        void SetCullingMask(unsigned int _CullingMask);
        unsigned int GetCullingMask() const;

        void SetProjectionType(EProjectionType  _ProjectionType);
        EProjectionType GetProjectionType() const;

        void SetSize(float _Size);
        float GetSize() const;

        void SetFoV(float _FoV);
        float GetFoV() const;

        void SetProjectionMatrix(const Base::Float3x3& _rProjection);
        Base::Float3x3& GetProjectionMatrix();
        const Base::Float3x3& GetProjectionMatrix() const;

        void SetNear(float _Near);
        float GetNear() const;

        void SetFar(float _Far);
        float GetFar() const;

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

        CCameraActorFacet();
        ~CCameraActorFacet();

    private:

        bool             m_IsMainCamera;                    //< Only one main camera can exist at once; first camera that is found will be used
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
        Base::Float3     m_BackgroundColor;                 //< Default background color of the camera (depending on clear flag)
        Base::Float3x3   m_ProjectionMatrix;                //< RAW projection matrix even RAW is active
        Base::AABB2Float m_ViewportRect;                    //< View port this camera should render
        EClearFlag       m_ClearFlag;                       //< Clear flag of the render target (@see EClearFlag)
        EProjectionType  m_ProjectionType;                  //< Camera can be orthographic or projection
        ECameraMode      m_CameraMode;                      //< Setup the camera as automatic or manual behavior for light sensitivity (default is auto with sunny16 setup)
    };
} // namespace Dt