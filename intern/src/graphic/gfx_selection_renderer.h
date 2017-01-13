//
//  gfx_selection_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 17/11/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include "graphic/gfx_selection.h"

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

    CSelectionTicket& AcquireTicket(int _OffsetX, int _OffsetY, int _SizeX, int _SizeY);
    void ReleaseTicket(CSelectionTicket& _rTicket);

    void PushPick(CSelectionTicket& _rTicket, const Base::Int2& _rCursor);
    bool PopPick(CSelectionTicket& _rTicket);

    void Clear(CSelectionTicket& _rTicket);

    bool IsEmpty(const CSelectionTicket& _rTicket);

    bool IsValid(const CSelectionTicket& _rTicket);
} // namespace SelectionRenderer
} // namespace Gfx
