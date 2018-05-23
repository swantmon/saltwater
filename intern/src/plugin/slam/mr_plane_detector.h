
#pragma once

#include "base/base_include_glm.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture.h"

#include <vector>

namespace MR
{
    class CPlaneDetector
    {
    public:

        typedef std::vector<glm::vec4> Vec4Vector;

        void SetImages(Gfx::CTexturePtr _VertexMap, Gfx::CTexturePtr _NormalMap);
        void DetectPlanes(const glm::mat4& _PoseMatrix, Gfx::CTexturePtr _VertexMap = nullptr, Gfx::CTexturePtr _NormalMap = nullptr);

        Gfx::CTexturePtr GetVertexMap();
        Gfx::CTexturePtr GetNormalMap();
        Gfx::CTexturePtr GetNormalHistogram();

        const Vec4Vector& GetPlanes();
        
    public:

        CPlaneDetector();
       ~CPlaneDetector();

	private:

        void CreateHistogram(const glm::mat4& _PoseMatrix);
        void ExtractPlaneCandidates();
        void FindPlaneEquations();
        void ExtractPlanes(Vec4Vector& _rPlanes);

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
        Gfx::CShaderPtr m_PlaneExtractionCSPtr;

        Gfx::CShaderPtr m_PlaneTestCSPtr;

        int m_MaxDetectablePlaneCount;

        Vec4Vector m_Planes;
    };
} // namespace MR