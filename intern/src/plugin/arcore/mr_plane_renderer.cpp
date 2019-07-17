
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
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"
#include "engine/data/data_transformation_facet.h"
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
#include "plugin/arcore/mr_plane_renderer.h"

#include <array>
#include <vector>

#include "arcore_c_api.h"

using namespace MR;
using namespace MR::PlaneRenderer;

namespace
{
    glm::vec3 GetPlaneColor(int _Index)
    {
        static constexpr int s_NumberOfPlaneColors = 12;

        constexpr unsigned int s_PlaneColorRGB[s_NumberOfPlaneColors] = {
                0xFFFFFFFF, 0xF44336FF, 0xE91E63FF, 0x9C27B0FF, 0x673AB7FF, 0x3F51B5FF,
                0x2196F3FF, 0x03A9F4FF, 0x00BCD4FF, 0x009688FF, 0x4CAF50FF, 0x8BC34AFF};

        const int RGB = s_PlaneColorRGB[_Index % s_NumberOfPlaneColors];

        return glm::vec3(((RGB >> 24) & 0xff) / 255.0f, ((RGB >> 16) & 0xff) / 255.0f, ((RGB >> 8) & 0xff) / 255.0f);
    }

    constexpr char c_VertexShaderPlane[] = R"(
        #version 320 es

        precision highp float;

        layout(std140, binding = 0) uniform UB0 { mat4 m_MVP; };

        layout(location = 0) in vec3 in_Vertex;

        layout(location = 0) out float out_Alpha;

        void main()
        {
          gl_Position = m_MVP * vec4(in_Vertex.x, 0.0f, in_Vertex.y, 1.0f);

          out_Alpha = in_Vertex.z;
        }
    )";

    constexpr char c_FragmentShaderPlane[] = R"(
        #version 320 es

        precision highp float;

        layout(binding = 1) uniform UB1 { vec4 m_Color; };

        layout(location = 0) in float in_Alpha;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            vec2 PixelPos = gl_FragCoord.xy + 0.5f;

            float PatternMask = mod((PixelPos.x / 2.0f + PixelPos.y / 2.0f), 2.0f);

            out_Output = mix(m_Color, vec4(0.0f, 0.0f ,0.0f, 0.0f), PatternMask) * in_Alpha;
        }
    )";

    static constexpr int s_MaxNumberOfVerticesPerPlane = 1024;
} // namespace

namespace
{
    class CMRPlaneRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRPlaneRenderer);

    public:

        CMRPlaneRenderer();
        ~CMRPlaneRenderer();

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

        Gfx::CShaderPtr m_PlaneVS;

        Gfx::CShaderPtr m_PlanePS;

        Gfx::CBufferPtr m_PlaneVerticesBufferPtr;

        Gfx::CBufferPtr m_PlaneIndicesBufferPtr;

        Gfx::CBufferPtr m_MatrixBufferPtr;

        Gfx::CBufferPtr m_ColorBufferPtr;

        Engine::CEventDelegates::HandleType m_RenderDelegateHandle;

        Scpt::CARSettingsScript* m_pARSettings;
    };
} // namespace

namespace
{
    CMRPlaneRenderer::CMRPlaneRenderer()
        : m_Settings         ( )
        , m_ARCToEngineMatrix(1.0f)
        , m_pARSettings      (nullptr)
    {
        m_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));
    }

    // -----------------------------------------------------------------------------

    CMRPlaneRenderer::~CMRPlaneRenderer()
    {
        m_pARSettings = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader for background and debug
        // -----------------------------------------------------------------------------
        m_PlaneVS = Gfx::ShaderManager::CompileVS(c_VertexShaderPlane, "main", nullptr, nullptr, 0, false, false, true);

        m_PlanePS = Gfx::ShaderManager::CompilePS(c_FragmentShaderPlane, "main", nullptr, nullptr, 0, false, false, true);

        const Gfx::SInputElementDescriptor InputLayoutPlane[] =
        {
                { "VERTEX", 0, Gfx::CInputLayout::Float3Format, 0, 0, 12, Gfx::CInputLayout::PerVertex, 0, },
        };

        Gfx::ShaderManager::CreateInputLayout(InputLayoutPlane, 1, m_PlaneVS);

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        Gfx::SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_MaxNumberOfVerticesPerPlane * sizeof(glm::vec3);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_PlaneVerticesBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::IndexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_MaxNumberOfVerticesPerPlane * sizeof(unsigned int);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_PlaneIndicesBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(glm::mat4);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_MatrixBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(glm::vec4);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_ColorBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        // Settings
        // -----------------------------------------------------------------------------
        ResetSettings();

        m_RenderDelegateHandle = Engine::RegisterEventHandler(Engine::EEvent::Gfx_OnRenderForward, std::bind(&CMRPlaneRenderer::Render, this));
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::OnExit()
    {
        m_PlaneVS                = nullptr;
        m_PlanePS                = nullptr;
        m_PlaneVerticesBufferPtr = nullptr;
        m_PlaneIndicesBufferPtr  = nullptr;
        m_MatrixBufferPtr        = nullptr;
        m_ColorBufferPtr         = nullptr;

        m_RenderDelegateHandle = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::Update()
    {
        if (m_pARSettings != nullptr && m_pARSettings->IsActiveAndUsable()) return;

        m_pARSettings = nullptr;

        auto ScriptComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CScriptComponent>();

        for (auto ScriptComponent : ScriptComponents)
        {
            auto pScriptComponent = static_cast<Dt::CScriptComponent*>(ScriptComponent);

            if (!pScriptComponent->IsActiveAndUsable()) continue;

            if (Base::CTypeInfo::IsEqual(pScriptComponent->GetScriptTypeInfo(), Base::CTypeInfo::Get<Scpt::CARSettingsScript>()))
            {
                m_pARSettings = static_cast<Scpt::CARSettingsScript*>(pScriptComponent);

                break;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::OnPause()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::OnResume()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::Render()
    {
        if (!m_Settings.m_ShowPlanes) return;

        if (m_pARSettings != nullptr && !m_pARSettings->m_RenderPlanes) return;

        ArSession* pARSession = MR::ControlManager::GetCurrentSession();

        if (pARSession == nullptr) return;

        Gfx::ContextManager::SetTargetSet(Gfx::TargetSetManager::GetLightAccumulationTargetSet());

        Gfx::ContextManager::SetBlendState(Gfx::StateManager::GetBlendState(Gfx::CBlendState::AlphaBlend));

        Gfx::ContextManager::SetDepthStencilState(Gfx::StateManager::GetDepthStencilState(Gfx::CDepthStencilState::NoWriteDepth));

        Gfx::ContextManager::SetRasterizerState(Gfx::StateManager::GetRasterizerState(Gfx::CRasterizerState::NoCull));

        Gfx::ContextManager::SetViewPortSet(Gfx::ViewManager::GetViewPortSet());

        std::vector<glm::vec3> PlaneVertices;
        std::vector<unsigned int> PlaneIndices;

        glm::mat4 PlaneModelMatrix = glm::mat4(1.0f);

        auto UpdateGeometryForPlane = [&](const ArPlane *_pPlane)
        {
            // -----------------------------------------------------------------------------
            // Settings:
            // Feather distance 0.2 meters.
            // Feather scale over the distance between plane center and vertices.
            // -----------------------------------------------------------------------------
            const float kFeatherLength = 0.2f;
            const float kFeatherScale = 0.2f;
            const float kOuterAlpha = 0.8f;
            const float kInnerAlpha = 0.8f;

            // -----------------------------------------------------------------------------
            // The following code generates a triangle mesh filling a convex polygon,
            // including a feathered edge for blending.
            //
            // The indices shown in the diagram are used in comments below.
            // _______________     0_______________1
            // |             |      |4___________5|
            // |             |      | |         | |
            // |             | =>   | |         | |
            // |             |      | |         | |
            // |             |      |7-----------6|
            // ---------------     3---------------2
            // -----------------------------------------------------------------------------

            PlaneVertices.clear();
            PlaneIndices.clear();

            int LengthOfPolygon;

            ArPlane_getPolygonSize(pARSession, _pPlane, &LengthOfPolygon);

            int NumberOfVertices = LengthOfPolygon / 2;

            std::vector<glm::vec2> VerticesRAW(NumberOfVertices);

            ArPlane_getPolygon(pARSession, _pPlane, &VerticesRAW.front()[0]);

            // -----------------------------------------------------------------------------
            // Fill vertex 0 to 3. Note that the vertex.xy are used for x and z
            // position. vertex.z is used for alpha. The outter polygon's alpha
            // is 0.
            // -----------------------------------------------------------------------------
            for (int IndexOfVertex = 0; IndexOfVertex < NumberOfVertices; ++IndexOfVertex)
            {
                PlaneVertices.push_back(glm::vec3(VerticesRAW[IndexOfVertex][0], VerticesRAW[IndexOfVertex][1], kOuterAlpha));
            }

            // -----------------------------------------------------------------------------
            // Generate pose and get model matrix
            // -----------------------------------------------------------------------------
            ArPose *Pose;

            ArPose_create(pARSession, nullptr, &Pose);

            ArPlane_getCenterPose(pARSession, _pPlane, Pose);

            ArPose_getMatrix(pARSession, Pose, glm::value_ptr(PlaneModelMatrix));

            ArPose_destroy(Pose);

            // -----------------------------------------------------------------------------
            // Get plane center in XZ axis.
            // -----------------------------------------------------------------------------
            glm::vec2 CenterOfPlane = glm::vec2(PlaneModelMatrix[3][0], PlaneModelMatrix[3][2]);

            // -----------------------------------------------------------------------------
            // Fill vertex 0 to 3, with alpha set to kAlpha.
            // -----------------------------------------------------------------------------
            for (auto Vertex : VerticesRAW)
            {
                glm::vec2 Direction = Vertex - CenterOfPlane;

                float Scale = 1.0f - std::min((kFeatherLength / glm::length(Direction)), kFeatherScale);

                glm::vec2 ResultVector = Scale * Direction + CenterOfPlane;

                PlaneVertices.push_back(glm::vec3(ResultVector[0], ResultVector[1], kInnerAlpha));
            }

            // -----------------------------------------------------------------------------
            // Generate vertices / triangles
            // -----------------------------------------------------------------------------
            NumberOfVertices = PlaneVertices.size();

            int NumberOfVerticesHalf = NumberOfVertices / 2.0f;

            // -----------------------------------------------------------------------------
            // Generate triangle (4, 5, 6) and (4, 6, 7).
            // -----------------------------------------------------------------------------
            for (int IndexOfIndice = NumberOfVerticesHalf + 1; IndexOfIndice < NumberOfVertices - 1; ++IndexOfIndice)
            {
                PlaneIndices.push_back(NumberOfVerticesHalf);
                PlaneIndices.push_back(IndexOfIndice);
                PlaneIndices.push_back(IndexOfIndice + 1);
            }

            // -----------------------------------------------------------------------------
            // Generate triangle (0, 1, 4), (4, 1, 5), (5, 1, 2), (5, 2, 6),
            // (6, 2, 3), (6, 3, 7), (7, 3, 0), (7, 0, 4)
            // -----------------------------------------------------------------------------
            for (int IndexOfIndice = 0; IndexOfIndice < NumberOfVerticesHalf; ++IndexOfIndice)
            {
                PlaneIndices.push_back(IndexOfIndice);
                PlaneIndices.push_back((IndexOfIndice + 1) % NumberOfVerticesHalf);
                PlaneIndices.push_back(IndexOfIndice + NumberOfVerticesHalf);

                PlaneIndices.push_back(IndexOfIndice + NumberOfVerticesHalf);
                PlaneIndices.push_back((IndexOfIndice + 1) % NumberOfVerticesHalf);
                PlaneIndices.push_back((IndexOfIndice + NumberOfVerticesHalf + 1) % NumberOfVerticesHalf + NumberOfVerticesHalf);
            }
        };

        // -----------------------------------------------------------------------------
        // Get trackable planes
        // -----------------------------------------------------------------------------
        ArTrackableList *ListOfPlanes = nullptr;

        ArTrackableList_create(pARSession, &ListOfPlanes);

        assert(ListOfPlanes != nullptr);

        ArSession_getAllTrackables(pARSession, AR_TRACKABLE_PLANE, ListOfPlanes);

        int NumberOfPlanes = 0;

        ArTrackableList_getSize(pARSession, ListOfPlanes, &NumberOfPlanes);

        // -----------------------------------------------------------------------------
        // Update every available plane
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::SetShaderVS(m_PlaneVS);

        Gfx::ContextManager::SetShaderPS(m_PlanePS);

        Gfx::ContextManager::SetVertexBuffer(m_PlaneVerticesBufferPtr);

        Gfx::ContextManager::SetIndexBuffer(m_PlaneIndicesBufferPtr, 0);

        Gfx::ContextManager::SetConstantBuffer(0, m_MatrixBufferPtr);

        Gfx::ContextManager::SetConstantBuffer(1, m_ColorBufferPtr);

        Gfx::ContextManager::SetInputLayout(m_PlaneVS->GetInputLayout());

        Gfx::ContextManager::SetTopology(Gfx::STopology::TriangleList);

        for (int IndexOfPlane = 0; IndexOfPlane < NumberOfPlanes; ++IndexOfPlane)
        {
            ArTrackable *pTrackableItem = nullptr;

            ArTrackableList_acquireItem(pARSession, ListOfPlanes, IndexOfPlane, &pTrackableItem);

            ArPlane *pPlane = ArAsPlane(pTrackableItem);

            ArTrackingState TrackableTrackingState;

            ArTrackable_getTrackingState(pARSession, pTrackableItem, &TrackableTrackingState);

            ArPlane *pSubsumedPlane;

            ArPlane_acquireSubsumedBy(pARSession, pPlane, &pSubsumedPlane);

            if (pSubsumedPlane != nullptr)
            {
                ArTrackable_release(ArAsTrackable(pSubsumedPlane));

                continue;
            }

            if (TrackableTrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArTrackingState PlaneTrackingState;

            ArTrackable_getTrackingState(pARSession, ArAsTrackable(pPlane), &PlaneTrackingState);

            if (PlaneTrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArTrackable_release(pTrackableItem);

            // -----------------------------------------------------------------------------
            // Generate planes and upload data
            // -----------------------------------------------------------------------------
            UpdateGeometryForPlane(pPlane);

            if (PlaneIndices.size() == 0 || PlaneVertices.size() == 0) continue;

            if (PlaneVertices.size() >= s_MaxNumberOfVerticesPerPlane || PlaneIndices.size() >= s_MaxNumberOfVerticesPerPlane)
            {
                ENGINE_CONSOLE_WARNING("Plane could not be rendered because of too many vertices.");
                continue;
            }

            // -----------------------------------------------------------------------------
            // Prepare model-view-projection matrix
            // TODO: Change color depending on height of the plane
            // -----------------------------------------------------------------------------
            auto MainCameraPtr = Gfx::ViewManager::GetMainCamera();

            glm::mat4 PlaneMVPMatrix = MainCameraPtr->GetViewProjectionMatrix() * glm::mat4(m_ARCToEngineMatrix) * PlaneModelMatrix;

            glm::vec4 Color = glm::vec4(GetPlaneColor(IndexOfPlane), 1.0f);

            // -----------------------------------------------------------------------------
            // Upload data
            // -----------------------------------------------------------------------------
            Gfx::BufferManager::UploadBufferData(m_PlaneVerticesBufferPtr, &PlaneVertices.front()[0], 0, PlaneVertices.size() * sizeof(glm::vec3));

            Gfx::BufferManager::UploadBufferData(m_PlaneIndicesBufferPtr, &PlaneIndices.front(), 0, PlaneIndices.size() * sizeof(unsigned int));

            Gfx::BufferManager::UploadBufferData(m_MatrixBufferPtr, &PlaneMVPMatrix);

            Gfx::BufferManager::UploadBufferData(m_ColorBufferPtr, &Color);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            Gfx::ContextManager::DrawIndexed(PlaneIndices.size(), 0, 0);
        }

        ArTrackableList_destroy(ListOfPlanes);

        ListOfPlanes = nullptr;

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

    const SSettings& CMRPlaneRenderer::GetSettings()
    {
        return m_Settings;
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::SetSettings(const SSettings& _rSettings)
    {
        m_Settings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    void CMRPlaneRenderer::ResetSettings()
    {
        m_Settings.m_ShowPlanes = Core::CProgramParameters::GetInstance().Get<bool>("mr:ar:debug:render_planes", true);
    }
} // namespace

namespace MR
{
namespace PlaneRenderer
{
    void OnStart()
    {
        CMRPlaneRenderer::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRPlaneRenderer::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRPlaneRenderer::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CMRPlaneRenderer::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CMRPlaneRenderer::GetInstance().OnResume();
    }

    // -----------------------------------------------------------------------------

    void Render()
    {
        CMRPlaneRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    const SSettings& GetSettings()
    {
        return CMRPlaneRenderer::GetInstance().GetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SSettings& _rSettings)
    {
        CMRPlaneRenderer::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CMRPlaneRenderer::GetInstance().ResetSettings();
    }
} // namespace PlaneRenderer
} // namespace MR

extern "C" CORE_PLUGIN_API_EXPORT void SetPlaneRendererSettings(MR::PlaneRenderer::SSettings _Settings)
{
    MR::PlaneRenderer::SetSettings(_Settings);
}