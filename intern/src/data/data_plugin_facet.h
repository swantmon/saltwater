//
//  data_plugin_facet.h
//  data
//
//  Created by Tobias Schwandt on 22/04/16.
//  Copyright © 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_string.h"
#include "base/base_vector2.h"

namespace Dt
{
    class CEntity;
    class CTexture2D;
    class CTextureCube;
} // namespace Dt

namespace Dt
{
    struct SPluginType
    {
        enum Enum
        {
            ARControlManager,
            ARTrackedObject,
            NumberOfTypes,
            UndefinedType = -1,
        };
    };
} // namespace Dt

namespace Dt
{
    class CARControllerPluginFacet
    {
    public:

        static const unsigned int s_MaxNumberOfMarker = 8;

    public:

        enum EType
        {
            Kinect,
            Webcam,
            NumberOfControls,
            Undefined = -1,
        };

    public:

        struct SMarker
        {
            enum EMarkerType
            {
                Square,
                SquareBarcode,
                Multimarker,
                NFT,
                NumberOfMarkerTypes,
                UndefinedMarker = -1
            };

            unsigned int     m_UID;
            EMarkerType      m_Type;
            Base::CharString m_PatternFile;
            float            m_WidthInMeter;
        };

    public:

        void SetCameraEntity(Dt::CEntity* _pCameraEntity);
        Dt::CEntity* GetCameraEntity();
        const Dt::CEntity* GetCameraEntity() const;

        void SetDeviceType(EType _DeviceType);
        EType GetDeviceType() const;

        void SetCameraParameterFile(const Base::Char* _pCameraParameterFile);
        const Base::Char* GetCameraParameterFile() const;

        void SetDeviceNumber(unsigned int _DeviceNumber);
        unsigned int GetDeviceNumber() const;

        void SetOutputBackground(Dt::CTexture2D* _pOutputBackground);
        Dt::CTexture2D* GetOutputBackground();
        const Dt::CTexture2D* GetOutputBackground() const;

        void SetOutputCubemap(Dt::CTextureCube* _pOutputCubemap);
        Dt::CTextureCube* GetOutputCubemap();
        const Dt::CTextureCube* GetOutputCubemap() const;

        void SetNumberOfMarker(unsigned int _NumberOfMarker);
        unsigned int GetNumberOfMarker() const;

        SMarker& GetMarker(unsigned int _Number);
        const SMarker& GetMarker(unsigned int _Number) const;

    public:

        CARControllerPluginFacet();
        ~CARControllerPluginFacet();

    private:

        Dt::CEntity*      m_pCameraEntity;
        Dt::CTexture2D*   m_pOutputBackground;
        Dt::CTextureCube* m_pOutputCubemap;
        EType             m_DeviceType;
        unsigned int      m_NumberOfMarker;
        unsigned int      m_DeviceNumber;
        Base::CharString  m_CameraParameterFile;
        SMarker           m_Marker[s_MaxNumberOfMarker];
    };
} // namespace Dt

namespace Dt
{
    class CARTrackedObjectPluginFacet
    {
    public:

        void SetUID(unsigned int _UID);
        unsigned int GetUID() const;

        void SetAppearCounter(unsigned int _AppearCounter);
        unsigned int GetAppearCounter() const;

        void SetIsFound(bool _Flag);
        bool IsFound() const;

    public:

        CARTrackedObjectPluginFacet();
        ~CARTrackedObjectPluginFacet();

    private:

        bool         m_IsFound;
        unsigned int m_UID;
        unsigned int m_AppearCounter;
    };
} // namespace Dt