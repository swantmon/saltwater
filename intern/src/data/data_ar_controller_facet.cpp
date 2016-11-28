
#include "data/data_precompiled.h"

#include "base/base_memory.h"

#include "data/data_ar_controller_facet.h"

namespace Dt
{
    CARControllerPluginFacet::CARControllerPluginFacet()
        : m_pCameraEntity      (nullptr)
        , m_pOutputBackground  (nullptr)
        , m_pOutputCubemap     (nullptr)
        , m_DeviceType         (Undefined)
        , m_NumberOfMarker     (0)
        , m_CameraParameterFile()
        , m_Configuration      ()
        , m_Marker             ()
    {
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet::~CARControllerPluginFacet()
    {
        m_CameraParameterFile.Clear();
        m_Configuration.Clear();
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

    void CARControllerPluginFacet::SetCameraParameterFile(const Base::Char* _pCameraParameterFile)
    {
        m_CameraParameterFile = _pCameraParameterFile;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CARControllerPluginFacet::GetCameraParameterFile() const
    {
        return m_CameraParameterFile.GetConst();
    }

    // -----------------------------------------------------------------------------

    void CARControllerPluginFacet::SetConfiguration(const Base::Char* _pConfiguration)
    {
        m_Configuration = _pConfiguration;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CARControllerPluginFacet::GetConfiguration() const
    {
        return m_Configuration.GetConst();
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

    void CARControllerPluginFacet::SetOutputCubemap(Dt::CTextureCube* _pOutputCubemap)
    {
        m_pOutputCubemap = _pOutputCubemap;
    }

    // -----------------------------------------------------------------------------

    Dt::CTextureCube* CARControllerPluginFacet::GetOutputCubemap()
    {
        return m_pOutputCubemap;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTextureCube* CARControllerPluginFacet::GetOutputCubemap() const
    {
        return m_pOutputCubemap;
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