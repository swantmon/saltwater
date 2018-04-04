
#pragma once

#include "graphic/gfx_blend_state.h"
#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_depth_stencil_state.h"
#include "graphic/gfx_export.h"
#include "graphic/gfx_input_layout.h"
#include "graphic/gfx_rasterizer_state.h"
#include "graphic/gfx_render_context.h"
#include "graphic/gfx_render_state.h"
#include "graphic/gfx_sampler_set.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_texture.h"
#include "graphic/gfx_texture_set.h"
#include "graphic/gfx_topology.h"
#include "graphic/gfx_view_port_set.h"

namespace Gfx
{
namespace ContextManager
{
    GFX_API void OnStart();
    GFX_API void OnExit();
    
    GFX_API CRenderContextPtr CreateRenderContext();
    
    GFX_API void ResetRenderContext();
    GFX_API void SetRenderContext(CRenderContextPtr _RenderContextPtr);

    GFX_API void SetRenderFlags(unsigned int _Flags);
    GFX_API unsigned int GetRenderFlags();

    GFX_API void ResetBlendState();
    GFX_API void SetBlendState(CBlendStatePtr _StatePtr);
    GFX_API CBlendStatePtr GetBlendState();

    GFX_API void ResetDepthStencilState();
    GFX_API void SetDepthStencilState(CDepthStencilStatePtr _StatePtr);
    GFX_API CDepthStencilStatePtr GetDepthStencilState();

    GFX_API void ResetRasterizerState();
    GFX_API void SetRasterizerState(CRasterizerStatePtr _StatePtr);
    GFX_API CRasterizerStatePtr GetRasterizerState();

    GFX_API void ResetTopology();
    GFX_API void SetTopology(STopology::Enum _Topology);
    GFX_API STopology::Enum GetTopology();

    GFX_API void ResetTargetSet();
    GFX_API void SetTargetSet(CTargetSetPtr _TargetSetPtr);
    GFX_API CTargetSetPtr GetTargetSet();

    GFX_API void ResetViewPortSet();
    GFX_API void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr);
    GFX_API CViewPortSetPtr GetViewPortSet();

    GFX_API void ResetInputLayout();
    GFX_API void SetInputLayout(CInputLayoutPtr _InputLayoutPtr);
    GFX_API CInputLayoutPtr GetInputLayout();

    GFX_API void ResetIndexBuffer();
    GFX_API void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset);
    GFX_API void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset);
    GFX_API CBufferPtr GetIndexBuffer();

    GFX_API void ResetVertexBuffer();
    GFX_API void SetVertexBuffer(CBufferPtr _BufferSetPtr, bool _UseActiveInputLayout = true);
    GFX_API CBufferPtr GetVertexBuffer();

    GFX_API void ResetShaderVS();
    GFX_API void SetShaderVS(CShaderPtr _ShaderSetPtr);
    GFX_API CShaderPtr GetShaderVS();
    GFX_API void ResetShaderHS();
    GFX_API void SetShaderHS(CShaderPtr _ShaderSetPtr);
    GFX_API CShaderPtr GetShaderHS();
    GFX_API void ResetShaderDS();
    GFX_API void SetShaderDS(CShaderPtr _ShaderSetPtr);
    GFX_API CShaderPtr GetShaderDS();
    GFX_API void ResetShaderGS();
    GFX_API void SetShaderGS(CShaderPtr _ShaderSetPtr);
    GFX_API CShaderPtr GetShaderGS();
    GFX_API void ResetShaderPS();
    GFX_API void SetShaderPS(CShaderPtr _ShaderSetPtr);
    GFX_API CShaderPtr GetShaderSetPS();
    GFX_API void ResetShaderCS();
    GFX_API void SetShaderCS(CShaderPtr _ShaderSetPtr);
    GFX_API CShaderPtr GetShaderCS();

    GFX_API void ResetSampler(unsigned int _Unit);
    GFX_API void SetSampler(unsigned int _Unit, CSamplerPtr _SamplerPtr);
    GFX_API CSamplerPtr GetSampler(unsigned int _Unit);

    GFX_API void ResetTexture(unsigned int _Unit);
    GFX_API void SetTexture(unsigned int _Unit, CTexturePtr _TextureBasePtr);
    GFX_API CTexturePtr GetTexture(unsigned int _Unit);

    GFX_API void ResetImageTexture(unsigned int _Unit);
    GFX_API void SetImageTexture(unsigned int _Unit, CTexturePtr _TextureBasePtr);
    GFX_API CTexturePtr GetImageTexture(unsigned int _Unit);

    GFX_API void ResetConstantBuffer(unsigned int _Unit);
    GFX_API void SetConstantBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    GFX_API void SetConstantBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
    GFX_API CBufferPtr GetConstantBuffer(unsigned int _Unit);

    GFX_API void ResetResourceBuffer(unsigned int _Unit);
    GFX_API void SetResourceBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    GFX_API void SetResourceBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
    GFX_API CBufferPtr GetResourceBuffer(unsigned int _Unit);

	GFX_API void ResetAtomicCounterBuffer(unsigned int _Unit);
	GFX_API void SetAtomicCounterBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    GFX_API void SetAtomicCounterBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
	GFX_API CBufferPtr GetAtomicCounterBuffer(unsigned int _Unit);

    GFX_API void Flush();

    GFX_API void Barrier();

    GFX_API void Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex);
    GFX_API void DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation);
    GFX_API void DrawInstanced(unsigned int _NumberOfVertices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstVertex);
    GFX_API void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation);
    GFX_API void DrawIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset = 0);
    GFX_API void DrawIndexedIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset = 0);

    GFX_API void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ);
    GFX_API void DispatchIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset = 0);
} // namespace ContextManager
} // namespace Gfx
