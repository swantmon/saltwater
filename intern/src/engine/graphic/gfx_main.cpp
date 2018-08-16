
#include "engine/engine_precompiled.h"
#include "engine/engine_config.h"

#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/data/data_map.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include <unordered_set>
#include <vector>

using namespace Gfx;

namespace
{
    class CGfxMain : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxMain)
        
    public:
        
        CGfxMain();
        ~CGfxMain();
        
    public:
        
        void OnStart();
        void OnExit();
        
    public:
        
        void RegisterResizeHandler(Gfx::Main::CResizeDelegate _NewDelgate);
        
    public:

        unsigned int RegisterWindow(void* _pWindow, int _VSync);

        unsigned int GetNumberOfWindows();

        void ActivateWindow(unsigned int _WindowID);

        const glm::ivec2& GetActiveWindowSize();
        const glm::ivec2& GetWindowSize(unsigned int _WindowID);

        const glm::ivec2& GetActiveNativeWindowSize();
        const glm::ivec2& GetNativeWindowSize(unsigned int _WindowID);

        void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

        void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);

        const CGraphicsInfo& GetGraphicsAPI();
        bool IsExtensionAvailable(const std::string& _Name);
        
    public:
        
        void BeginFrame();
        void EndFrame();
        
    public:
        
        void CreatePerFrameConstantBuffers();
        void DestroyPerFrameConstantBuffers();
        void UploadPerFrameConstantBuffers();
        
        CBufferPtr GetPerFrameConstantBuffer();

    private:

        static const unsigned int s_MaxNumberOfWindows = 4;
        
    private:

        class CInternGraphicsInfo : public CGraphicsInfo
        {
        public:

            enum EPixelMatching
            {
                PixelPerfect,       //< The resolution of the renderer is equal to the native size
                Scale,              //< The resolution of the renderer is scaled
                Fix,                //< The resolution of the renderer is fixed
            };

            EPixelMatching m_PixelMatching;

        };

        struct SWindowInfo
        {
            void* m_pNativeWindowHandle;

#ifdef PLATFORM_ANDROID
            EGLDisplay m_EglDisplay;
            EGLConfig  m_EglConfig;
            EGLSurface m_EglSurface;
            EGLContext m_EglContext;
#else
            HDC   m_pNativeDeviceContextHandle;
            HGLRC m_pNativeOpenGLContextHandle;
#endif
            glm::ivec2 m_InternalWindowSize;
            glm::ivec2 m_NativeWindowSize;
            int        m_VSync;
        };
        
        struct SPerFrameConstantBuffer
        {
            glm::mat4 m_WorldToScreen;
            glm::mat4 m_WorldToQuad;
            glm::mat4 m_WorldToView;
            glm::mat4 m_ViewToScreen;
            glm::mat4 m_ScreenToView;
            glm::mat4 m_ViewToWorld;
            glm::vec4   m_ViewPosition;
            glm::vec4   m_ViewDirection;
            glm::mat4 m_PreviousWorldToView;
            glm::mat4 m_PreviousViewToScreen;
            glm::mat4 m_PreviousScreenToView;
            glm::mat4 m_PreviousViewToWorld;
            glm::vec4   m_PreviousViewPosition;
            glm::vec4   m_PreviousViewDirection;
            glm::vec4   m_InvertedScreensizeAndScreensize;
            glm::vec4   m_ScreenPositionScaleBias;
            glm::vec4   m_CameraParameters0;
            glm::vec4   m_WorldParameters0;
            glm::vec4   m_FrameParameters0;
        };
        
    private:
        
        typedef std::vector<Gfx::Main::CResizeDelegate> CResizeDelegates;
        typedef CResizeDelegates::iterator              CResizeDelegateIterator;
        
    private:

        CInternGraphicsInfo m_GraphicsInfo;

        SWindowInfo  m_WindowInfos[s_MaxNumberOfWindows];
        SWindowInfo* m_pActiveWindowInfo;
        unsigned int m_NumberOfWindows;

        CResizeDelegates m_ResizeDelegates;

        SPerFrameConstantBuffer m_PerFrameConstantBuffer;

        CBufferPtr m_PerFrameConstantBufferBufferPtr;

        std::unordered_set<std::string> m_AvailableExtensions;

    private:

        void SetWindowSize(SWindowInfo* _pWindowInfo, int _Width, int _Height);
    };
} // namespace

namespace
{
    CGfxMain::CGfxMain()
        : m_pActiveWindowInfo              (0)
        , m_NumberOfWindows                (0)
        , m_ResizeDelegates                ()
        , m_PerFrameConstantBuffer         ()
        , m_PerFrameConstantBufferBufferPtr()
    {
        m_GraphicsInfo.m_GraphicsAPI   = CGraphicsInfo::UndefinedAPI;
        m_GraphicsInfo.m_MajorVersion  = 0;
        m_GraphicsInfo.m_MinorVersion  = 0;
        m_GraphicsInfo.m_PixelMatching = CInternGraphicsInfo::PixelPerfect;
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxMain::~CGfxMain()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Load graphics API settings
        // -----------------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
        m_GraphicsInfo.m_GraphicsAPI  = Core::CProgramParameters::GetInstance().Get("graphics:api:type", CGraphicsInfo::OpenGLES);
        m_GraphicsInfo.m_MajorVersion = Core::CProgramParameters::GetInstance().Get("graphics:api:major_version", 3);
        m_GraphicsInfo.m_MinorVersion = Core::CProgramParameters::GetInstance().Get("graphics:api:minor_version", 2);
#else
        m_GraphicsInfo.m_GraphicsAPI  = Core::CProgramParameters::GetInstance().Get("graphics:api:type", CGraphicsInfo::OpenGL);
        m_GraphicsInfo.m_MajorVersion = Core::CProgramParameters::GetInstance().Get("graphics:api:major_version", 4);
        m_GraphicsInfo.m_MinorVersion = Core::CProgramParameters::GetInstance().Get("graphics:api:minor_version", 5);
#endif        

        // -----------------------------------------------------------------------------
        // Load pixel matching behavior
        // -----------------------------------------------------------------------------
        m_GraphicsInfo.m_PixelMatching = static_cast<CInternGraphicsInfo::EPixelMatching>(Core::CProgramParameters::GetInstance().Get("graphics:pixel_matching:type", 0));

        // -----------------------------------------------------------------------------
        // Show information of windows and initialize them
        // -----------------------------------------------------------------------------
        unsigned int IndexOfWindow = 0;

        for (IndexOfWindow = 0; IndexOfWindow < m_NumberOfWindows; ++IndexOfWindow)
        {
            SWindowInfo& rWindowInfo = m_WindowInfos[IndexOfWindow];

#ifdef PLATFORM_ANDROID
            EGLNativeWindowType  pNativeWindowHandle;
            EGLBoolean           Status;
            EGLint               Error;

            // -----------------------------------------------------------------------------
            // Check OpenGLES
            // -----------------------------------------------------------------------------
            if (m_GraphicsInfo.m_GraphicsAPI != CGraphicsInfo::OpenGLES)
            {
                BASE_THROWM("Only OpenGLES is supported on Android devices. Please change graphics API in the config file.")
            }

            if (!(m_GraphicsInfo.m_MajorVersion >= 3 && m_GraphicsInfo.m_MinorVersion >= 2))
            {
                BASE_THROWM("Lower versions as OpenGLES 3.2 is not supported.")
            }

            // -----------------------------------------------------------------------------
            // Cast data
            // -----------------------------------------------------------------------------
            pNativeWindowHandle = static_cast<EGLNativeWindowType>(rWindowInfo.m_pNativeWindowHandle);

            // -----------------------------------------------------------------------------
            // Create OpenGLES
            // -----------------------------------------------------------------------------
            rWindowInfo.m_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

            if (rWindowInfo.m_EglDisplay == EGL_NO_DISPLAY)
            {
                BASE_THROWM("Failed to get an EGLDisplay.");
            }

            Status = eglInitialize(rWindowInfo.m_EglDisplay, 0, 0);

            if (Status == EGL_FALSE)
            {
                BASE_THROWM("Failed to initialize the EGLDisplay.");
            }

            // -----------------------------------------------------------------------------
            // Configuration
            // -----------------------------------------------------------------------------
            const EGLint ConfigAttributes[] =
            {
                EGL_SURFACE_TYPE,    EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                EGL_BLUE_SIZE,       8,
                EGL_GREEN_SIZE,      8,
                EGL_RED_SIZE,        8,
                EGL_DEPTH_SIZE,      24,
                EGL_NONE
            };

            EGLint NumConfigs;

            eglChooseConfig(rWindowInfo.m_EglDisplay, ConfigAttributes, &rWindowInfo.m_EglConfig, 1, &NumConfigs);

            if (NumConfigs != 1)
            {
                BASE_THROWM("Failed to choose config.");
            }

            // -----------------------------------------------------------------------------
            // Format
            // -----------------------------------------------------------------------------
            EGLint Format;

            Status = eglGetConfigAttrib(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglConfig, EGL_NATIVE_VISUAL_ID, &Format);

            if (Status == EGL_FALSE)
            {
                BASE_THROWM("Failed to get native visual ID.");
            }

            ANativeWindow_setBuffersGeometry(pNativeWindowHandle, 0, 0, Format);

            // -----------------------------------------------------------------------------
            // Surface
            // -----------------------------------------------------------------------------
            rWindowInfo.m_EglSurface = eglCreateWindowSurface(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglConfig, pNativeWindowHandle, NULL);

            Error = eglGetError();

            if (rWindowInfo.m_EglSurface == EGL_NO_SURFACE || Error != EGL_SUCCESS)
            {
                BASE_THROWV("Unable to create surface because of %i.", Error);
            }

            // -----------------------------------------------------------------------------
            // Context
            // -----------------------------------------------------------------------------
            EGLint ContextAttributes[] =
            {
                EGL_CONTEXT_CLIENT_VERSION, m_GraphicsInfo.m_MajorVersion,
                EGL_NONE
            };

            rWindowInfo.m_EglContext = eglCreateContext(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglConfig, EGL_NO_CONTEXT, ContextAttributes);

            Error = eglGetError();

            if (rWindowInfo.m_EglContext == EGL_NO_CONTEXT || Error != EGL_SUCCESS)
            {
                BASE_THROWV("Unable to create context because of %i.", Error);
            }

            // -----------------------------------------------------------------------------
            // Make current
            // -----------------------------------------------------------------------------
            Status = eglMakeCurrent(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglSurface, rWindowInfo.m_EglSurface, rWindowInfo.m_EglContext);

            Error = eglGetError();

            if (Status == EGL_FALSE || Error != EGL_SUCCESS)
            {
                BASE_THROWV("Unable to set current EGL stuff because of %i.", Error);
            }

            // -----------------------------------------------------------------------------
            // Get native resolution
            // -----------------------------------------------------------------------------
            int Width, Height;

            eglQuerySurface(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglSurface, EGL_WIDTH, &Width);
            eglQuerySurface(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglSurface, EGL_HEIGHT, &Height);

            SetWindowSize(m_pActiveWindowInfo, Width, Height);
#else
            HWND  pNativeWindowHandle;
            HDC   pNativeDeviceContextHandle;
            HGLRC pNativeOpenGLContextHandle;
            HGLRC pDummyNativeOpenGLContextHandle;
            int   Format;

            const PIXELFORMATDESCRIPTOR PixelFormatDesc =
            {
                sizeof(PIXELFORMATDESCRIPTOR),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //< Flags
                PFD_TYPE_RGBA,                                                 //< RGBA or palette.
                32,                                                            //< Depth of color frame buffer.
                0, 0, 0, 0, 0, 0,
                0,
                0,
                0,
                0, 0, 0, 0,
                32,                                                            //< Number of bits for the depth buffer
                8,                                                             //< Number of bits for the stencil buffer
                0,                                                             //< Number of Aux buffers in the frame buffer.
                PFD_MAIN_PLANE,
                0,
                0, 0, 0
            };

            // -----------------------------------------------------------------------------
            // Create OpenGL specific stuff with dummy context
            // -----------------------------------------------------------------------------
            pNativeWindowHandle = static_cast<HWND>(rWindowInfo.m_pNativeWindowHandle);

            pNativeDeviceContextHandle = ::GetDC(pNativeWindowHandle);

            Format = ChoosePixelFormat(pNativeDeviceContextHandle, &PixelFormatDesc);

            SetPixelFormat(pNativeDeviceContextHandle, Format, &PixelFormatDesc);

            pDummyNativeOpenGLContextHandle = ::wglCreateContext(pNativeDeviceContextHandle);

            if (pDummyNativeOpenGLContextHandle == 0)
            {
                BASE_THROWM("OpenGL dummy context creation failed.");
            }

            wglMakeCurrent(pNativeDeviceContextHandle, pDummyNativeOpenGLContextHandle);

            // -----------------------------------------------------------------------------
            // Check if OpenGLES is available on desktop graphics card
            // -----------------------------------------------------------------------------
            if (m_GraphicsInfo.m_GraphicsAPI == CGraphicsInfo::OpenGLES)
            {
                typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC hdc);
                PROC Function = wglGetProcAddress("wglGetExtensionsStringARB");
                PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(Function);

                std::string wglExtensions = wglGetExtensionsStringARB(pNativeDeviceContextHandle);

                std::size_t found = wglExtensions.find("WGL_EXT_create_context_es2_profile");
                bool GLESAvailable = found != std::string::npos;

                if (!GLESAvailable)
                {
                    BASE_THROWM("OpenGL ES 3.2 extension on desktop is not available! Please change graphics API back to \"gl\" inside config.");
                }
            }

            // -----------------------------------------------------------------------------
            // Activate GLEW. GLEW is an extension manager which handles all different
            // OpenGL extensions.
            //
            // glewExperimental defines a possible option for extensions in experimental
            // state. It is needed to start this on while a dummy context is created.
            // Depending on this dummy context GLEW initialize possible functionality.
            // -----------------------------------------------------------------------------
            glewExperimental = GL_TRUE;

            GLenum res = glewInit();

            if (res != GLEW_OK)
            {
                BASE_THROWV("GLEW can't be initialized on this system because '%s'", glewGetErrorString(res));
            }

            // -----------------------------------------------------------------------------
            // Create final OpenGL context with attributes
            // -----------------------------------------------------------------------------
            const int Attributes[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, m_GraphicsInfo.m_MajorVersion,
                WGL_CONTEXT_MINOR_VERSION_ARB, m_GraphicsInfo.m_MinorVersion,
                WGL_CONTEXT_PROFILE_MASK_ARB , m_GraphicsInfo.m_GraphicsAPI == CGraphicsInfo::OpenGLES ? WGL_CONTEXT_ES2_PROFILE_BIT_EXT : WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef ENGINE_DEBUG_MODE
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#else
                WGL_CONTEXT_FLAGS_ARB, 0,
#endif // APP_DEBUG_MODE
                0,
            };

            pNativeOpenGLContextHandle = ::wglCreateContextAttribsARB(pNativeDeviceContextHandle, 0, Attributes);

            if (pNativeDeviceContextHandle == 0)
            {
                BASE_THROWM("OpenGL context creation failed.");
            }

            wglMakeCurrent(pNativeDeviceContextHandle, pNativeOpenGLContextHandle);

            // -----------------------------------------------------------------------------
            // VSync
            // -----------------------------------------------------------------------------
            wglSwapIntervalEXT(rWindowInfo.m_VSync);

            // -----------------------------------------------------------------------------
            // Clip Control
            // -----------------------------------------------------------------------------
#ifdef GLM_FORCE_DEPTH_ZERO_TO_ONE
            glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
#endif

            // -----------------------------------------------------------------------------
            // Get native resolution
            // -----------------------------------------------------------------------------
            RECT WindowRect;

            GetWindowRect(pNativeWindowHandle, &WindowRect);

            int Width  = glm::abs(WindowRect.right - WindowRect.left);
            int Height = glm::abs(WindowRect.bottom - WindowRect.top);

            SetWindowSize(m_pActiveWindowInfo, Width, Height);

            // -----------------------------------------------------------------------------
            // Save created data
            // -----------------------------------------------------------------------------
            rWindowInfo.m_pNativeDeviceContextHandle = pNativeDeviceContextHandle;
            rWindowInfo.m_pNativeOpenGLContextHandle = pNativeOpenGLContextHandle;
#endif

            // -----------------------------------------------------------------------------
            // Check specific OpenGL(ES) versions and availability
            // -----------------------------------------------------------------------------
            const unsigned char* pInfoGLVersion     = glGetString(GL_VERSION);                  //< Returns a version or release number.
            const unsigned char* pInfoGLVendor      = glGetString(GL_VENDOR);                   //< Returns the company responsible for this GL implementation. This name does not change from release to release.
            const unsigned char* pInfoGLRenderer    = glGetString(GL_RENDERER);                 //< Returns the name of the renderer. This name is typically specific to a particular configuration of a hardware platform. It does not change from release to release.
            const unsigned char* pInfoGLGLSLVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); //< Returns a version or release number for the shading language.

            assert(pInfoGLVersion && pInfoGLGLSLVersion && pInfoGLVendor && pInfoGLRenderer);

            ENGINE_CONSOLE_INFOV("Window ID: %i", IndexOfWindow);
            ENGINE_CONSOLE_INFOV("GL:        %s", pInfoGLVersion);
            ENGINE_CONSOLE_INFOV("GLSL:      %s", pInfoGLGLSLVersion);
            ENGINE_CONSOLE_INFOV("Vendor:    %s", pInfoGLVendor);
            ENGINE_CONSOLE_INFOV("Renderer:  %s", pInfoGLRenderer);

            // -----------------------------------------------------------------------------
            // Extensions
            // -----------------------------------------------------------------------------
            GLint ExtensionCount;
            glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);

            for (int i = 0; i < ExtensionCount; ++i)
            {
                std::string Name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));

                m_AvailableExtensions.insert(Name);
            }
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::OnExit()
    {
#ifdef PLATFORM_ANDROID
        for (SWindowInfo& rWindowInfo : m_WindowInfos)
        {
            eglMakeCurrent(rWindowInfo.m_pNativeWindowHandle, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            eglTerminate(rWindowInfo.m_pNativeWindowHandle);
        }
#endif // PLATFORM_ANDROID
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::RegisterResizeHandler(Gfx::Main::CResizeDelegate _NewDelgate)
    {
        m_ResizeDelegates.push_back(_NewDelgate);
    }

    // -----------------------------------------------------------------------------

    unsigned int CGfxMain::RegisterWindow(void* _pWindow, int _VSync)
    {
        if (_pWindow == 0 || m_NumberOfWindows == s_MaxNumberOfWindows) return 0;

        // -----------------------------------------------------------------------------
        // Save data to new window
        // -----------------------------------------------------------------------------
        SWindowInfo& rNewWindow = m_WindowInfos[m_NumberOfWindows];

        rNewWindow.m_pNativeWindowHandle        = _pWindow;
        rNewWindow.m_VSync                      = _VSync;

        ++ m_NumberOfWindows;

        return m_NumberOfWindows - 1;
    }

    // -----------------------------------------------------------------------------

    unsigned int CGfxMain::GetNumberOfWindows()
    {
        return m_NumberOfWindows;
    }

    // -----------------------------------------------------------------------------

    void CGfxMain::ActivateWindow(unsigned int _WindowID)
    {
        if (_WindowID >= m_NumberOfWindows) return;

        m_pActiveWindowInfo = &m_WindowInfos[_WindowID];
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& CGfxMain::GetActiveWindowSize()
    {
        assert(m_pActiveWindowInfo != 0);

        return m_pActiveWindowInfo->m_InternalWindowSize;
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& CGfxMain::GetWindowSize(unsigned int _WindowID)
    {
        assert(_WindowID < m_NumberOfWindows);

        return m_WindowInfos[_WindowID].m_InternalWindowSize;
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& CGfxMain::GetActiveNativeWindowSize()
    {
        assert(m_pActiveWindowInfo != 0);

        return m_pActiveWindowInfo->m_NativeWindowSize;
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& CGfxMain::GetNativeWindowSize(unsigned int _WindowID)
    {
        assert(_WindowID < m_NumberOfWindows);

        return m_WindowInfos[_WindowID].m_NativeWindowSize;
    }

    // -----------------------------------------------------------------------------

    void CGfxMain::OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height)
    {
        assert(_Width > 0 && _Height > 0);

        if (_WindowID >= m_NumberOfWindows) return;

        // -----------------------------------------------------------------------------
        // Setup window info
        // -----------------------------------------------------------------------------
        SetWindowSize(&m_WindowInfos[_WindowID], _Width, _Height);

        // -----------------------------------------------------------------------------
        // Send to every delegate that resize has changed
        // -----------------------------------------------------------------------------
        CResizeDelegateIterator EndOfDelegates = m_ResizeDelegates.end();

        for (CResizeDelegateIterator CurrentDelegate = m_ResizeDelegates.begin(); CurrentDelegate != EndOfDelegates; ++CurrentDelegate)
        {
            (*CurrentDelegate)(m_WindowInfos[_WindowID].m_InternalWindowSize[0], m_WindowInfos[_WindowID].m_InternalWindowSize[1]);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxMain::TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile)
    {
        BASE_UNUSED(_WindowID);
        BASE_UNUSED(_pPathToFile);

        ENGINE_CONSOLE_WARNING("Taking screenshots is currently not supported!");

        /*assert(_WindowID < m_NumberOfWindows);

        unsigned int Width;
        unsigned int Height;
        char*        pPixels;

        // -----------------------------------------------------------------------------
        // Read pixels from last frame buffer
        // -----------------------------------------------------------------------------
        SWindowInfo& rWindowInfo = m_WindowInfos[_WindowID];

        Width  = rWindowInfo.m_NativeWindowSize[0];
        Height = rWindowInfo.m_NativeWindowSize[1];

        pPixels = static_cast<char*>(Base::CMemory::Allocate(3 * Width * Height * sizeof(char)));

        glReadPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, pPixels);

        // -----------------------------------------------------------------------------
        // Create texture
        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Free data
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pPixels);*/
    }
    
    // -----------------------------------------------------------------------------

    const CGraphicsInfo&  CGfxMain::GetGraphicsAPI()
    {
        return m_GraphicsInfo;
    }

    // -----------------------------------------------------------------------------

    bool CGfxMain::IsExtensionAvailable(const std::string& _Name)
    {
        return m_AvailableExtensions.count(_Name) == 1;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxMain::BeginFrame()
    {
        assert(m_pActiveWindowInfo != 0);

        SWindowInfo& rWindowInfo = *m_pActiveWindowInfo;

#ifdef PLATFORM_ANDROID
        eglMakeCurrent(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglSurface, rWindowInfo.m_EglSurface, rWindowInfo.m_EglContext);
#else
        wglMakeCurrent(rWindowInfo.m_pNativeDeviceContextHandle, rWindowInfo.m_pNativeOpenGLContextHandle);
#endif

        Gfx::TargetSetManager::ClearTargetSet(Gfx::TargetSetManager::GetSystemTargetSet());
        Gfx::TargetSetManager::ClearTargetSet(Gfx::TargetSetManager::GetDefaultTargetSet(), 1.0f);
        Gfx::TargetSetManager::ClearTargetSet(Gfx::TargetSetManager::GetDeferredTargetSet());
        Gfx::TargetSetManager::ClearTargetSet(Gfx::TargetSetManager::GetLightAccumulationTargetSet());
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::EndFrame()
    {
        assert(m_pActiveWindowInfo != 0);

        SWindowInfo& rWindowInfo = *m_pActiveWindowInfo;

#ifdef PLATFORM_ANDROID
        eglSwapBuffers(rWindowInfo.m_EglDisplay, rWindowInfo.m_EglSurface);
#else
        SwapBuffers(rWindowInfo.m_pNativeDeviceContextHandle);
#endif
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::CreatePerFrameConstantBuffers()
    {
        assert(m_PerFrameConstantBufferBufferPtr == nullptr);
        
        SBufferDescriptor BufferDescription;
        
        BufferDescription.m_Stride        = 0;
        BufferDescription.m_Usage         = Gfx::CBuffer::GPUReadWrite;
        BufferDescription.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        BufferDescription.m_Access        = Gfx::CBuffer::CPUWrite;
        BufferDescription.m_pBytes        = 0;
        BufferDescription.m_pClassKey     = 0;
        BufferDescription.m_NumberOfBytes = sizeof(SPerFrameConstantBuffer);
        
        m_PerFrameConstantBufferBufferPtr = BufferManager::CreateBuffer(BufferDescription);

        BufferManager::SetBufferLabel(m_PerFrameConstantBufferBufferPtr, "Global Constant Buffer");

        // -----------------------------------------------------------------------------
        // Setup default values
        // -----------------------------------------------------------------------------
        m_PerFrameConstantBuffer.m_WorldToScreen                  = glm::mat4(1.0f);
        m_PerFrameConstantBuffer.m_WorldToQuad                    = glm::mat4(1.0f);
        m_PerFrameConstantBuffer.m_WorldToView                    = glm::mat4(1.0f);
        m_PerFrameConstantBuffer.m_ViewToScreen                   = glm::mat4(1.0f);
        m_PerFrameConstantBuffer.m_ScreenToView                   = glm::mat4(1.0f);
        m_PerFrameConstantBuffer.m_ViewToWorld                    = glm::mat4(1.0f);
        m_PerFrameConstantBuffer.m_ViewPosition                   = glm::vec4(0.0f);
        m_PerFrameConstantBuffer.m_ViewDirection                  = glm::vec4(0.0f);
        m_PerFrameConstantBuffer.m_InvertedScreensizeAndScreensize= glm::vec4(0.0f);
        m_PerFrameConstantBuffer.m_ScreenPositionScaleBias        = glm::vec4(0.0f);
        m_PerFrameConstantBuffer.m_CameraParameters0              = glm::vec4(0.0f);
        m_PerFrameConstantBuffer.m_WorldParameters0               = glm::vec4(0.0f);
        m_PerFrameConstantBuffer.m_FrameParameters0               = glm::vec4(0.0f);
        m_PerFrameConstantBuffer.m_PreviousWorldToView            = m_PerFrameConstantBuffer.m_WorldToView;
        m_PerFrameConstantBuffer.m_PreviousViewToScreen           = m_PerFrameConstantBuffer.m_ViewToScreen;
        m_PerFrameConstantBuffer.m_PreviousScreenToView           = m_PerFrameConstantBuffer.m_ScreenToView;
        m_PerFrameConstantBuffer.m_PreviousViewToWorld            = m_PerFrameConstantBuffer.m_ViewToWorld;
        m_PerFrameConstantBuffer.m_PreviousViewPosition           = m_PerFrameConstantBuffer.m_ViewPosition;
        m_PerFrameConstantBuffer.m_PreviousViewDirection          = m_PerFrameConstantBuffer.m_ViewDirection;

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::DestroyPerFrameConstantBuffers()
    {
        m_PerFrameConstantBufferBufferPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::UploadPerFrameConstantBuffers()
    {
        assert(m_PerFrameConstantBufferBufferPtr != nullptr);
        
        // -----------------------------------------------------------------------------
        // Prepare data
        // -----------------------------------------------------------------------------
        float NumberOfMetersPerMapRow;
        float NumberOfMetersPerMapColumn;
        float NumberOfRegionsPerMapRow;
        float NumberOfRegionsPerMapColumn;
        float NumberOfMetersPerRegionRow;
        float NumberOfMetersPerRegionColumn;
        float ScreensizeX;
        float ScreensizeY;
        float InvertedScreensizeX;
        float InvertedScreensizeY;
        float Near;
        float Far;
        float WorldNumberOfMetersX;
        float WorldNumberOfMetersY;
        float WorldNumberOfMetersZ;
        float FrameNumber;
        float FrameDeltaTime;
        float FrameTotalTime;
        
        CCameraPtr MainCameraPtr   = ViewManager::GetMainCamera ();
        CCameraPtr DecalCameraPtr  = ViewManager::GetDecalCamera();
        CCameraPtr ScreenCameraPtr = ViewManager::GetFullQuadCamera();
        
        CViewPtr MainViewPtr = MainCameraPtr->GetView();
        
        NumberOfMetersPerMapRow       = static_cast<float>(Dt::Map::GetNumberOfMetersX());
        NumberOfMetersPerMapColumn    = static_cast<float>(Dt::Map::GetNumberOfMetersY());
        NumberOfRegionsPerMapRow      = static_cast<float>(Dt::Map::GetNumberOfRegionsX());
        NumberOfRegionsPerMapColumn   = static_cast<float>(Dt::Map::GetNumberOfRegionsY());
        
        NumberOfMetersPerRegionRow    = static_cast<float>(Dt::CRegion::s_NumberOfMetersX);
        NumberOfMetersPerRegionColumn = static_cast<float>(Dt::CRegion::s_NumberOfMetersY);
        
        ScreensizeX = static_cast<float>(m_pActiveWindowInfo->m_InternalWindowSize[0]);
        ScreensizeY = static_cast<float>(m_pActiveWindowInfo->m_InternalWindowSize[1]);

        InvertedScreensizeX = 1.0f / ScreensizeX;
        InvertedScreensizeY = 1.0f / ScreensizeY;

        Near = MainCameraPtr->GetNear();
        Far  = MainCameraPtr->GetFar();

        WorldNumberOfMetersX = static_cast<float>(Dt::Map::GetNumberOfMetersX());
        WorldNumberOfMetersY = static_cast<float>(Dt::Map::GetNumberOfMetersY());
        WorldNumberOfMetersZ = static_cast<float>(128.0f);

        FrameNumber    = static_cast<float>(Core::Time::GetNumberOfFrame());
        FrameDeltaTime = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());
        FrameTotalTime = static_cast<float>(Core::Time::GetTime());

        // -----------------------------------------------------------------------------
        // Set previous values
        // -----------------------------------------------------------------------------
        m_PerFrameConstantBuffer.m_PreviousWorldToView   = m_PerFrameConstantBuffer.m_WorldToView;
        m_PerFrameConstantBuffer.m_PreviousViewToScreen  = m_PerFrameConstantBuffer.m_ViewToScreen;
        m_PerFrameConstantBuffer.m_PreviousScreenToView  = m_PerFrameConstantBuffer.m_ScreenToView;
        m_PerFrameConstantBuffer.m_PreviousViewToWorld   = m_PerFrameConstantBuffer.m_ViewToWorld;
        m_PerFrameConstantBuffer.m_PreviousViewPosition  = m_PerFrameConstantBuffer.m_ViewPosition;
        m_PerFrameConstantBuffer.m_PreviousViewDirection = m_PerFrameConstantBuffer.m_ViewDirection;

        // -----------------------------------------------------------------------------
        // Set new values;
        // -----------------------------------------------------------------------------
        m_PerFrameConstantBuffer.m_WorldToScreen                   = (MainCameraPtr->GetViewProjectionMatrix());
        m_PerFrameConstantBuffer.m_WorldToQuad                     = (ScreenCameraPtr->GetViewProjectionMatrix());
        m_PerFrameConstantBuffer.m_WorldToView                     = (MainViewPtr->GetViewMatrix());
        m_PerFrameConstantBuffer.m_ViewToScreen                    = (MainCameraPtr->GetProjectionMatrix());
        m_PerFrameConstantBuffer.m_ScreenToView                    = glm::inverse(MainCameraPtr->GetProjectionMatrix());
        m_PerFrameConstantBuffer.m_ViewToWorld                     = glm::inverse(MainViewPtr->GetViewMatrix());
        m_PerFrameConstantBuffer.m_ViewPosition                    = glm::vec4(MainViewPtr->GetPosition(), 1.0f);
        m_PerFrameConstantBuffer.m_ViewDirection                   = glm::vec4(MainViewPtr->GetViewDirection(), 0.0f);
        m_PerFrameConstantBuffer.m_InvertedScreensizeAndScreensize = glm::vec4(InvertedScreensizeX, InvertedScreensizeY, ScreensizeX, ScreensizeY);
        m_PerFrameConstantBuffer.m_ScreenPositionScaleBias         = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
        m_PerFrameConstantBuffer.m_CameraParameters0               = glm::vec4(Near, Far, 0.0f, 0.0f);
        m_PerFrameConstantBuffer.m_WorldParameters0                = glm::vec4(WorldNumberOfMetersX, WorldNumberOfMetersY, WorldNumberOfMetersZ, 0.0f);
        m_PerFrameConstantBuffer.m_FrameParameters0                = glm::vec4(FrameNumber, FrameDeltaTime, FrameTotalTime, 0.0f);
        
        BufferManager::UploadBufferData(m_PerFrameConstantBufferBufferPtr, &m_PerFrameConstantBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CGfxMain::GetPerFrameConstantBuffer()
    {
        return m_PerFrameConstantBufferBufferPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxMain::SetWindowSize(SWindowInfo* _pWindowInfo, int _Width, int _Height)
    {
        // -----------------------------------------------------------------------------
        // Set native resolution
        // -----------------------------------------------------------------------------
        _pWindowInfo->m_NativeWindowSize[0] = _Width;
        _pWindowInfo->m_NativeWindowSize[1] = _Height;

        // -----------------------------------------------------------------------------
        // Calculate internal resolution
        // -----------------------------------------------------------------------------
        _pWindowInfo->m_InternalWindowSize[0] = _pWindowInfo->m_NativeWindowSize[0];
        _pWindowInfo->m_InternalWindowSize[1] = _pWindowInfo->m_NativeWindowSize[1];

        switch (m_GraphicsInfo.m_PixelMatching)
        {
        case CInternGraphicsInfo::Scale:
        {
            float Scale = Core::CProgramParameters::GetInstance().Get<float>("graphics:pixel_matching:scale", 1.0f);

            _pWindowInfo->m_InternalWindowSize[0] = static_cast<int>(static_cast<float>(_pWindowInfo->m_NativeWindowSize[0]) * Scale);
            _pWindowInfo->m_InternalWindowSize[1] = static_cast<int>(static_cast<float>(_pWindowInfo->m_NativeWindowSize[1]) * Scale);
        }
        break;
        case CInternGraphicsInfo::Fix:
        {
            _pWindowInfo->m_InternalWindowSize[0] = Core::CProgramParameters::GetInstance().Get<unsigned int>("graphics:pixel_matching:fixed:w", _Width);
            _pWindowInfo->m_InternalWindowSize[1] = Core::CProgramParameters::GetInstance().Get<unsigned int>("graphics:pixel_matching:fixed:h", _Height);
        }
        break;
        };
    }
} // namespace

namespace Gfx
{
namespace Main
{
    void OnStart()
    {
        CGfxMain::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxMain::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void RegisterResizeHandler(CResizeDelegate _NewDelgate)
    {
        CGfxMain::GetInstance().RegisterResizeHandler(_NewDelgate);
    }

    // -----------------------------------------------------------------------------

    unsigned int RegisterWindow(void* _pWindow, int _VSync)
    {
        return CGfxMain::GetInstance().RegisterWindow(_pWindow, _VSync);
    }

    // -----------------------------------------------------------------------------

    unsigned int GetNumberOfWindows()
    {
        return CGfxMain::GetInstance().GetNumberOfWindows();
    }

    // -----------------------------------------------------------------------------

    void ActivateWindow(unsigned int _WindowID)
    {
        return CGfxMain::GetInstance().ActivateWindow(_WindowID);
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& GetActiveWindowSize()
    {
        return CGfxMain::GetInstance().GetActiveWindowSize();
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& GetWindowSize(unsigned int _WindowID)
    {
        return CGfxMain::GetInstance().GetWindowSize(_WindowID);
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& GetActiveNativeWindowSize()
    {
        return CGfxMain::GetInstance().GetActiveNativeWindowSize();
    }

    // -----------------------------------------------------------------------------

    const glm::ivec2& GetNativeWindowSize(unsigned int _WindowID)
    {
        return CGfxMain::GetInstance().GetNativeWindowSize(_WindowID);
    }

    // -----------------------------------------------------------------------------

    void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height)
    {
        return CGfxMain::GetInstance().OnResize(_WindowID, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile)
    {
        CGfxMain::GetInstance().TakeScreenshot(_WindowID, _pPathToFile);
    }

    // -----------------------------------------------------------------------------

    const CGraphicsInfo& GetGraphicsAPI()
    {
        return CGfxMain::GetInstance().GetGraphicsAPI();
    }

    // -----------------------------------------------------------------------------

    bool IsExtensionAvailable(const std::string& _Name)
    {
        return CGfxMain::GetInstance().IsExtensionAvailable(_Name);
    }
    
    // -----------------------------------------------------------------------------
    
    void BeginFrame()
    {
        CGfxMain::GetInstance().BeginFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    void EndFrame()
    {
        CGfxMain::GetInstance().EndFrame();
    }
    
    // -----------------------------------------------------------------------------
    
    void CreatePerFrameConstantBuffers()
    {
        CGfxMain::GetInstance().CreatePerFrameConstantBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void DestroyPerFrameConstantBuffers()
    {
        CGfxMain::GetInstance().DestroyPerFrameConstantBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void UploadPerFrameConstantBuffers()
    {
        CGfxMain::GetInstance().UploadPerFrameConstantBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr GetPerFrameConstantBuffer()
    {
        return CGfxMain::GetInstance().GetPerFrameConstantBuffer();
    }
} // namespace Main
} // namespace Gfx