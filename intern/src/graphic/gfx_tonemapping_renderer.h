//
//  gfx_tonemapping_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 26/05/15.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector3.h"

namespace Gfx
{
    struct SColorGradingSettings
    {
        Base::Float3 m_Tint;                ///< White point of input color (normalized).
        Base::Float3 m_DarkTint;            ///< Tint applied to darks of the scene (normalized).
        float        m_DarkTintBlend;       ///< Controls the quickness of the transition between tint color and dark tint color.
        float        m_DarkTintStrength;    ///< Weights the dark tint color.
        float        m_Saturation;
        float        m_Contrast;            ///< Controls the slope of the tone-mapping curve.
        float        m_DynamicRange;        ///< Controls the amount of dynamic range before the tone-mapping curve starts to clip.
        float        m_CrushBrights;        ///< Controls the length of the linear section of the tone-mapper curve on the bright side of 18% grey.
        float        m_CrushDarks;          ///< Controls the length of the linear section of the tone-mapper curve on the dark side of 18% grey.
        Base::Float3 m_Red;
        Base::Float3 m_Green;
        Base::Float3 m_Blue;
    };
} // namespace Gfx

namespace Gfx
{
namespace TonemappingRenderer
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
    void OnNewMap();
    void OnUnloadMap();
    
    void Update();
    void Render();

    void SetSettings();
    void SetSettings(const SColorGradingSettings& _rSettings);

    const SColorGradingSettings& GetSettings();
} // namespace TonemappingRenderer
} // namespace Gfx