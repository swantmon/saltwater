
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"
#include "base/base_serialize_glm.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class CEntity;
} // namespace Dt


namespace Dt
{
    class ENGINE_API CARControllerPluginComponent : public CComponent<CARControllerPluginComponent>
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

            unsigned int m_UID;
            EMarkerType  m_Type;
            std::string  m_PatternFile;
            float        m_WidthInMeter;
        };

    public:

        void SetCameraEntity(Dt::CEntity* _pCameraEntity);
        Dt::CEntity* GetCameraEntity();
        const Dt::CEntity* GetCameraEntity() const;

        void SetDeviceType(EType _DeviceType);
        EType GetDeviceType() const;

        void SetFreezeOutput(bool _Flag);
        bool GetFreezeLastFrame() const;

        void SetConfiguration(const std::string& _rConfiguration);
        const std::string& GetConfiguration() const;

        void SetCameraParameterFile(const std::string& _rCameraParameterFile);
        const std::string& GetCameraParameterFile() const;

        void SetNumberOfMarker(unsigned int _NumberOfMarker);
        unsigned int GetNumberOfMarker() const;

        SMarker& GetMarker(unsigned int _Number);
        const SMarker& GetMarker(unsigned int _Number) const;

    public:

        CARControllerPluginComponent();
        ~CARControllerPluginComponent();

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CARControllerPluginComponent();
        }

    private:

        Dt::CEntity* m_pCameraEntity;
        EType        m_DeviceType;
        bool         m_FreezeOutput;
        unsigned int m_NumberOfMarker;
        std::string  m_CameraParameterFile;
        std::string  m_Configuration;
        SMarker      m_Marker[s_MaxNumberOfMarker];
    };
} // namespace Dt