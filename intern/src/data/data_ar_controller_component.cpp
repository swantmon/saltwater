
#include "data/data_precompiled.h"

#include "base/base_memory.h"

#include "data/data_ar_controller_component.h"

namespace Dt
{
    CARControllerPluginComponent::CARControllerPluginComponent()
        : m_pCameraEntity      (nullptr)
        , m_pOutputBackground  (nullptr)
        , m_DeviceType         (Undefined)
        , m_FreezeOutput       (false)
        , m_NumberOfMarker     (0)
        , m_CameraParameterFile()
        , m_Configuration      ()
        , m_Marker             ()
    {
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginComponent::~CARControllerPluginComponent()
    {
        m_CameraParameterFile.clear();
        m_Configuration.clear();
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginComponent::SetCameraEntity(Dt::CEntity* _pCameraEntity)
    {
        m_pCameraEntity = _pCameraEntity;
    }

    // -----------------------------------------------------------------------------

    Dt::CEntity* CARControllerPluginComponent::GetCameraEntity()
    {
        return m_pCameraEntity;
    }

    // -----------------------------------------------------------------------------

    const Dt::CEntity* CARControllerPluginComponent::GetCameraEntity() const
    {
        return m_pCameraEntity;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginComponent::SetDeviceType(EType _DeviceType)
    {
        m_DeviceType = _DeviceType;
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginComponent::EType CARControllerPluginComponent::GetDeviceType() const
    {
        return m_DeviceType;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginComponent::SetFreezeOutput(bool _Flag)
    {
        m_FreezeOutput = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CARControllerPluginComponent::GetFreezeLastFrame() const
    {
        return m_FreezeOutput;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginComponent::SetCameraParameterFile(const std::string& _rCameraParameterFile)
    {
        m_CameraParameterFile = _rCameraParameterFile;
    }

    // -----------------------------------------------------------------------------

    const std::string& CARControllerPluginComponent::GetCameraParameterFile() const
    {
        return m_CameraParameterFile;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginComponent::SetConfiguration(const std::string& _rConfiguration)
    {
        m_Configuration = _rConfiguration;
    }

    // -----------------------------------------------------------------------------

    const std::string& CARControllerPluginComponent::GetConfiguration() const
    {
        return m_Configuration;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginComponent::SetOutputBackground(Dt::CTexture2D* _pOutputBackground)
    {
        m_pOutputBackground = _pOutputBackground;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CARControllerPluginComponent::GetOutputBackground()
    {
        return m_pOutputBackground;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CARControllerPluginComponent::GetOutputBackground() const
    {
        return m_pOutputBackground;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginComponent::SetNumberOfMarker(unsigned int _NumberOfMarker)
    {
        m_NumberOfMarker = _NumberOfMarker;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARControllerPluginComponent::GetNumberOfMarker() const
    {
        return m_NumberOfMarker;
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginComponent::SMarker& CARControllerPluginComponent::GetMarker(unsigned int _Number)
    {
        assert(_Number >= 0 && _Number < s_MaxNumberOfMarker);

        return m_Marker[_Number];
    }

    // -----------------------------------------------------------------------------

    const CARControllerPluginComponent::SMarker& CARControllerPluginComponent::GetMarker(unsigned int _Number) const
    {
        assert(_Number >= 0 && _Number < s_MaxNumberOfMarker);

        return m_Marker[_Number];
    }
} // namespace Dt