
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "editor/edit_slam_helper.h"
#include "mr/mr_slam_reconstructor.h"
#include "graphic/gfx_reconstruction_renderer.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace
{
    class CSLAMHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CSLAMHelper)

    public:

        CSLAMHelper();
        ~CSLAMHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnNewSLAMReconstruction(Edit::CMessage& _rMessage);
        void OnSLAMReconstructionUpdate(Edit::CMessage& _rMessage);
        void OnSLAMReconstructionPauseDepth(Edit::CMessage& _rMessage);
        void OnSLAMReconstructionPauseColor(Edit::CMessage& _rMessage);
    };
} // namespace

namespace
{
    CSLAMHelper::CSLAMHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CSLAMHelper::~CSLAMHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::MR_SLAM_NewReconstruction, EDIT_RECEIVE_MESSAGE(&CSLAMHelper::OnNewSLAMReconstruction));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Update, EDIT_RECEIVE_MESSAGE(&CSLAMHelper::OnSLAMReconstructionUpdate));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Depth, EDIT_RECEIVE_MESSAGE(&CSLAMHelper::OnSLAMReconstructionPauseDepth));
        Edit::MessageManager::Register(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Pause_Color, EDIT_RECEIVE_MESSAGE(&CSLAMHelper::OnSLAMReconstructionPauseColor));
    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnNewSLAMReconstruction(Edit::CMessage& _rMessage)
    {
        BASE_CONSOLE_INFO("New Reconstruction");
    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnSLAMReconstructionUpdate(Edit::CMessage& _rMessage)
    {        
        MR::CSLAMReconstructor::SReconstructionSettings Settings;

        Settings.m_VolumeSize = _rMessage.GetFloat();
        Settings.m_VolumeResolution = _rMessage.GetInt();
        Settings.m_TruncatedDistance = _rMessage.GetFloat();
        Settings.m_MaxIntegrationWeight = _rMessage.GetInt();
        Settings.m_DepthThreshold[0] = _rMessage.GetInt();
        Settings.m_DepthThreshold[1] = _rMessage.GetInt();
        Settings.m_CaptureColor = _rMessage.GetBool();

        Gfx::ReconstructionRenderer::OnReconstructionUpdate(Settings);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnSLAMReconstructionPauseDepth(Edit::CMessage& _rMessage)
    {
        bool Pause = _rMessage.GetBool();

        Gfx::ReconstructionRenderer::PauseDepthIntegration(Pause);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnSLAMReconstructionPauseColor(Edit::CMessage& _rMessage)
    {
        bool Pause = _rMessage.GetBool();

        Gfx::ReconstructionRenderer::PauseColorIntegration(Pause);

        _rMessage.SetResult(1);
    }
} // namespace

namespace Edit
{
namespace Helper
{
namespace SLAM
{
    void OnStart()
    {
        CSLAMHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CSLAMHelper::GetInstance().OnExit();
    }
} // namespace SLAM
} // namespace Helper
} // namespace Edit