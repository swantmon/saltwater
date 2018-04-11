
#pragma once

#include "engine/engine_config.h"

#include "engine/graphic/gfx_blend_state.h"
#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_buffer_set.h"
#include "engine/graphic/gfx_depth_stencil_state.h"
#include "engine/graphic/gfx_input_layout.h"
#include "engine/graphic/gfx_rasterizer_state.h"
#include "engine/graphic/gfx_render_context.h"
#include "engine/graphic/gfx_render_state.h"
#include "engine/graphic/gfx_sampler_set.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_set.h"
#include "engine/graphic/gfx_topology.h"
#include "engine/graphic/gfx_view_port_set.h"

namespace Gfx
{
namespace ContextManager
{
    void OnStart();
    void OnExit();
    
    ENGINE_API CRenderContextPtr CreateRenderContext();
    
    ENGINE_API void ResetRenderContext();
    ENGINE_API void SetRenderContext(CRenderContextPtr _RenderContextPtr);

    ENGINE_API void SetRenderFlags(unsigned int _Flags);
    ENGINE_API unsigned int GetRenderFlags();

    ENGINE_API void ResetBlendState();
    ENGINE_API void SetBlendState(CBlendStatePtr _StatePtr);
    ENGINE_API CBlendStatePtr GetBlendState();

    ENGINE_API void ResetDepthStencilState();
    ENGINE_API void SetDepthStencilState(CDepthStencilStatePtr _StatePtr);
    ENGINE_API CDepthStencilStatePtr GetDepthStencilState();

    ENGINE_API void ResetRasterizerState();
    ENGINE_API void SetRasterizerState(CRasterizerStatePtr _StatePtr);
    ENGINE_API CRasterizerStatePtr GetRasterizerState();

    ENGINE_API void ResetTopology();
    ENGINE_API void SetTopology(STopology::Enum _Topology);
    ENGINE_API STopology::Enum GetTopology();

    ENGINE_API void ResetTargetSet();
    ENGINE_API void SetTargetSet(CTargetSetPtr _TargetSetPtr);
    ENGINE_API CTargetSetPtr GetTargetSet();

    ENGINE_API void ResetViewPortSet();
    ENGINE_API void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr);
    ENGINE_API CViewPortSetPtr GetViewPortSet();

    ENGINE_API void ResetInputLayout();
    ENGINE_API void SetInputLayout(CInputLayoutPtr _InputLayoutPtr);
    ENGINE_API CInputLayoutPtr GetInputLayout();

    ENGINE_API void ResetIndexBuffer();
    ENGINE_API void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset);
    ENGINE_API void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset);
    ENGINE_API CBufferPtr GetIndexBuffer();

    ENGINE_API void ResetVertexBuffer();
    ENGINE_API void SetVertexBuffer(CBufferPtr _BufferSetPtr, bool _UseActiveInputLayout = true);
    ENGINE_API CBufferPtr GetVertexBuffer();

    ENGINE_API void ResetShaderVS();
    ENGINE_API void SetShaderVS(CShaderPtr _ShaderSetPtr);
    ENGINE_API CShaderPtr GetShaderVS();
    ENGINE_API void ResetShaderHS();
    ENGINE_API void SetShaderHS(CShaderPtr _ShaderSetPtr);
    ENGINE_API CShaderPtr GetShaderHS();
    ENGINE_API void ResetShaderDS();
    ENGINE_API void SetShaderDS(CShaderPtr _ShaderSetPtr);
    ENGINE_API CShaderPtr GetShaderDS();
    ENGINE_API void ResetShaderGS();
    ENGINE_API void SetShaderGS(CShaderPtr _ShaderSetPtr);
    ENGINE_API CShaderPtr GetShaderGS();
    ENGINE_API void ResetShaderPS();
    ENGINE_API void SetShaderPS(CShaderPtr _ShaderSetPtr);
    ENGINE_API CShaderPtr GetShaderSetPS();
    ENGINE_API void ResetShaderCS();
    ENGINE_API void SetShaderCS(CShaderPtr _ShaderSetPtr);
    ENGINE_API CShaderPtr GetShaderCS();

    ENGINE_API void ResetSampler(unsigned int _Unit);
    ENGINE_API void SetSampler(unsigned int _Unit, CSamplerPtr _SamplerPtr);
    ENGINE_API CSamplerPtr GetSampler(unsigned int _Unit);

    ENGINE_API void ResetTexture(unsigned int _Unit);
    ENGINE_API void SetTexture(unsigned int _Unit, CTexturePtr _TextureBasePtr);
    ENGINE_API CTexturePtr GetTexture(unsigned int _Unit);

    ENGINE_API void ResetImageTexture(unsigned int _Unit);
    ENGINE_API void SetImageTexture(unsigned int _Unit, CTexturePtr _TextureBasePtr);
    ENGINE_API CTexturePtr GetImageTexture(unsigned int _Unit);

    ENGINE_API void ResetConstantBuffer(unsigned int _Unit);
    ENGINE_API void SetConstantBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    ENGINE_API void SetConstantBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
    ENGINE_API CBufferPtr GetConstantBuffer(unsigned int _Unit);

    ENGINE_API void ResetResourceBuffer(unsigned int _Unit);
    ENGINE_API void SetResourceBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    ENGINE_API void SetResourceBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
    ENGINE_API CBufferPtr GetResourceBuffer(unsigned int _Unit);

	ENGINE_API void ResetAtomicCounterBuffer(unsigned int _Unit);
	ENGINE_API void SetAtomicCounterBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    ENGINE_API void SetAtomicCounterBufferRange(unsigned int _Unit, CBufferPtr _BufferPtr, unsigned int _Offset, unsigned int _Range);
	ENGINE_API CBufferPtr GetAtomicCounterBuffer(unsigned int _Unit);

    ENGINE_API void Flush();

    ENGINE_API void Barrier();

    ENGINE_API void Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex);
    ENGINE_API void DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation);
    ENGINE_API void DrawInstanced(unsigned int _NumberOfVertices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstVertex);
    ENGINE_API void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation);
    ENGINE_API void DrawIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset = 0);
    ENGINE_API void DrawIndexedIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset = 0);

    ENGINE_API void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ);
    ENGINE_API void DispatchIndirect(CBufferPtr _IndirectBufferPtr, unsigned int _Offset = 0);
} // namespace ContextManager
} // namespace Gfx
