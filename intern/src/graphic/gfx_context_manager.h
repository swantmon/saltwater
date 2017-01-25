
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
#include "graphic/gfx_texture_base.h"
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

    void ResetTargetSet();
    void SetTargetSet(CTargetSetPtr _TargetSetPtr);
    CTargetSetPtr GetTargetSet();

    void ResetViewPortSet();
    void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr);
    CViewPortSetPtr GetViewPortSet();

    void ResetInputLayout();
    void SetInputLayout(CInputLayoutPtr _InputLayoutPtr);
    CInputLayoutPtr GetInputLayout();

    void ResetIndexBuffer();
    void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Offset);
    void SetIndexBuffer(CBufferPtr _BufferPtr, unsigned int _Stride, unsigned int _Offset);
    CBufferPtr GetIndexBuffer();

    void ResetVertexBufferSet();
    void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pOffsets);
    void SetVertexBufferSet(CBufferSetPtr _BufferSetPtr, const unsigned int* _pStrides, const unsigned int* _pOffsets);
    CBufferSetPtr GetVertexBufferSet();

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

    void ResetSampler(unsigned int _Unit);
    void SetSampler(unsigned int _Unit, CSamplerPtr _SamplerPtr);
    CSamplerPtr GetSampler(unsigned int _Unit);

    void ResetTexture(unsigned int _Unit);
    void SetTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr);
    CTextureBasePtr GetTexture(unsigned int _Unit);

    void ResetImageTexture(unsigned int _Unit);
    void SetImageTexture(unsigned int _Unit, CTextureBasePtr _TextureBasePtr);
    CTextureBasePtr GetImageTexture(unsigned int _Unit);

    void ResetResourceBuffer(unsigned int _Unit);
    void SetResourceBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    CBufferPtr GetResourceBuffer(unsigned int _Unit);

    void ResetConstantBuffer(unsigned int _Unit);
    void SetConstantBuffer(unsigned int _Unit, CBufferPtr _BufferPtr);
    CBufferPtr GetConstantBuffer(unsigned int _Unit);

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

    void Draw(unsigned int _NumberOfVertices, unsigned int _IndexOfFirstVertex);
    void DrawIndexed(unsigned int _NumberOfIndices, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation);
    void DrawIndexedInstanced(unsigned int _NumberOfIndices, unsigned int _NumberOfInstances, unsigned int _IndexOfFirstIndex, int _BaseVertexLocation, unsigned int _StartInstanceLocation);
    
    void Dispatch(unsigned int _NumberOfThreadGroupsX, unsigned int _NumberOfThreadGroupsY, unsigned int _NumberOfThreadGroupsZ);
} // namespace ContextManager
} // namespace Gfx
