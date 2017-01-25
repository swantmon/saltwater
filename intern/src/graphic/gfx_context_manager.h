
#pragma once

#include "graphic/gfx_blend_state.h"
#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_depth_stencil_state.h"
#include "graphic/gfx_input_layout.h"
#include "graphic/gfx_rasterizer_state.h"
#include "graphic/gfx_render_context.h"
#include "graphic/gfx_render_state.h"
#include "graphic/gfx_sampler_set.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_texture_set.h"
#include "graphic/gfx_topology.h"
#include "graphic/gfx_view_port_set.h"

namespace Gfx
{
namespace ContextManager
{
    void OnStart();
    void OnExit();
    
    CRenderContextPtr CreateRenderContext();
    
    void ResetRenderContext();
    void SetRenderContext(CRenderContextPtr _RenderContextPtr);

    void SetRenderFlags(unsigned int _Flags);
    unsigned int GetRenderFlags();

    void ResetBlendState();
    void SetBlendState(CBlendStatePtr _StatePtr);
    CBlendStatePtr GetBlendState();

    void ResetDepthStencilState();
    void SetDepthStencilState(CDepthStencilStatePtr _StatePtr);
    CDepthStencilStatePtr GetDepthStencilState();

    void ResetRasterizerState();
    void SetRasterizerState(CRasterizerStatePtr _StatePtr);
    CRasterizerStatePtr GetRasterizerState();

    void ResetTopology();
    void SetTopology(STopology::Enum _Topology);
    STopology::Enum GetTopology();

    void ResetIndexBuffer();
    void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset);
    void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset);
    CBufferPtr GetIndexBuffer();

    void ResetVertexBufferSet();
    void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pOffsets);
    void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pStrides, const unsigned int* _pOffsets);
    CBufferSetPtr GetVertexBufferSet();

    void ResetConstantBufferSetVS();
    void SetConstantBufferSetVS(CBufferSetPtr _BufferSetPtr);
    CBufferSetPtr GetConstantBufferSetVS();
    void ResetConstantBufferSetHS();
    void SetConstantBufferSetHS(CBufferSetPtr _BufferSetPtr);
    CBufferSetPtr GetConstantBufferSetHS();
    void ResetConstantBufferSetDS();
    void SetConstantBufferSetDS(CBufferSetPtr _BufferSetPtr);
    CBufferSetPtr GetConstantBufferSetDS();
    void ResetConstantBufferSetGS();
    void SetConstantBufferSetGS(CBufferSetPtr _BufferSetPtr);
    CBufferSetPtr GetConstantBufferSetGS();
    void ResetConstantBufferSetPS();
    void SetConstantBufferSetPS(CBufferSetPtr _BufferSetPtr);
    CBufferSetPtr GetConstantBufferSetPS();
    void ResetConstantBufferSetCS();
    void SetConstantBufferSetCS(CBufferSetPtr _BufferSetPtr);
    CBufferSetPtr GetConstantBufferSetCS();

    void ResetInputLayout();
    void SetInputLayout(CInputLayoutPtr _InputLayoutPtr);
    CInputLayoutPtr GetInputLayout();
    
    void ResetShaderVS();
    void SetShaderVS(CShaderPtr _ShaderSetPtr);
    CShaderPtr GetShaderVS();
    void ResetShaderHS();
    void SetShaderHS(CShaderPtr _ShaderSetPtr);
    CShaderPtr GetShaderHS();
    void ResetShaderDS();
    void SetShaderDS(CShaderPtr _ShaderSetPtr);
    CShaderPtr GetShaderDS();
    void ResetShaderGS();
    void SetShaderGS(CShaderPtr _ShaderSetPtr);
    CShaderPtr GetShaderGS();
    void ResetShaderPS();
    void SetShaderPS(CShaderPtr _ShaderSetPtr);
    CShaderPtr GetShaderSetPS();
    void ResetShaderCS();
    void SetShaderCS(CShaderPtr _ShaderSetPtr);
    CShaderPtr GetShaderCS();

    void ResetSamplerSetVS();
    void SetSamplerSetVS(CSamplerSetPtr _SamplerSetPtr);
    CSamplerSetPtr GetSamplerSetVS();
    void ResetSamplerSetHS();
    void SetSamplerSetHS(CSamplerSetPtr _SamplerSetPtr);
    CSamplerSetPtr GetSamplerSetHS();
    void ResetSamplerSetDS();
    void SetSamplerSetDS(CSamplerSetPtr _SamplerSetPtr);
    CSamplerSetPtr GetSamplerSetDS();
    void ResetSamplerSetGS();
    void SetSamplerSetGS(CSamplerSetPtr _SamplerSetPtr);
    CSamplerSetPtr GetSamplerSetGS();
    void ResetSamplerSetPS();
    void SetSamplerSetPS(CSamplerSetPtr _SamplerSetPtr);
    CSamplerSetPtr GetSamplerSetPS();
    void ResetSamplerSetCS();
    void SetSamplerSetCS(CSamplerSetPtr _SamplerSetPtr);
    CSamplerSetPtr GetSamplerSetCS();

    void ResetTextureSetVS();
    void SetTextureSetVS(CTextureSetPtr _TextureSetPtr);
    CTextureSetPtr GetTextureSetVS();
    void ResetTextureSetHS();
    void SetTextureSetHS(CTextureSetPtr _TextureSetPtr);
    CTextureSetPtr GetTextureSetHS();
    void ResetTextureSetDS();
    void SetTextureSetDS(CTextureSetPtr _TextureSetPtr);
    CTextureSetPtr GetTextureSetDS();
    void ResetTextureSetGS();
    void SetTextureSetGS(CTextureSetPtr _TextureSetPtr);
    CTextureSetPtr GetTextureSetGS();
    void ResetTextureSetPS();
    void SetTextureSetPS(CTextureSetPtr _TextureSetPtr);
    CTextureSetPtr GetTextureSetPS();
    void ResetTextureSetCS();
    void SetTextureSetCS(CTextureSetPtr _TextureSetPtr);
    CTextureSetPtr GetTextureSetCS();

    void ResetTargetSet();
    void SetTargetSet(CTargetSetPtr _TargetSetPtr);
    CTargetSetPtr GetTargetSet();

    void ResetViewPortSet();
    void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr);
    CViewPortSetPtr GetViewPortSet();

    void Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex);
    void DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation);
    void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation);
    
    void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ);
} // namespace ContextManager
} // namespace Gfx
