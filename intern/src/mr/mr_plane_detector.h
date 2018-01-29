
#pragma once

#include "base/base_matrix4x4.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture.h"

namespace MR
{
    class CPlaneDetector
    {
    public:

        void SetImages(Gfx::CTexturePtr _VertexMap, Gfx::CTexturePtr _NormalMap);
        void DetectPlanes(const Base::Float4x4& _PoseMatrix, Gfx::CTexturePtr _VertexMap = nullptr, Gfx::CTexturePtr _NormalMap = nullptr);

        Gfx::CTexturePtr GetVertexMap();
        Gfx::CTexturePtr GetNormalMap();
        Gfx::CTexturePtr GetNormalHistogram();
        
    public:

        CPlaneDetector();
       ~CPlaneDetector();

	private:

        void CreateHistogram(const Base::Float4x4& _PoseMatrix);
        void ExtractPlanes();

        void ClearData();

    private:

        Gfx::CTexturePtr m_VertexMap;
        Gfx::CTexturePtr m_NormalMap;
        Gfx::CTexturePtr m_NormalHistogram;

        Gfx::CBufferPtr m_HistogramConstantBuffer;
        Gfx::CBufferPtr m_PlaneBuffer;

        Gfx::CShaderPtr m_HistogramCreationCSPtr;
        Gfx::CShaderPtr m_PlaneExtractionCSPtr;

        int m_MaxPlanes;
    };
} // namespace MR