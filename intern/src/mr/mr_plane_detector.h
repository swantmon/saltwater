
#pragma once

#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture.h"

namespace MR
{
    class CPlaneDetector
    {
    public:

        void SetImages(Gfx::CTexturePtr _VertexMap, Gfx::CTexturePtr _NormalMap);
        void DetectPlanes(Gfx::CTexturePtr _VertexMap = nullptr, Gfx::CTexturePtr _NormalMap = nullptr);

        Gfx::CTexturePtr GetVertexMap();
        Gfx::CTexturePtr GetNormalMap();
        Gfx::CTexturePtr GetNormalHistogram();
        
    public:

        CPlaneDetector();
       ~CPlaneDetector();

	private:

        Gfx::CTexturePtr m_VertexMap;
        Gfx::CTexturePtr m_NormalMap;
        Gfx::CTexturePtr m_NormalHistogram;

        Gfx::CShaderPtr m_NormalHistogramCSPtr;

    };
} // namespace MR