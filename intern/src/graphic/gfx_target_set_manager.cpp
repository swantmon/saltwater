
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_vector2.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_native_target_set.h"
#include "graphic/gfx_native_texture_2d.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"

#include "GL/glew.h"

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

        CTargetSetPtr CreateTargetSet(CTextureBasePtr* _pTargetPtrs, unsigned int _NumberOfTargets);
        
        void ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth);
        void ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor);
        void ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor, float _Depth);
        
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

        void ResizeTargetSet(CTargetSetPtr _TargetSetPtr, CTextureBasePtr* _pTargetPtrs, unsigned int _NumberOfTargets);
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
        Base::Int2 Size = Main::GetActiveWindowSize();
        
        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr AlbedoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // RGB Albedo
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer1Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 1
        
        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer2Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 2
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer3Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 3
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R16G16B16A16_FLOAT;
        
        CTexture2DPtr LightAccumulationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Light Accumulation (HDR)
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R32_UINT;

        CTexture2DPtr HitProxyTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Hit Proxy (ID)
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::DepthStencilTarget | CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R32_FLOAT;
        
        CTexture2DPtr DepthTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth
        
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
        CTextureBasePtr DefaultRenderbuffer[2];
        
        DefaultRenderbuffer[0] = AlbedoPtr;
        DefaultRenderbuffer[1] = DepthTexturePtr;
        
        m_DefaultTargetSet = CreateTargetSet(DefaultRenderbuffer, 2);
        
        // -----------------------------------------------------------------------------
        // Create deferred target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr DeferredRenderbuffer[4];
        
        DeferredRenderbuffer[0] = GBuffer1Ptr;
        DeferredRenderbuffer[1] = GBuffer2Ptr;
        DeferredRenderbuffer[2] = GBuffer3Ptr;
        DeferredRenderbuffer[3] = DepthTexturePtr;
        
        m_DeferredTargetSet = CreateTargetSet(DeferredRenderbuffer, 4);
        
        // -----------------------------------------------------------------------------
        // Create light accumulation target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr LightAccumulationRenderbuffer[1];
        
        LightAccumulationRenderbuffer[0] = LightAccumulationTexturePtr;

        m_LightAccumulationTargetSet = CreateTargetSet(LightAccumulationRenderbuffer, 1);

        // -----------------------------------------------------------------------------
        // Create hit proxy target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr HitProxyRenderbuffer[2];

        HitProxyRenderbuffer[0] = HitProxyTexturePtr;
        HitProxyRenderbuffer[1] = DepthTexturePtr;

        m_HitProxyTargetSet = CreateTargetSet(HitProxyRenderbuffer, 2);
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
    
    CTargetSetPtr CGfxTargetSetManager::CreateTargetSet(CTextureBasePtr* _pTargetPtrs, unsigned int _NumberOfTargets)
    {
        unsigned int NumberOfColorAttachments = 0;
        
        CTargetSets::CPtr TargetSetPtr = m_TargetSets.Allocate();
            
        CInternTargetSet& rTargetSet = *TargetSetPtr;
            
        // -----------------------------------------------------------------------------
        
        GLuint Framebuffer;
            
        glCreateFramebuffers(1, &Framebuffer);
            
        for (unsigned int IndexOfTexture = 0; IndexOfTexture < _NumberOfTargets; ++ IndexOfTexture)
        {
            CNativeTexture2D& rNativeTexture = *static_cast<CNativeTexture2D*>(_pTargetPtrs[IndexOfTexture].GetPtr());
                
            GLuint TextureHandle = rNativeTexture.m_NativeTexture;

            unsigned int MipmapLevel = rNativeTexture.GetCurrentMipLevel();
                
            if ((rNativeTexture.GetBinding() & CTexture2D::DepthStencilTarget) != 0)
            {
                glNamedFramebufferTexture(Framebuffer, GL_DEPTH_ATTACHMENT, TextureHandle, MipmapLevel);
                    
                rTargetSet.m_DepthStencilTargetPtr = _pTargetPtrs[IndexOfTexture];
            }
            else if ((rNativeTexture.GetBinding() & CTexture2D::RenderTarget) != 0)
            {
                glNamedFramebufferTexture(Framebuffer, GL_COLOR_ATTACHMENT0 + NumberOfColorAttachments, TextureHandle, MipmapLevel);
                    
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
        GLenum Status = glCheckNamedFramebufferStatus(Framebuffer, GL_FRAMEBUFFER);
            
        if(Status != GL_FRAMEBUFFER_COMPLETE)
        {
            BASE_THROWM("Can't create an acceptable frame buffer.");
        }
        
        return CTargetSetPtr(TargetSetPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth)
    {
        CNativeTargetSet& rNativeTargetSet = *static_cast<CNativeTargetSet*>(_TargetPtr.GetPtr());

        glClearNamedFramebufferfv(rNativeTargetSet.m_NativeTargetSet, GL_DEPTH, 0, &_Depth);
    }

    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor)
    {
        CNativeTargetSet& rNativeTargetSet = *static_cast<CNativeTargetSet*>(_TargetPtr.GetPtr());

        for (unsigned int Index = 0; Index <_TargetPtr->GetNumberOfRenderTargets(); ++ Index)
        {
            glClearNamedFramebufferfv(rNativeTargetSet.m_NativeTargetSet, GL_COLOR, Index, const_cast<GLfloat*>(&_rColor[0]));
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxTargetSetManager::ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor, float _Depth)
    {
        CNativeTargetSet& rNativeTargetSet = *static_cast<CNativeTargetSet*>(_TargetPtr.GetPtr());
       
        for (unsigned int Index = 0; Index < _TargetPtr->GetNumberOfRenderTargets(); ++Index)
        {
            glClearNamedFramebufferfv(rNativeTargetSet.m_NativeTargetSet, GL_COLOR, Index, const_cast<GLfloat*>(&_rColor[0]));
        }

        glClearNamedFramebufferfv(rNativeTargetSet.m_NativeTargetSet, GL_DEPTH, 0, &_Depth);
    }
    
    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::OnResize(int _Width, int _Height)
    {
        // -----------------------------------------------------------------------------
        // Initiate target set
        // -----------------------------------------------------------------------------
        Base::Int2 Size(_Width, _Height);
        
        // -----------------------------------------------------------------------------
        // Create render target textures
        // -----------------------------------------------------------------------------
        STextureDescriptor RendertargetDescriptor;
        
        RendertargetDescriptor.m_NumberOfPixelsU  = Size[0];
        RendertargetDescriptor.m_NumberOfPixelsV  = Size[1];
        RendertargetDescriptor.m_NumberOfPixelsW  = 1;
        RendertargetDescriptor.m_NumberOfMipMaps  = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding          = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Access           = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format           = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage            = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic         = CTextureBase::Diffuse;
        RendertargetDescriptor.m_pFileName        = 0;
        RendertargetDescriptor.m_pPixels          = 0;
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr AlbedoPtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // RGB Albedo
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer1Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 1
        
        // -----------------------------------------------------------------------------

        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer2Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 2
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R8G8B8A8_UBYTE;
        
        CTexture2DPtr GBuffer3Ptr = TextureManager::CreateTexture2D(RendertargetDescriptor); // G-Buffer 3
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R16G16B16A16_FLOAT;
        
        CTexture2DPtr LightAccumulationTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Light Accumulation (HDR)
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R32_UINT;

        CTexture2DPtr HitProxyTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Hit Proxy (ID)
        
        // -----------------------------------------------------------------------------
        
        RendertargetDescriptor.m_Binding       = CTextureBase::DepthStencilTarget | CTextureBase::RenderTarget;
        RendertargetDescriptor.m_Format        = CTextureBase::R32_FLOAT;
        
        CTexture2DPtr DepthTexturePtr = TextureManager::CreateTexture2D(RendertargetDescriptor); // Depth
                
        // -----------------------------------------------------------------------------
        // Create default target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr DefaultRenderbuffer[2];
        
        DefaultRenderbuffer[0] = AlbedoPtr;
        DefaultRenderbuffer[1] = DepthTexturePtr;
        
        ResizeTargetSet(m_DefaultTargetSet, DefaultRenderbuffer, 2);
        
        // -----------------------------------------------------------------------------
        // Create deferred target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr DeferredRenderbuffer[4];
        
        DeferredRenderbuffer[0] = GBuffer1Ptr;
        DeferredRenderbuffer[1] = GBuffer2Ptr;
        DeferredRenderbuffer[2] = GBuffer3Ptr;
        DeferredRenderbuffer[3] = DepthTexturePtr;
        
        ResizeTargetSet(m_DeferredTargetSet, DeferredRenderbuffer, 4);
        
        // -----------------------------------------------------------------------------
        // Create light accumulation target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr LightAccumulationRenderbuffer[1];
        
        LightAccumulationRenderbuffer[0] = LightAccumulationTexturePtr;

        ResizeTargetSet(m_LightAccumulationTargetSet, LightAccumulationRenderbuffer, 1);

        // -----------------------------------------------------------------------------
        // Create hit proxy target set
        // -----------------------------------------------------------------------------
        CTextureBasePtr HitProxyRenderbuffer[2];

        HitProxyRenderbuffer[0] = HitProxyTexturePtr;
        HitProxyRenderbuffer[1] = DepthTexturePtr;

        ResizeTargetSet(m_HitProxyTargetSet, HitProxyRenderbuffer, 2);
    }

    // -----------------------------------------------------------------------------

    void CGfxTargetSetManager::ResizeTargetSet(CTargetSetPtr _TargetSetPtr, CTextureBasePtr* _pTargetPtrs, unsigned int _NumberOfTargets)
    {
        unsigned int NumberOfColorAttachments = 0;

        CInternTargetSet& rTargetSet = *static_cast<CInternTargetSet*>(_TargetSetPtr.GetPtr());

        // -----------------------------------------------------------------------------

        for (unsigned int IndexOfTexture = 0; IndexOfTexture < _NumberOfTargets; ++IndexOfTexture)
        {
            CNativeTexture2D& rNativeTexture = *static_cast<CNativeTexture2D*>(_pTargetPtrs[IndexOfTexture].GetPtr());

            GLuint TextureHandle = rNativeTexture.m_NativeTexture;

            unsigned int MipmapLevel = rNativeTexture.GetCurrentMipLevel();

            if ((rNativeTexture.GetBinding() & CTexture2D::DepthStencilTarget) != 0)
            {
                glNamedFramebufferTexture(rTargetSet.m_NativeTargetSet, GL_DEPTH_ATTACHMENT, TextureHandle, MipmapLevel);

                rTargetSet.m_DepthStencilTargetPtr = _pTargetPtrs[IndexOfTexture];
            }
            else if ((rNativeTexture.GetBinding() & CTexture2D::RenderTarget) != 0)
            {
                glNamedFramebufferTexture(rTargetSet.m_NativeTargetSet, GL_COLOR_ATTACHMENT0 + NumberOfColorAttachments, TextureHandle, MipmapLevel);

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
        GLenum Status = glCheckNamedFramebufferStatus(rTargetSet.m_NativeTargetSet, GL_FRAMEBUFFER);

        if (Status != GL_FRAMEBUFFER_COMPLETE)
        {
            BASE_THROWM("Can't create an acceptable frame buffer.");
        }
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
    
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr)
    {
        CTextureBasePtr TargetPtrs[] = { _Target1Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 1);
    }

    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr, CTextureBasePtr _Target2Ptr)
    {
        CTextureBasePtr TargetPtrs[] = { _Target1Ptr, _Target2Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 2);
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr, CTextureBasePtr _Target2Ptr, CTextureBasePtr _Target3Ptr)
    {
        CTextureBasePtr TargetPtrs[] = { _Target1Ptr, _Target2Ptr, _Target3Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 3);
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr, CTextureBasePtr _Target2Ptr, CTextureBasePtr _Target3Ptr, CTextureBasePtr _Target4Ptr)
    {
        CTextureBasePtr TargetPtrs[] = { _Target1Ptr, _Target2Ptr, _Target3Ptr, _Target4Ptr, };
        
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(TargetPtrs, 4);
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CreateTargetSet(CTextureBasePtr* _pTargetPtrs, unsigned int _NumberOfTargets)
    {
        return CGfxTargetSetManager::GetInstance().CreateTargetSet(_pTargetPtrs, _NumberOfTargets);
    }
    
    // -----------------------------------------------------------------------------
    
    void ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor, float _Depth)
    {
        CGfxTargetSetManager::GetInstance().ClearTargetSet(_TargetPtr, _rColor, _Depth);
    }
    
    // -----------------------------------------------------------------------------
    
    void ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor)
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
        CGfxTargetSetManager::GetInstance().ClearTargetSet(_TargetPtr, Base::Float4(0.0f), 1.0f);
    }
} // namespace TargetSetManager
} // namespace Gfx
