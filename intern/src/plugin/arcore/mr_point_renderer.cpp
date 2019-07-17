
#include "plugin/arcore/mr_precompiled.h"

#include "base/base_coordinate_system.h"
#include "base/base_input_event.h"
#include "base/base_pool.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/engine.h"

#include "engine/core/core_plugin.h"
#include "engine/core/core_jni_interface.h"

#include "engine/data/data_ar_controller_component.h"
#include "engine/data/data_camera_component.h"
#include "engine/data/data_script_component.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "engine/gui/gui_event_handler.h"

#include "engine/script/script_ar_settings_script.h"

#include "plugin/arcore/mr_control_manager.h"
#include "plugin/arcore/mr_point_renderer.h"

#include <array>
#include <vector>

#include "arcore_c_api.h"

using namespace MR;
using namespace MR::PointRenderer;

namespace
{
    constexpr char c_VertexShaderPoint[] = R"(
        #version 320 es

        layout(std140, binding = 0) uniform UB0 { mat4 m_MVP; };

        layout(location = 0) in vec4 in_Vertex;

        void main()
        {
            gl_PointSize = 4.0;

            gl_Position = m_MVP * vec4(in_Vertex.xyz, 1.0);
        }
    )";

    constexpr char c_FragmentShaderPoint[] = R"(
        #version  320 es

        precision lowp float;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            out_Output = vec4(0.1215f, 0.7372f, 0.8235f, 1.0f);
        }
    )";

    static constexpr int s_MaxNumberOfPoints = 1024;
} // namespace

namespace
{
    class CMRPointRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRPointRenderer);

    public:

        CMRPointRenderer();
        ~CMRPointRenderer();

    public:

        void OnStart();
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

        void Render();

        const SSettings& GetSettings();
        void SetSettings(const SSettings& _rSettings);
        void ResetSettings();

    private:

        SSettings m_Settings;

        glm::mat3 m_ARCToEngineMatrix;

        Gfx::CShaderPtr m_PointVS;

        Gfx::CShaderPtr m_PointPS;

        Gfx::CBufferPtr m_PointVerticesBufferPtr;

        Gfx::CBufferPtr m_MatrixBufferPtr;

        Engine::CEventDelegates::HandleType m_RenderDelegateHandle;

        Dt::CComponentManager::CComponentDelegate::HandleType m_OnDirtyComponentDelegate;

    private:

        void OnDirtyComponent(Dt::IComponent* _pComponent);
    };
} // namespace

namespace
{
    CMRPointRenderer::CMRPointRenderer()
            : m_Settings         ( )
            , m_ARCToEngineMatrix(1.0f)
    {
        m_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));

        m_OnDirtyComponentDelegate = Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(std::bind(&CMRPointRenderer::OnDirtyComponent, this, std::placeholders::_1));
    }

    // -----------------------------------------------------------------------------

    CMRPointRenderer::~CMRPointRenderer()
    {
        m_OnDirtyComponentDelegate = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        m_PointVS = Gfx::ShaderManager::CompileVS(c_VertexShaderPoint, "main", nullptr, nullptr, 0, false, false, true);

        m_PointPS = Gfx::ShaderManager::CompilePS(c_FragmentShaderPoint, "main", nullptr, nullptr, 0, false, false, true);

        const Gfx::SInputElementDescriptor InputLayoutPoint[] =
                {
                        { "POINT", 0, Gfx::CInputLayout::Float4Format, 0, 0, 16, Gfx::CInputLayout::PerVertex, 0, },
                };

        Gfx::ShaderManager::CreateInputLayout(InputLayoutPoint, 1, m_PointVS);

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        Gfx::SBufferDescriptor ConstanteBufferDesc = { };

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_MaxNumberOfPoints * sizeof(glm::vec3);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_PointVerticesBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(glm::mat4);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_MatrixBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        // Settings
        // -----------------------------------------------------------------------------
        ResetSettings();

        m_RenderDelegateHandle = Engine::RegisterEventHandler(Engine::EEvent::Gfx_OnRenderForward, std::bind(&CMRPointRenderer::Render, this));
    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::OnExit()
    {
        m_PointVS                = 0;
        m_PointPS                = 0;
        m_PointVerticesBufferPtr = 0;
        m_MatrixBufferPtr        = 0;

        m_RenderDelegateHandle = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::OnPause()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::OnResume()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::Render()
    {
        if (!m_Settings.m_ShowPoints) return;

        ArSession* pARSession = MR::ControlManager::GetCurrentSession();
        ArFrame* pARFrame = MR::ControlManager::GetCurrentFrame();

        if (pARSession == nullptr || pARFrame == nullptr) return;

        Gfx::ContextManager::SetTargetSet(Gfx::TargetSetManager::GetLightAccumulationTargetSet());

        Gfx::ContextManager::SetBlendState(Gfx::StateManager::GetBlendState(Gfx::CBlendState::AlphaBlend));

        Gfx::ContextManager::SetDepthStencilState(Gfx::StateManager::GetDepthStencilState(Gfx::CDepthStencilState::NoWriteDepth));

        Gfx::ContextManager::SetRasterizerState(Gfx::StateManager::GetRasterizerState(Gfx::CRasterizerState::NoCull));

        Gfx::ContextManager::SetViewPortSet(Gfx::ViewManager::GetViewPortSet());

        ArPointCloud *pPointCloud = nullptr;

        ArStatus Status = ArFrame_acquirePointCloud(pARSession, pARFrame, &pPointCloud);

        if (Status == AR_SUCCESS)
        {
            // -----------------------------------------------------------------------------
            // Generate points and upload data
            // -----------------------------------------------------------------------------
            int NumberOfPoints = 0;

            ArPointCloud_getNumberOfPoints(pARSession, pPointCloud, &NumberOfPoints);

            if (NumberOfPoints > 0)
            {
                const float *pPointCloudData;

                ArPointCloud_getData(pARSession, pPointCloud, &pPointCloudData);

                auto MainCameraPtr = Gfx::ViewManager::GetMainCamera();

                glm::mat4 PointMVPMatrix = MainCameraPtr->GetViewProjectionMatrix() *
                                           glm::mat4(m_ARCToEngineMatrix);

                // -----------------------------------------------------------------------------
                // Upload
                // -----------------------------------------------------------------------------
                Gfx::BufferManager::UploadBufferData(m_PointVerticesBufferPtr, pPointCloudData, 0, sizeof(glm::vec4) * glm::min(NumberOfPoints, s_MaxNumberOfPoints));

                Gfx::BufferManager::UploadBufferData(m_MatrixBufferPtr, &PointMVPMatrix);

                // -----------------------------------------------------------------------------
                // Draw
                // -----------------------------------------------------------------------------
                Gfx::ContextManager::SetShaderVS(m_PointVS);

                Gfx::ContextManager::SetShaderPS(m_PointPS);

                Gfx::ContextManager::SetVertexBuffer(m_PointVerticesBufferPtr);

                Gfx::ContextManager::SetConstantBuffer(0, m_MatrixBufferPtr);

                Gfx::ContextManager::SetTopology(Gfx::STopology::PointList);

                Gfx::ContextManager::SetInputLayout(m_PointVS->GetInputLayout());

                Gfx::ContextManager::Draw(NumberOfPoints, 0);

                ArPointCloud_release(pPointCloud);
            }
        }

        Gfx::ContextManager::ResetShaderVS();

        Gfx::ContextManager::ResetShaderPS();

        Gfx::ContextManager::ResetVertexBuffer();

        Gfx::ContextManager::ResetIndexBuffer();

        Gfx::ContextManager::ResetInputLayout();

        Gfx::ContextManager::ResetTexture(0);

        Gfx::ContextManager::ResetViewPortSet();

        Gfx::ContextManager::ResetRenderContext();
    }

    // -----------------------------------------------------------------------------

    const SSettings& CMRPointRenderer::GetSettings()
    {
        return m_Settings;
    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::SetSettings(const SSettings& _rSettings)
    {
        m_Settings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::ResetSettings()
    {
        m_Settings.m_ShowPoints = Core::CProgramParameters::GetInstance().Get<bool>("mr:ar:debug:render_points", true);
    }

    // -----------------------------------------------------------------------------

    void CMRPointRenderer::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (!Base::CTypeInfo::IsEqual(_pComponent->GetTypeInfo(), Base::CTypeInfo::Get<Dt::CScriptComponent>())) return;

        auto* pScriptComponent = static_cast<Dt::CScriptComponent*>(_pComponent);

        if (!pScriptComponent->IsActiveAndUsable()) return;

        if (Base::CTypeInfo::IsEqual(pScriptComponent->GetScriptTypeInfo(), Base::CTypeInfo::Get<Scpt::CARSettingsScript>()))
        {
            auto pARSettings = static_cast<Scpt::CARSettingsScript*>(pScriptComponent);

            m_Settings.m_ShowPoints = pARSettings->m_RenderPoints;
        }
    }
} // namespace

namespace MR
{
namespace PointRenderer
{
    void OnStart()
    {
        CMRPointRenderer::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRPointRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRPointRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CMRPointRenderer::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CMRPointRenderer::GetInstance().OnResume();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        CMRPointRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SSettings& _rSettings)
    {
        CMRPointRenderer::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CMRPointRenderer::GetInstance().ResetSettings();
    }
} // namespace PointRenderer
} // namespace MR

extern "C" CORE_PLUGIN_API_EXPORT void SetPointRendererSettings(MR::PointRenderer::SSettings _Settings)
{
    MR::PointRenderer::SetSettings(_Settings);
}