
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include "core/core_time.h"
#include "core/core_config.h"

#include "data/data_map.h"
#include "data/data_texture_manager.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "GL/glew.h"
#include "GL/wglew.h"

#include <unordered_set>
#include <vector>
#include <windows.h>

using namespace Gfx;

namespace 
{
    void GLAPIENTRY OpenGLDebugCallback(GLenum _Source, GLenum _Type, GLuint _Id, GLenum _Severity, GLsizei _Length, const GLchar* _pMessage, const GLvoid* _pUserParam)
    {
        BASE_UNUSED(_Source);
        BASE_UNUSED(_Type);
        BASE_UNUSED(_Id);
        BASE_UNUSED(_Severity);
        BASE_UNUSED(_Length);
        BASE_UNUSED(_pUserParam);

        switch (_Type)
        {
        case GL_DEBUG_TYPE_ERROR:
            BASE_CONSOLE_ERRORV("%s\n", _pMessage);
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            BASE_CONSOLE_WARNINGV("%s\n", _pMessage);
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            BASE_CONSOLE_WARNINGV("%s\n", _pMessage);
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            BASE_CONSOLE_INFOV("%s\n", _pMessage);
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            //BASE_CONSOLE_INFOV("%s\n", _pMessage);
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

        unsigned int RegisterWindow(void* _pWindow, unsigned int _VSync);

        unsigned int GetNumberOfWindows();

        void ActivateWindow(unsigned int _WindowID);

        const Base::Int2& GetActiveWindowSize();
        const Base::Int2& GetWindowSize(unsigned int _WindowID);

        void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

        void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);

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

        struct SWindowInfo
        {
            HWND         m_pNativeWindowHandle;
            HDC          m_pNativeDeviceContextHandle;
            HGLRC        m_pNativeOpenGLContextHandle;
            Base::Int2   m_WindowSize;
            unsigned int m_VSync;
        };
        
        struct SPerFrameConstantBuffer
        {
            Base::Float4x4 m_WorldToScreen;
            Base::Float4x4 m_WorldToQuad;
            Base::Float4x4 m_WorldToView;
            Base::Float4x4 m_ViewToScreen;
            Base::Float4x4 m_ScreenToView;
            Base::Float4x4 m_ViewToWorld;
            Base::Float4   m_ViewPosition;
            Base::Float4   m_ViewDirection;
            Base::Float4x4 m_PreviousWorldToView;
            Base::Float4x4 m_PreviousViewToScreen;
            Base::Float4x4 m_PreviousScreenToView;
            Base::Float4x4 m_PreviousViewToWorld;
            Base::Float4   m_PreviousViewPosition;
            Base::Float4   m_PreviousViewDirection;
            Base::Float4   m_InvertedScreensizeAndScreensize;
            Base::Float4   m_ScreenPositionScaleBias;
            Base::Float4   m_CameraParameters0;
            Base::Float4   m_WorldParameters0;
            Base::Float4   m_FrameParameters0;
        };
        
    private:
        
        typedef std::vector<Gfx::Main::CResizeDelegate> CResizeDelegates;
        typedef CResizeDelegates::iterator              CResizeDelegateIterator;
        
    private:

        SWindowInfo  m_WindowInfos[s_MaxNumberOfWindows];
        SWindowInfo* m_pActiveWindowInfo;
        unsigned int m_NumberOfWindows;

        CResizeDelegates m_ResizeDelegates;

        SPerFrameConstantBuffer m_PerFrameConstantBuffer;

        CBufferPtr m_PerFrameConstantBufferBufferPtr;

        std::unordered_set<std::string> m_AvailableExtensions;
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
        
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxMain::~CGfxMain()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::OnStart()
    {                
        // -----------------------------------------------------------------------------
        // Show information of windows and initialize them
        // -----------------------------------------------------------------------------
        unsigned int IndexOfWindow = 0;

        for (IndexOfWindow = 0; IndexOfWindow < m_NumberOfWindows; ++IndexOfWindow)
        {
            SWindowInfo& rWindowInfo = m_WindowInfos[IndexOfWindow];

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

            const int Attributes[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 5,
                WGL_CONTEXT_PROFILE_MASK_ARB , WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_FLAGS_ARB        , APP_DEBUG_MODE ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
                0,        //End
            };

            // -----------------------------------------------------------------------------
            // Create OpenGL specific stuff with dummy context
            // -----------------------------------------------------------------------------
            pNativeWindowHandle = rWindowInfo.m_pNativeWindowHandle;

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
            // Check specific OpenGL versions and availability
            // -----------------------------------------------------------------------------
            char VSyncInvertal[33];

            _itoa_s(rWindowInfo.m_VSync, VSyncInvertal, 10);

            const unsigned char* pInfoGLEWVersion   = glewGetString(GLEW_VERSION);
            const unsigned char* pInfoGLVersion     = glGetString(GL_VERSION);                  //< Returns a version or release number.
            const unsigned char* pInfoGLVendor      = glGetString(GL_VENDOR);                   //< Returns the company responsible for this GL implementation. This name does not change from release to release.
            const unsigned char* pInfoGLRenderer    = glGetString(GL_RENDERER);                 //< Returns the name of the renderer. This name is typically specific to a particular configuration of a hardware platform. It does not change from release to release.
            const unsigned char* pInfoGLGLSLVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); //< Returns a version or release number for the shading language.
            const char*          pInfoVSync         = VSyncInvertal;                            //< Indicates the minimal interval of buffer swaps

            assert(pInfoGLEWVersion && pInfoGLVersion && pInfoGLGLSLVersion && pInfoGLVendor && pInfoGLRenderer);

            if (!GLEW_VERSION_4_5)
            {
                BASE_THROWV("GL 4.5 can't be initialized. Available version %s is to old!", pInfoGLVersion);
            }

            BASE_CONSOLE_INFOV("Window ID: %i", IndexOfWindow);
            BASE_CONSOLE_INFOV("GLEW:      %s", pInfoGLEWVersion);
            BASE_CONSOLE_INFOV("GL:        %s", pInfoGLVersion);
            BASE_CONSOLE_INFOV("GLSL:      %s", pInfoGLGLSLVersion);
            BASE_CONSOLE_INFOV("Vendor:    %s", pInfoGLVendor);
            BASE_CONSOLE_INFOV("Renderer:  %s", pInfoGLRenderer);
            BASE_CONSOLE_INFOV("VSync:     %s", pInfoVSync);

#if APP_DEBUG_MODE == 1
            glDebugMessageCallback(OpenGLDebugCallback, NULL);

            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

            glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

            GLint ExtensionCount;
            glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);

            for (int i = 0; i < ExtensionCount; ++i)
            {
                std::string Name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));

                m_AvailableExtensions.insert(Name);
            }

            // -----------------------------------------------------------------------------
            // Save created data
            // -----------------------------------------------------------------------------
            rWindowInfo.m_pNativeDeviceContextHandle = pNativeDeviceContextHandle;
            rWindowInfo.m_pNativeOpenGLContextHandle = pNativeOpenGLContextHandle;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::OnExit()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::RegisterResizeHandler(Gfx::Main::CResizeDelegate _NewDelgate)
    {
        m_ResizeDelegates.push_back(_NewDelgate);
    }

    // -----------------------------------------------------------------------------

    unsigned int CGfxMain::RegisterWindow(void* _pWindow, unsigned int _VSync)
    {
        if (_pWindow == 0 || m_NumberOfWindows == s_MaxNumberOfWindows) return 0;

        HWND  pNativeWindowHandle;

        // -----------------------------------------------------------------------------
        // Cast data
        // -----------------------------------------------------------------------------
        pNativeWindowHandle = static_cast<HWND>(_pWindow);

        // -----------------------------------------------------------------------------
        // Save data to new window
        // -----------------------------------------------------------------------------
        SWindowInfo& rNewWindow = m_WindowInfos[m_NumberOfWindows];

        rNewWindow.m_pNativeWindowHandle        = pNativeWindowHandle;
        rNewWindow.m_pNativeDeviceContextHandle = 0;
        rNewWindow.m_pNativeOpenGLContextHandle = 0;
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

    const Base::Int2& CGfxMain::GetActiveWindowSize()
    {
        assert(m_pActiveWindowInfo != 0);

        return m_pActiveWindowInfo->m_WindowSize;
    }

    // -----------------------------------------------------------------------------

    const Base::Int2& CGfxMain::GetWindowSize(unsigned int _WindowID)
    {
        assert(_WindowID < m_NumberOfWindows);

        return m_WindowInfos[_WindowID].m_WindowSize;
    }

    // -----------------------------------------------------------------------------

    void CGfxMain::OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height)
    {
        assert(_Width > 0 && _Height > 0);

        if (_WindowID >= m_NumberOfWindows) return;

        // -----------------------------------------------------------------------------
        // Setup view port of render target (back buffer, ...).
        // -----------------------------------------------------------------------------
        m_WindowInfos[_WindowID].m_WindowSize[0] = _Width;
        m_WindowInfos[_WindowID].m_WindowSize[1] = _Height;

        // -----------------------------------------------------------------------------
        // Send to every delegate that resize has changed
        // -----------------------------------------------------------------------------
        CResizeDelegateIterator EndOfDelegates = m_ResizeDelegates.end();

        for (CResizeDelegateIterator CurrentDelegate = m_ResizeDelegates.begin(); CurrentDelegate != EndOfDelegates; ++CurrentDelegate)
        {
            (*CurrentDelegate)(_Width, _Height);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxMain::TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile)
    {
        assert(_WindowID < m_NumberOfWindows);

        unsigned int Width;
        unsigned int Height;
        char*        pPixels;

        // -----------------------------------------------------------------------------
        // Read pixels from last frame buffer
        // -----------------------------------------------------------------------------
        SWindowInfo& rWindowInfo = m_WindowInfos[_WindowID];

        Width  = rWindowInfo.m_WindowSize[0];
        Height = rWindowInfo.m_WindowSize[1];

        pPixels = static_cast<char*>(Base::CMemory::Allocate(3 * Width * Height * sizeof(char)));

        glReadPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, pPixels);

        // -----------------------------------------------------------------------------
        // Create texture
        // -----------------------------------------------------------------------------
        Dt::STextureDescriptor TextureDesc;

        TextureDesc.m_pIdentifier      = 0;
        TextureDesc.m_NumberOfPixelsU  = Width;
        TextureDesc.m_NumberOfPixelsV  = Height;
        TextureDesc.m_NumberOfPixelsW  = 1;
        TextureDesc.m_NumberOfTextures = 1;
        TextureDesc.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDesc.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDesc.m_Binding          = Dt::CTextureBase::CPU;
        TextureDesc.m_pFileName        = 0;
        TextureDesc.m_pPixels          = pPixels;

        Dt::CTexture2D* pScreenshot = Dt::TextureManager::CreateTexture2D(TextureDesc);

        // -----------------------------------------------------------------------------
        // Save texture to file
        // -----------------------------------------------------------------------------
        Dt::TextureManager::SaveTexture2DToFile(pScreenshot, _pPathToFile);

        // -----------------------------------------------------------------------------
        // Free data
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pPixels);
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

        wglMakeCurrent(rWindowInfo.m_pNativeDeviceContextHandle, rWindowInfo.m_pNativeOpenGLContextHandle);

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

        SwapBuffers(rWindowInfo.m_pNativeDeviceContextHandle);
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

        // -----------------------------------------------------------------------------
        // Setup default values
        // -----------------------------------------------------------------------------
        m_PerFrameConstantBuffer.m_WorldToScreen                  .SetIdentity();
        m_PerFrameConstantBuffer.m_WorldToQuad                    .SetIdentity();
        m_PerFrameConstantBuffer.m_WorldToView                    .SetIdentity();
        m_PerFrameConstantBuffer.m_ViewToScreen                   .SetIdentity();
        m_PerFrameConstantBuffer.m_ScreenToView                   .SetIdentity();
        m_PerFrameConstantBuffer.m_ViewToWorld                    .SetIdentity();
        m_PerFrameConstantBuffer.m_ViewPosition                   .SetZero();
        m_PerFrameConstantBuffer.m_ViewDirection                  .SetZero();
        m_PerFrameConstantBuffer.m_InvertedScreensizeAndScreensize.SetZero();
        m_PerFrameConstantBuffer.m_ScreenPositionScaleBias        .SetZero();
        m_PerFrameConstantBuffer.m_CameraParameters0              .SetZero();
        m_PerFrameConstantBuffer.m_WorldParameters0               .SetZero();
        m_PerFrameConstantBuffer.m_FrameParameters0               .SetZero();
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
        
        ScreensizeX = static_cast<float>(m_pActiveWindowInfo->m_WindowSize[0]);
        ScreensizeY = static_cast<float>(m_pActiveWindowInfo->m_WindowSize[1]);

        InvertedScreensizeX = 1.0f / ScreensizeX;
        InvertedScreensizeY = 1.0f / ScreensizeY;

        Near = MainCameraPtr->GetNear();
        Far  = MainCameraPtr->GetFar();

        WorldNumberOfMetersX = static_cast<float>(Dt::Map::GetNumberOfMetersX());
        WorldNumberOfMetersY = static_cast<float>(Dt::Map::GetNumberOfMetersY());
        WorldNumberOfMetersZ = static_cast<float>(128.0f);

        FrameNumber    = static_cast<float>(Core::Time::GetNumberOfFrame());
        FrameDeltaTime = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

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
        m_PerFrameConstantBuffer.m_WorldToScreen                  .Set(MainCameraPtr->GetViewProjectionMatrix());
        m_PerFrameConstantBuffer.m_WorldToQuad                    .Set(ScreenCameraPtr->GetViewProjectionMatrix());
        m_PerFrameConstantBuffer.m_WorldToView                    .Set(MainViewPtr->GetViewMatrix());
        m_PerFrameConstantBuffer.m_ViewToScreen                   .Set(MainCameraPtr->GetProjectionMatrix());
        m_PerFrameConstantBuffer.m_ScreenToView                   .Set(MainCameraPtr->GetProjectionMatrix().GetInverted());
        m_PerFrameConstantBuffer.m_ViewToWorld                    .Set(MainViewPtr->GetViewMatrix().GetInverted());
        m_PerFrameConstantBuffer.m_ViewPosition                   .Set(MainViewPtr->GetPosition(), 1.0f);
        m_PerFrameConstantBuffer.m_ViewDirection                  .Set(MainViewPtr->GetViewDirection(), 0.0f);
        m_PerFrameConstantBuffer.m_InvertedScreensizeAndScreensize.Set(InvertedScreensizeX, InvertedScreensizeY, ScreensizeX, ScreensizeY);
        m_PerFrameConstantBuffer.m_ScreenPositionScaleBias        .Set(0.5f, 0.5f, 0.5f, 0.5f);
        m_PerFrameConstantBuffer.m_CameraParameters0              .Set(Near, Far, 0.0f, 0.0f);
        m_PerFrameConstantBuffer.m_WorldParameters0               .Set(WorldNumberOfMetersX, WorldNumberOfMetersY, WorldNumberOfMetersZ, 0.0f);
        m_PerFrameConstantBuffer.m_FrameParameters0               .Set(FrameNumber, FrameDeltaTime, 0.0f, 0.0f);
        
        BufferManager::UploadBufferData(m_PerFrameConstantBufferBufferPtr, &m_PerFrameConstantBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CGfxMain::GetPerFrameConstantBuffer()
    {
        return m_PerFrameConstantBufferBufferPtr;
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

    unsigned int RegisterWindow(void* _pWindow, unsigned int _VSync)
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

    const Base::Int2& GetActiveWindowSize()
    {
        return CGfxMain::GetInstance().GetActiveWindowSize();
    }

    // -----------------------------------------------------------------------------

    const Base::Int2& GetWindowSize(unsigned int _WindowID)
    {
        return CGfxMain::GetInstance().GetWindowSize(_WindowID);
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