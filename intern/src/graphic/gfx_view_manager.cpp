
#include "graphic/gfx_precompiled.h"

#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_managed_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_component.h"
#include "data/data_component_manager.h"
#include "data/data_camera_component.h"
#include "data/data_texture_manager.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_view_manager.h"
#include "graphic/gfx_texture_manager.h"

using namespace Gfx;

namespace
{
    class CGfxViewManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxViewManager)
        
    public:
        
        CGfxViewManager();
        ~CGfxViewManager();
        
    public:
        
        void OnStart();
        void OnExit();
        
    public:
        
        CViewPortPtr GetViewPort();
        CViewPortSetPtr GetViewPortSet();
        
    public:
        
        CViewPtr CreateView();
        CCameraPtr CreateCamera(CViewPtr _ViewPtr);
        CViewPortPtr CreateViewPort(const SViewPortDescriptor& _rDescriptor);
        CViewPortSetPtr CreateViewPortSet(CViewPortPtr* _pViewPortPtrs, unsigned int _NumberOfViewPorts);
        
    public:
        
        CCameraPtr GetMainCamera();
        CCameraPtr GetDebugCamera();
        CCameraPtr GetDecalCamera();
        CCameraPtr GetGUICamera();
        CCameraPtr GetFullQuadCamera();
        
    private:
        
        class CInternCamera : public CCamera
        {
        public:
            
            CInternCamera();
            ~CInternCamera();
            
        public:
            
            void Attach(CViewPtr _ViewPtr);
            void Detach();
        };
        
        class CInternView : public CView
        {
        public:
            
            CInternView();
            ~CInternView();
            
        private:
            
            friend class CInternCamera;
        };
        
        // -----------------------------------------------------------------------------
        // Represents a unique view port.
        // -----------------------------------------------------------------------------
        class CInternViewPort : public CViewPort
        {
        public:
            
            CInternViewPort();
            ~CInternViewPort();
            
        public:
            
            bool operator == (const CInternViewPort& _rInternViewPort) const;
            bool operator != (const CInternViewPort& _rInternViewPort) const;
            
        private:
            
            friend class CGfxViewManager;
        };
        
        // -----------------------------------------------------------------------------
        // Represents a unique combination of up to 16 view ports.
        // -----------------------------------------------------------------------------
        class CInternViewPortSet : public CViewPortSet
        {
        public:
            
            CInternViewPortSet();
            ~CInternViewPortSet();
            
        public:
            
            bool operator == (const CInternViewPortSet& _rViewPortSet) const;
            bool operator != (const CInternViewPortSet& _rViewPortSet) const;
            
        private:
            
            friend class CGfxViewManager;
        };
        
        typedef Base::CManagedPool<CInternView       , 16> CViews;
        typedef Base::CManagedPool<CInternCamera     , 16> CCameras;
        typedef Base::CManagedPool<CInternViewPort   , 16> CViewPorts;
        typedef Base::CManagedPool<CInternViewPortSet, 16> CViewPortSets;

    private:
        
        void OnResize(unsigned int _Width, unsigned int _Height);
        
        void ResizeCameras(float _Width, float _Height);
        void ResizeViewPort(float _Width, float _Height);

        void OnDirtyComponent(Dt::IComponent* _pComponent);
        
    private:
        
        // -----------------------------------------------------------------------------
        // Possible other cameras and view ports depending on renderer
        // -----------------------------------------------------------------------------
        CViews          m_Views;
        CCameras        m_Cameras;
        CViewPorts      m_ViewPorts;
        CViewPortSets   m_ViewPortSets;
        
        // -----------------------------------------------------------------------------
        // Main camera and view ports
        // -----------------------------------------------------------------------------
        CCameraPtr      m_MainCameraPtr;
        CCameraPtr      m_DebugCameraPtr;
        CCameraPtr      m_DecalCameraPtr;
        CCameraPtr      m_GUICameraPtr;
        CCameraPtr      m_FullQuadCameraPtr;
        CViewPortPtr    m_ViewPortPtr;
        CViewPortSetPtr m_ViewPortSetPtr;
    };
} // namespace

namespace
{
    CGfxViewManager::CGfxViewManager()
        : m_Views       ()
        , m_Cameras     ()
        , m_ViewPorts   ()
        , m_ViewPortSets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxViewManager::~CGfxViewManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxViewManager::OnStart()
    {
        float AspectRatio;
        float Width;
        float Height;
        
        try
        {
            glm::ivec2 ScreenSize = Main::GetActiveWindowSize();
            
            Width  = static_cast<float>(ScreenSize[0]);
            Height = static_cast<float>(ScreenSize[1]);
            
            SViewPortDescriptor ViewPortDescriptor;
            
            ViewPortDescriptor.m_TopLeftX = 0.0f;
            ViewPortDescriptor.m_TopLeftY = 0.0f;
            ViewPortDescriptor.m_Width    = Width;
            ViewPortDescriptor.m_Height   = Height;
            ViewPortDescriptor.m_MinDepth = 0.0f;
            ViewPortDescriptor.m_MaxDepth = 1.0f;
            
            CViewPtr MainViewPtr  = CreateView();
            CViewPtr DebugViewPtr = CreateView();
            CViewPtr GUIViewPtr   = CreateView();
            CViewPtr PostViewPtr  = CreateView();
            
            m_MainCameraPtr     = ViewManager::CreateCamera(MainViewPtr);
            m_DebugCameraPtr    = ViewManager::CreateCamera(DebugViewPtr);
            m_DecalCameraPtr    = ViewManager::CreateCamera(MainViewPtr);
            m_GUICameraPtr      = ViewManager::CreateCamera(GUIViewPtr);
            m_FullQuadCameraPtr = ViewManager::CreateCamera(PostViewPtr);
            m_ViewPortPtr       = ViewManager::CreateViewPort(ViewPortDescriptor);
            m_ViewPortSetPtr    = ViewManager::CreateViewPortSet(m_ViewPortPtr);
            
            AspectRatio = Width / Height;
        
            m_MainCameraPtr ->SetFieldOfView(60.0f, AspectRatio, 0.01f, 4096.0f);
            m_DebugCameraPtr->SetFieldOfView(60.0f, AspectRatio, 0.01f, 4096.0f);
            m_DecalCameraPtr->SetFieldOfView(60.0f, AspectRatio, 0.01f, 8192.0f);
        
            m_MainCameraPtr ->Update();
            m_DebugCameraPtr->Update();
            m_DecalCameraPtr->Update();

            ResizeCameras(Width, Height);
            
            Gfx::Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxViewManager::OnResize));

            Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(BASE_DIRTY_COMPONENT_METHOD(&CGfxViewManager::OnDirtyComponent));
        }
        catch (...)
        {
            m_ViewPortPtr    = nullptr;
            m_ViewPortSetPtr = nullptr;
            
            BASE_THROWM("Error creating view port.");
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxViewManager::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Make sure to release the view ports before clearing the pools.
        // -----------------------------------------------------------------------------
        m_MainCameraPtr     = nullptr;
        m_DebugCameraPtr    = nullptr;
        m_DecalCameraPtr    = nullptr;
        m_GUICameraPtr      = nullptr;
        m_FullQuadCameraPtr = nullptr;
        m_ViewPortPtr       = nullptr;
        m_ViewPortSetPtr    = nullptr;
        
        // -----------------------------------------------------------------------------
        // Clear the pools.
        // -----------------------------------------------------------------------------
        m_Views       .Clear();
        m_Cameras     .Clear();
        m_ViewPorts   .Clear();
        m_ViewPortSets.Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxViewManager::OnResize(unsigned int _Width, unsigned int _Height)
    {
        float Width;
        float Height;
        
        Width  = static_cast<float>(_Width);
        Height = static_cast<float>(_Height);
        
        ResizeCameras (Width, Height);
        ResizeViewPort(Width, Height);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxViewManager::ResizeCameras(float _Width, float _Height)
    {
        float AspectRatio;
        
        // -----------------------------------------------------------------------------
        // Setup the perspective cameras.
        // -----------------------------------------------------------------------------
        assert(_Height >= 1.0f);
        
        AspectRatio = _Width / _Height;
        
        m_MainCameraPtr ->SetAspectRatio(AspectRatio);
        m_DebugCameraPtr->SetAspectRatio(AspectRatio);
        m_DecalCameraPtr->SetAspectRatio(AspectRatio);
        
        m_MainCameraPtr ->Update();
        m_DebugCameraPtr->Update();
        m_DecalCameraPtr->Update();
        
        // -----------------------------------------------------------------------------
        // Setup the GUI camera.
        // -----------------------------------------------------------------------------
        m_GUICameraPtr->SetOrthographic(0.0f, _Width, 0.0f, _Height, 0.0f, 32.0f);
        
        // -----------------------------------------------------------------------------
        // Define position and orientation of the camera.
        // -----------------------------------------------------------------------------
        CViewPtr GUIViewPtr = m_GUICameraPtr->GetView();
        
        assert(GUIViewPtr != nullptr);
        
        GUIViewPtr->SetPosition(0.0f, _Height, 0.0f);
        
        // -----------------------------------------------------------------------------
        // Flip the camera around the x-axis, so that the origin of the GUI coordinate
        // system is in the upper left corner of the render area.
        // -----------------------------------------------------------------------------
        glm::mat3 Rotation(1.0f);

        Rotation = glm::eulerAngleX(glm::radians(180.0f));
        
        GUIViewPtr->SetRotationMatrix(Rotation);
        
        // -----------------------------------------------------------------------------
        // Updating the view updates also all the cameras associated with this view.
        // -----------------------------------------------------------------------------
        GUIViewPtr->Update();
        
        // -----------------------------------------------------------------------------
        // Setup the post effect camera.
        // -----------------------------------------------------------------------------
        m_FullQuadCameraPtr->SetOrthographic(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
        
        // -----------------------------------------------------------------------------
        // Define position and orientation of the camera.
        // -----------------------------------------------------------------------------
        CViewPtr PostViewPtr = m_FullQuadCameraPtr->GetView();
        
        assert(PostViewPtr != nullptr);
        
        PostViewPtr->SetPosition(0.0f, 1.0f, 0.0f);
        
        // -----------------------------------------------------------------------------
        // Flip the camera around the x-axis, so that the origin of the post coordinate
        // system is in the upper left corner of the render area.
        // -----------------------------------------------------------------------------
        Rotation = glm::eulerAngleX(glm::radians(180.0f));
        
        PostViewPtr->SetRotationMatrix(Rotation);
        
        // -----------------------------------------------------------------------------
        // Updating the view updates also all the cameras associated with this view.
        // -----------------------------------------------------------------------------
        PostViewPtr->Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxViewManager::ResizeViewPort(float _Width, float _Height)
    {
        // -----------------------------------------------------------------------------
        // Resizing a view port implies a new hash value, so we have to remove the port
        // from the hash table and after recalculating the value we insert it again.
        // -----------------------------------------------------------------------------
        CInternViewPort& rViewPort = static_cast<CInternViewPort&>(*m_ViewPortPtr);
        
        rViewPort.m_Port.m_TopLeftX = 0.0f;
        rViewPort.m_Port.m_TopLeftY = 0.0f;
        rViewPort.m_Port.m_Width    = _Width;
        rViewPort.m_Port.m_Height   = _Height;
        rViewPort.m_Port.m_MinDepth = 0.0f;
        rViewPort.m_Port.m_MaxDepth = 1.0f;
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPtr CGfxViewManager::CreateView()
    {
        // -----------------------------------------------------------------------------
        // Return a smart pointer to the visible base class.
        // -----------------------------------------------------------------------------
        return CViewPtr(m_Views.Allocate());
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CGfxViewManager::CreateCamera(CViewPtr _ViewPtr)
    {
        CCameras::CPtr CameraPtr = m_Cameras.Allocate();
        
        CInternCamera& rCamera = *CameraPtr;
        
        rCamera.Attach(_ViewPtr);
        
        // -----------------------------------------------------------------------------
        // Return a smart pointer to the visible base class.
        // -----------------------------------------------------------------------------
        return CCameraPtr(CameraPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortPtr CGfxViewManager::CreateViewPort(const SViewPortDescriptor& _rDescriptor)
    {
        // -----------------------------------------------------------------------------
        // Create a temporary view port to check for redundant entries.
        // -----------------------------------------------------------------------------
        CInternViewPort TmpViewPort;
        
        TmpViewPort.m_Port.m_TopLeftX = _rDescriptor.m_TopLeftX;
        TmpViewPort.m_Port.m_TopLeftY = _rDescriptor.m_TopLeftY;
        TmpViewPort.m_Port.m_Width    = _rDescriptor.m_Width;
        TmpViewPort.m_Port.m_Height   = _rDescriptor.m_Height;
        TmpViewPort.m_Port.m_MinDepth = _rDescriptor.m_MinDepth;
        TmpViewPort.m_Port.m_MaxDepth = _rDescriptor.m_MaxDepth;
        
        // -----------------------------------------------------------------------------
        // We did not find a matching entry, so create a new view port.
        // -----------------------------------------------------------------------------
        Base::CManagedPoolItemPtr<CInternViewPort> ViewPortPtr = m_ViewPorts.Allocate();
        
        CInternViewPort& rViewPort = *ViewPortPtr;
        
        rViewPort.m_Port.m_TopLeftX = _rDescriptor.m_TopLeftX;
        rViewPort.m_Port.m_TopLeftY = _rDescriptor.m_TopLeftY;
        rViewPort.m_Port.m_Width    = _rDescriptor.m_Width;
        rViewPort.m_Port.m_Height   = _rDescriptor.m_Height;
        rViewPort.m_Port.m_MinDepth = _rDescriptor.m_MinDepth;
        rViewPort.m_Port.m_MaxDepth = _rDescriptor.m_MaxDepth;
        
        // -----------------------------------------------------------------------------
        // Return a smart pointer to the visible base class.
        // -----------------------------------------------------------------------------
        return CViewPortPtr(ViewPortPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CGfxViewManager::CreateViewPortSet(CViewPortPtr* _pViewPortPtrs, unsigned int _NumberOfViewPorts)
    {
        unsigned int IndexOfViewPort;
        
        assert((_pViewPortPtrs != nullptr) && (_NumberOfViewPorts < CViewPortSet::s_MaxNumberOfViewPorts));
        
        // -----------------------------------------------------------------------------
        // Create a temporary view port set to check for redundant entries. We only need
        // the smart pointers, because they are unique.
        // -----------------------------------------------------------------------------
        CInternViewPortSet TmpViewPortSet;
        
        for (IndexOfViewPort = 0; IndexOfViewPort < _NumberOfViewPorts; ++ IndexOfViewPort)
        {
            TmpViewPortSet.m_ViewPortPtrs[IndexOfViewPort] = _pViewPortPtrs[IndexOfViewPort];
        }
        
        TmpViewPortSet.m_NumberOfViewPorts = _NumberOfViewPorts;
        
        // -----------------------------------------------------------------------------
        // We did not find a matching entry, so create a new view port set.
        // -----------------------------------------------------------------------------
        CViewPortSets::CPtr ViewPortSetPtr = m_ViewPortSets.Allocate();
        
        CInternViewPortSet& rViewPortSet = *ViewPortSetPtr;
        
        for (IndexOfViewPort = 0; IndexOfViewPort < _NumberOfViewPorts; ++ IndexOfViewPort)
        {
            CInternViewPort& rViewPort = static_cast<CInternViewPort&>(*_pViewPortPtrs[IndexOfViewPort]);
            
            rViewPortSet.m_ViewPortPtrs[IndexOfViewPort] = &rViewPort;
            rViewPortSet.m_ViewPorts   [IndexOfViewPort] =  rViewPort.m_Port;
        }
        
        rViewPortSet.m_NumberOfViewPorts = _NumberOfViewPorts;
        
        // -----------------------------------------------------------------------------
        // Return a smart pointer to the visible base class.
        // -----------------------------------------------------------------------------
        return CViewPortSetPtr(ViewPortSetPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CGfxViewManager::GetMainCamera()
    {
        return m_MainCameraPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CGfxViewManager::GetDebugCamera()
    {
        return m_DebugCameraPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CGfxViewManager::GetDecalCamera()
    {
        return m_DecalCameraPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CGfxViewManager::GetGUICamera()
    {
        return m_GUICameraPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CGfxViewManager::GetFullQuadCamera()
    {
        return m_FullQuadCameraPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortPtr CGfxViewManager::GetViewPort()
    {
        return m_ViewPortPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CGfxViewManager::GetViewPortSet()
    {
        return m_ViewPortSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxViewManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CCameraComponent>()) return;

        Dt::CCameraComponent* pCameraComponent = static_cast<Dt::CCameraComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pCameraComponent->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Check if it is a new actor
        // -----------------------------------------------------------------------------
        CInternCamera* pGraphicCamera = 0;

        if ((DirtyFlags & Dt::CCameraComponent::DirtyCreate) != 0)
        {
            CViewPtr ViewPtr = CreateView();

            pGraphicCamera = static_cast<CInternCamera*>(CreateCamera(ViewPtr).GetPtr());

            pCameraComponent->SetFacet(Dt::CCameraComponent::Graphic, pGraphicCamera);
        }
        else if ((DirtyFlags & Dt::CCameraComponent::DirtyInfo) != 0)
        {
            pGraphicCamera = static_cast<CInternCamera*>(pCameraComponent->GetFacet(Dt::CCameraComponent::Graphic));
        }

        if (pGraphicCamera)
        {
            Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

            pGraphicCamera->SetTimeStamp(FrameTime);

            if (pCameraComponent->GetClearFlag() == Dt::CCameraComponent::Texture)
            {
                if (pCameraComponent->GetTexture() != nullptr)
                {
                    unsigned int Hash = pCameraComponent->GetTexture()->GetHash();

                    CTexturePtr BackgroundTexturePtr = TextureManager::GetTextureByHash(Hash);

                    if (BackgroundTexturePtr.IsValid())
                    {
                        pGraphicCamera->SetBackgroundTexture2D(BackgroundTexturePtr);
                    }
                }
            }
        }
    }
} // namespace

namespace
{
    CGfxViewManager::CInternView::CInternView()
    : CView()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxViewManager::CInternView::~CInternView()
    {
    }
} // namespace

namespace
{
    CGfxViewManager::CInternCamera::CInternCamera()
    : CCamera()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxViewManager::CInternCamera::~CInternCamera()
    {
        Detach();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxViewManager::CInternCamera::Attach(CViewPtr _ViewPtr)
    {
        CInternView*   pView;
        CInternCamera* pCamera;
        
        assert(_ViewPtr.IsValid());
        
        m_ViewPtr = _ViewPtr;
        
        // --------------------------------------------------------------------------------
        // Attach the camera to the view instance.
        // --------------------------------------------------------------------------------
        pView = static_cast<CInternView*>(m_ViewPtr.GetPtr());
        
        if (pView->m_pFirstCamera == nullptr)
        {
            pView->m_pFirstCamera = this;
        }
        else
        {
            for (pCamera = static_cast<CInternCamera*>(pView->m_pFirstCamera); pCamera->m_pSibling != nullptr; pCamera = static_cast<CInternCamera*>(pCamera->m_pSibling));
            
            pCamera->m_pSibling = this;
        }
        
        m_pSibling = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxViewManager::CInternCamera::Detach()
    {
        CInternView*   pView;
        CInternCamera* pCamera;
        
        // --------------------------------------------------------------------------------
        // Remove the camera from the view instance.
        // --------------------------------------------------------------------------------
        pView = static_cast<CInternView*>(m_ViewPtr.GetPtr());
        
        if (pView->m_pFirstCamera == this)
        {
            pView->m_pFirstCamera = m_pSibling;
        }
        else
        {
            for (pCamera = static_cast<CInternCamera*>(pView->m_pFirstCamera); pCamera->m_pSibling != this; pCamera = static_cast<CInternCamera*>(pCamera->m_pSibling));
            
            pCamera->m_pSibling = m_pSibling;
        }
    }
} // namespace

namespace
{
    CGfxViewManager::CInternViewPort::CInternViewPort()
        : CViewPort  ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxViewManager::CInternViewPort::~CInternViewPort()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    bool CGfxViewManager::CInternViewPort::operator == (const CInternViewPort& _rViewPort) const
    {
        if (m_Port.m_Width    != _rViewPort.m_Port.m_Width   ) return false;
        if (m_Port.m_Height   != _rViewPort.m_Port.m_Height  ) return false;
        if (m_Port.m_MinDepth != _rViewPort.m_Port.m_MinDepth) return false;
        if (m_Port.m_MaxDepth != _rViewPort.m_Port.m_MaxDepth) return false;
        if (m_Port.m_TopLeftX != _rViewPort.m_Port.m_TopLeftX) return false;
        if (m_Port.m_TopLeftY != _rViewPort.m_Port.m_TopLeftY) return false;
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool CGfxViewManager::CInternViewPort::operator != (const CInternViewPort& _rViewPort) const
    {
        if (m_Port.m_Width    != _rViewPort.m_Port.m_Width   ) return true;
        if (m_Port.m_Height   != _rViewPort.m_Port.m_Height  ) return true;
        if (m_Port.m_MinDepth != _rViewPort.m_Port.m_MinDepth) return true;
        if (m_Port.m_MaxDepth != _rViewPort.m_Port.m_MaxDepth) return true;
        if (m_Port.m_TopLeftX != _rViewPort.m_Port.m_TopLeftX) return true;
        if (m_Port.m_TopLeftY != _rViewPort.m_Port.m_TopLeftY) return true;
        
        return false;
    }
} // namespace

namespace
{
    CGfxViewManager::CInternViewPortSet::CInternViewPortSet()
        : CViewPortSet()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxViewManager::CInternViewPortSet::~CInternViewPortSet()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    bool CGfxViewManager::CInternViewPortSet::operator == (const CInternViewPortSet& _rViewPortSet) const
    {
        unsigned int IndexOfViewPort;
        
        if (m_NumberOfViewPorts != _rViewPortSet.m_NumberOfViewPorts)
        {
            return false;
        }
        
        for (IndexOfViewPort = 0; IndexOfViewPort < m_NumberOfViewPorts; ++ IndexOfViewPort)
        {
            if (m_ViewPortPtrs[IndexOfViewPort] != _rViewPortSet.m_ViewPortPtrs[IndexOfViewPort])
            {
                return false;
            }
        }
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool CGfxViewManager::CInternViewPortSet::operator != (const CInternViewPortSet& _rViewPortSet) const
    {
        Base::Size IndexOfViewPorts;
        
        if (m_NumberOfViewPorts != _rViewPortSet.m_NumberOfViewPorts)
        {
            return true;
        }
        
        for (IndexOfViewPorts = 0; IndexOfViewPorts < m_NumberOfViewPorts; ++ IndexOfViewPorts)
        {
            if (m_ViewPortPtrs[IndexOfViewPorts] != _rViewPortSet.m_ViewPortPtrs[IndexOfViewPorts])
            {
                return true;
            }
        }
        
        return false;
    }
} // namespace

namespace Gfx
{
namespace ViewManager
{
    void OnStart()
    {
        CGfxViewManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxViewManager::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPtr CreateView()
    {
        return CGfxViewManager::GetInstance().CreateView();
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CreateCamera(CViewPtr _ViewPtr)
    {
        return CGfxViewManager::GetInstance().CreateCamera(_ViewPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortPtr CreateViewPort(const SViewPortDescriptor& _rDescriptor)
    {
        return CGfxViewManager::GetInstance().CreateViewPort(_rDescriptor);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr)
    {
        CViewPortPtr ViewPortPtrs[] = { _ViewPort1Ptr, };
        
        return CGfxViewManager::GetInstance().CreateViewPortSet(ViewPortPtrs, 1);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr)
    {
        CViewPortPtr ViewPortPtrs[] = { _ViewPort1Ptr, _ViewPort2Ptr, };
        
        return CGfxViewManager::GetInstance().CreateViewPortSet(ViewPortPtrs, 2);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr)
    {
        CViewPortPtr ViewPortPtrs[] = { _ViewPort1Ptr, _ViewPort2Ptr, _ViewPort3Ptr, };
        
        return CGfxViewManager::GetInstance().CreateViewPortSet(ViewPortPtrs, 3);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr, CViewPortPtr _ViewPort4Ptr)
    {
        CViewPortPtr ViewPortPtrs[] = { _ViewPort1Ptr, _ViewPort2Ptr, _ViewPort3Ptr, _ViewPort4Ptr, };
        
        return CGfxViewManager::GetInstance().CreateViewPortSet(ViewPortPtrs, 4);
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr* _pViewPortPtrs, unsigned int _NumberOfViewPorts)
    {
        return CGfxViewManager::GetInstance().CreateViewPortSet(_pViewPortPtrs, _NumberOfViewPorts);
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr GetMainCamera()
    {
        return CGfxViewManager::GetInstance().GetMainCamera();
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr GetDebugCamera()
    {
        return CGfxViewManager::GetInstance().GetDebugCamera();
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr GetDecalCamera()
    {
        return CGfxViewManager::GetInstance().GetDecalCamera();
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr GetGUICamera()
    {
        return CGfxViewManager::GetInstance().GetGUICamera();
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr GetFullQuadCamera()
    {
        return CGfxViewManager::GetInstance().GetFullQuadCamera();
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortPtr GetViewPort()
    {
        return CGfxViewManager::GetInstance().GetViewPort();
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr GetViewPortSet()
    {
        return CGfxViewManager::GetInstance().GetViewPortSet();
    }
} // namespace ViewManager
} // namespace Gfx

