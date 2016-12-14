
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
    class CARControllerPluginFacet
    {
    public:

        static const unsigned int s_MaxNumberOfMarker = 8;

    public:

        enum EType
        {
            Webcam,
            Kinect,
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

        void SetConfiguration(const Base::Char* _pConfiguration);
        const Base::Char* GetConfiguration() const;

        void SetCameraParameterFile(const Base::Char* _pCameraParameterFile);
        const Base::Char* GetCameraParameterFile() const;

        void SetOutputBackground(Dt::CTexture2D* _pOutputBackground);
        Dt::CTexture2D* GetOutputBackground();
        const Dt::CTexture2D* GetOutputBackground() const;

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
        EType             m_DeviceType;
        unsigned int      m_NumberOfMarker;
        Base::CharString  m_CameraParameterFile;
        Base::CharString  m_Configuration;
        SMarker           m_Marker[s_MaxNumberOfMarker];
    };
} // namespace Dt