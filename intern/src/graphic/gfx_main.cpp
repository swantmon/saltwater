
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

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_view_manager.h"

#include "GL/glew.h"
#include "GL/wglew.h"

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
            BASE_CONSOLE_INFOV("%s\n", _pMessage);
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

        unsigned int RegisterWindow(void* _pWindow);

        unsigned int GetNumberOfWindows();

        void ActivateWindow(unsigned int _WindowID);

        const Base::Int2& GetActiveWindowSize();
        const Base::Int2& GetWindowSize(unsigned int _WindowID);

        void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);
        
    public:
        
        void BeginFrame();
        void EndFrame();
        
    public:
        
        void CreatePerFrameConstantBuffers();
        void DestroyPerFrameConstantBuffers();
        void UploadPerFrameConstantBuffers();
        
        CBufferPtr GetPerFrameConstantBufferVS();
        CBufferPtr GetPerFrameConstantBufferHS();
        CBufferPtr GetPerFrameConstantBufferDS();
        CBufferPtr GetPerFrameConstantBufferGS();
        CBufferPtr GetPerFrameConstantBufferPS();

    private:

        static const unsigned int s_MaxNumberOfWindows = 4;
        
    private:

        struct SWindowInfo
        {
            HWND       m_pNativeWindowHandle;
            HDC        m_pNativeDeviceContextHandle;
            HGLRC      m_pNativeOpenGLContextHandle;
            Base::Int2 m_WindowSize;
        };
        
        struct SPerFrameConstantBufferVS
        {
            Base::Float4   vs_InvertedScreensize;
            Base::Float4x4 vs_ViewProjectionMatrix;
            Base::Float4x4 vs_ViewProjectionScreenMatrix;
        };

        struct SPerFrameConstantBufferHS
        {
            Base::Float4 hs_ViewPosition;
        };
        
        struct SPerFrameConstantBufferDS
        {
            Base::Float4x4 ds_ViewProjectionMatrix;
        };
        
        struct SPerFrameConstantBufferGS
        {
            Base::Float4x4 gs_ViewProjectionScreenMatrix;
        };
        
        struct SPerFrameConstantBufferPS
        {
            Base::Float4x4 ps_WorldToView;
            Base::Float4x4 ps_ViewToScreen;
            Base::Float4x4 ps_ScreenToView;
            Base::Float4x4 ps_ViewToWorld;
            Base::Float4   ps_ViewPosition;
            Base::Float4   ps_ViewDirection;
            Base::Float4x4 ps_PreviousWorldToView;
            Base::Float4x4 ps_PreviousViewToScreen;
            Base::Float4x4 ps_PreviousScreenToView;
            Base::Float4x4 ps_PreviousViewToWorld;
            Base::Float4   ps_PreviousViewPosition;
            Base::Float4   ps_PreviousViewDirection;
            Base::Float4   ps_InvertedScreensizeAndScreensize;
            Base::Float4   ps_ScreenPositionScaleBias;
        };
        
    private:
        
        typedef std::vector<Gfx::Main::CResizeDelegate> CResizeDelegates;
        typedef CResizeDelegates::iterator              CResizeDelegateIterator;
        
    private:

        SWindowInfo  m_WindowInfos[s_MaxNumberOfWindows];
        SWindowInfo* m_pActiveWindowInfo;
        unsigned int m_NumberOfWindows;

        CResizeDelegates m_ResizeDelegates;

        SPerFrameConstantBufferPS m_PerFrameConstantBufferPS;

        CBufferPtr m_PerFrameConstantBufferVSBufferPtr;
        CBufferPtr m_PerFrameConstantBufferHSBufferPtr;
        CBufferPtr m_PerFrameConstantBufferDSBufferPtr;
        CBufferPtr m_PerFrameConstantBufferGSBufferPtr;
        CBufferPtr m_PerFrameConstantBufferPSBufferPtr;      
    };
} // namespace

namespace
{
    CGfxMain::CGfxMain()
        : m_pActiveWindowInfo                (0)
        , m_NumberOfWindows                  (0)
        , m_ResizeDelegates                  ()
        , m_PerFrameConstantBufferPS         ()
        , m_PerFrameConstantBufferVSBufferPtr()
        , m_PerFrameConstantBufferHSBufferPtr()
        , m_PerFrameConstantBufferDSBufferPtr()
        , m_PerFrameConstantBufferGSBufferPtr()
        , m_PerFrameConstantBufferPSBufferPtr()
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
            // Check specific OpenGL versions and availability
            // -----------------------------------------------------------------------------
            const unsigned char* pInfoGLEWVersion   = glewGetString(GLEW_VERSION);
            const unsigned char* pInfoGLVersion     = glGetString(GL_VERSION);                      //< Returns a version or release number.
            const unsigned char* pInfoGLVendor      = glGetString(GL_VENDOR);                       //< Returns the company responsible for this GL implementation. This name does not change from release to release.
            const unsigned char* pInfoGLRenderer    = glGetString(GL_RENDERER);                     //< Returns the name of the renderer. This name is typically specific to a particular configuration of a hardware platform. It does not change from release to release.
            const unsigned char* pInfoGLGLSLVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);     //< Returns a version or release number for the shading language.

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

#if APP_DEBUG_MODE == 1
            glDebugMessageCallback(OpenGLDebugCallback, NULL);

            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

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

    unsigned int CGfxMain::RegisterWindow(void* _pWindow)
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
    
    void CGfxMain::BeginFrame()
    {
        assert(m_pActiveWindowInfo != 0);

        SWindowInfo& rWindowInfo = *m_pActiveWindowInfo;

        wglMakeCurrent(rWindowInfo.m_pNativeDeviceContextHandle, rWindowInfo.m_pNativeOpenGLContextHandle);

        Gfx::TargetSetManager::ClearTargetSet(Gfx::TargetSetManager::GetSystemTargetSet());
        Gfx::TargetSetManager::ClearTargetSet(Gfx::TargetSetManager::GetDefaultTargetSet());
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
        assert(m_PerFrameConstantBufferVSBufferPtr == nullptr);
        assert(m_PerFrameConstantBufferHSBufferPtr == nullptr);
        assert(m_PerFrameConstantBufferDSBufferPtr == nullptr);
        assert(m_PerFrameConstantBufferGSBufferPtr == nullptr);
        assert(m_PerFrameConstantBufferPSBufferPtr == nullptr);
        
        SBufferDescriptor BufferDescription;
        
        BufferDescription.m_Stride        = 0;
        BufferDescription.m_Usage         = Gfx::CBuffer::GPUReadWrite;
        BufferDescription.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        BufferDescription.m_Access        = Gfx::CBuffer::CPUWrite;
        BufferDescription.m_pBytes        = 0;
        BufferDescription.m_pClassKey     = 0;
        
        // -----------------------------------------------------------------------------
        
        BufferDescription.m_NumberOfBytes = sizeof(SPerFrameConstantBufferVS);
        
        m_PerFrameConstantBufferVSBufferPtr = BufferManager::CreateBuffer(BufferDescription);
        
        // -----------------------------------------------------------------------------
        
        BufferDescription.m_NumberOfBytes = sizeof(SPerFrameConstantBufferHS);
        
        m_PerFrameConstantBufferHSBufferPtr = BufferManager::CreateBuffer(BufferDescription);
        
        // -----------------------------------------------------------------------------
        
        BufferDescription.m_NumberOfBytes = sizeof(SPerFrameConstantBufferDS);
        
        m_PerFrameConstantBufferDSBufferPtr = BufferManager::CreateBuffer(BufferDescription);

        // -----------------------------------------------------------------------------

        BufferDescription.m_NumberOfBytes = sizeof(SPerFrameConstantBufferGS);

        m_PerFrameConstantBufferGSBufferPtr = BufferManager::CreateBuffer(BufferDescription);
        
        // -----------------------------------------------------------------------------
        
        BufferDescription.m_NumberOfBytes = sizeof(SPerFrameConstantBufferPS);
        
        m_PerFrameConstantBufferPSBufferPtr = BufferManager::CreateBuffer(BufferDescription);

        // -----------------------------------------------------------------------------
        // Setup default values
        // -----------------------------------------------------------------------------
        m_PerFrameConstantBufferPS.ps_WorldToView                    .SetIdentity();
        m_PerFrameConstantBufferPS.ps_ViewToScreen                   .SetIdentity();
        m_PerFrameConstantBufferPS.ps_ScreenToView                   .SetIdentity();
        m_PerFrameConstantBufferPS.ps_ViewToWorld                    .SetIdentity();
        m_PerFrameConstantBufferPS.ps_ViewPosition                   .SetZero();
        m_PerFrameConstantBufferPS.ps_ViewDirection                  .SetZero();
        m_PerFrameConstantBufferPS.ps_InvertedScreensizeAndScreensize.SetZero();
        m_PerFrameConstantBufferPS.ps_ScreenPositionScaleBias        .SetZero();
        m_PerFrameConstantBufferPS.ps_PreviousWorldToView            = m_PerFrameConstantBufferPS.ps_WorldToView;
        m_PerFrameConstantBufferPS.ps_PreviousViewToScreen           = m_PerFrameConstantBufferPS.ps_ViewToScreen;
        m_PerFrameConstantBufferPS.ps_PreviousScreenToView           = m_PerFrameConstantBufferPS.ps_ScreenToView;
        m_PerFrameConstantBufferPS.ps_PreviousViewToWorld            = m_PerFrameConstantBufferPS.ps_ViewToWorld;
        m_PerFrameConstantBufferPS.ps_PreviousViewPosition           = m_PerFrameConstantBufferPS.ps_ViewPosition;
        m_PerFrameConstantBufferPS.ps_PreviousViewDirection          = m_PerFrameConstantBufferPS.ps_ViewDirection;

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::DestroyPerFrameConstantBuffers()
    {
        m_PerFrameConstantBufferVSBufferPtr = 0;
        m_PerFrameConstantBufferHSBufferPtr = 0;
        m_PerFrameConstantBufferDSBufferPtr = 0;
        m_PerFrameConstantBufferGSBufferPtr = 0;
        m_PerFrameConstantBufferPSBufferPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxMain::UploadPerFrameConstantBuffers()
    {
        assert(m_PerFrameConstantBufferVSBufferPtr != nullptr);
        assert(m_PerFrameConstantBufferHSBufferPtr != nullptr);
        assert(m_PerFrameConstantBufferDSBufferPtr != nullptr);
        assert(m_PerFrameConstantBufferGSBufferPtr != nullptr);
        assert(m_PerFrameConstantBufferPSBufferPtr != nullptr);
        
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
        
        // -----------------------------------------------------------------------------
        // Upload data to vertex shader including infos about:
        // -----------------------------------------------------------------------------
        
        SPerFrameConstantBufferVS* pPerFrameConstantBufferVS = static_cast<SPerFrameConstantBufferVS*>(BufferManager::MapConstantBuffer(m_PerFrameConstantBufferVSBufferPtr));
        
        pPerFrameConstantBufferVS->vs_InvertedScreensize        .Set(InvertedScreensizeX, InvertedScreensizeY, ScreensizeX, ScreensizeY);
        pPerFrameConstantBufferVS->vs_ViewProjectionMatrix      .Set(MainCameraPtr  ->GetViewProjectionMatrix());
        pPerFrameConstantBufferVS->vs_ViewProjectionScreenMatrix.Set(ScreenCameraPtr->GetViewProjectionMatrix());
        
        BufferManager::UnmapConstantBuffer(m_PerFrameConstantBufferVSBufferPtr);
        
        // -----------------------------------------------------------------------------
        // Upload data to tesselation control / hull shader including infos about:
        // -----------------------------------------------------------------------------
        
        SPerFrameConstantBufferHS* pPerFrameConstantBufferHS = static_cast<SPerFrameConstantBufferHS*>(BufferManager::MapConstantBuffer(m_PerFrameConstantBufferHSBufferPtr));
        
        pPerFrameConstantBufferHS->hs_ViewPosition.Set(MainViewPtr->GetPosition(), 0.0f);
        
        BufferManager::UnmapConstantBuffer(m_PerFrameConstantBufferHSBufferPtr);
        
        // -----------------------------------------------------------------------------
        // Upload data to tesselation evaluation / domain shader including infos about:
        // -----------------------------------------------------------------------------
        
        SPerFrameConstantBufferDS* pPerFrameConstantBufferDS = static_cast<SPerFrameConstantBufferDS*>(BufferManager::MapConstantBuffer(m_PerFrameConstantBufferDSBufferPtr));
        
        pPerFrameConstantBufferDS->ds_ViewProjectionMatrix.Set(MainCameraPtr->GetViewProjectionMatrix());
        
        BufferManager::UnmapConstantBuffer(m_PerFrameConstantBufferDSBufferPtr);

        // -----------------------------------------------------------------------------
        // Upload data to geometry shader including infos about:
        // -----------------------------------------------------------------------------

        SPerFrameConstantBufferGS* pPerFrameConstantBufferGS = static_cast<SPerFrameConstantBufferGS*>(BufferManager::MapConstantBuffer(m_PerFrameConstantBufferGSBufferPtr));

        pPerFrameConstantBufferGS->gs_ViewProjectionScreenMatrix.Set(ScreenCameraPtr->GetViewProjectionMatrix());

        BufferManager::UnmapConstantBuffer(m_PerFrameConstantBufferGSBufferPtr);
        
        // -----------------------------------------------------------------------------
        // Upload data to fragment / pixel shader including infos about:
        // -----------------------------------------------------------------------------
        SPerFrameConstantBufferPS* pPerFrameConstantBufferPS = static_cast<SPerFrameConstantBufferPS*>(BufferManager::MapConstantBuffer(m_PerFrameConstantBufferPSBufferPtr));
        
        // -----------------------------------------------------------------------------
        // Set previous values
        // -----------------------------------------------------------------------------
        m_PerFrameConstantBufferPS.ps_PreviousWorldToView   = m_PerFrameConstantBufferPS.ps_WorldToView;
        m_PerFrameConstantBufferPS.ps_PreviousViewToScreen  = m_PerFrameConstantBufferPS.ps_ViewToScreen;
        m_PerFrameConstantBufferPS.ps_PreviousScreenToView  = m_PerFrameConstantBufferPS.ps_ScreenToView;
        m_PerFrameConstantBufferPS.ps_PreviousViewToWorld   = m_PerFrameConstantBufferPS.ps_ViewToWorld;
        m_PerFrameConstantBufferPS.ps_PreviousViewPosition  = m_PerFrameConstantBufferPS.ps_ViewPosition;
        m_PerFrameConstantBufferPS.ps_PreviousViewDirection = m_PerFrameConstantBufferPS.ps_ViewDirection;

        // -----------------------------------------------------------------------------
        // Set new values;
        // -----------------------------------------------------------------------------
        m_PerFrameConstantBufferPS.ps_WorldToView                    .Set(MainViewPtr->GetViewMatrix());
        m_PerFrameConstantBufferPS.ps_ViewToScreen                   .Set(MainCameraPtr->GetProjectionMatrix());
        m_PerFrameConstantBufferPS.ps_ScreenToView                   .Set(MainCameraPtr->GetProjectionMatrix().GetInverted());
        m_PerFrameConstantBufferPS.ps_ViewToWorld                    .Set(MainViewPtr->GetViewMatrix().GetInverted());
        m_PerFrameConstantBufferPS.ps_ViewPosition                   .Set(MainViewPtr->GetPosition(), 1.0f);
        m_PerFrameConstantBufferPS.ps_ViewDirection                  .Set(MainViewPtr->GetViewDirection(), 0.0f);
        m_PerFrameConstantBufferPS.ps_InvertedScreensizeAndScreensize.Set(InvertedScreensizeX, InvertedScreensizeY, ScreensizeX, ScreensizeY);
        m_PerFrameConstantBufferPS.ps_ScreenPositionScaleBias        .Set(0.5f, 0.5f, 0.5f, 0.5f);

        Base::CMemory::Copy(pPerFrameConstantBufferPS, &m_PerFrameConstantBufferPS, sizeof(SPerFrameConstantBufferPS));
        
        BufferManager::UnmapConstantBuffer(m_PerFrameConstantBufferPSBufferPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CGfxMain::GetPerFrameConstantBufferVS()
    {
        return m_PerFrameConstantBufferVSBufferPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CGfxMain::GetPerFrameConstantBufferHS()
    {
        return m_PerFrameConstantBufferHSBufferPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CGfxMain::GetPerFrameConstantBufferDS()
    {
        return m_PerFrameConstantBufferDSBufferPtr;
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxMain::GetPerFrameConstantBufferGS()
    {
        return m_PerFrameConstantBufferGSBufferPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr CGfxMain::GetPerFrameConstantBufferPS()
    {
        return m_PerFrameConstantBufferPSBufferPtr;
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

    unsigned int RegisterWindow(void* _pWindow)
    {
        return CGfxMain::GetInstance().RegisterWindow(_pWindow);
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
    
    CBufferPtr GetPerFrameConstantBufferVS()
    {
        return CGfxMain::GetInstance().GetPerFrameConstantBufferVS();
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr GetPerFrameConstantBufferHS()
    {
        return CGfxMain::GetInstance().GetPerFrameConstantBufferHS();
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr GetPerFrameConstantBufferDS()
    {
        return CGfxMain::GetInstance().GetPerFrameConstantBufferDS();
    }

    // -----------------------------------------------------------------------------

    CBufferPtr GetPerFrameConstantBufferGS()
    {
        return CGfxMain::GetInstance().GetPerFrameConstantBufferGS();
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferPtr GetPerFrameConstantBufferPS()
    {
        return CGfxMain::GetInstance().GetPerFrameConstantBufferPS();
    }
} // namespace Main
} // namespace Gfx