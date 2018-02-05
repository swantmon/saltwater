
#include "mr/mr_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_pool.h"
#include "base/base_program_parameters.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_jni_interface.h"
#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_ar_controller_facet.h"
#include "data/data_camera_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_plugin_type.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_camera_interface.h"

#include "gui/gui_event_handler.h"

#include "mr/mr_control_manager.h"

#if PLATFORM_ANDROID
#include "arcore_c_api.h"

#include "GLES3/gl32.h"
#endif

#include <assert.h>
#include <unordered_set>
#include <string>
#include <vector>

#if PLATFORM_ANDROID

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
    const float k_Uvs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr char k_VertexShaderWebcam[] = R"(
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

    constexpr char k_FragmentShaderWebcam[] = R"(
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

    constexpr char k_VertexShaderPlane[] = R"(
        #version 320 es

        precision highp float;

        layout(location = 0) uniform mat4 m_MVP;

        layout(location = 0) in vec3 in_Vertex;

        layout(location = 0) out float out_Alpha;

        void main()
        {
          gl_Position = m_MVP * vec4(in_Vertex.x, in_Vertex.y, 0.0f, 1.0f);

          out_Alpha = in_Vertex.z;
        }
    )";

    constexpr char k_FragmentShaderPlane[] = R"(
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

    constexpr char k_VertexShaderPoint[] = R"(
        #version 320 es

        layout(location = 0) uniform mat4 m_MVP;

        layout(location = 0) in vec4 in_Vertex;

        void main()
        {
            gl_PointSize = 10.0;

            gl_Position = m_MVP * vec4(in_Vertex.xyz, 1.0);
        }
    )";

    constexpr char k_FragmentShaderPoint[] = R"(
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

    private:

        typedef std::unordered_set<ArAnchor*> CTrackedObjects;

    private:

        static std::string s_Permissions[];

    private:

        ArSession* m_pARSession;
        ArFrame* m_pARFrame;
        CTrackedObjects m_TrackedObjects;

        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionMatrix;

        Dt::CEntity* m_pEntity;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
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
        : m_pARSession        (0)
        , m_pARFrame          (0)
        , m_TrackedObjects    ()
        , m_ViewMatrix        (glm::mat4(1.0f))
        , m_ProjectionMatrix  (glm::mat4(1.0f))
        , m_pEntity           (0)
    {
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart(const SConfiguration& _rConfiguration)
    {
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CMRControlManager::OnEvent));

        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        ArStatus Status;

        Status = ArSession_create(_rConfiguration.m_pEnv, _rConfiguration.m_pContext, &m_pARSession);

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

        ArSession_setDisplayGeometry(m_pARSession, _rConfiguration.m_Rotation, _rConfiguration.m_Width, _rConfiguration.m_Height);

        // -----------------------------------------------------------------------------
        // OpenGLES
        // -----------------------------------------------------------------------------
        g_ShaderProgramWebcam = CreateProgram(k_VertexShaderWebcam, k_FragmentShaderWebcam);

        if (g_ShaderProgramWebcam == 0) BASE_THROWM("Failed creating shader capturing webcam image.")

        glGenTextures(1, &g_TextureID);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_TextureID);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenBuffers(1, &g_AttributeUVs);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

        glBufferData(GL_ARRAY_BUFFER, sizeof(k_Uvs), &k_Uvs, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        ArSession_setCameraTextureName(m_pARSession, g_TextureID);

        // -----------------------------------------------------------------------------

        g_ShaderProgramPlane = CreateProgram(k_VertexShaderPlane, k_FragmentShaderPlane);

        glGenBuffers(1, &g_AttributePlaneVertices);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributePlaneVertices);

        glBufferData(GL_ARRAY_BUFFER, s_MaxNumberOfVerticesPerPlane * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &g_PlaneIndices);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_PlaneIndices);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_MaxNumberOfVerticesPerPlane * 3 * sizeof(GLushort), 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // -----------------------------------------------------------------------------

        g_ShaderProgramPoint = CreateProgram(k_VertexShaderPoint, k_FragmentShaderPoint);

        glGenBuffers(1, &g_AttributePointVertices);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributePointVertices);

        glBufferData(GL_ARRAY_BUFFER, s_MaxNumberOfVerticesPerPoint * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
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
        ArStatus Result;

        Result = ArSession_update(m_pARSession, m_pARFrame);

        if (Result != AR_SUCCESS) return;

        // -----------------------------------------------------------------------------
        // Update camera
        // -----------------------------------------------------------------------------
        float Near = Base::CProgramParameters::GetInstance().GetFloat("mr:ar:camera:near", 0.1f);
        float Far  = Base::CProgramParameters::GetInstance().GetFloat("mr:ar:camera:far", 100.0f);

        ArCamera* pARCamera;

        ArFrame_acquireCamera(m_pARSession, m_pARFrame, &pARCamera);

        ArCamera_getViewMatrix(m_pARSession, pARCamera, &m_ViewMatrix[0][0]);

        ArCamera_getProjectionMatrix(m_pARSession, pARCamera, Near, Far, &m_ProjectionMatrix[0][0]);

        ArCamera_release(pARCamera);

        m_ViewMatrix.Transpose();

        m_ProjectionMatrix.Transpose();

        Gfx::Cam::SetViewMatrix(m_ViewMatrix);

        Gfx::Cam::SetProjectionMatrix(m_ProjectionMatrix, Near, Far);

        Gfx::Cam::Update();

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

            ArPose* pARPose = 0;

            ArPose_create(m_pARSession, 0, &pARPose);

            ArAnchor_getPose(m_pARSession, rObject, pARPose);

            ArPose_getMatrix(m_pARSession, pARPose, &ModelMatrix[0][0]);

            ModelMatrix.Transpose();

            if (m_pEntity != 0)
            {
                Dt::CTransformationFacet* pTransformation = m_pEntity->GetTransformationFacet();

                glm::vec3 Position;
                glm::mat3 Rotation;
                glm::vec3 EulerRotation;

                ModelMatrix.GetRotation(EulerRotation);

                ModelMatrix.GetTranslation(Position);

                pTransformation->SetPosition(Position);

                pTransformation->SetRotation(EulerRotation);

                Dt::EntityManager::MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove | Dt::CEntity::DirtyDetail);
            }

            ArPose_destroy(pARPose);
        }

        // TODO: render objects depending on the position / model matrix
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnPause()
    {
        ArSession_pause(m_pARSession);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnResume()
    {
        if(!Core::JNI::CheckPermission(s_Permissions[0]))
        {
            Core::JNI::AcquirePermissions(s_Permissions, 1);
        }

        ArSession_resume(m_pARSession);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        ArSession_setDisplayGeometry(m_pARSession, _DisplayRotation, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDraw()
    {
        int32_t HasGeometryChanged = 0;

        ArFrame_getDisplayGeometryChanged(m_pARSession, m_pARFrame, &HasGeometryChanged);

        if (HasGeometryChanged != 0 || g_IsUVsInitialized == false)
        {
            ArFrame_transformDisplayUvCoords(m_pARSession, m_pARFrame, s_NumberOfVertices * 2, k_Uvs, g_TransformedUVs);

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
        bool RenderPlanes = Base::CProgramParameters::GetInstance().GetBoolean("mr:ar:debug:render_planes", true);

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

            ArPose_getMatrix(m_pARSession, Pose, &PlaneModelMatrix[0][0]);

            ArPose_destroy(Pose);

            // -----------------------------------------------------------------------------
            // Get plane center in XZ axis.
            // -----------------------------------------------------------------------------
            glm::vec2 CenterOfPlane = glm::vec2(PlaneModelMatrix[3][0], PlaneModelMatrix[3][2]);

            // -----------------------------------------------------------------------------
            // Fill vertex 0 to 3, with alpha set to kAlpha.
            // -----------------------------------------------------------------------------
            for (int i = 0; i < NumberOfVertices; ++i)
            {
                glm::vec2 Direction = VerticesRAW[i] - CenterOfPlane;

                float Scale = 1.0f - std::min((kFeatherLength / 2.0f), kFeatherScale);

                glm::vec2 ResultVector = glm::vec2(Scale) * Direction + CenterOfPlane;

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
            ArTrackable* TrackableItem = nullptr;

            ArTrackableList_acquireItem(m_pARSession, ListOfPlanes, IndexOfPlane, &TrackableItem);

            ArPlane* Plane = ArAsPlane(TrackableItem);

            // -----------------------------------------------------------------------------
            // Generate planes and upload data
            // -----------------------------------------------------------------------------
            UpdateGeometryForPlane(Plane);

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
            glm::mat4 PlaneMVPMatrix = Gfx::Cam::GetProjectionMatrix() * Gfx::Cam::GetViewMatrix() * PlaneModelMatrix.GetTransposed() * glm::mat4().SetRotationX(Base::DegreesToRadians(-90.0f));

            glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            glEnable(GL_BLEND);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUseProgram(g_ShaderProgramPlane);

            glUniformMatrix4fv(0, 1, GL_TRUE, &(PlaneMVPMatrix[0][0]));

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

            ArTrackable_release(TrackableItem);
        }

        ArTrackableList_destroy(ListOfPlanes);

        ListOfPlanes = nullptr;

        // -----------------------------------------------------------------------------
        // Render planes
        // -----------------------------------------------------------------------------
        bool RenderPoints = Base::CProgramParameters::GetInstance().GetBoolean("mr:ar:debug:render_points", true);

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

                glm::mat4 PointMVPMatrix = Gfx::Cam::GetProjectionMatrix() * Gfx::Cam::GetViewMatrix();

                // -----------------------------------------------------------------------------
                // Draw
                // -----------------------------------------------------------------------------
                glUseProgram(g_ShaderProgramPoint);

                glUniformMatrix4fv(0, 1, GL_TRUE, &(PointMVPMatrix[0][0]));

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

    void CMRControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (_pEntity->GetCategory() != Dt::SEntityCategory::Actor) return;

        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            if (_pEntity->GetType() == Dt::SActorType::Node)
            {
                m_pEntity = _pEntity;
            }
        }
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
                for (int IndexOfHit = 0; IndexOfHit < NumberOfHits; ++IndexOfHit)
                {
                    ArHitResult* pHitResult = nullptr;

                    ArHitResult_create(m_pARSession, &pHitResult);

                    ArHitResultList_getItem(m_pARSession, pHitResultList, IndexOfHit, pHitResult);

                    if (pHitResult == nullptr)
                    {
                        return;
                    }

                    // -----------------------------------------------------------------------------
                    // Only consider planes for this sample app.
                    // -----------------------------------------------------------------------------
                    ArTrackable* pTrackable = nullptr;

                    ArHitResult_acquireTrackable(m_pARSession, pHitResult, &pTrackable);

                    ArTrackableType TrackableType = AR_TRACKABLE_NOT_VALID;

                    ArTrackable_getType(m_pARSession, pTrackable, &TrackableType);

                    if (TrackableType != AR_TRACKABLE_PLANE)
                    {
                        ArTrackable_release(pTrackable);

                        continue;
                    }

                    ArPose* pPose = nullptr;

                    ArPose_create(m_pARSession, nullptr, &pPose);

                    ArHitResult_getHitPose(m_pARSession, pHitResult, pPose);

                    int32_t IsPoseInPolygon = 0;

                    ArPlane* pPlane = ArAsPlane(pTrackable);

                    ArPlane_isPoseInPolygon(m_pARSession, pPlane, pPose, &IsPoseInPolygon);

                    ArTrackable_release(pTrackable);

                    ArPose_destroy(pPose);

                    if (!IsPoseInPolygon)
                    {
                        continue;
                    }

                    // -----------------------------------------------------------------------------
                    // Note that the application is responsible for releasing the pAnchor
                    // pointer after using it. Call ArAnchor_release(pAnchor) to release.
                    // -----------------------------------------------------------------------------
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

                        continue;
                    }

                    m_TrackedObjects.insert(pAnchor);

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

    void OnDraw()
    {
        CMRControlManager::GetInstance().OnDraw();
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
    }

    void OnDraw()
    {
    }
} // namespace ControlManager
} // namespace MR
#endif // !PLATFORM_ANDROID