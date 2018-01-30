
#pragma once

#include "base/base_matrix4x4.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture.h"

#include <vector>

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

        typedef std::vector<Base::Float4> Float4Vector;

        void CreateHistogram(const Base::Float4x4& _PoseMatrix);
        void ExtractPlaneCandidates(Float4Vector& _rPlanes);
        void FindPlaneEquations(Float4Vector& _rPlanes);

        void ClearData();

    private:

        Gfx::CTexturePtr m_VertexMap;
        Gfx::CTexturePtr m_NormalMap;
        Gfx::CTexturePtr m_Histogram;

        Gfx::CBufferPtr m_HistogramConstantBuffer;
        Gfx::CBufferPtr m_PlaneCountBuffer;
        Gfx::CBufferPtr m_PlaneBuffer;

        Gfx::CShaderPtr m_HistogramCreationCSPtr;
        Gfx::CShaderPtr m_PlaneCandidatesCSPtr;
        Gfx::CShaderPtr m_PlaneEquationCSPtr;

        int m_MaxDetectablePlaneCount;

        Float4Vector m_Planes;
    };
} // namespace MR