
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_histogram_renderer.h"
#include "graphic/gfx_shader_manager.h"
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

        const Gfx::SHistogramSettings& rSettings = Gfx::HistogramRenderer::GetSettings();

        Edit::CMessage NewMessage;

        NewMessage.Put(rSettings.m_HistogramLowerBound);
        NewMessage.Put(rSettings.m_HistogramUpperBound);
        NewMessage.Put(rSettings.m_HistogramLogMin);
        NewMessage.Put(rSettings.m_HistogramLogMax);
        NewMessage.Put(rSettings.m_EyeAdaptionSpeedUp);
        NewMessage.Put(rSettings.m_EyeAdaptionSpeedDown);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Graphic_Histogram_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnHistogramInfo(Edit::CMessage& _rMessage)
    {
        Gfx::SHistogramSettings Settings;

        Settings.m_HistogramLowerBound  = _rMessage.Get<float>();
        Settings.m_HistogramUpperBound  = _rMessage.Get<float>();
        Settings.m_HistogramLogMin      = _rMessage.Get<float>();
        Settings.m_HistogramLogMax      = _rMessage.Get<float>();
        Settings.m_EyeAdaptionSpeedUp   = _rMessage.Get<float>();
        Settings.m_EyeAdaptionSpeedDown = _rMessage.Get<float>();

        Gfx::HistogramRenderer::SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnRequestToneMappingInfo(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        const Gfx::SColorGradingSettings& rSettings = Gfx::TonemappingRenderer::GetSettings();

        Edit::CMessage NewMessage;

        NewMessage.Put(rSettings.m_Tint[0]);
        NewMessage.Put(rSettings.m_Tint[1]);
        NewMessage.Put(rSettings.m_Tint[2]);
        NewMessage.Put(rSettings.m_DarkTint[0]);
        NewMessage.Put(rSettings.m_DarkTint[1]);
        NewMessage.Put(rSettings.m_DarkTint[2]);
        NewMessage.Put(rSettings.m_DarkTintBlend);
        NewMessage.Put(rSettings.m_DarkTintStrength);
        NewMessage.Put(rSettings.m_Saturation);
        NewMessage.Put(rSettings.m_Contrast);
        NewMessage.Put(rSettings.m_DynamicRange);
        NewMessage.Put(rSettings.m_CrushBrights);
        NewMessage.Put(rSettings.m_CrushDarks);
        NewMessage.Put(rSettings.m_Red[0]);
        NewMessage.Put(rSettings.m_Red[1]);
        NewMessage.Put(rSettings.m_Red[2]);
        NewMessage.Put(rSettings.m_Green[0]);
        NewMessage.Put(rSettings.m_Green[1]);
        NewMessage.Put(rSettings.m_Green[2]);
        NewMessage.Put(rSettings.m_Blue[0]);
        NewMessage.Put(rSettings.m_Blue[1]);
        NewMessage.Put(rSettings.m_Blue[2]);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Graphic_ToneMapping_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnToneMappingInfo(Edit::CMessage& _rMessage)
    {
        Gfx::SColorGradingSettings Settings;

        Settings.m_Tint[0]          = _rMessage.Get<float>();
        Settings.m_Tint[1]          = _rMessage.Get<float>();
        Settings.m_Tint[2]          = _rMessage.Get<float>();
        Settings.m_DarkTint[0]      = _rMessage.Get<float>();
        Settings.m_DarkTint[1]      = _rMessage.Get<float>();
        Settings.m_DarkTint[2]      = _rMessage.Get<float>();
        Settings.m_DarkTintBlend    = _rMessage.Get<float>();
        Settings.m_DarkTintStrength = _rMessage.Get<float>();
        Settings.m_Saturation       = _rMessage.Get<float>();
        Settings.m_Contrast         = _rMessage.Get<float>();
        Settings.m_DynamicRange     = _rMessage.Get<float>();
        Settings.m_CrushBrights     = _rMessage.Get<float>();
        Settings.m_CrushDarks       = _rMessage.Get<float>();
        Settings.m_Red[0]           = _rMessage.Get<float>();
        Settings.m_Red[1]           = _rMessage.Get<float>();
        Settings.m_Red[2]           = _rMessage.Get<float>();
        Settings.m_Green[0]         = _rMessage.Get<float>();
        Settings.m_Green[1]         = _rMessage.Get<float>();
        Settings.m_Green[2]         = _rMessage.Get<float>();
        Settings.m_Blue[0]          = _rMessage.Get<float>();
        Settings.m_Blue[1]          = _rMessage.Get<float>();
        Settings.m_Blue[2]          = _rMessage.Get<float>();

        Gfx::TonemappingRenderer::SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnReloadAllRenderer(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CGraphicHelper::OnReloadAllShader(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        Gfx::ShaderManager::ReloadAllShaders();
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