
#include "mr/mr_precompiled.h"

#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_pool.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_ar_controller_facet.h"
#include "data/data_camera_actor_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_plugin_type.h"
#include "data/data_transformation_facet.h"

#include "mr/mr_control_manager.h"

#include "arcore_c_api.h"

#include "GLES3/gl3.h"
#define __gl2_h_
#include "GLES2/gl2ext.h"

#include <assert.h>
#include <unordered_set>
#include <vector>

using namespace MR;
using namespace MR::ControlManager;

namespace
{
    const float kVertices[] = {
            -1.0f, -1.0f, 0.0f, +1.0f, -1.0f, 0.0f,
            -1.0f, +1.0f, 0.0f, +1.0f, +1.0f, 0.0f,
    };

    const float kUvs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr char kVertexShader[] = R"(
    #version 100
    attribute vec4 vertex;
    attribute vec2 textureCoords;
    varying vec2 v_textureCoords;
    void main() {
      v_textureCoords = textureCoords;
      gl_Position = vertex;
    })";

    constexpr char kFragmentShader[] = R"(
    #version 100
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    uniform samplerExternalOES texture;
    varying vec2 v_textureCoords;
    void main() {
      gl_FragColor = texture2D(texture, v_textureCoords);
    })";

    static GLuint LoadShader(GLenum _Type, const char* _pSource)
    {
        GLuint Shader = glCreateShader(_Type);

        if (!Shader)
        {
            return Shader;
        }

        glShaderSource(Shader, 1, &_pSource, nullptr);

        glCompileShader(Shader);

        GLint compiled = 0;

        glGetShaderiv(Shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
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

            GLint link_status = GL_FALSE;

            glGetProgramiv(Program, GL_LINK_STATUS, &link_status);

            if (link_status != GL_TRUE)
            {
                glDeleteProgram(Program);

                Program = 0;
            }
        }

        return Program;
    }

    unsigned int s_ShaderProgram;
    unsigned int s_TextureID;
    unsigned int s_UniformTexture;
    unsigned int s_AttributeVertices;
    unsigned int s_AttributeUVs;

    static constexpr int s_NumberOfVertices = 4;
    bool s_IsUVsInitialized = false;
    float s_TransformedUVs[s_NumberOfVertices * 2];
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

        ArSession* m_pARSession;
        ArFrame* m_pARFrame;
        CTrackedObjects m_TrackedObjects;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CMRControlManager::CMRControlManager()
        : m_pARSession    (0)
        , m_pARFrame      (0)
        , m_TrackedObjects()
    {
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart(const SConfiguration& _rConfiguration)
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        ArStatus Status;

        Status = ArSession_create(_rConfiguration.m_pEnv, _rConfiguration.m_pContext, &m_pARSession);

        assert(Status == AR_SUCCESS);

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
        s_ShaderProgram = CreateProgram(kVertexShader, kFragmentShader);

        glGenTextures(1, &s_TextureID);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, s_TextureID);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        s_UniformTexture    = glGetUniformLocation(s_ShaderProgram, "texture");
        s_AttributeVertices = glGetAttribLocation(s_ShaderProgram, "vertex");
        s_AttributeUVs      = glGetAttribLocation(s_ShaderProgram, "textureCoords");

        ArSession_setCameraTextureName(m_pARSession, s_TextureID);
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
        glDeleteProgram(s_ShaderProgram);

        glDeleteTextures(1, &s_TextureID);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        ArStatus Result;

        Result = ArSession_update(m_pARSession, m_pARFrame);

        if (Result != AR_SUCCESS) BASE_CONSOLE_ERROR("ArSession_update error");

        // -----------------------------------------------------------------------------
        // Update camera
        // -----------------------------------------------------------------------------
        ArCamera* pARCamera;

        ArFrame_acquireCamera(m_pARSession, m_pARFrame, &pARCamera);

        Base::Float4x4 ViewMatrix       = Base::Float4x4::s_Identity;
        Base::Float4x4 ProjectionMatrix = Base::Float4x4::s_Identity;

        ArCamera_getViewMatrix(m_pARSession, pARCamera, &ViewMatrix[0][0]);

        ArCamera_getProjectionMatrix(m_pARSession, pARCamera, 0.1f, 100.0f, &ProjectionMatrix[0][0]);

        ArCamera_release(pARCamera);

        // TODO: send view and proj. matrix to camera

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
        Base::Float4x4 ModelMatrix = Base::Float4x4::s_Identity;

        for (const auto& rObject : m_TrackedObjects)
        {
            ArTrackingState TrackingState = AR_TRACKING_STATE_STOPPED;

            ArPose* pARPose = 0;

            ArPose_create(m_pARSession, 0, &pARPose);

            ArAnchor_getPose(m_pARSession, rObject, pARPose);

            ArPose_getMatrix(m_pARSession, pARPose, &ModelMatrix[0][0]);

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
        ArStatus Status = ArSession_resume(m_pARSession);

        assert(Status == AR_SUCCESS);
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

        if (HasGeometryChanged != 0 || s_IsUVsInitialized == false)
        {
            ArFrame_transformDisplayUvCoords(m_pARSession, m_pARFrame, s_NumberOfVertices * 2, kUvs, s_TransformedUVs);

            s_IsUVsInitialized = true;
        }

        glUseProgram(s_ShaderProgram);

        glDepthMask(GL_FALSE);

        glUniform1i(s_UniformTexture, 1);

        glActiveTexture(GL_TEXTURE1);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, s_TextureID);

        glEnableVertexAttribArray(s_AttributeVertices);

        glVertexAttribPointer(s_AttributeVertices, 3, GL_FLOAT, GL_FALSE, 0, kVertices);

        glEnableVertexAttribArray(s_AttributeUVs);

        glVertexAttribPointer(s_AttributeUVs, 2, GL_FLOAT, GL_FALSE, 0, s_TransformedUVs);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(s_AttributeUVs);

        glDisableVertexAttribArray(s_AttributeVertices);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

        glDepthMask(GL_TRUE);

        glUseProgram(0);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (_pEntity->GetCategory() != Dt::SEntityCategory::Plugin) return;

        unsigned int DirtyFlags;

        DirtyFlags = _pEntity->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CEntity::DirtyCreate) != 0)
        {
            // ...
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
