
#include "base/base_memory.h"

#include "data/data_Plugin_facet.h"

namespace Dt
{
    CARControllerPluginFacet::CARControllerPluginFacet()
        : m_pCameraEntity      (nullptr)
        , m_pOutputBackground  (nullptr)
        , m_pOutputCubemap     (nullptr)
        , m_DeviceType         (Undefined)
        , m_NumberOfMarker     (0)
        , m_DeviceNumber       (0)
        , m_CameraParameterFile()
        , m_Marker             ()
    {
    }

    // -----------------------------------------------------------------------------

    CARControllerPluginFacet::~CARControllerPluginFacet()
    {
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

    void CARControllerPluginFacet::SetDeviceNumber(unsigned int _DeviceNumber)
    {
        m_DeviceNumber = _DeviceNumber;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARControllerPluginFacet::GetDeviceNumber() const
    {
        return m_DeviceNumber;
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

namespace Dt
{
    CARTrackedObjectPluginFacet::CARTrackedObjectPluginFacet()
        : m_IsFound      (false)
        , m_UID          (0)
        , m_AppearCounter(0)
    {
    }

    // -----------------------------------------------------------------------------

    CARTrackedObjectPluginFacet::~CARTrackedObjectPluginFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginFacet::SetUID(unsigned int _UID)
    {
        m_UID = _UID;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARTrackedObjectPluginFacet::GetUID() const
    {
        return m_UID;
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginFacet::SetAppearCounter(unsigned int _AppearCounter)
    {
        m_AppearCounter = _AppearCounter;
    }

    // -----------------------------------------------------------------------------

    unsigned int CARTrackedObjectPluginFacet::GetAppearCounter() const
    {
        return m_AppearCounter;
    }

    // -----------------------------------------------------------------------------

    void CARTrackedObjectPluginFacet::SetIsFound(bool _Flag)
    {
        m_IsFound = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CARTrackedObjectPluginFacet::IsFound() const
    {
        return m_IsFound;
    }
} // namespace Dt