
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_selection_renderer.h"

#include "editor/edit_graphic_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

namespace
{
    class CGraphicHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGraphicHelper)

    public:

        CGraphicHelper();
        ~CGraphicHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnRequestHistogramInfo(Edit::CMessage& _rMessage);
        void OnHistogramInfo(Edit::CMessage& _rMessage);
        void OnHighlightEntity(Edit::CMessage& _rMessage);
    };
} // namespace

namespace
{
    CGraphicHelper::CGraphicHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CGraphicHelper::~CGraphicHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_Histogram_Info, EDIT_RECEIVE_MESSAGE(&CGraphicHelper::OnRequestHistogramInfo));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_Histogram_Update, EDIT_RECEIVE_MESSAGE(&CGraphicHelper::OnHistogramInfo));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_HighlightEntity, EDIT_RECEIVE_MESSAGE(&CGraphicHelper::OnHighlightEntity));
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnRequestHistogramInfo(Edit::CMessage& _rMessage)
    {
        Gfx::SHistogramSettings& rSettings = Gfx::HistogramRenderer::GetSettings();

        Edit::CMessage NewMessage;

        NewMessage.PutFloat(rSettings.m_HistogramLowerBound);
        NewMessage.PutFloat(rSettings.m_HistogramUpperBound);
        NewMessage.PutFloat(rSettings.m_HistogramLogMin);
        NewMessage.PutFloat(rSettings.m_HistogramLogMax);
        NewMessage.PutFloat(rSettings.m_EyeAdaptionSpeedUp);
        NewMessage.PutFloat(rSettings.m_EyeAdaptionSpeedDown);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Graphic_Histogram_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnHistogramInfo(Edit::CMessage& _rMessage)
    {
        Gfx::SHistogramSettings Settings;

        Settings.m_HistogramLowerBound  = _rMessage.GetFloat();
        Settings.m_HistogramUpperBound  = _rMessage.GetFloat();
        Settings.m_HistogramLogMin      = _rMessage.GetFloat();
        Settings.m_HistogramLogMax      = _rMessage.GetFloat();
        Settings.m_EyeAdaptionSpeedUp   = _rMessage.GetFloat();
        Settings.m_EyeAdaptionSpeedDown = _rMessage.GetFloat();

        Gfx::HistogramRenderer::SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnHighlightEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        if (EntityID >= 0)
        {
            unsigned int SelectedEntity = static_cast<unsigned int>(EntityID);

            Gfx::SelectionRenderer::SelectEntity(SelectedEntity);
        }
        else
        {
            Gfx::SelectionRenderer::UnselectEntity();
        }
    }
} // namespace

namespace Edit
{
namespace Helper
{
namespace Graphic
{
    void OnStart()
    {
        CGraphicHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGraphicHelper::GetInstance().OnExit();
    }
} // namespace Graphic
} // namespace Helper
} // namespace Edit