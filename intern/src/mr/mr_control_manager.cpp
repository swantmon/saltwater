
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

#include "mr/mr_camera_manager.h"
#include "mr/mr_control_manager.h"
#include "mr/mr_light_estimation_manager.h"
#include "mr/mr_marker_manager.h"
#include "mr/mr_session_manager.h"

#include <array>
#include <vector>

#include "arcore_c_api.h"

#if PLATFORM_ANDROID
#include "GLES3/gl32.h"

#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES   0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES 0x8D68
#endif /* GL_OES_EGL_image_external */
#endif

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

#if PLATFORM_ANDROID
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
#endif

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

        void OnStart();
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

        void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

        void OnDraw();

    private:

        static std::string s_Permissions[];

    private:

        class CInternSession : public CSession
        {
        public:

            ArSession* m_pARSession;
            ArFrame* m_pARFrame;

        private:

            friend class CMRControlManager;
        };

    private:

        bool m_InstallRequested;

        glm::mat3 m_ARCToEngineMatrix;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
        void OnDirtyComponent(Dt::IComponent* _pComponent);
    };
} // namespace

namespace
{
    CMRControlManager::CMRControlManager()
        : m_InstallRequested (false)
        , m_ARCToEngineMatrix(1.0f)
    {
        m_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart()
    {
#if PLATFORM_ANDROID
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

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, 0, GL_DYNAMIC_DRAW);

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
#endif

        // -----------------------------------------------------------------------------
        // Handler
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));

        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CMRControlManager::OnDirtyComponent));
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnExit()
    {
#if PLATFORM_ANDROID
        // -----------------------------------------------------------------------------
        // OpenGLES
        // -----------------------------------------------------------------------------
        glDeleteProgram(g_ShaderProgramWebcam);

        glDeleteTextures(1, &g_TextureID);
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        MR::SessionManager::SetTexture(g_TextureID);

        MR::SessionManager::Update();
        MR::CameraManager::Update();
        MR::MarkerManager::Update();
        MR::LightEstimationManager::Update();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnPause()
    {
        MR::SessionManager::OnPause();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnResume()
    {
#ifdef PLATFORM_ANDROID
        std::string s_Permissions[] = { "android.permission.CAMERA" };

        void* pEnvironment = Core::JNI::GetJavaEnvironment();
        void* pActivity    = Core::JNI::GetActivity();

        if(!Core::JNI::CheckPermission(s_Permissions[0]))
        {
            Core::JNI::AcquirePermissions(s_Permissions, 1);
        }

        if (pEnvironment != nullptr && pActivity != nullptr && Core::JNI::CheckPermission(s_Permissions[0]))
        {
            ArStatus Status;

            // -----------------------------------------------------------------------------
            // AR installation
            // -----------------------------------------------------------------------------
            ArInstallStatus InstallStatus;

            bool UserRequestedInstallation = !m_InstallRequested;

            Status = ArCoreApk_requestInstall(pEnvironment, pActivity, UserRequestedInstallation, &InstallStatus);

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
            // Create session
            // -----------------------------------------------------------------------------
            MR::SessionManager::Initialize();
        }
#endif

        MR::SessionManager::OnResume();
    }

    // -----------------------------------------------------------------------------


    void CMRControlManager::OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        SessionManager::OnDisplayGeometryChanged(static_cast<SessionManager::SDisplayRotation::EDisplayRotation>(_DisplayRotation), _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDraw()
    {
#if PLATFORM_ANDROID
        const CSession& rActiveSession = SessionManager::GetSession();

        if (rActiveSession.GetSessionState() != MR::CSession::Success) return;

        ArSession* pARSession = static_cast<ArSession*>(rActiveSession.GetSession());
        ArFrame*   pARFrame = static_cast<ArFrame*>(rActiveSession.GetFrame());

        // -----------------------------------------------------------------------------
        // Background
        // -----------------------------------------------------------------------------
        if (rActiveSession.HasGeometryChanged())
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MR::CSession::CUVs), &rActiveSession.GetTransformedUVs()[0]);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
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
            ArPose* Pose;

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
        ArTrackableList* ListOfPlanes = nullptr;

        ArTrackableList_create(pARSession, &ListOfPlanes);

        assert(ListOfPlanes != nullptr);

        ArSession_getAllTrackables(pARSession, AR_TRACKABLE_PLANE, ListOfPlanes);

        int NumberOfPlanes = 0;

        ArTrackableList_getSize(pARSession, ListOfPlanes, &NumberOfPlanes);

        // -----------------------------------------------------------------------------
        // Update every available plane
        // -----------------------------------------------------------------------------
        for (int IndexOfPlane = 0; IndexOfPlane < NumberOfPlanes; ++IndexOfPlane)
        {
            ArTrackable* pTrackableItem = nullptr;

            ArTrackableList_acquireItem(pARSession, ListOfPlanes, IndexOfPlane, &pTrackableItem);

            ArPlane* pPlane = ArAsPlane(pTrackableItem);

            ArTrackingState TrackableTrackingState;

            ArTrackable_getTrackingState(pARSession, pTrackableItem, &TrackableTrackingState);

            ArPlane* pSubsumedPlane;

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
                const float* pPointCloudData;

                ArPointCloud_getData(pARSession, pPointCloud, &pPointCloudData);

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
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        BASE_UNUSED(_pEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        BASE_UNUSED(_pComponent);
    }
} // namespace

namespace MR
{
namespace ControlManager
{
    void OnStart()
    {
        CMRControlManager::GetInstance().OnStart();
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
        return CMRControlManager::GetInstance().OnDraw();
    }
} // namespace ControlManager
} // namespace MR