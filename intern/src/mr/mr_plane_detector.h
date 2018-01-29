
#pragma once

#include "graphic/gfx_texture.h"

namespace MR
{
    class CPlaneDetector
    {
    public:

        void SetDepthImage(Gfx::CTexturePtr _DepthImage);
        void DetectPlanes(Gfx::CTexturePtr _DepthImage = nullptr);
        
    public:

        CPlaneDetector();
       ~CPlaneDetector();

	private:

        Gfx::CTexturePtr m_DepthImage;

    };
} // namespace MR