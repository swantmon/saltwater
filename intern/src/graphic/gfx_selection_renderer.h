
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

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

    ENGINE_API void SelectEntity(Base::ID _EntityID);
    ENGINE_API void UnselectEntity();

    ENGINE_API CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY, unsigned int _Flags = SPickFlag::Nothing);
    ENGINE_API void ReleaseTicket(CSelectionTicket& _rTicket);

    ENGINE_API void PushPick(CSelectionTicket& _rTicket, const glm::ivec2& _rCursor);
    ENGINE_API bool PopPick(CSelectionTicket& _rTicket);

    ENGINE_API void Clear(CSelectionTicket& _rTicket);

    ENGINE_API bool IsEmpty(const CSelectionTicket& _rTicket);

    ENGINE_API bool IsValid(const CSelectionTicket& _rTicket);

    ENGINE_API void ResetSettings();
    ENGINE_API void SetSettings(const SSelectionSettings& _rSettings);
    ENGINE_API const SSelectionSettings& GetSettings();
} // namespace SelectionRenderer
} // namespace Gfx
