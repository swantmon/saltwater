
#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_kinect_control.h"
#include "mr/mr_webcam_control.h"

#include <assert.h>

using namespace MR;

namespace
{
    class CMRControlManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRControlManager);
        
    public:

        CMRControlManager();
       ~CMRControlManager();

    public:

        void CreateControl(CControl::EType _Type);

        void SetActiveControl(CControl::EType _Type);
        CControl& GetActiveControl();
        const CControl& GetActiveControl() const;

    public:

        void Start(const SControlDescription& _rDescriptor);
        void Stop();

        void Update();

        bool IsActive();

        SDeviceParameter& GetCameraParameters();

        Base::Float3x3& GetProjectionMatrix();

    private:

        CControl* m_pActiveControl;
        CControl* m_pControls[CControl::NumberOfControls];
    };
} // namespace

namespace
{
    CMRControlManager::CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::CreateControl(CControl::EType _Type)
    {
        if (m_pControls[_Type] == nullptr)
        {
            switch (_Type)
            {
                case CControl::Webcam:
                {
                    m_pControls[_Type] = Base::CMemory::NewObject<CWebcamControl>();
                }
                break;
                case CControl::Kinect:
                {
                    m_pControls[_Type] = Base::CMemory::NewObject<CKinectControl>();
                }
                break;
                    
                default:
                    BASE_CONSOLE_STREAMWARNING("Can't create camera control of type " << _Type);
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::SetActiveControl(CControl::EType _Type)
    {
        assert((_Type >= 0) && (_Type < CControl::NumberOfControls));
        
        m_pActiveControl = m_pControls[_Type];
    }

    // -----------------------------------------------------------------------------

    CControl& CMRControlManager::GetActiveControl()
    {
        assert(m_pActiveControl != nullptr);

        return *m_pActiveControl;
    }

    // -----------------------------------------------------------------------------

    const CControl& CMRControlManager::GetActiveControl() const
    {
        assert(m_pActiveControl != nullptr);
        
        return *m_pActiveControl;
    }

    void CMRControlManager::Start(const SControlDescription& _rDescriptor)
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->Start(_rDescriptor);
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Stop()
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->Stop();
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->Update();
        }
    }

    // -----------------------------------------------------------------------------

    bool CMRControlManager::IsActive()
    {
        return (m_pActiveControl != nullptr && m_pActiveControl->IsStarted());
    }
} // namespace

namespace MR
{
namespace ControlManager
{
    void CreateControl(CControl::EType _Type)
    {
        CMRControlManager::GetInstance().CreateControl(_Type);
    }

    // -----------------------------------------------------------------------------

    void SetActiveControl(CControl::EType _Type)
    {
        CMRControlManager::GetInstance().SetActiveControl(_Type);
    }

    // -----------------------------------------------------------------------------

    CControl& GetActiveControl()
    {
        return CMRControlManager::GetInstance().GetActiveControl();
    }

    // -----------------------------------------------------------------------------

    void Start(const SControlDescription& _rDescriptor)
    {
        CMRControlManager::GetInstance().Start(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    void Stop()
    {
        CMRControlManager::GetInstance().Stop();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRControlManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    bool IsActive()
    {
        return CMRControlManager::GetInstance().IsActive();
    }
} // namespace ControlManager
} // namespace MR
