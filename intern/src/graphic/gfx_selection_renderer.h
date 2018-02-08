
#pragma once

#include "base/base_include_glm.h"

#include "graphic/gfx_selection.h"

namespace Gfx
{
    struct SSelectionSettings
    {
        glm::vec4 m_HighlightColor;
        bool      m_HighlightUseDepth;
    };
} // namespace Gfx

namespace Gfx
{
namespace SelectionRenderer
{
    void OnStart();
    void OnExit();
    
    void OnSetupShader();
    void OnSetupKernels();
    void OnSetupRenderTargets();
    void OnSetupStates();
    void OnSetupTextures();
    void OnSetupBuffers();
    void OnSetupResources();
    void OnSetupModels();
    void OnSetupEnd();
    
    void OnReload();
    
    void Update();
    void Render();

    void SelectEntity(unsigned int _EntityID);
    void UnselectEntity();

    CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags = SPickFlag::Nothing);
    void ReleaseTicket(CSelectionTicket& _rTicket);

    void PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor);
    bool PopPick(CSelectionTicket& _rTicket);

    void Clear(CSelectionTicket& _rTicket);

    bool IsEmpty(const CSelectionTicket& _rTicket);

    bool IsValid(const CSelectionTicket& _rTicket);

    void ResetSettings();
    void SetSettings(const SSelectionSettings& _rSettings);
    const SSelectionSettings& GetSettings();
} // namespace SelectionRenderer
} // namespace Gfx
