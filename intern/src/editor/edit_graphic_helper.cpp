
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_manager_interface.h"
#include "graphic/gfx_renderer_interface.h"
#include "graphic/gfx_tonemapping_renderer.h"

#include "editor/edit_graphic_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

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
        void OnRequestToneMappingInfo(Edit::CMessage& _rMessage);
        void OnToneMappingInfo(Edit::CMessage& _rMessage);
        void OnReloadAllRenderer(Edit::CMessage& _rMessage);
        void OnReloadAllShader(Edit::CMessage& _rMessage);
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

        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_ToneMapping_Info, EDIT_RECEIVE_MESSAGE(&CGraphicHelper::OnRequestToneMappingInfo));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_ToneMapping_Update, EDIT_RECEIVE_MESSAGE(&CGraphicHelper::OnToneMappingInfo));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_ReloadRenderer, EDIT_RECEIVE_MESSAGE(&CGraphicHelper::OnReloadAllRenderer));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_ReloadAllShader, EDIT_RECEIVE_MESSAGE(&CGraphicHelper::OnReloadAllShader));
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnRequestHistogramInfo(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

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

    void CGraphicHelper::OnRequestToneMappingInfo(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        const Gfx::SColorGradingSettings& rSettings = Gfx::TonemappingRenderer::GetSettings();

        Edit::CMessage NewMessage;

        NewMessage.PutFloat(rSettings.m_Tint[0]);
        NewMessage.PutFloat(rSettings.m_Tint[1]);
        NewMessage.PutFloat(rSettings.m_Tint[2]);
        NewMessage.PutFloat(rSettings.m_DarkTint[0]);
        NewMessage.PutFloat(rSettings.m_DarkTint[1]);
        NewMessage.PutFloat(rSettings.m_DarkTint[2]);
        NewMessage.PutFloat(rSettings.m_DarkTintBlend);
        NewMessage.PutFloat(rSettings.m_DarkTintStrength);
        NewMessage.PutFloat(rSettings.m_Saturation);
        NewMessage.PutFloat(rSettings.m_Contrast);
        NewMessage.PutFloat(rSettings.m_DynamicRange);
        NewMessage.PutFloat(rSettings.m_CrushBrights);
        NewMessage.PutFloat(rSettings.m_CrushDarks);
        NewMessage.PutFloat(rSettings.m_Red[0]);
        NewMessage.PutFloat(rSettings.m_Red[1]);
        NewMessage.PutFloat(rSettings.m_Red[2]);
        NewMessage.PutFloat(rSettings.m_Green[0]);
        NewMessage.PutFloat(rSettings.m_Green[1]);
        NewMessage.PutFloat(rSettings.m_Green[2]);
        NewMessage.PutFloat(rSettings.m_Blue[0]);
        NewMessage.PutFloat(rSettings.m_Blue[1]);
        NewMessage.PutFloat(rSettings.m_Blue[2]);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Graphic_ToneMapping_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnToneMappingInfo(Edit::CMessage& _rMessage)
    {
        Gfx::SColorGradingSettings Settings;

        Settings.m_Tint[0]          = _rMessage.GetFloat();
        Settings.m_Tint[1]          = _rMessage.GetFloat();
        Settings.m_Tint[2]          = _rMessage.GetFloat();
        Settings.m_DarkTint[0]      = _rMessage.GetFloat();
        Settings.m_DarkTint[1]      = _rMessage.GetFloat();
        Settings.m_DarkTint[2]      = _rMessage.GetFloat();
        Settings.m_DarkTintBlend    = _rMessage.GetFloat();
        Settings.m_DarkTintStrength = _rMessage.GetFloat();
        Settings.m_Saturation       = _rMessage.GetFloat();
        Settings.m_Contrast         = _rMessage.GetFloat();
        Settings.m_DynamicRange     = _rMessage.GetFloat();
        Settings.m_CrushBrights     = _rMessage.GetFloat();
        Settings.m_CrushDarks       = _rMessage.GetFloat();
        Settings.m_Red[0]           = _rMessage.GetFloat();
        Settings.m_Red[1]           = _rMessage.GetFloat();
        Settings.m_Red[2]           = _rMessage.GetFloat();
        Settings.m_Green[0]         = _rMessage.GetFloat();
        Settings.m_Green[1]         = _rMessage.GetFloat();
        Settings.m_Green[2]         = _rMessage.GetFloat();
        Settings.m_Blue[0]          = _rMessage.GetFloat();
        Settings.m_Blue[1]          = _rMessage.GetFloat();
        Settings.m_Blue[2]          = _rMessage.GetFloat();

        Gfx::TonemappingRenderer::SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnReloadAllRenderer(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        Gfx::Renderer::ReloadRenderer();
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnReloadAllShader(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        Gfx::Manager::ReloadAllShaders();
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