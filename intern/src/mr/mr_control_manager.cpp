
#include "mr/mr_precompiled.h"

#include "base/base_coordinate_system.h"
#include "base/base_input_event.h"
#include "base/base_pool.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_jni_interface.h"

#include "data/data_ar_controller_component.h"
#include "data/data_camera_component.h"
#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_camera_interface.h"

#include "gui/gui_event_handler.h"

#include "mr/mr_control_manager.h"

#include <array>
#include <vector>

#if PLATFORM_ANDROID
#include "arcore_c_api.h"

#include "GLES3/gl32.h"

#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES   0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES 0x8D68
#endif /* GL_OES_EGL_image_external */

using namespace MR;
using namespace MR::ControlManager;

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

    const float c_Uvs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr char c_VertexShaderWebcam[] = R"(
        #version  320 es

        layout(location = 0) in vec2 in_UV;

        layout(location = 0) out vec2 out_UV;

        void main()
        {
            vec2 Vertices[4];

            Vertices[0] = vec2(-1.0f, -1.0f);
            Vertices[1] = vec2( 1.0f, -1.0f);
            Vertices[2] = vec2(-1.0f,  1.0f);
            Vertices[3] = vec2( 1.0f,  1.0f);

            out_UV = in_UV;

            gl_Position = vec4(Vertices[gl_VertexID], 0.0f, 1.0f);
        }
    )";

    constexpr char c_FragmentShaderWebcam[] = R"(
        #version 320 es

        #extension GL_OES_EGL_image_external_essl3 : require

        precision mediump float;

        layout(location = 0) uniform samplerExternalOES in_ExtOESTexture;

        layout(location = 0) in vec2 in_UV;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            out_Output = texture(in_ExtOESTexture, in_UV);
        }
    )";

    constexpr char c_VertexShaderPlane[] = R"(
        #version 320 es

        precision highp float;

        layout(location = 0) uniform mat4 m_MVP;

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

        layout(location = 1) uniform vec4 m_Color;

        layout(location = 0) in float in_Alpha;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            vec2 PixelPos = gl_FragCoord.xy + 0.5f;

            float PatternMask = mod((PixelPos.x / 2.0f + PixelPos.y / 2.0f), 2.0f);

            out_Output = mix(m_Color, vec4(0.0f, 0.0f ,0.0f, 0.0f), PatternMask) * in_Alpha;
        }
    )";

    constexpr char c_VertexShaderPoint[] = R"(
        #version 320 es

        layout(location = 0) uniform mat4 m_MVP;

        layout(location = 0) in vec4 in_Vertex;

        void main()
        {
            gl_PointSize = 10.0;

            gl_Position = m_MVP * vec4(in_Vertex.xyz, 1.0);
        }
    )";

    constexpr char c_FragmentShaderPoint[] = R"(
        #version 320 es

        precision lowp float;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            out_Output = vec4(0.1215f, 0.7372f, 0.8235f, 1.0f);
        }
    )";

    static GLuint LoadShader(GLenum _Type, const char* _pSource)
    {
        GLuint Shader = glCreateShader(_Type);

        if (!Shader)
        {
            return Shader;
        }

        glShaderSource(Shader, 1, &_pSource, nullptr);

        glCompileShader(Shader);

        GLint CompileStatus = 0;

        glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

        if (!CompileStatus)
        {
            GLint InfoLogLength = 0;

            glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLogLength);

            if (!InfoLogLength)
            {
                return Shader;
            }

            char* pBuffer = reinterpret_cast<char*>(malloc(InfoLogLength));

            if (!pBuffer)
            {
                return Shader;
            }

            glGetShaderInfoLog(Shader, InfoLogLength, nullptr, pBuffer);

            BASE_CONSOLE_WARNINGV("Could not compile shader in MR %d:\n%s\n", _Type, pBuffer);

            free(pBuffer);

            glDeleteShader(Shader);

            Shader = 0;
        }

        return Shader;
    }

    static unsigned int CreateProgram(const char* _pVertexSource, const char* _pFragmentSource)
    {
        unsigned int VertexShader = LoadShader(GL_VERTEX_SHADER, _pVertexSource);

        if (!VertexShader)
        {
            return 0;
        }

        unsigned int FragmentShader = LoadShader(GL_FRAGMENT_SHADER, _pFragmentSource);

        if (!FragmentShader)
        {
            return 0;
        }

        unsigned int Program = glCreateProgram();

        if (Program)
        {
            glAttachShader(Program, VertexShader);

            glAttachShader(Program, FragmentShader);

            glLinkProgram(Program);

            GLint LinkStatus= GL_FALSE;

            glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);

            if (LinkStatus != GL_TRUE)
            {
                glDeleteProgram(Program);

                Program = 0;
            }
        }

        return Program;
    }

    unsigned int g_ShaderProgramWebcam;
    unsigned int g_ShaderProgramPlane;
    unsigned int g_ShaderProgramPoint;
    unsigned int g_TextureID;
    unsigned int g_AttributeUVs;
    unsigned int g_AttributePlaneVertices;
    unsigned int g_AttributePointVertices;
    unsigned int g_PlaneIndices;

    static constexpr int s_MaxNumberOfVerticesPerPlane = 1024;
    static constexpr int s_MaxNumberOfVerticesPerPoint = 512;

    static constexpr int s_NumberOfVertices = 4;
    bool g_IsUVsInitialized = false;
    float g_TransformedUVs[s_NumberOfVertices * 2];
} // namespace

namespace
{
    class CMRControlManager : private Base::CUncopyable
    {
    BASE_SINGLETON_FUNC(CMRControlManager);

    public:

        CMRControlManager();
        ~CMRControlManager();

    public:

        void OnStart(const SConfiguration& _rConfiguration);
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

        void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

        void OnDraw();

        const CCamera& GetCamera();

    private:

        typedef std::vector<ArAnchor*> CTrackedObjects;

    private:

        static std::string s_Permissions[];

    private:

        class CInternCamera : public CCamera
        {
        private:

            friend class CMRControlManager;
        };

    private:

        bool m_InstallRequested;

        SConfiguration m_Configuration;

        ArSession* m_pARSession;
        ArFrame* m_pARFrame;
        CTrackedObjects m_TrackedObjects;

        CInternCamera m_Camera;

        Dt::CEntity* m_pEntity;

        glm::mat3 m_ARCToEngineMatrix;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
        void OnDirtyComponent(Dt::IComponent* _pComponent);
        void OnEvent(const Base::CInputEvent& _rEvent);
    };
} // namespace

namespace
{
    std::string CMRControlManager::s_Permissions[] = { "android.permission.CAMERA" };
}

namespace
{
    CMRControlManager::CMRControlManager()
        : m_InstallRequested(false)
        , m_Configuration    ( )
        , m_pARSession       (0)
        , m_pARFrame         (0)
        , m_TrackedObjects   ( )
        , m_pEntity          (0)
        , m_ARCToEngineMatrix(1.0f)
    {
        m_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart(const SConfiguration& _rConfiguration)
    {
        // -----------------------------------------------------------------------------
        // Save configuration
        // -----------------------------------------------------------------------------
        m_Configuration = _rConfiguration;

        // -----------------------------------------------------------------------------
        // OpenGLES
        // -----------------------------------------------------------------------------
        g_ShaderProgramWebcam = CreateProgram(c_VertexShaderWebcam, c_FragmentShaderWebcam);

        if (g_ShaderProgramWebcam == 0) BASE_THROWM("Failed creating shader capturing webcam image.")

        glGenTextures(1, &g_TextureID);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_TextureID);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenBuffers(1, &g_AttributeUVs);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

        glBufferData(GL_ARRAY_BUFFER, sizeof(c_Uvs), &c_Uvs, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // -----------------------------------------------------------------------------

        g_ShaderProgramPlane = CreateProgram(c_VertexShaderPlane, c_FragmentShaderPlane);

        glGenBuffers(1, &g_AttributePlaneVertices);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributePlaneVertices);

        glBufferData(GL_ARRAY_BUFFER, s_MaxNumberOfVerticesPerPlane * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &g_PlaneIndices);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_PlaneIndices);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_MaxNumberOfVerticesPerPlane * 3 * sizeof(GLushort), 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // -----------------------------------------------------------------------------

        g_ShaderProgramPoint = CreateProgram(c_VertexShaderPoint, c_FragmentShaderPoint);

        glGenBuffers(1, &g_AttributePointVertices);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributePointVertices);

        glBufferData(GL_ARRAY_BUFFER, s_MaxNumberOfVerticesPerPoint * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // -----------------------------------------------------------------------------
        // Handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CMRControlManager::OnEvent));

        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));

        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CMRControlManager::OnDirtyComponent));
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnExit()
    {
        m_TrackedObjects.clear();

        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        ArSession_destroy(m_pARSession);

        ArFrame_destroy(m_pARFrame);

        // -----------------------------------------------------------------------------
        // OpenGLES
        // -----------------------------------------------------------------------------
        glDeleteProgram(g_ShaderProgramWebcam);

        glDeleteTextures(1, &g_TextureID);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        if (m_pARSession == nullptr) return;

        ArStatus Result;

        ArSession_setCameraTextureName(m_pARSession, g_TextureID);

        Result = ArSession_update(m_pARSession, m_pARFrame);

        if (Result != AR_SUCCESS) return;

        // -----------------------------------------------------------------------------
        // Update camera
        // -----------------------------------------------------------------------------
        float Near = Base::CProgramParameters::GetInstance().Get<float>("mr:ar:camera:near", 0.1f);
        float Far  = Base::CProgramParameters::GetInstance().Get<float>("mr:ar:camera:far", 100.0f);
        glm::mat4 ViewMatrix(1.0f);
        glm::mat4 ProjectionMatrix(1.0f);

        ArCamera* pARCamera;

        ArFrame_acquireCamera(m_pARSession, m_pARFrame, &pARCamera);

        ArCamera_getViewMatrix(m_pARSession, pARCamera, glm::value_ptr(ViewMatrix));

        ArCamera_getProjectionMatrix(m_pARSession, pARCamera, Near, Far, glm::value_ptr(ProjectionMatrix));

        ArTrackingState CameraTrackingState;

        ArCamera_getTrackingState(m_pARSession, pARCamera, &CameraTrackingState);

        ArCamera_release(pARCamera);

        m_Camera.m_ViewMatrix       = ViewMatrix;
        m_Camera.m_ProjectionMatrix = ProjectionMatrix;
        m_Camera.m_Near             = Near;
        m_Camera.m_Far              = Far;

        switch(CameraTrackingState)
        {
            case AR_TRACKING_STATE_PAUSED:
                m_Camera.m_TrackingState = CCamera::Paused;
                break;
            case AR_TRACKING_STATE_STOPPED:
                m_Camera.m_TrackingState = CCamera::Stopped;
                break;
            case AR_TRACKING_STATE_TRACKING:
                m_Camera.m_TrackingState = CCamera::Tracking;
                break;
        }

        if (CameraTrackingState != AR_TRACKING_STATE_TRACKING) return;

        // -----------------------------------------------------------------------------
        // Light estimation
        // Intensity value ranges from 0.0f to 1.0f.
        // -----------------------------------------------------------------------------
        ArLightEstimate* ARLightEstimate;
        ArLightEstimateState ARLightEstimateState;

        ArLightEstimate_create(m_pARSession, &ARLightEstimate);

        ArFrame_getLightEstimate(m_pARSession, m_pARFrame, ARLightEstimate);

        ArLightEstimate_getState(m_pARSession, ARLightEstimate, &ARLightEstimateState);

        float LightIntensity = 0.8f;

        if (ARLightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_getPixelIntensity(m_pARSession, ARLightEstimate, &LightIntensity);
        }

        ArLightEstimate_destroy(ARLightEstimate);

        ARLightEstimate = nullptr;

        // TODO: use light estimation for our lighting

        // -----------------------------------------------------------------------------
        // Use tracked objects matrices
        // -----------------------------------------------------------------------------
        glm::mat4 ModelMatrix = glm::mat4(1.0f);

        for (const auto& rObject : m_TrackedObjects)
        {
            ArTrackingState TrackingState = AR_TRACKING_STATE_STOPPED;

            ArAnchor_getTrackingState(m_pARSession, rObject, &TrackingState);

            if (TrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArPose* pARPose = 0;

            ArPose_create(m_pARSession, 0, &pARPose);

            ArAnchor_getPose(m_pARSession, rObject, pARPose);

            ArPose_getMatrix(m_pARSession, pARPose, glm::value_ptr(ModelMatrix));

            ArPose_destroy(pARPose);

            if (m_pEntity != 0)
            {
                Dt::CTransformationFacet* pTransformation = m_pEntity->GetTransformationFacet();

                pTransformation->SetPosition(glm::mat4(m_ARCToEngineMatrix) * ModelMatrix[3]);

                pTransformation->SetRotation(glm::eulerAngles(glm::toQuat(glm::mat3(ModelMatrix))));

                Dt::EntityManager::MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnPause()
    {
        if (m_pARSession == nullptr) return;

        ArSession_pause(m_pARSession);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnResume()
    {
        if(!Core::JNI::CheckPermission(s_Permissions[0]))
        {
            Core::JNI::AcquirePermissions(s_Permissions, 1);
        }

        if (m_pARSession == nullptr && m_Configuration.m_pEnv != nullptr && m_Configuration.m_pActivity != nullptr && Core::JNI::CheckPermission(s_Permissions[0]))
        {
            ArStatus Status;

            // -----------------------------------------------------------------------------
            // AR installation
            // -----------------------------------------------------------------------------
            ArInstallStatus InstallStatus;

            bool UserRequestedInstallation = !m_InstallRequested;

            Status = ArCoreApk_requestInstall(m_Configuration.m_pEnv, m_Configuration.m_pActivity, UserRequestedInstallation, &InstallStatus);

            if (Status != AR_SUCCESS) return;

            switch (InstallStatus)
            {
                case AR_INSTALL_STATUS_INSTALLED:
                    break;
                case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                {
                    BASE_CONSOLE_INFO("ArCore is not installed on this device.");

                    m_InstallRequested = true;

                    return;
                } break;
            }

            // -----------------------------------------------------------------------------
            // AR session and frame
            // -----------------------------------------------------------------------------
            Status = ArSession_create(m_Configuration.m_pEnv, m_Configuration.m_pContext, &m_pARSession);

            if (Status != AR_SUCCESS) BASE_THROWM("Application has to be closed because of unsupported ArCore.");

            assert(m_pARSession != 0);

            ArConfig* ARConfig = 0;

            ArConfig_create(m_pARSession, &ARConfig);

            assert(ARConfig != 0);

            Status = ArSession_checkSupported(m_pARSession, ARConfig);

            assert(Status == AR_SUCCESS);

            Status = ArSession_configure(m_pARSession, ARConfig);

            assert(Status == AR_SUCCESS);

            ArConfig_destroy(ARConfig);

            ArFrame_create(m_pARSession, &m_pARFrame);

            assert(m_pARFrame != 0);

            ArSession_setDisplayGeometry(m_pARSession, m_Configuration.m_Rotation, m_Configuration.m_Width, m_Configuration.m_Height);
        }

        if (m_pARSession != nullptr)
        {
            ArSession_resume(m_pARSession);
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        ArSession_setDisplayGeometry(m_pARSession, _DisplayRotation, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDraw()
    {
        if (m_pARSession == nullptr) return;

        // -----------------------------------------------------------------------------
        // Background
        // -----------------------------------------------------------------------------
        int32_t HasGeometryChanged = 0;

        ArFrame_getDisplayGeometryChanged(m_pARSession, m_pARFrame, &HasGeometryChanged);

        if (HasGeometryChanged != 0 || g_IsUVsInitialized == false)
        {
            ArFrame_transformDisplayUvCoords(m_pARSession, m_pARFrame, s_NumberOfVertices * 2, c_Uvs, g_TransformedUVs);

            glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_TransformedUVs), &g_TransformedUVs);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            g_IsUVsInitialized = true;
        }

        glUseProgram(g_ShaderProgramWebcam);

        glDisable(GL_DEPTH_TEST);

        glDisable(GL_BLEND);

        glDisable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_TextureID);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

        glUseProgram(0);

        // -----------------------------------------------------------------------------
        // Render planes
        // -----------------------------------------------------------------------------
        bool RenderPlanes = Base::CProgramParameters::GetInstance().Get<bool>("mr:ar:debug:render_planes", true);

        if (RenderPlanes == false) return;

        std::vector<glm::vec3> PlaneVertices;
        std::vector<GLushort> PlaneIndices;

        glm::mat4 PlaneModelMatrix = glm::mat4(1.0f);

        auto UpdateGeometryForPlane = [&](const ArPlane* _pPlane)
        {
            // -----------------------------------------------------------------------------
            // Settings:
            // Feather distance 0.2 meters.
            // Feather scale over the distance between plane center and vertices.
            // -----------------------------------------------------------------------------
            const float kFeatherLength = 0.2f;
            const float kFeatherScale  = 0.2f;
            const float kOuterAlpha    = 0.8f;
            const float kInnerAlpha    = 0.8f;

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
            PlaneIndices .clear();

            int LengthOfPolygon;

            ArPlane_getPolygonSize(m_pARSession, _pPlane, &LengthOfPolygon);

            int NumberOfVertices = LengthOfPolygon / 2;

            std::vector<glm::vec2> VerticesRAW(NumberOfVertices);

            ArPlane_getPolygon(m_pARSession, _pPlane, &VerticesRAW.front()[0]);

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
            ArPose* Pose;

            ArPose_create(m_pARSession, nullptr, &Pose);

            ArPlane_getCenterPose(m_pARSession, _pPlane, Pose);

            ArPose_getMatrix(m_pARSession, Pose, glm::value_ptr(PlaneModelMatrix));

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
        ArTrackableList* ListOfPlanes = nullptr;

        ArTrackableList_create(m_pARSession, &ListOfPlanes);

        assert(ListOfPlanes != nullptr);

        ArSession_getAllTrackables(m_pARSession, AR_TRACKABLE_PLANE, ListOfPlanes);

        int NumberOfPlanes = 0;

        ArTrackableList_getSize(m_pARSession, ListOfPlanes, &NumberOfPlanes);

        // -----------------------------------------------------------------------------
        // Update every available plane
        // -----------------------------------------------------------------------------
        for (int IndexOfPlane = 0; IndexOfPlane < NumberOfPlanes; ++IndexOfPlane)
        {
            ArTrackable* pTrackableItem = nullptr;

            ArTrackableList_acquireItem(m_pARSession, ListOfPlanes, IndexOfPlane, &pTrackableItem);

            ArPlane* pPlane = ArAsPlane(pTrackableItem);

            ArTrackingState TrackableTrackingState;

            ArTrackable_getTrackingState(m_pARSession, pTrackableItem, &TrackableTrackingState);

            ArPlane* pSubsumedPlane;

            ArPlane_acquireSubsumedBy(m_pARSession, pPlane, &pSubsumedPlane);

            if (pSubsumedPlane != nullptr)
            {
                ArTrackable_release(ArAsTrackable(pSubsumedPlane));

                continue;
            }

            if (TrackableTrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArTrackingState PlaneTrackingState;

            ArTrackable_getTrackingState(m_pARSession, ArAsTrackable(pPlane), &PlaneTrackingState);

            if (PlaneTrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArTrackable_release(pTrackableItem);

            // -----------------------------------------------------------------------------
            // Generate planes and upload data
            // -----------------------------------------------------------------------------
            UpdateGeometryForPlane(pPlane);

            if (PlaneIndices.size() == 0 || PlaneVertices.size() == 0) continue;

            if (PlaneVertices.size() >= s_MaxNumberOfVerticesPerPlane)
            {
                BASE_CONSOLE_WARNING("Plane could not be rendered because of too many vertices.");
                continue;
            }

            glBindBuffer(GL_ARRAY_BUFFER, g_AttributePlaneVertices);

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * PlaneVertices.size(), &PlaneVertices.front()[0]);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_PlaneIndices);

            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * PlaneIndices.size(), &PlaneIndices.front());

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            // -----------------------------------------------------------------------------
            // Prepare model-view-projection matrix
            // TODO: Change color depending on height of the plane
            // -----------------------------------------------------------------------------
            glm::mat4 PlaneMVPMatrix = Gfx::Cam::GetProjectionMatrix() * Gfx::Cam::GetViewMatrix() * glm::mat4(m_ARCToEngineMatrix) * PlaneModelMatrix;

            glm::vec4 Color = glm::vec4(GetPlaneColor(IndexOfPlane), 1.0f);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            glEnable(GL_BLEND);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUseProgram(g_ShaderProgramPlane);

            glUniformMatrix4fv(0, 1, GL_FALSE, &(PlaneMVPMatrix[0][0]));

            glUniform4fv(1, 1, &(Color[0]));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_PlaneIndices);

            glBindBuffer(GL_ARRAY_BUFFER, g_AttributePlaneVertices);

            glEnableVertexAttribArray(0);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glDrawElements(GL_TRIANGLES, PlaneIndices.size(), GL_UNSIGNED_SHORT, 0);

            glDisableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glUseProgram(0);

            glDisable(GL_BLEND);
        }

        ArTrackableList_destroy(ListOfPlanes);

        ListOfPlanes = nullptr;

        // -----------------------------------------------------------------------------
        // Render planes
        // -----------------------------------------------------------------------------
        bool RenderPoints = Base::CProgramParameters::GetInstance().Get<bool>("mr:ar:debug:render_points", true);

        if (RenderPoints == false) return;

        ArPointCloud* pPointCloud = nullptr;

        ArStatus Status = ArFrame_acquirePointCloud(m_pARSession, m_pARFrame, &pPointCloud);

        if (Status == AR_SUCCESS)
        {
            // -----------------------------------------------------------------------------
            // Generate points and upload data
            // -----------------------------------------------------------------------------
            int NumberOfPoints = 0;

            ArPointCloud_getNumberOfPoints(m_pARSession, pPointCloud, &NumberOfPoints);

            if (NumberOfPoints > 0)
            {
                const float* pPointCloudData;

                ArPointCloud_getData(m_pARSession, pPointCloud, &pPointCloudData);

                glBindBuffer(GL_ARRAY_BUFFER, g_AttributePointVertices);

                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * NumberOfPoints, pPointCloudData);

                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glm::mat4 PointMVPMatrix = Gfx::Cam::GetProjectionMatrix() * Gfx::Cam::GetViewMatrix() * glm::mat4(m_ARCToEngineMatrix);

                // -----------------------------------------------------------------------------
                // Draw
                // -----------------------------------------------------------------------------
                glUseProgram(g_ShaderProgramPoint);

                glUniformMatrix4fv(0, 1, GL_FALSE, &(PointMVPMatrix[0][0]));

                glBindBuffer(GL_ARRAY_BUFFER, g_AttributePointVertices);

                glEnableVertexAttribArray(0);

                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

                glDrawArrays(GL_POINTS, 0, NumberOfPoints);

                glDisableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glUseProgram(0);

                ArPointCloud_release(pPointCloud);
            }
        }
    }

    // -----------------------------------------------------------------------------

    const CCamera& CMRControlManager::GetCamera()
    {
        return m_Camera;
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        auto DirtyFlag = _pEntity->GetDirtyFlags();

        if ((DirtyFlag & Dt::CEntity::DirtyAdd) != 0)
        {
            if (_pEntity->GetName() == "Box")
            {
                m_pEntity = _pEntity;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        BASE_UNUSED(_pComponent);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnEvent(const Base::CInputEvent& _rEvent)
    {
        if (_rEvent.GetAction() == Base::CInputEvent::TouchPressed)
        {
            Base::CInputEvent::EKey Key = static_cast<Base::CInputEvent::EKey>(_rEvent.GetKey());

            float x = _rEvent.GetCursorPosition()[0];
            float y = _rEvent.GetCursorPosition()[1];

            if (m_pARFrame != nullptr && m_pARSession != nullptr)
            {
                ArHitResultList* pHitResultList = 0;

                ArHitResultList_create(m_pARSession, &pHitResultList);

                assert(pHitResultList);

                ArFrame_hitTest(m_pARSession, m_pARFrame, x, y, pHitResultList);

                int NumberOfHits = 0;

                ArHitResultList_getSize(m_pARSession, pHitResultList, &NumberOfHits);

                // -----------------------------------------------------------------------------
                // The hitTest method sorts the resulting list by distance from the camera,
                // increasing.  The first hit result will usually be the most relevant when
                // responding to user input
                // -----------------------------------------------------------------------------
                ArHitResult* pHitResult = nullptr;

                for (int IndexOfHit = 0; IndexOfHit < NumberOfHits; ++IndexOfHit)
                {
                    ArHitResult* pEstimatedHitResult = nullptr;

                    ArHitResult_create(m_pARSession, &pEstimatedHitResult);

                    ArHitResultList_getItem(m_pARSession, pHitResultList, IndexOfHit, pEstimatedHitResult);

                    if (pEstimatedHitResult == nullptr)
                    {
                        return;
                    }

                    // -----------------------------------------------------------------------------
                    // Get trackables
                    // -----------------------------------------------------------------------------
                    ArTrackable* pTrackable = nullptr;

                    ArHitResult_acquireTrackable(m_pARSession, pEstimatedHitResult, &pTrackable);

                    ArTrackableType TrackableType = AR_TRACKABLE_NOT_VALID;

                    ArTrackable_getType(m_pARSession, pTrackable, &TrackableType);

                    switch (TrackableType)
                    {
                        case AR_TRACKABLE_PLANE:
                        {
                            ArPose* pPose = nullptr;

                            ArPose_create(m_pARSession, nullptr, &pPose);

                            ArHitResult_getHitPose(m_pARSession, pEstimatedHitResult, pPose);

                            int32_t IsPoseInPolygon = 0;

                            ArPlane* pPlane = ArAsPlane(pTrackable);

                            ArPlane_isPoseInPolygon(m_pARSession, pPlane, pPose, &IsPoseInPolygon);

                            ArTrackable_release(pTrackable);

                            ArPose_destroy(pPose);

                            if (!IsPoseInPolygon)
                            {
                                continue;
                            }

                            pHitResult = pEstimatedHitResult;
                        } break;

                        case AR_TRACKABLE_POINT:
                        {
                            ArPoint* pPoint = ArAsPoint(pTrackable);

                            ArPointOrientationMode OrientationMode;

                            ArPoint_getOrientationMode(m_pARSession, pPoint, &OrientationMode);

                            if (OrientationMode == AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL)
                            {
                                pHitResult = pEstimatedHitResult;
                            }
                        } break;

                        default:
                        {
                            ArTrackable_release(pTrackable);

                            BASE_CONSOLE_INFO("Undefined trackable type found.")
                        }
                    };
                }

                if (pHitResult != nullptr)
                {
                    ArAnchor* pAnchor = nullptr;

                    if (ArHitResult_acquireNewAnchor(m_pARSession, pHitResult, &pAnchor) != AR_SUCCESS)
                    {
                        return;
                    }

                    ArTrackingState TrackingState = AR_TRACKING_STATE_STOPPED;

                    ArAnchor_getTrackingState(m_pARSession, pAnchor, &TrackingState);

                    if (TrackingState != AR_TRACKING_STATE_TRACKING)
                    {
                        ArAnchor_release(pAnchor);

                        return;
                    }

                    m_TrackedObjects.push_back(pAnchor);

                    ArHitResult_destroy(pHitResult);

                    pHitResult = nullptr;
                }

                ArHitResultList_destroy(pHitResultList);

                pHitResultList = nullptr;
            }
        }
    }
} // namespace

namespace MR
{
namespace ControlManager
{
    void OnStart(const SConfiguration& _rConfiguration)
    {
        CMRControlManager::GetInstance().OnStart(_rConfiguration);
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRControlManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRControlManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CMRControlManager::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CMRControlManager::GetInstance().OnResume();
    }

    // -----------------------------------------------------------------------------

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        CMRControlManager::GetInstance().OnDisplayGeometryChanged(_DisplayRotation, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void OnDraw()
    {
        CMRControlManager::GetInstance().OnDraw();
    }

    // -----------------------------------------------------------------------------

    const CCamera& GetCamera()
    {
        return CMRControlManager::GetInstance().GetCamera();
    }
} // namespace ControlManager
} // namespace MR
#else // PLATFORM_ANDROID
namespace MR
{
namespace ControlManager
{
    void OnStart(const SConfiguration& _rConfiguration)
    {
        BASE_UNUSED(_rConfiguration);
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
    }

    // -----------------------------------------------------------------------------

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        BASE_UNUSED(_DisplayRotation);
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }

    // -----------------------------------------------------------------------------

    void OnDraw()
    {
    }

    // -----------------------------------------------------------------------------

    const CCamera& GetCamera()
    {
        return CCamera();
    }
} // namespace ControlManager
} // namespace MR
#endif // !PLATFORM_ANDROID