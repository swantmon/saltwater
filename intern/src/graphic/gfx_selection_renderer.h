
#pragma once

#include "base/base_include_glm.h"

#include "graphic/gfx_export.h"
#include "graphic/gfx_selection.h"

namespace Gfx
{
    struct SSelectionSettings
    {
        glm::vec4 m_HighlightColor;
        bool      m_HighlightUseDepth;
        bool      m_HighlightUseWireframe;
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

    GFX_API void SelectEntity(Base::ID _EntityID);
    GFX_API void UnselectEntity();

    GFX_API CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags = SPickFlag::Nothing);
    GFX_API void ReleaseTicket(CSelectionTicket& _rTicket);

    GFX_API void PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor);
    GFX_API bool PopPick(CSelectionTicket& _rTicket);

    GFX_API void Clear(CSelectionTicket& _rTicket);

    GFX_API bool IsEmpty(const CSelectionTicket& _rTicket);

    GFX_API bool IsValid(const CSelectionTicket& _rTicket);

    GFX_API void ResetSettings();
    GFX_API void SetSettings(const SSelectionSettings& _rSettings);
    GFX_API const SSelectionSettings& GetSettings();
} // namespace SelectionRenderer
} // namespace Gfx
