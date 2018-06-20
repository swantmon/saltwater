
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/graphic/gfx_debug.h"

#include <stack>

using namespace Gfx;

namespace
{
    void GFX_APIENTRY OpenGLDebugCallback(GLenum _Source, GLenum _Type, GLuint _Id, GLenum _Severity, GLsizei _Length, const GLchar* _pMessage, const GLvoid* _pUserParam)
    {
        BASE_UNUSED(_Source);
        BASE_UNUSED(_Severity);
        BASE_UNUSED(_Length);
        BASE_UNUSED(_pUserParam);

        switch (_Type)
        {
        case GL_DEBUG_TYPE_ERROR:
            ENGINE_CONSOLE_ERRORV("%s (ID=%i)", _pMessage, _Id);
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            ENGINE_CONSOLE_WARNINGV("%s (ID=%i)", _pMessage, _Id);
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            ENGINE_CONSOLE_WARNINGV("%s (ID=%i)", _pMessage, _Id);
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            ENGINE_CONSOLE_INFOV("%s (ID=%i)", _pMessage, _Id);
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            ENGINE_CONSOLE_DEBUGV("%s (ID=%i)", _pMessage, _Id);
            break;
        case GL_DEBUG_TYPE_OTHER:
            // -----------------------------------------------------------------------------
            // Nothing to output here because that is only resource creation thing
            // -----------------------------------------------------------------------------
            break;
        }
    }
} // namespace 

namespace 
{
    class CGfxDebug : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxDebug)

    public:

        void OnStart();
        void OnExit();

        void Push(int _ID);
        void Pop();

    private:

        std::stack<GLuint> m_IDs;
    };
} // namespace 

namespace 
{
    void CGfxDebug::OnStart()
    {
        // -----------------------------------------------------------------------------
        // DEBUG
        // -----------------------------------------------------------------------------
#if APP_DEBUG_MODE == 1
        glDebugMessageCallback(OpenGLDebugCallback, NULL);

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    }

    // -----------------------------------------------------------------------------

    void CGfxDebug::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxDebug::Push(int _ID)
    {
        GLuint ID = static_cast<GLuint>(_ID);

        m_IDs.push(ID);

        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 1, &ID, GL_FALSE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 1, &ID, GL_FALSE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DONT_CARE, 1, &ID, GL_FALSE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PORTABILITY, GL_DONT_CARE, 1, &ID, GL_FALSE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 1, &ID, GL_FALSE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &ID, GL_FALSE);
    }

    // -----------------------------------------------------------------------------

    void CGfxDebug::Pop()
    {
        GLuint ID = m_IDs.top();

        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 1, &ID, GL_TRUE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 1, &ID, GL_TRUE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DONT_CARE, 1, &ID, GL_TRUE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PORTABILITY, GL_DONT_CARE, 1, &ID, GL_TRUE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 1, &ID, GL_TRUE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &ID, GL_TRUE);

        m_IDs.pop();
    }
} // namespace 

namespace Gfx
{
namespace Debug
{
    void OnStart()
    {
        CGfxDebug::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxDebug::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Push(int _ID)
    {
        CGfxDebug::GetInstance().Push(_ID);
    }

    // -----------------------------------------------------------------------------

    void Pop()
    {
        CGfxDebug::GetInstance().Pop();
    }
} // namespace Debug
} // namespace Gfx