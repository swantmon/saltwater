
#include "engine/engine_precompiled.h"

#include "base/base_console.h"
#include "base/base_include_glm.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_native_target_set.h"
#include "graphic/gfx_native_texture.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"

using namespace Gfx;

namespace
{
    class CGfxTargetSetManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxTargetSetManager)
        
    public:
        
        CGfxTargetSetManager();
        ~CGfxTargetSetManager();
        
    public:
        
        void OnStart();
        void OnExit();
        
        CTargetSetPtr GetSystemTargetSet();
        CTargetSetPtr GetDefaultTargetSet();
        CTargetSetPtr GetDeferredTargetSet();
        CTargetSetPtr GetLightAccumulationTargetSet();
        CTargetSetPtr GetHitProxyTargetSet();

        CTargetSetPtr CreateTargetSet(CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets);

        CTargetSetPtr CreateEmptyTargetSet(int _Width, int _Height, int _Layers);
        
        void ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth);
        void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor);
        void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor, float _Depth);

        void SetTargetSetLabel(CTargetSetPtr _TargetSetPtr, const char* _pLabel);
        
    private:
        
        // -----------------------------------------------------------------------------
        // Represents a unique combination of up to 16 render targets.
        // -----------------------------------------------------------------------------
        class CInternTargetSet : public CNativeTargetSet
        {
        public:
            
            CInternTargetSet();
            ~CInternTargetSet();
            
        public:
            
            bool operator == (const CInternTargetSet& _rTargetSet) const;
            bool operator != (const CInternTargetSet& _rTargetSet) const;
            
        private:
            
            friend class CGfxTargetSetManager;
        };
        
        typedef Base::CManagedPool<CInternTargetSet, 16> CTargetSets;
        
    private:
        
        CTargetSets m_TargetSets;
        
        CTargetSetPtr m_SystemTargetSet;
        CTargetSetPtr m_DefaultTargetSet;
        CTargetSetPtr m_DeferredTargetSet;
        CTargetSetPtr m_LightAccumulationTargetSet;
        CTargetSetPtr m_HitProxyTargetSet;
        
    private:
        
        void OnResize(int _Width, int _Height);

        void ResizeTargetSet(CTargetSetPtr _TargetSetPtr, CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets);
    };
} // namespace

namespace
{
    CGfxTargetSetManager::CGfxTargetSetManager()
        : m_TargetSets                ()
        , m_SystemTargetSet           ()
        , m_DefaultTargetSet          ()
        , m_DeferredTargetSet         ()
        , m_LightAccumulationTargetSet()
        , m_HitProxyTargetSet         ()
    {
        // -----------------------------------------------------------------------------
        // Register for resizing events
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxTargetSetManager::OnResize));
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxTargetSetManager::~CGfxTargetSetManager()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxTargetSetManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        glm::ivec2 Size = Main::GetActiveWindowSize();
        
        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::Unknown;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr AlbedoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // RGB Albedo

        TextureManager::SetTextureLabel(AlbedoPtr, "RT: Default RGB");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr GBuffer1Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 1

        TextureManager::SetTextureLabel(GBuffer1Ptr, "RT: Gbuffer 1");
        
        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr GBuffer2Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 2

        TextureManager::SetTextureLabel(GBuffer2Ptr, "RT: Gbuffer 2");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr GBuffer3Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 3

        TextureManager::SetTextureLabel(GBuffer3Ptr, "RT: Gbuffer 3");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R16G16B16A16_FLOAT;
        
        CTexturePtr LightAccumulationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Light Accumulation (HDR)

        TextureManager::SetTextureLabel(LightAccumulationTexturePtr, "RT: Light Accumulation (HDR)");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R32_UINT;

        CTexturePtr HitProxyTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Hit Proxy (ID)

        TextureManager::SetTextureLabel(HitProxyTexturePtr, "RT: Hit Proxy (ID)");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::DepthStencilTarget | CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R32_FLOAT;
        
        CTexturePtr DepthTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth

        TextureManager::SetTextureLabel(DepthTexturePtr, "RT: Depth");
        
        // -----------------------------------------------------------------------------
        // Create system target set
        // -----------------------------------------------------------------------------
        CTargetSets::CPtr TargetSetPtr = m_TargetSets.Allocate();
        
        CInternTargetSet& rTargetSet = *TargetSetPtr;
        
        rTargetSet.m_NumberOfRenderTargets = 0;
        rTargetSet.m_NativeTargetSet       = 0;

        m_SystemTargetSet = CTargetSetPtr(TargetSetPtr);
        
        // -----------------------------------------------------------------------------
        // Create default target set
        // -----------------------------------------------------------------------------
        CTexturePtr DefaultRenderbuffer[2];
        
        DefaultRenderbuffer[0] = AlbedoPtr;
        DefaultRenderbuffer[1] = DepthTexturePtr;
        
        m_DefaultTargetSet = CreateTargetSet(DefaultRenderbuffer, 2);

        SetTargetSetLabel(m_DefaultTargetSet, "FB: Default");
        
        // -----------------------------------------------------------------------------
        // Create deferred target set
        // -----------------------------------------------------------------------------
        CTexturePtr DeferredRenderbuffer[4];
        
        DeferredRenderbuffer[0] = GBuffer1Ptr;
        DeferredRenderbuffer[1] = GBuffer2Ptr;
        DeferredRenderbuffer[2] = GBuffer3Ptr;
        DeferredRenderbuffer[3] = DepthTexturePtr;
        
        m_DeferredTargetSet = CreateTargetSet(DeferredRenderbuffer, 4);

        SetTargetSetLabel(m_DeferredTargetSet, "FB: Deferred");
        
        // -----------------------------------------------------------------------------
        // Create light accumulation target set
        // -----------------------------------------------------------------------------
        CTexturePtr LightAccumulationRenderbuffer[1];
        
        LightAccumulationRenderbuffer[0] = LightAccumulationTexturePtr;

        m_LightAccumulationTargetSet = CreateTargetSet(LightAccumulationRenderbuffer, 1);

        SetTargetSetLabel(m_LightAccumulationTargetSet, "FB: Light Accumulation");

        // -----------------------------------------------------------------------------
        // Create hit proxy target set
        // -----------------------------------------------------------------------------
        CTexturePtr HitProxyRenderbuffer[2];

        HitProxyRenderbuffer[0] = HitProxyTexturePtr;
        HitProxyRenderbuffer[1] = DepthTexturePtr;

        m_HitProxyTargetSet = CreateTargetSet(HitProxyRenderbuffer, 2);

        SetTargetSetLabel(m_HitProxyTargetSet, "FB: Hit Proxy");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxTargetSetManager::OnExit()
    {
        m_SystemTargetSet            = nullptr;
        m_DefaultTargetSet           = nullptr;
        m_DeferredTargetSet          = nullptr;
        m_LightAccumulationTargetSet = nullptr;
        m_HitProxyTargetSet          = nullptr;
        
        m_TargetSets.Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CGfxTargetSetManager::GetSystemTargetSet()
    {
        return m_SystemTargetSet;
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CGfxTargetSetManager::GetDefaultTargetSet()
    {
        return m_DefaultTargetSet;
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CGfxTargetSetManager::GetDeferredTargetSet()
    {
        return m_DeferredTargetSet;
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CGfxTargetSetManager::GetLightAccumulationTargetSet()
    {
        return m_LightAccumulationTargetSet;
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr CGfxTargetSetManager::GetHitProxyTargetSet()
    {
        return m_HitProxyTargetSet;
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CGfxTargetSetManager::CreateTargetSet(CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets)
    {
        unsigned int NumberOfColorAttachments = 0;
        
        CTargetSets::CPtr TargetSetPtr = m_TargetSets.Allocate();
            
        CInternTargetSet& rTargetSet = *TargetSetPtr;
            
        // -----------------------------------------------------------------------------
        
        GLuint Framebuffer;
            
        glGenFramebuffers(1, &Framebuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
            
        for (unsigned int IndexOfTexture = 0; IndexOfTexture < _NumberOfTargets; ++ IndexOfTexture)
        {
            CTexturePtr TextureBasePtr = _pTargetPtrs[IndexOfTexture];

            GLuint TextureHandle = 0;

            CNativeTexture& rNativeTexture = *static_cast<CNativeTexture*>(TextureBasePtr.GetPtr());

            TextureHandle = rNativeTexture.GetNativeHandle();

            unsigned int MipmapLevel = TextureBasePtr->GetCurrentMipLevel();

            if ((TextureBasePtr->GetBinding() & CTexture::DepthStencilTarget) != 0)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TextureHandle, MipmapLevel);

                rTargetSet.m_DepthStencilTargetPtr = _pTargetPtrs[IndexOfTexture];
            }
            else if ((TextureBasePtr->GetBinding() & CTexture::RenderTarget) != 0)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + NumberOfColorAttachments, TextureHandle, MipmapLevel);

                rTargetSet.m_RenderTargetPtrs[NumberOfColorAttachments] = _pTargetPtrs[IndexOfTexture];
                    
                ++ NumberOfColorAttachments;
            }
            else
            {
                BASE_CONSOLE_STREAMWARNING("Tried to bind a texture to frame buffer (target set) without subsided binding.");
            }
        }
            
        rTargetSet.m_NumberOfRenderTargets = NumberOfColorAttachments;
        rTargetSet.m_NativeTargetSet       = Framebuffer;
            
        // -----------------------------------------------------------------------------
        // Check status
        // -----------------------------------------------------------------------------
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            
        if(Status != GL_FRAMEBUFFER_COMPLETE)
        {
            BASE_THROWM("Can't create an acceptable frame buffer.");
        }

        // -----------------------------------------------------------------------------
        // Unbind
        // -----------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        return CTargetSetPtr(TargetSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr CGfxTargetSetManager::CreateEmptyTargetSet(int _Width, int _Height, int _Layers)
    {
        CTargetSets::CPtr TargetSetPtr = m_TargetSets.Allocate();

        CInternTargetSet& rTargetSet = *TargetSetPtr;

        GLuint Framebuffer;

        // -----------------------------------------------------------------------------
        // ARB_framebuffer_no_attachments
        // -----------------------------------------------------------------------------

        glGenFramebuffers(1, &Framebuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);

        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, _Width);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _Height);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS, _Layers);

        rTargetSet.m_NumberOfRenderTargets = 0;
        rTargetSet.m_NativeTargetSet = Framebuffer;

        // -----------------------------------------------------------------------------
        // Check status
        // -----------------------------------------------------------------------------
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (Status != GL_FRAMEBUFFER_COMPLETE)
        {
            BASE_THROWM("Can't create an acceptable frame buffer.");
        }

        // -----------------------------------------------------------------------------
        // Unbind
        // -----------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return CTargetSetPtr(TargetSetPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth)
    {
        CNativeTargetSet& rNativeTargetSet = *static_cast<CNativeTargetSet*>(_TargetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, rNativeTargetSet.m_NativeTargetSet);

        glClearBufferfv(GL_DEPTH, 0, &_Depth);
    }

    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor)
    {
        CNativeTargetSet& rNativeTargetSet = *static_cast<CNativeTargetSet*>(_TargetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, rNativeTargetSet.m_NativeTargetSet);

        for (unsigned int Index = 0; Index <_TargetPtr->GetNumberOfRenderTargets(); ++ Index)
        {
            glClearBufferfv(GL_COLOR, Index, const_cast<GLfloat*>(&_rColor[0]));
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxTargetSetManager::ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor, float _Depth)
    {
        CNativeTargetSet& rNativeTargetSet = *static_cast<CNativeTargetSet*>(_TargetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, rNativeTargetSet.m_NativeTargetSet);
       
        for (unsigned int Index = 0; Index < _TargetPtr->GetNumberOfRenderTargets(); ++Index)
        {
            glClearBufferfv(GL_COLOR, Index, const_cast<GLfloat*>(&_rColor[0]));
        }

        glClearBufferfv(GL_DEPTH, 0, &_Depth);
    }

    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::SetTargetSetLabel(CTargetSetPtr _TargetSetPtr, const char* _pLabel)
    {
        assert(_pLabel != nullptr);

        CNativeTargetSet* pNativeTargetSet = static_cast<CNativeTargetSet*>(_TargetSetPtr.GetPtr());

        glObjectLabel(GL_FRAMEBUFFER, pNativeTargetSet->m_NativeTargetSet, -1, _pLabel);
    }
    
    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::OnResize(int _Width, int _Height)
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        glm::ivec2 Size(_Width, _Height);
        
        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTexture::RenderTarget;
        RendertargetDescriptor.m_Access           = CTexture::CPUWrite;
        RendertargetDescriptor.m_Format           = CTexture::Unknown;
        RendertargetDescriptor.m_Usage            = CTexture::GPURead;
        RendertargetDescriptor.m_Semantic         = CTexture::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr AlbedoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // RGB Albedo

        TextureManager::SetTextureLabel(AlbedoPtr, "RT: Default RGB");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr GBuffer1Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 1

        TextureManager::SetTextureLabel(GBuffer1Ptr, "RT: Gbuffer 1");
        
        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr GBuffer2Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 2

        TextureManager::SetTextureLabel(GBuffer2Ptr, "RT: Gbuffer 2");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R8G8B8A8_UBYTE;
        
        CTexturePtr GBuffer3Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 3

        TextureManager::SetTextureLabel(GBuffer3Ptr, "RT: Gbuffer 3");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R16G16B16A16_FLOAT;
        
        CTexturePtr LightAccumulationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Light Accumulation (HDR)

        TextureManager::SetTextureLabel(LightAccumulationTexturePtr, "RT: Light Accumulation (HDR)");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R32_UINT;

        CTexturePtr HitProxyTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Hit Proxy (ID)

        TextureManager::SetTextureLabel(HitProxyTexturePtr, "RT: Hit Proxy (ID)");
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTexture::DepthStencilTarget | CTexture::RenderTarget;
        RendertargetDescriptor.m_Format        = CTexture::R32_FLOAT;
        
        CTexturePtr DepthTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth

        TextureManager::SetTextureLabel(DepthTexturePtr, "RT: Depth");
                
        // -----------------------------------------------------------------------------
        // Create default target set
        // -----------------------------------------------------------------------------
        CTexturePtr DefaultRenderbuffer[2];
        
        DefaultRenderbuffer[0] = AlbedoPtr;
        DefaultRenderbuffer[1] = DepthTexturePtr;
        
        ResizeTargetSet(m_DefaultTargetSet, DefaultRenderbuffer, 2);
        
        // -----------------------------------------------------------------------------
        // Create deferred target set
        // -----------------------------------------------------------------------------
        CTexturePtr DeferredRenderbuffer[4];
        
        DeferredRenderbuffer[0] = GBuffer1Ptr;
        DeferredRenderbuffer[1] = GBuffer2Ptr;
        DeferredRenderbuffer[2] = GBuffer3Ptr;
        DeferredRenderbuffer[3] = DepthTexturePtr;
        
        ResizeTargetSet(m_DeferredTargetSet, DeferredRenderbuffer, 4);
        
        // -----------------------------------------------------------------------------
        // Create light accumulation target set
        // -----------------------------------------------------------------------------
        CTexturePtr LightAccumulationRenderbuffer[1];
        
        LightAccumulationRenderbuffer[0] = LightAccumulationTexturePtr;

        ResizeTargetSet(m_LightAccumulationTargetSet, LightAccumulationRenderbuffer, 1);

        // -----------------------------------------------------------------------------
        // Create hit proxy target set
        // -----------------------------------------------------------------------------
        CTexturePtr HitProxyRenderbuffer[2];

        HitProxyRenderbuffer[0] = HitProxyTexturePtr;
        HitProxyRenderbuffer[1] = DepthTexturePtr;

        ResizeTargetSet(m_HitProxyTargetSet, HitProxyRenderbuffer, 2);
    }

    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::ResizeTargetSet(CTargetSetPtr _TargetSetPtr, CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets)
    {
        unsigned int NumberOfColorAttachments = 0;

        CInternTargetSet& rTargetSet = *static_cast<CInternTargetSet*>(_TargetSetPtr.GetPtr());

        // -----------------------------------------------------------------------------
        // Binding
        // -----------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, rTargetSet.m_NativeTargetSet);

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < _NumberOfTargets; ++IndexOfTexture)
        {
            CNativeTexture& rNativeTexture = *static_cast<CNativeTexture*>(_pTargetPtrs[IndexOfTexture].GetPtr());

            GLuint TextureHandle = rNativeTexture.GetNativeHandle();

            unsigned int MipmapLevel = rNativeTexture.GetCurrentMipLevel();

            unsigned int BindingTarget = rNativeTexture.IsCube() == true ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;

            glBindTexture(BindingTarget, TextureHandle);

            if ((rNativeTexture.GetBinding() & CTexture::DepthStencilTarget) != 0)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TextureHandle, MipmapLevel);

                rTargetSet.m_DepthStencilTargetPtr = 0;
                rTargetSet.m_DepthStencilTargetPtr = _pTargetPtrs[IndexOfTexture];
            }
            else if ((rNativeTexture.GetBinding() & CTexture::RenderTarget) != 0)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + NumberOfColorAttachments, TextureHandle, MipmapLevel);

                rTargetSet.m_RenderTargetPtrs[NumberOfColorAttachments] = 0;
                rTargetSet.m_RenderTargetPtrs[NumberOfColorAttachments] = _pTargetPtrs[IndexOfTexture];

                ++NumberOfColorAttachments;
            }
            else
            {
                BASE_CONSOLE_STREAMWARNING("Tried to bind a texture to frame buffer (target set) without subsided binding.");
            }
        }

        assert(rTargetSet.m_NumberOfRenderTargets == NumberOfColorAttachments);

        // -----------------------------------------------------------------------------
        // Check status
        // -----------------------------------------------------------------------------
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (Status != GL_FRAMEBUFFER_COMPLETE)
        {
            BASE_THROWM("Can't create an acceptable frame buffer.");
        }

        // -----------------------------------------------------------------------------
        // Unbind
        // -----------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} // namespace

namespace
{
    CGfxTargetSetManager::CInternTargetSet::CInternTargetSet()
        : CNativeTargetSet()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxTargetSetManager::CInternTargetSet::~CInternTargetSet()
    {
        if (m_NativeTargetSet != 0)
        {
            for (unsigned int IndexOfTexture = 0; IndexOfTexture < m_NumberOfRenderTargets; ++IndexOfTexture)
            {
                m_RenderTargetPtrs[IndexOfTexture] = 0;
            }

            if (m_DepthStencilTargetPtr != 0)
            {
                m_DepthStencilTargetPtr = 0;
            }

            glDeleteFramebuffers(1, &m_NativeTargetSet);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    bool CGfxTargetSetManager::CInternTargetSet::operator == (const CInternTargetSet& _rTargetSet) const
    {
        unsigned int IndexOfTarget;
        
        if (m_NumberOfRenderTargets != _rTargetSet.m_NumberOfRenderTargets)
        {
            return false;
        }
        
        // -----------------------------------------------------------------------------
        // Important not to use the native interfaces here, because maybe we want to
        // exchange those without the need to rehash the set.
        // -----------------------------------------------------------------------------
        for (IndexOfTarget = 0; IndexOfTarget < m_NumberOfRenderTargets; ++ IndexOfTarget)
        {
            if (m_RenderTargetPtrs[IndexOfTarget] != _rTargetSet.m_RenderTargetPtrs[IndexOfTarget])
            {
                return false;
            }
        }
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool CGfxTargetSetManager::CInternTargetSet::operator != (const CInternTargetSet& _rTargetSet) const
    {
        Base::Size IndexOfTargets;
        
        if (m_NumberOfRenderTargets != _rTargetSet.m_NumberOfRenderTargets)
        {
            return true;
        }
        
        // -----------------------------------------------------------------------------
        // Important not to use the native interfaces here, because maybe we want to
        // exchange those without the need to rehash the set.
        // -----------------------------------------------------------------------------
        for (IndexOfTargets = 0; IndexOfTargets < m_NumberOfRenderTargets; ++ IndexOfTargets)
        {
            if (m_RenderTargetPtrs[IndexOfTargets] != _rTargetSet.m_RenderTargetPtrs[IndexOfTargets])
            {
                return true;
            }
        }
        
        return false;
    }
} // namespace

namespace Gfx
{
namespace TargetSetManager
{
    void OnStart()
    {
        CGfxTargetSetManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxTargetSetManager::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr GetSystemTargetSet()
    {
        return CGfxTargetSetManager::GetInstance().GetSystemTargetSet();
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr GetDefaultTargetSet()
    {
        return CGfxTargetSetManager::GetInstance().GetDefaultTargetSet();
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr GetDeferredTargetSet()
    {
        return CGfxTargetSetManager::GetInstance().GetDeferredTargetSet();
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr GetLightAccumulationTargetSet()
    {
        return CGfxTargetSetManager::GetInstance().GetLightAccumulationTargetSet();
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr GetHitProxyTargetSet()
    {
        return CGfxTargetSetManager::GetInstance().GetHitProxyTargetSet();
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr)
    {
        CTexturePtr TargetPtrs[] = { _Target1Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 1);
    }

    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr)
    {
        CTexturePtr TargetPtrs[] = { _Target1Ptr, _Target2Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 2);
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr)
    {
        CTexturePtr TargetPtrs[] = { _Target1Ptr, _Target2Ptr, _Target3Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 3);
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr, CTexturePtr _Target4Ptr)
    {
        CTexturePtr TargetPtrs[] = { _Target1Ptr, _Target2Ptr, _Target3Ptr, _Target4Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 4);
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets)
    {
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(_pTargetPtrs, _NumberOfTargets);
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr CreateEmptyTargetSet(int _Width, int _Height, int _Layers /* = 0 */)
    {
        return CGfxTargetSetManager::GetInstance().CreateEmptyTargetSet(_Width, _Height, _Layers);
    }
    
    // -----------------------------------------------------------------------------
    
    void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor, float _Depth)
    {
        CGfxTargetSetManager::GetInstance().ClearTargetSet(_TargetPtr, _rColor, _Depth);
    }
    
    // -----------------------------------------------------------------------------
    
    void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor)
    {
        CGfxTargetSetManager::GetInstance().ClearTargetSet(_TargetPtr, _rColor);
    }
    
    // -----------------------------------------------------------------------------
    
    void ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth)
    {
        CGfxTargetSetManager::GetInstance().ClearTargetSet(_TargetPtr, _Depth);
    }
    
    // -----------------------------------------------------------------------------
    
    void ClearTargetSet(CTargetSetPtr _TargetPtr)
    {
        CGfxTargetSetManager::GetInstance().ClearTargetSet(_TargetPtr, glm::vec4(0.0f), 1.0f);
    }

    // -----------------------------------------------------------------------------

    void SetTargetSetLabel(CTargetSetPtr _TargetSetPtr, const char* _pLabel)
    {
        CGfxTargetSetManager::GetInstance().SetTargetSetLabel(_TargetSetPtr, _pLabel);
    }
} // namespace TargetSetManager
} // namespace Gfx
