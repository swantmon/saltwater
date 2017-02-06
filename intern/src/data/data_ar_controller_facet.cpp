
#include "data/data_precompiled.h"

#include "base/base_memory.h"

#include "data/data_ar_controller_facet.h"

namespace Dt
{
    CARControllerPluginFacet::CARControllerPluginFacet()
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

    CARControllerPluginFacet::~CARControllerPluginFacet()
    {
        m_CameraParameterFile.clear();
        m_Configuration.clear();
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetCameraEntity(Dt::CEntity* _pCameraEntity)
    {
        m_pCameraEntity = _pCameraEntity;
    }

    // -----------------------------------------------------------------------------

    Dt::CEntity* CARControllerPluginFacet::GetCameraEntity()
    {
        return m_pCameraEntity;
    }

    // -----------------------------------------------------------------------------

    const Dt::CEntity* CARControllerPluginFacet::GetCameraEntity() const
    {
        return m_pCameraEntity;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetDeviceType(EType _DeviceType)
    {
        m_DeviceType = _DeviceType;
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet::EType CARControllerPluginFacet::GetDeviceType() const
    {
        return m_DeviceType;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetFreezeOutput(bool _Flag)
    {
        m_FreezeOutput = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CARControllerPluginFacet::GetFreezeLastFrame() const
    {
        return m_FreezeOutput;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetCameraParameterFile(const Base::Char* _pCameraParameterFile)
    {
        m_CameraParameterFile = _pCameraParameterFile;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CARControllerPluginFacet::GetCameraParameterFile() const
    {
        return m_CameraParameterFile.c_str();
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetConfiguration(const Base::Char* _pConfiguration)
    {
        m_Configuration = _pConfiguration;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CARControllerPluginFacet::GetConfiguration() const
    {
        return m_Configuration.c_str();
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetOutputBackground(Dt::CTexture2D* _pOutputBackground)
    {
        m_pOutputBackground = _pOutputBackground;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CARControllerPluginFacet::GetOutputBackground()
    {
        return m_pOutputBackground;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CARControllerPluginFacet::GetOutputBackground() const
    {
        return m_pOutputBackground;
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetNumberOfMarker(unsigned int _NumberOfMarker)
    {
        m_NumberOfMarker = _NumberOfMarker;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARControllerPluginFacet::GetNumberOfMarker() const
    {
        return m_NumberOfMarker;
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet::SMarker& CARControllerPluginFacet::GetMarker(unsigned int _Number)
    {
        assert(_Number >= 0 && _Number < s_MaxNumberOfMarker);

        return m_Marker[_Number];
    }

    // -----------------------------------------------------------------------------

    const CARControllerPluginFacet::SMarker& CARControllerPluginFacet::GetMarker(unsigned int _Number) const
    {
        assert(_Number >= 0 && _Number < s_MaxNumberOfMarker);

        return m_Marker[_Number];
    }
} // namespace Dt