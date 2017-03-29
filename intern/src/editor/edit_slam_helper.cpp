
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "editor/edit_slam_helper.h"

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

        Edit::MessageManager::Register(Edit::SGUIMessageType::MR_SLAM_Reconstruction_Info_Update, EDIT_RECEIVE_MESSAGE(&CSLAMHelper::OnSLAMReconstructionUpdate));
    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnNewSLAMReconstruction(Edit::CMessage& _rMessage)
    {
        
    }

    // -----------------------------------------------------------------------------

    void CSLAMHelper::OnSLAMReconstructionUpdate(Edit::CMessage& _rMessage)
    {
        
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