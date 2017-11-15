
#include "graphic/gfx_precompiled.h"

#include "app_droid/app_application.h"

#include "base/base_crc.h"
#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_texture_manager.h"
#include "data/data_texture_base.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_native_texture_2d.h"
#include "graphic/gfx_native_texture_3d.h"
#include "graphic/gfx_texture_manager.h"

// #include "IL/il.h"
// #include "IL/ilu.h"

#include <unordered_map>

using namespace Gfx;

namespace
{
    std::string g_PathToAssets       = "/assets/";
    std::string g_PathToDataTextures = "/data/graphic/textures/";
} // namespace

namespace
{
    class CGfxTextureManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxTextureManager)
        
    public:

        CGfxTextureManager();
       ~CGfxTextureManager();

    public:

        void OnStart();
        void OnExit();

    public:

        void OnResize(int _Width, int _Height);

    public:

        CTexture1DPtr GetDummyTexture1D();
        CTexture2DPtr GetDummyTexture2D();
        CTexture3DPtr GetDummyTexture3D();
        CTexture2DPtr GetDummyCubeTexture();

        CTexture1DPtr CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);
        CTexture2DPtr CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);
        CTexture3DPtr CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        CTexture2DPtr CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        CTextureSetPtr CreateTextureSet(CTextureBasePtr* _pTexturePtrs, unsigned int _NumberOfTextures);

        CTexture1DPtr GetTexture1DByHash(unsigned int _Hash);
        CTexture2DPtr GetTexture2DByHash(unsigned int _Hash);
        CTexture3DPtr GetTexture3DByHash(unsigned int _Hash);

        void ClearTexture1D(CTexture1DPtr _TexturePtr, const Base::Float4& _rColor);
        void ClearTexture2D(CTexture2DPtr _TexturePtr, const Base::Float4& _rColor);
        void ClearTexture3D(CTexture3DPtr _TexturePtr, const Base::Float4& _rColor);

        void CopyToTexture2D(CTexture2DPtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels);
        void CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels);
        void CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexture2DPtr _TexturePtr, bool _UpdateMipLevels);

        CTexture2DPtr GetMipmapFromTexture2D(CTexture2DPtr _TexturePtr, unsigned int _Mipmap);
        
        void UpdateMipmap(CTexture2DPtr _TexturePtr);

        void SetTexture2DLabel(CTexture2DPtr _TexturePtr, const char* _pLabel);
        void SetTexture3DLabel(CTexture3DPtr _TexturePtr, const char* _pLabel);

    private:

        // -----------------------------------------------------------------------------
        // Represents a 1D texture.
        // -----------------------------------------------------------------------------
        class CInternTexture1D : public CTexture1D
        {
            public:

                CInternTexture1D();
               ~CInternTexture1D();

            private:

                friend class  CGfxTextureManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a 2D texture.
        // -----------------------------------------------------------------------------
        class CInternTexture2D : public CNativeTexture2D
        {
            public:

                CInternTexture2D();
               ~CInternTexture2D();

            private:

                friend class  CGfxTextureManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a 3D texture.
        // -----------------------------------------------------------------------------
        class CInternTexture3D : public CNativeTexture3D
        {
            public:

                CInternTexture3D();
               ~CInternTexture3D();

            private:

                friend class  CGfxTextureManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a unique combination of up to 16 textures.
        // -----------------------------------------------------------------------------
        class CInternTextureSet : public CTextureSet
        {
            public:

                CInternTextureSet();
               ~CInternTextureSet();

            public:

                bool operator == (const CInternTextureSet& _rTextureSet) const;
                bool operator != (const CInternTextureSet& _rTextureSet) const;

            private:

                friend class CGfxTextureManager;
        };

        typedef Base::CManagedPool<CInternTextureSet, 16> CTextureSets;

        // -----------------------------------------------------------------------------
        // There are way more 2D textures than 1D or 3D ones, so use bigger pages here.
        // -----------------------------------------------------------------------------
        typedef Base::CManagedPool<CInternTexture1D,  16, 0> CTexture1Ds;
        typedef Base::CManagedPool<CInternTexture2D, 256, 0> CTexture2Ds;
        typedef Base::CManagedPool<CInternTexture3D,  16, 0> CTexture3Ds;
        
        typedef std::unordered_map<unsigned int, CTexture1DPtr> CTexture1DByHashs;
        typedef std::unordered_map<unsigned int, CTexture2DPtr> CTexture2DByHashs;
        typedef std::unordered_map<unsigned int, CTexture3DPtr> CTexture3DByHashs;

    private:

        CTexture1Ds          m_Textures1D;
        CTexture2Ds          m_Textures2D;
        CTexture3Ds          m_Textures3D;
        CTexture1DByHashs    m_Textures1DByHash;
        CTexture2DByHashs    m_Textures2DByHash;
        CTexture3DByHashs    m_Textures3DByHash;
        CTextureSets         m_TextureSets;
        
        CTexture1DPtr        m_Texture1DPtr;
        CTexture2DPtr        m_Texture2DPtr;

    private:

        void OnDirtyTexture(Dt::CTextureBase* _pTexture);

        CTexture1DPtr InternCreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);
        CTexture2DPtr InternCreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);
        CTexture3DPtr InternCreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        CTexture2DPtr InternCreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        int ConvertGLFormatToBytesPerPixel(Gfx::CTextureBase::EFormat _Format) const;
        int ConvertGLImageUsage(Gfx::CTextureBase::EUsage _Usage) const;
        int ConvertGLInternalImageFormat(Gfx::CTextureBase::EFormat _Format) const;
        int ConvertGLImageFormat(Gfx::CTextureBase::EFormat _Format) const;
        int ConvertGLImageType(Gfx::CTextureBase::EFormat _Format) const;

//         ILenum ConvertILImageFormat(Gfx::CTextureBase::EFormat _Format) const;
//         ILenum ConvertILImageType(Gfx::CTextureBase::EFormat _Format) const;

        Gfx::CTextureBase::EDimension ConvertDataDimension(Dt::CTextureBase::EDimension _Dimension);
        Gfx::CTextureBase::EFormat ConvertDataFormat(Dt::CTextureBase::EFormat _Format);
        Gfx::CTextureBase::ESemantic ConvertDataSemantic(Dt::CTextureBase::ESemantic _Semantic);
        unsigned int ConvertDataBinding(unsigned int _Binding);

    private:

        friend class CInternTextureSet;
        friend class CInternTargetSet;
    };
} // namespace

namespace
{
    CGfxTextureManager::CGfxTextureManager()
        : m_Textures1D               ()
        , m_Textures2D               ()
        , m_Textures3D               ()
        , m_Textures1DByHash         ()
        , m_Textures2DByHash         ()
        , m_Textures3DByHash         ()
        , m_TextureSets              ()
        , m_Texture1DPtr             ()
        , m_Texture2DPtr             ()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxTextureManager::~CGfxTextureManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Initialize devil image engine. But we only initialize core part.
        // -----------------------------------------------------------------------------
//         ilInit();
        
        // -----------------------------------------------------------------------------
        // Register for resizing events
        // -----------------------------------------------------------------------------
        Gfx::Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxTextureManager::OnResize));
        
        // -----------------------------------------------------------------------------
        // Create dummy textures from file
        // -----------------------------------------------------------------------------
        Gfx::STextureDescriptor TextureDescriptor;
        
        TextureDescriptor.m_NumberOfPixelsU  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = Gfx::STextureDescriptor::s_NumberOfTexturesFromSource;
        TextureDescriptor.m_Binding          = Gfx::CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = Gfx::CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = Gfx::CTextureBase::Unknown;
        TextureDescriptor.m_Usage            = Gfx::CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = Gfx::CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName        = "dummy_2d.tga";
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = Gfx::CTextureBase::R8G8B8_UBYTE;
        
        m_Texture2DPtr = CreateTexture2D(TextureDescriptor, true, SDataBehavior::LeftAlone);

        SetTexture2DLabel(m_Texture2DPtr, "Dummy Texture 2D");
        
        // -----------------------------------------------------------------------------
        // Setup default settings in OpenGL
        // -----------------------------------------------------------------------------
#ifndef __ANDROID__
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif // __ANDROID__

        // -----------------------------------------------------------------------------
        // Set dirty handler of data textures
        // -----------------------------------------------------------------------------
        Dt::TextureManager::RegisterDirtyTextureHandler(DATA_DIRTY_TEXTURE_METHOD(&CGfxTextureManager::OnDirtyTexture));
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::OnExit()
    {
        m_Texture1DPtr = 0;
        m_Texture2DPtr = 0;

        // -----------------------------------------------------------------------------
        // Clear all the pools with the textures.
        // -----------------------------------------------------------------------------
        m_Textures1DByHash.clear();
        m_Textures2DByHash.clear();
        m_Textures3DByHash.clear();

        m_Textures1D.Clear();
        m_Textures2D.Clear();
        m_Textures3D.Clear();

        // -----------------------------------------------------------------------------
        // Clear all the sets.
        // -----------------------------------------------------------------------------
        m_TextureSets.Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::OnResize(int _Width, int _Height)
    {
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }
    
    // -----------------------------------------------------------------------------
    
    CTexture1DPtr CGfxTextureManager::GetDummyTexture1D()
    {
        return m_Texture1DPtr;
    }
    
    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::GetDummyTexture2D()
    {
        return m_Texture2DPtr;
    }
    
    // -----------------------------------------------------------------------------

    CTexture3DPtr CGfxTextureManager::GetDummyTexture3D()
    {
        return nullptr;
    }
    
    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::GetDummyCubeTexture()
    {
        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTexture1DPtr CGfxTextureManager::CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return InternCreateTexture1D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        int          NumberOfBytes;
        unsigned int Hash;

        Hash = 0;

        // -----------------------------------------------------------------------------
        // Create hash value over filename
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && strlen(_rDescriptor.m_pFileName))
        {
            NumberOfBytes     = static_cast<unsigned int>(strlen(_rDescriptor.m_pFileName) * sizeof(char));
            const void* pData = static_cast<const void*>(_rDescriptor.m_pFileName);
            
            Hash = Base::CRC32(pData, NumberOfBytes);
            
            if (m_Textures2DByHash.find(Hash) != m_Textures2DByHash.end())
            {
                return m_Textures2DByHash.at(Hash);
            }
        }            

        // -----------------------------------------------------------------------------
        // Texture
        // -----------------------------------------------------------------------------
        CTexture2DPtr Texture2DPtr = InternCreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior);

        CInternTexture2D* pInternTexture2D = static_cast<CInternTexture2D*>(Texture2DPtr.GetPtr());

        if (pInternTexture2D == nullptr)
        {
            return GetDummyTexture2D();
        }

        if (Hash != 0)
        {
            pInternTexture2D->m_Hash = Hash;

            m_Textures2DByHash[Hash] = Texture2DPtr;
        }

        return Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture3DPtr CGfxTextureManager::CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return InternCreateTexture3D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return InternCreateCubeTexture(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGfxTextureManager::CreateTextureSet(CTextureBasePtr* _pTexturePtrs, unsigned int _NumberOfTextures)
    {
        // -----------------------------------------------------------------------------
        // Allocate item in texture set array
        // -----------------------------------------------------------------------------
        CTextureSets::CPtr TextureSetPtr = m_TextureSets.Allocate();
        
        CInternTextureSet& rTextureSet = *TextureSetPtr;
        
        // -----------------------------------------------------------------------------
        // Fill internal buffer set with general data
        // -----------------------------------------------------------------------------
        rTextureSet.m_NumberOfTextures = _NumberOfTextures;
        
        // -----------------------------------------------------------------------------
        // Setup internal buffer set with buffer information and define buffer array
        // by binding all buffers to it.
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfTexture = 0; IndexOfTexture < _NumberOfTextures; ++ IndexOfTexture)
        {
            rTextureSet.m_TexturePtrs[IndexOfTexture] = _pTexturePtrs[IndexOfTexture];
        }
        
        return CTextureSetPtr(TextureSetPtr);
    }

    // -----------------------------------------------------------------------------

    CTexture1DPtr CGfxTextureManager::GetTexture1DByHash(unsigned int _Hash)
    {
        BASE_UNUSED(_Hash);

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::GetTexture2DByHash(unsigned int _Hash)
    {
        if (m_Textures2DByHash.find(_Hash) != m_Textures2DByHash.end())
        {
            return m_Textures2DByHash.at(_Hash);
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTexture3DPtr CGfxTextureManager::GetTexture3DByHash(unsigned int _Hash)
    {
        BASE_UNUSED(_Hash);

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::ClearTexture1D(CTexture1DPtr _TexturePtr, const Base::Float4& _rColor)
    {
        BASE_UNUSED(_TexturePtr);
        BASE_UNUSED(_rColor);
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::ClearTexture2D(CTexture2DPtr _TexturePtr, const Base::Float4& _rColor)
    {
        BASE_UNUSED(_TexturePtr);
        BASE_UNUSED(_rColor);
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::ClearTexture3D(CTexture3DPtr _TexturePtr, const Base::Float4& _rColor)
    {
        BASE_UNUSED(_TexturePtr);
        BASE_UNUSED(_rColor);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxTextureManager::CopyToTexture2D(CTexture2DPtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        BASE_UNUSED(_NumberOfBytesPerLine);

        // -----------------------------------------------------------------------------
        // Get informations
        // -----------------------------------------------------------------------------
        assert(_TexturePtr.IsValid());
        assert(_pBytes != 0);
        
        Base::UInt2 Offset     = _rTargetRect[0];
        Base::UInt2 UpdateSize = _rTargetRect[1] - _rTargetRect[0];
        
        assert(_TexturePtr->GetNumberOfPixelsU() <= UpdateSize[0] + Offset[0]);
        assert(_TexturePtr->GetNumberOfPixelsV() <= UpdateSize[1] + Offset[1]);
        
        CInternTexture2D* pInternTexture = static_cast<CInternTexture2D*>(_TexturePtr.GetPtr());
        
        Gfx::CNativeTextureHandle TextureHandle = pInternTexture->m_NativeTexture;
        
        int Format = ConvertGLImageFormat(_TexturePtr->GetFormat());
        int Type   = ConvertGLImageType  (_TexturePtr->GetFormat());
        
        // -----------------------------------------------------------------------------
        // Upload data to texture
        // -----------------------------------------------------------------------------
        glBindTexture(GL_TEXTURE_2D, TextureHandle);

        glTexSubImage2D(GL_TEXTURE_2D, 0, Offset[0], Offset[1], UpdateSize[0], UpdateSize[1], Format, Type, _pBytes);
        
        if (_UpdateMipLevels)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        BASE_UNUSED(_UpdateMipLevels);
        BASE_UNUSED(_NumberOfBytesPerLine);
                
        // -----------------------------------------------------------------------------
        // Get informations
        // -----------------------------------------------------------------------------
        Base::UInt2 Size = _rTargetRect.Size();

        assert(_pBytes != 0);
        assert(_TextureArrayPtr.IsValid());
        assert(_TextureArrayPtr->GetNumberOfPixelsU() == Size[0] && _TextureArrayPtr->GetNumberOfPixelsV() == Size[1]);
        
        Base::UInt2 Offset     = Base::UInt2(0);
        Base::UInt2 UpdateSize = Base::UInt2(_TextureArrayPtr->GetNumberOfPixelsU(), _TextureArrayPtr->GetNumberOfPixelsV());
        
        assert(Size[0] <= UpdateSize[0] + Offset[0]);
        assert(Size[1] <= UpdateSize[1] + Offset[1]);
        
        CInternTexture2D* pInternTextureArray = static_cast<CInternTexture2D*>(_TextureArrayPtr.GetPtr());

        Gfx::CNativeTextureHandle TextureHandle = pInternTextureArray->m_NativeTexture;

        int Format = ConvertGLImageFormat(pInternTextureArray->GetFormat());
        int Type   = ConvertGLImageType  (pInternTextureArray->GetFormat());
        
        // -----------------------------------------------------------------------------
        // Upload data to texture
        // -----------------------------------------------------------------------------
        if (pInternTextureArray->m_Info.m_IsCubeTexture)
        {
            glBindTexture(GL_TEXTURE_2D, TextureHandle);

            glTexSubImage3D(GL_TEXTURE_2D, 0, Offset[0], Offset[1], _IndexOfSlice, UpdateSize[0], UpdateSize[1], 1, Format, Type, _pBytes);
        }
        else
        {
            // TODO by tschwandt
            // Do this not only for cubemaps!

            BASE_CONSOLE_STREAMWARNING("Copy to texture array is actually not supported.");
        }
    }

    // -----------------------------------------------------------------------------
    
    void CGfxTextureManager::CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexture2DPtr _TexturePtr, bool _UpdateMipLevels)
    {
        BASE_UNUSED(_UpdateMipLevels);

        // -----------------------------------------------------------------------------
        // Get informations
        // -----------------------------------------------------------------------------
        assert(_TextureArrayPtr.IsValid());
        assert(_TexturePtr     .IsValid());
        assert(_TextureArrayPtr->GetNumberOfPixelsU() == _TexturePtr->GetNumberOfPixelsU() && _TextureArrayPtr->GetNumberOfPixelsV() == _TexturePtr->GetNumberOfPixelsV());
        
        Base::UInt2 Offset     = Base::UInt2(0);
        Base::UInt2 UpdateSize = Base::UInt2(_TextureArrayPtr->GetNumberOfPixelsU(), _TextureArrayPtr->GetNumberOfPixelsV());
        
        assert(_TexturePtr->GetNumberOfPixelsU() <= UpdateSize[0] + Offset[0]);
        assert(_TexturePtr->GetNumberOfPixelsV() <= UpdateSize[1] + Offset[1]);
        
        CInternTexture2D* pInternTextureArray = static_cast<CInternTexture2D*>(_TextureArrayPtr.GetPtr());
        CInternTexture2D* pInternTexture = static_cast<CInternTexture2D*>(_TexturePtr.GetPtr());
        
        Gfx::CNativeTextureHandle TextureHandle = pInternTextureArray->m_NativeTexture;
        
        int Format = ConvertGLImageFormat(pInternTextureArray->GetFormat());
        int Type   = ConvertGLImageType  (pInternTextureArray->GetFormat());
        
        // -----------------------------------------------------------------------------
        // Upload data to texture
        // -----------------------------------------------------------------------------
        if (pInternTextureArray->m_Info.m_IsCubeTexture)
        {
            glBindTexture(GL_TEXTURE_2D, TextureHandle);

            glTexSubImage3D(GL_TEXTURE_2D, 0, Offset[0], Offset[1], _IndexOfSlice, UpdateSize[0], UpdateSize[1], 1, Format, Type, pInternTexture->GetPixels());
        }
        else
        {
            // TODO by tschwandt
            // Do this not only for cubemaps!

            BASE_CONSOLE_STREAMWARNING("Copy to texture array is actually not supported.");
        }
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::GetMipmapFromTexture2D(CTexture2DPtr _TexturePtr, unsigned int _Mipmap)
    {
        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CTexture2Ds::CPtr Texture2DPtr;

        assert(_Mipmap < _TexturePtr->GetNumberOfMipLevels());

        try
        {
            Texture2DPtr = m_Textures2D.Allocate();
            
            CInternTexture2D& rTexture = *Texture2DPtr;
            
            unsigned int MipmapPow = Base::Pow(2, _Mipmap);

            rTexture.m_FileName          = _TexturePtr->GetFileName();
            rTexture.m_pPixels           = _TexturePtr->GetPixels();
            rTexture.m_NumberOfPixels[0] = static_cast<Base::U16>(Base::Max(static_cast<unsigned int>(_TexturePtr->GetNumberOfPixelsU()) / MipmapPow, 1u));
            rTexture.m_NumberOfPixels[1] = static_cast<Base::U16>(Base::Max(static_cast<unsigned int>(_TexturePtr->GetNumberOfPixelsV()) / MipmapPow, 1u));
            
            rTexture.m_Info.m_Access            = _TexturePtr->GetAccess();
            rTexture.m_Info.m_Binding           = _TexturePtr->GetBinding();
            rTexture.m_Info.m_Dimension         = _TexturePtr->GetDimension();
            rTexture.m_Info.m_Format            = _TexturePtr->GetFormat();
            rTexture.m_Info.m_IsCubeTexture     = _TexturePtr->IsCube();
            rTexture.m_Info.m_IsDeletable       = false;
            rTexture.m_Info.m_IsDummyTexture    = _TexturePtr->IsDummy();
            rTexture.m_Info.m_NumberOfTextures  = _TexturePtr->GetNumberOfTextures();
            rTexture.m_Info.m_NumberOfMipLevels = 1;
            rTexture.m_Info.m_CurrentMipLevel   = _Mipmap;
            rTexture.m_Info.m_Semantic          = _TexturePtr->GetSemantic();
            rTexture.m_Info.m_Usage             = _TexturePtr->GetUsage();
            
            CInternTexture2D* pInternalTexture = static_cast<CInternTexture2D*>(_TexturePtr.GetPtr());

            rTexture.m_NativeTexture        = pInternalTexture->m_NativeTexture;
            rTexture.m_NativeDimension      = pInternalTexture->m_NativeDimension;
            rTexture.m_NativeInternalFormat = pInternalTexture->m_NativeInternalFormat;
            rTexture.m_NativeUsage          = pInternalTexture->m_NativeUsage;
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }
        
        return CTexture2DPtr(Texture2DPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxTextureManager::UpdateMipmap(CTexture2DPtr _TexturePtr)
    {
        assert(_TexturePtr != 0);
        
        CInternTexture2D* pInternTexture = static_cast<CInternTexture2D*>(_TexturePtr.GetPtr());

        assert(pInternTexture);

        glBindTexture(GL_TEXTURE_2D, pInternTexture->m_NativeTexture);

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::SetTexture2DLabel(CTexture2DPtr _TexturePtr, const char* _pLabel)
    {
        assert(_pLabel != nullptr);

        CInternTexture2D* pInternTexture = static_cast<CInternTexture2D*>(_TexturePtr.GetPtr());

        glObjectLabel(GL_TEXTURE, pInternTexture->m_NativeTexture, -1, _pLabel);
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::SetTexture3DLabel(CTexture3DPtr _TexturePtr, const char* _pLabel)
    {
        assert(_pLabel != nullptr);

        CInternTexture3D* pInternTexture = static_cast<CInternTexture3D*>(_TexturePtr.GetPtr());

        glObjectLabel(GL_TEXTURE, pInternTexture->m_NativeTexture, -1, _pLabel);
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::OnDirtyTexture(Dt::CTextureBase* _pTexture)
    {
        if (_pTexture == nullptr) return;

        unsigned int DirtyFlags = _pTexture->GetDirtyFlags();
        unsigned int Hash       = _pTexture->GetHash();
        unsigned int Binding    = _pTexture->GetBinding();

        // -----------------------------------------------------------------------------
        // Check if binding is related to graphics
        // -----------------------------------------------------------------------------
        if (Binding == Dt::CTextureBase::CPU) return;

        // -----------------------------------------------------------------------------
        // Create
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CTextureBase::DirtyCreate) != 0)
        {
            // -----------------------------------------------------------------------------
            // Create descriptor
            // -----------------------------------------------------------------------------
            STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = 1;
            TextureDescriptor.m_NumberOfPixelsV  = 1;
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
            TextureDescriptor.m_Usage            = CTextureBase::GPURead;
            TextureDescriptor.m_Semantic         = ConvertDataSemantic(_pTexture->GetSemantic());
            TextureDescriptor.m_pFileName        = 0;
            TextureDescriptor.m_pPixels          = _pTexture->GetPixels();
            TextureDescriptor.m_Binding          = ConvertDataBinding(_pTexture->GetBinding());
            TextureDescriptor.m_Format           = ConvertDataFormat(_pTexture->GetFormat());

            if (_pTexture->GetFileName().length() > 0) TextureDescriptor.m_pFileName = _pTexture->GetFileName().c_str();

            // -----------------------------------------------------------------------------
            // Depending on dimension create the texture
            // -----------------------------------------------------------------------------
            if (_pTexture->GetDimension() == Dt::CTextureBase::Dim1D)
            {
                Dt::CTexture1D* pDataTexture = static_cast<Dt::CTexture1D*>(_pTexture);

                TextureDescriptor.m_NumberOfPixelsU = pDataTexture->GetNumberOfPixelsU();

                InternCreateTexture1D(TextureDescriptor, true, Gfx::SDataBehavior::LeftAlone);
            }
            else if (_pTexture->GetDimension() == Dt::CTextureBase::Dim2D)
            {
                CTexture2DPtr Texture2DPtr = nullptr;

                if (m_Textures2DByHash.find(Hash) != m_Textures2DByHash.end())
                {
                    BASE_CONSOLE_STREAMWARNING("Trying to re-create an already created data texture in graphics texture manager. Creation aborted...");

                    return ;
                }

                // -----------------------------------------------------------------------------
                // Create
                // -----------------------------------------------------------------------------
                if (_pTexture->IsCube())
                {
                    Dt::CTextureCube* pDataTexture = static_cast<Dt::CTextureCube*>(_pTexture);

                    TextureDescriptor.m_NumberOfPixelsU  = pDataTexture->GetNumberOfPixelsU();
                    TextureDescriptor.m_NumberOfPixelsV  = pDataTexture->GetNumberOfPixelsV();
                    TextureDescriptor.m_NumberOfTextures = 6;

                    Texture2DPtr = InternCreateCubeTexture(TextureDescriptor, true, Gfx::SDataBehavior::LeftAlone);
                }
                else
                {
                    Dt::CTexture2D* pDataTexture = static_cast<Dt::CTexture2D*>(_pTexture);

                    TextureDescriptor.m_NumberOfPixelsU = pDataTexture->GetNumberOfPixelsU();
                    TextureDescriptor.m_NumberOfPixelsV = pDataTexture->GetNumberOfPixelsV();

                    Texture2DPtr = InternCreateTexture2D(TextureDescriptor, true, Gfx::SDataBehavior::LeftAlone);
                }

                // -----------------------------------------------------------------------------
                // Label if an identifier exists
                // -----------------------------------------------------------------------------
                const char* pLabel = _pTexture->GetIdentifier().length() > 0 ? _pTexture->GetIdentifier().c_str() : 0;

                if (pLabel != 0)
                {
                    SetTexture2DLabel(Texture2DPtr, pLabel);
                }

                // -----------------------------------------------------------------------------
                // Set to container
                // -----------------------------------------------------------------------------
                CInternTexture2D* pInternTexture2D = static_cast<CInternTexture2D*>(Texture2DPtr.GetPtr());

                if (pInternTexture2D == nullptr)
                {
                    pInternTexture2D = static_cast<CInternTexture2D*>(m_Texture2DPtr.GetPtr());
                }

                if (Hash != 0)
                {
                    pInternTexture2D->m_Hash = Hash;

                    m_Textures2DByHash[Hash] = pInternTexture2D;
                }
            }
            else if (_pTexture->GetDimension() == Dt::CTextureBase::Dim3D)
            {
                // TODO by tschwandt
                // Not implemented yet

                BASE_CONSOLE_STREAMWARNING("Texture 3D from data textures is not yet supported!");
            }
        }

        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        if ((DirtyFlags & Dt::CTextureBase::DirtyData) != 0)
        {
            if (_pTexture->GetDimension() == Dt::CTextureBase::Dim2D)
            {
                if (m_Textures2DByHash.find(Hash) == m_Textures2DByHash.end())
                {
                    BASE_CONSOLE_STREAMWARNING("Data texture manager tried to update data from non-created graphic texture.");

                    return;
                }

                if (_pTexture->IsCube())
                {
                    Gfx::CTexture2D*  pGraphicTexture = m_Textures2DByHash.at(Hash);
                    Dt::CTextureCube* pDataTexture = static_cast<Dt::CTextureCube*>(_pTexture);

                    Base::UInt2 CubemapResolution = Base::UInt2(pDataTexture->GetNumberOfPixelsU(), pDataTexture->GetNumberOfPixelsV());

                    Base::AABB2UInt CubemapRect(Base::UInt2(0), CubemapResolution);

                    CopyToTextureArray2D(pGraphicTexture, 0, CubemapRect, CubemapRect[1][0], pDataTexture->GetFace(Dt::CTextureCube::Right)->GetPixels(), false);
                    CopyToTextureArray2D(pGraphicTexture, 1, CubemapRect, CubemapRect[1][0], pDataTexture->GetFace(Dt::CTextureCube::Left)->GetPixels(), false);
                    CopyToTextureArray2D(pGraphicTexture, 2, CubemapRect, CubemapRect[1][0], pDataTexture->GetFace(Dt::CTextureCube::Top)->GetPixels(), false);
                    CopyToTextureArray2D(pGraphicTexture, 3, CubemapRect, CubemapRect[1][0], pDataTexture->GetFace(Dt::CTextureCube::Bottom)->GetPixels(), false);
                    CopyToTextureArray2D(pGraphicTexture, 4, CubemapRect, CubemapRect[1][0], pDataTexture->GetFace(Dt::CTextureCube::Front)->GetPixels(), false);
                    CopyToTextureArray2D(pGraphicTexture, 5, CubemapRect, CubemapRect[1][0], pDataTexture->GetFace(Dt::CTextureCube::Back)->GetPixels(), false);

                    UpdateMipmap(pGraphicTexture);
                }
                else
                {
                    Gfx::CTexture2D* pGraphicTexture = m_Textures2DByHash.at(Hash);
                    Dt::CTexture2D*  pDataTexture = static_cast<Dt::CTexture2D*>(_pTexture);

                    if (pDataTexture->GetPixels() != nullptr)
                    {   
                        Base::UInt2 TextureResolution = Base::UInt2(pDataTexture->GetNumberOfPixelsU(), pDataTexture->GetNumberOfPixelsV());

                        Base::AABB2UInt TargetRect(Base::UInt2(0), TextureResolution);

                        CopyToTexture2D(pGraphicTexture, TargetRect, TargetRect[1][0], pDataTexture->GetPixels(), true);
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    CTexture1DPtr CGfxTextureManager::InternCreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        BASE_UNUSED(_rDescriptor);
        BASE_UNUSED(_IsDeleteable);
        BASE_UNUSED(_Behavior);

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::InternCreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        bool         Result;
        void*        pBytes;
        void*        pTextureData;
        unsigned int ImageWidth;
        unsigned int ImageHeight;
        unsigned int NumberOfPixel;
        unsigned int NumberOfBytes;
        unsigned int NumberOfMipmaps;
        int          GLInternalFormat;
        int          GLFormat;
        int          GLUsage;
        int          GLType;
        GLuint       NativeTextureHandle;
//         ILuint       NativeImageName;
//         ILenum       NativeILFormat;
//         ILenum       NativeILType;

        pBytes       = nullptr;
        pTextureData = nullptr;
        
        // -----------------------------------------------------------------------------
        // Setup variables
        // -----------------------------------------------------------------------------
        ImageWidth       = _rDescriptor.m_NumberOfPixelsU;
        ImageHeight      = _rDescriptor.m_NumberOfPixelsV;
        pBytes           = nullptr;
        pTextureData     = _rDescriptor.m_pPixels;
        NumberOfPixel    = ImageWidth * ImageHeight;
        NumberOfMipmaps  = _rDescriptor.m_NumberOfMipMaps;
        NumberOfBytes    = ConvertGLFormatToBytesPerPixel(_rDescriptor.m_Format) * NumberOfPixel;
        GLInternalFormat = ConvertGLInternalImageFormat(_rDescriptor.m_Format);
        GLFormat         = ConvertGLImageFormat(_rDescriptor.m_Format);
        GLType           = ConvertGLImageType(_rDescriptor.m_Format);
        GLUsage          = ConvertGLImageUsage(_rDescriptor.m_Usage);

//         NativeILFormat  = ConvertILImageFormat(_rDescriptor.m_Format);
//         NativeILType    = ConvertILImageType(_rDescriptor.m_Format);
// 
//         NativeImageName = 0;

        // -----------------------------------------------------------------------------
        // Load texture from file if one is set in descriptor
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr)
        {
            // -----------------------------------------------------------------------------
            // Create and bin texture on DevIL
            // -----------------------------------------------------------------------------
//             NativeImageName = ilGenImage();
// 
//             ilBindImage(NativeImageName);
// 
//             // -----------------------------------------------------------------------------
//             // Load texture from file (either in assets or data)
//             // -----------------------------------------------------------------------------
//             std::string PathToTexture;
// 
//             PathToTexture = App::Application::GetAssetPath() + g_PathToAssets + _rDescriptor.m_pFileName;
// 
// #ifdef __ANDROID__
//             const char* pPathToTexture = 0;
// 
//             pPathToTexture = PathToTexture.c_str();
// #else
//             const wchar_t* pPathToTexture = 0;
// 
//             pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
// #endif
// 
//             Result = ilLoadImage(pPathToTexture) == IL_TRUE;
// 
//             if (!Result)
//             {
//                 PathToTexture = App::Application::GetAssetPath() + g_PathToDataTextures + _rDescriptor.m_pFileName;
// 
// #ifdef __ANDROID__
//                 pPathToTexture = PathToTexture.c_str();
// #else
//                 pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
// #endif
//                 
// 
//                 Result = ilLoadImage(pPathToTexture) == IL_TRUE;
//             }
//             
//             if (Result)
//             {
//                 ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
//                 ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);
// 
//                 if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
//                 {
//                     ilConvertImage(NativeILFormat, NativeILType);
//                 }
//                 
//                 pTextureData = ilGetData();
//                 
//                 ImageWidth    = ilGetInteger(IL_IMAGE_WIDTH);
//                 ImageHeight   = ilGetInteger(IL_IMAGE_HEIGHT);
//                 NumberOfBytes = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
//                 
//                 assert(ImageWidth    > 0);
//                 assert(ImageHeight   > 0);
//                 assert(NumberOfBytes > 0);
//             }
//             else
//             {
//                 BASE_CONSOLE_STREAMERROR("Failed loading image '" << PathToTexture.c_str() << "' from file.");
// 
//                 ilDeleteImage(NativeImageName);
// 
//                 return nullptr;
//             }
        }

        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            NumberOfMipmaps = static_cast<int>(Base::Log2(static_cast<float>(Base::Max(ImageWidth, ImageHeight)))) + 1;
        }
        else if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_NumberOfMipMapsFromSource)
        {
            // NumberOfMipmaps = ilGetInteger(IL_NUM_MIPMAPS);
        }
        
        // -----------------------------------------------------------------------------
        // Generate OpenGL texture or render buffer
        // -----------------------------------------------------------------------------
        glGenTextures(1, &NativeTextureHandle);

        glBindTexture(GL_TEXTURE_2D, NativeTextureHandle);

        // -----------------------------------------------------------------------------
        // Label texture if file name exists
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr)
        {
            glObjectLabel(GL_TEXTURE, NativeTextureHandle, -1, _rDescriptor.m_pFileName);
        }

        // -----------------------------------------------------------------------------
        // Binding
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_Binding & Gfx::CTextureBase::DepthStencilTarget)
        {
            glTexStorage2D(GL_TEXTURE_2D, NumberOfMipmaps, GL_DEPTH_COMPONENT32F, ImageWidth, ImageHeight);
        }
        else if (_rDescriptor.m_Binding & Gfx::CTextureBase::RenderTarget)
        {   
            glTexStorage2D(GL_TEXTURE_2D, NumberOfMipmaps, GLInternalFormat, ImageWidth, ImageHeight);
        }
        else
        {
            glTexStorage2D(GL_TEXTURE_2D, NumberOfMipmaps, GLInternalFormat, ImageWidth, ImageHeight);
        }

        // -----------------------------------------------------------------------------
        // Is data available, then upload it to graphic card
        // -----------------------------------------------------------------------------
        if (pTextureData != 0)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ImageWidth, ImageHeight, GLFormat, GLType, pTextureData);

            if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_NumberOfMipMapsFromSource && NumberOfMipmaps > 1)
            {
                for (unsigned int IndexOfMipMap = 1; IndexOfMipMap < NumberOfMipmaps; ++IndexOfMipMap)
                {
//                     ilBindImage(NativeImageName);
// 
//                     ilActiveMipmap(IndexOfMipMap);
// 
//                     ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
//                     ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);
// 
//                     if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
//                     {
//                         ilConvertImage(NativeILFormat, NativeILType);
//                     }
// 
//                     ImageWidth   = ilGetInteger(IL_IMAGE_WIDTH);
//                     ImageHeight  = ilGetInteger(IL_IMAGE_HEIGHT);
//                     pTextureData = ilGetData();
// 
//                     glTexSubImage2D(GL_TEXTURE_2D, IndexOfMipMap, 0, 0, ImageWidth, ImageHeight, GLFormat, GLType, pTextureData);
                }
            }
        }

        // -----------------------------------------------------------------------------
        // Create mipmaps depending on uploaded data
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // -----------------------------------------------------------------------------
        // Unbind
        // -----------------------------------------------------------------------------
        glBindTexture(GL_TEXTURE_2D, 0);

        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CTexture2DPtr Texture2DPtr = static_cast<CTexture2DPtr>(m_Textures2D.Allocate());
        
        assert(NumberOfBytes > 0);
        
        try
        {
            CInternTexture2D& rTexture = *static_cast<CInternTexture2D*>(Texture2DPtr.GetPtr());
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != 0) rTexture.m_FileName = _rDescriptor.m_pFileName;

            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Gfx::CTextureBase::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Gfx::CTextureBase::BPixels>(ImageHeight);
            rTexture.m_Hash              = 0;
            
            rTexture.m_Info.m_Access            = _rDescriptor.m_Access;
            rTexture.m_Info.m_Binding           = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension         = CTextureBase::Dim2D;
            rTexture.m_Info.m_Format            = _rDescriptor.m_Format;
            rTexture.m_Info.m_IsCubeTexture     = false;
            rTexture.m_Info.m_IsDeletable       = _IsDeleteable;
            rTexture.m_Info.m_IsDummyTexture    = false;
            rTexture.m_Info.m_NumberOfTextures  = _rDescriptor.m_NumberOfTextures;
            rTexture.m_Info.m_NumberOfMipLevels = NumberOfMipmaps;
            rTexture.m_Info.m_CurrentMipLevel   = 0;
            rTexture.m_Info.m_Semantic          = _rDescriptor.m_Semantic;
            rTexture.m_Info.m_Usage             = _rDescriptor.m_Usage;
            
            rTexture.m_NativeTexture        = NativeTextureHandle;
            rTexture.m_NativeUsage          = GLUsage;
            rTexture.m_NativeInternalFormat = GLInternalFormat;
            rTexture.m_NativeDimension      = GL_TEXTURE_2D;
            
            // -----------------------------------------------------------------------------
            // Check the behavior.
            // -----------------------------------------------------------------------------
            if (_Behavior == SDataBehavior::Copy || _Behavior == SDataBehavior::CopyAndDelete)
            {
                pBytes = Base::CMemory::Allocate(NumberOfBytes);
                
                if (pTextureData)
                {
                    pBytes = Base::CMemory::Copy(NumberOfBytes, pTextureData);
                }
                else if (_rDescriptor.m_pPixels)
                {
                    pBytes = Base::CMemory::Copy(NumberOfBytes, _rDescriptor.m_pPixels);
                }
            }
            
            switch (_Behavior)
            {
                case SDataBehavior::LeftAlone:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                    rTexture.m_pPixels             = nullptr;
                }
                break;
                    
                case SDataBehavior::DeleteAfterUpload:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                    rTexture.m_pPixels             = nullptr;
                    
                    Base::CMemory::Free(_rDescriptor.m_pPixels);
                }
                break;
                    
                case SDataBehavior::TakeOwnerShip:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = _rDescriptor.m_pPixels;
                }
                break;
                    
                case SDataBehavior::Copy:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = pBytes;
                }
                break;
                    
                case SDataBehavior::CopyAndDelete:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = pBytes;
                    
                    Base::CMemory::Free(_rDescriptor.m_pPixels);
                }
                break;
                    
                default:
                    BASE_CONSOLE_STREAMWARNING("Undefined texture data behavior while creating an texture.");
                    break;
            }

            // -----------------------------------------------------------------------------
            // Delete image on DevIL because it isn't needed anymore
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr)
            {
//                 ilDeleteImage(NativeImageName);
// 
//                 ilBindImage(0);
            }
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }
        
        return Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture3DPtr CGfxTextureManager::InternCreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        void*        pBytes;
        void*        pTextureData;
        unsigned int ImageWidth;
        unsigned int ImageHeight;
        unsigned int ImageDepth;
        unsigned int NumberOfPixel;
        unsigned int NumberOfBytes;
        unsigned int NumberOfMipmaps;
        int          GLInternalFormat;
        int          GLFormat;
        int          GLUsage;
        int          GLType;
        GLuint       NativeTextureHandle;
//         ILenum       NativeILFormat;
//         ILenum       NativeILType;

        pBytes       = nullptr;
        pTextureData = nullptr;

        // -----------------------------------------------------------------------------
        // Setup variables
        // -----------------------------------------------------------------------------
        ImageWidth       = _rDescriptor.m_NumberOfPixelsU;
        ImageHeight      = _rDescriptor.m_NumberOfPixelsV;
        ImageDepth       = _rDescriptor.m_NumberOfPixelsW;
        pBytes           = nullptr;
        pTextureData     = _rDescriptor.m_pPixels;
        NumberOfPixel    = ImageWidth * ImageHeight;
        NumberOfMipmaps  = _rDescriptor.m_NumberOfMipMaps;
        NumberOfBytes    = ConvertGLFormatToBytesPerPixel(_rDescriptor.m_Format) * NumberOfPixel;
        GLInternalFormat = ConvertGLInternalImageFormat(_rDescriptor.m_Format);
        GLFormat         = ConvertGLImageFormat(_rDescriptor.m_Format);
        GLType           = ConvertGLImageType(_rDescriptor.m_Format);
        GLUsage          = ConvertGLImageUsage(_rDescriptor.m_Usage);

//         NativeILFormat  = ConvertILImageFormat(_rDescriptor.m_Format);
//         NativeILType    = ConvertILImageType(_rDescriptor.m_Format);

        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            NumberOfMipmaps = static_cast<int>(Base::Log2(static_cast<float>(Base::Max(ImageWidth, ImageHeight)))) + 1;
        }
        
        // -----------------------------------------------------------------------------
        // Generate OpenGL texture or render buffer
        // -----------------------------------------------------------------------------
        glGenTextures(1, &NativeTextureHandle);

        glBindTexture(GL_TEXTURE_3D, NativeTextureHandle);

        // -----------------------------------------------------------------------------
        // Label texture if file name exists
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr)
        {
            glObjectLabel(GL_TEXTURE, NativeTextureHandle, -1, _rDescriptor.m_pFileName);
        }

        // -----------------------------------------------------------------------------
        // Binding
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_Binding & Gfx::CTextureBase::DepthStencilTarget)
        {
            glTexStorage3D(GL_TEXTURE_3D, NumberOfMipmaps, GL_DEPTH_COMPONENT32F, ImageWidth, ImageHeight, ImageDepth);
        }
        else if (_rDescriptor.m_Binding & Gfx::CTextureBase::RenderTarget)
        {
            glTexStorage3D(GL_TEXTURE_3D, NumberOfMipmaps, GLInternalFormat, ImageWidth, ImageHeight, ImageDepth);
        }
        else
        {
            glTexStorage3D(GL_TEXTURE_3D, NumberOfMipmaps, GLInternalFormat, ImageWidth, ImageHeight, ImageDepth);
        }

        // -----------------------------------------------------------------------------
        // Is data available, then upload it to graphic card
        // -----------------------------------------------------------------------------
        if (pTextureData != 0)
        {
            glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, ImageWidth, ImageHeight, ImageDepth, GLFormat, GLType, pTextureData);
        }

        // -----------------------------------------------------------------------------
        // Create mip maps depending on uploaded data
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            glGenerateMipmap(GL_TEXTURE_3D);
        }

        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CTexture3DPtr Texture3DPtr = static_cast<CTexture3DPtr>(m_Textures3D.Allocate());

        assert(NumberOfBytes > 0);

        try
        {
            CInternTexture3D& rTexture = *static_cast<CInternTexture3D*>(Texture3DPtr.GetPtr());
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != 0) rTexture.m_FileName = _rDescriptor.m_pFileName;

            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Gfx::CTextureBase::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Gfx::CTextureBase::BPixels>(ImageHeight);
            rTexture.m_NumberOfPixels[2] = static_cast<Gfx::CTextureBase::BPixels>(ImageDepth);
            rTexture.m_Hash              = 0;
            
            rTexture.m_Info.m_Access            = _rDescriptor.m_Access;
            rTexture.m_Info.m_Binding           = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension         = CTextureBase::Dim3D;
            rTexture.m_Info.m_Format            = _rDescriptor.m_Format;
            rTexture.m_Info.m_IsCubeTexture     = false;
            rTexture.m_Info.m_IsDeletable       = _IsDeleteable;
            rTexture.m_Info.m_IsDummyTexture    = false;
            rTexture.m_Info.m_NumberOfTextures  = _rDescriptor.m_NumberOfTextures;
            rTexture.m_Info.m_NumberOfMipLevels = NumberOfMipmaps;
            rTexture.m_Info.m_CurrentMipLevel   = 0;
            rTexture.m_Info.m_Semantic          = _rDescriptor.m_Semantic;
            rTexture.m_Info.m_Usage             = _rDescriptor.m_Usage;
            
            rTexture.m_NativeTexture        = NativeTextureHandle;
            rTexture.m_NativeUsage          = GLUsage;
            rTexture.m_NativeInternalFormat = GLInternalFormat;
            rTexture.m_NativeDimension      = GL_TEXTURE_3D;
            
            // -----------------------------------------------------------------------------
            // Check the behavior.
            // -----------------------------------------------------------------------------
            if (_Behavior == SDataBehavior::Copy || _Behavior == SDataBehavior::CopyAndDelete)
            {
                pBytes = Base::CMemory::Allocate(NumberOfBytes);
                
                if (pTextureData)
                {
                    pBytes = Base::CMemory::Copy(NumberOfBytes, pTextureData);
                }
                else if (_rDescriptor.m_pPixels)
                {
                    pBytes = Base::CMemory::Copy(NumberOfBytes, _rDescriptor.m_pPixels);
                }
            }
            
            switch (_Behavior)
            {
                case SDataBehavior::LeftAlone:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                    rTexture.m_pPixels             = nullptr;
                }
                break;
                    
                case SDataBehavior::DeleteAfterUpload:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                    rTexture.m_pPixels             = nullptr;
                    
                    Base::CMemory::Free(_rDescriptor.m_pPixels);
                }
                break;
                    
                case SDataBehavior::TakeOwnerShip:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = _rDescriptor.m_pPixels;
                }
                break;
                    
                case SDataBehavior::Copy:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = pBytes;
                }
                break;
                    
                case SDataBehavior::CopyAndDelete:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = pBytes;
                    
                    Base::CMemory::Free(_rDescriptor.m_pPixels);
                }
                break;
                    
                default:
                    BASE_CONSOLE_STREAMWARNING("Undefined texture data behavior while creating an texture.");
                    break;
            }
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }
        
        return Texture3DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGfxTextureManager::InternCreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        bool         ImageIsLoaded;
        void*        pBytes;
        void*        pTextureData;
        unsigned int ImageWidth;
        unsigned int ImageHeight;
        unsigned int NumberOfPixel;
        unsigned int NumberOfBytes;
        unsigned int NumberOfMipmaps;
        int          GLInternalFormat;
        int          GLFormat;
        int          GLUsage;
        int          GLType;
        GLuint       NativeTextureHandle;
//         ILuint       NumberOfFaces;
//         ILuint       NativeImageName;
//         ILenum       NativeILFormat;
//         ILenum       NativeILType;
//         ILinfo       NativeILImageInfo;

        assert(_rDescriptor.m_NumberOfTextures == 6 && _rDescriptor.m_NumberOfPixelsW == 1 && _rDescriptor.m_pPixels == 0);

        ImageIsLoaded = false;
//         NumberOfFaces = 1;
        pBytes        = nullptr;
        pTextureData  = nullptr;
        
        // -----------------------------------------------------------------------------
        // Setup variables
        // -----------------------------------------------------------------------------
        ImageWidth       = _rDescriptor.m_NumberOfPixelsU;
        ImageHeight      = _rDescriptor.m_NumberOfPixelsV;
        pBytes           = nullptr;
        pTextureData     = _rDescriptor.m_pPixels;
        NumberOfPixel    = ImageWidth * ImageHeight;
        NumberOfMipmaps  = _rDescriptor.m_NumberOfMipMaps;
        NumberOfBytes    = ConvertGLFormatToBytesPerPixel(_rDescriptor.m_Format) * NumberOfPixel;
        GLInternalFormat = ConvertGLInternalImageFormat(_rDescriptor.m_Format);
        GLFormat         = ConvertGLImageFormat(_rDescriptor.m_Format);
        GLType           = ConvertGLImageType(_rDescriptor.m_Format);
        GLUsage          = ConvertGLImageUsage(_rDescriptor.m_Usage);

//         NativeILFormat  = ConvertILImageFormat(_rDescriptor.m_Format);
//         NativeILType    = ConvertILImageType(_rDescriptor.m_Format);
// 
//         NativeImageName = 0;

        // -----------------------------------------------------------------------------
        // Load texture from file if one is set in descriptor
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr)
        {
            // -----------------------------------------------------------------------------
            // Create and bind texture on DevIL
            // -----------------------------------------------------------------------------
//             NativeImageName = ilGenImage();
// 
//             ilBindImage(NativeImageName);
// 
//             // -----------------------------------------------------------------------------
//             // Load texture from file (either in assets or data)
//             // -----------------------------------------------------------------------------
//             std::string PathToTexture;
// 
//             PathToTexture = App::Application::GetAssetPath() + g_PathToAssets + _rDescriptor.m_pFileName;
// 
// #ifdef __ANDROID__
//             const char* pPathToTexture = 0;
// 
//             pPathToTexture = PathToTexture.c_str();
// #else
//             const wchar_t* pPathToTexture = 0;
// 
//             pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
// #endif
//             
//             ImageIsLoaded = ilLoadImage(pPathToTexture) == IL_TRUE;
// 
//             if (!ImageIsLoaded)
//             {
//                 PathToTexture = App::Application::GetAssetPath() + g_PathToDataTextures + _rDescriptor.m_pFileName;
// 
// #ifdef __ANDROID__
//                 pPathToTexture = PathToTexture.c_str();
// #else
//                 pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
// #endif
// 
//                 ImageIsLoaded = ilLoadImage(pPathToTexture) == IL_TRUE;
//             }
// 
//             if (ImageIsLoaded)
//             {
//                 NumberOfFaces = ilGetInteger(IL_NUM_FACES);
//             }
//             
//             if (ImageIsLoaded && NumberOfFaces == 5)
//             {
//                 ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
//                 ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);
// 
//                 if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
//                 {
//                     ilConvertImage(NativeILFormat, NativeILType);
//                 }
//                 
//                 pTextureData = ilGetData();
//                 
//                 ImageWidth    = ilGetInteger(IL_IMAGE_WIDTH);
//                 ImageHeight   = ilGetInteger(IL_IMAGE_HEIGHT);
//                 NumberOfBytes = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
//                 
//                 assert(ImageWidth    > 0);
//                 assert(ImageHeight   > 0);
//                 assert(NumberOfBytes > 0);
//             }
//             else
//             {
//                 BASE_CONSOLE_STREAMERROR("Failed loading cubemap '" << PathToTexture.c_str() << "' from file.");
// 
//                 ilDeleteImage(NativeImageName);
// 
//                 ilBindImage(0);
// 
//                 return nullptr;
//             }
        }

        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            NumberOfMipmaps = static_cast<int>(Base::Log2(static_cast<float>(Base::Max(ImageWidth, ImageHeight)))) + 1;
        }
        
        // -----------------------------------------------------------------------------
        // Generate OpenGL texture or render buffer
        // -----------------------------------------------------------------------------
        glGenTextures(1, &NativeTextureHandle);

        glBindTexture(GL_TEXTURE_CUBE_MAP, NativeTextureHandle);

        // -----------------------------------------------------------------------------
        // Label texture if file name exists
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr)
        {
            glObjectLabel(GL_TEXTURE, NativeTextureHandle, -1, _rDescriptor.m_pFileName);
        }

        // -----------------------------------------------------------------------------
        // Binding
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_Binding & Gfx::CTextureBase::DepthStencilTarget)
        {
            glTexStorage2D(GL_TEXTURE_CUBE_MAP, NumberOfMipmaps, GL_DEPTH_COMPONENT32F, ImageWidth, ImageHeight);
        }
        else if (_rDescriptor.m_Binding & Gfx::CTextureBase::RenderTarget)
        {
            glTexStorage2D(GL_TEXTURE_CUBE_MAP, NumberOfMipmaps, GLInternalFormat, ImageWidth, ImageHeight);
        }
        else
        {
            glTexStorage2D(GL_TEXTURE_CUBE_MAP, NumberOfMipmaps, GLInternalFormat, ImageWidth, ImageHeight);
        }

        // -----------------------------------------------------------------------------
        // Is data available, then upload it to graphic card
        // -----------------------------------------------------------------------------
        if (ImageIsLoaded)
        {
//             for (int IndexOfFace = 0; IndexOfFace <= NumberOfFaces; ++IndexOfFace)
            {
//                 ilBindImage(NativeImageName);
// 
//                 ilActiveFace(IndexOfFace);
// 
//                 ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
//                 ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);
// 
//                 if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
//                 {
//                     ilConvertImage(NativeILFormat, NativeILType);
//                 }
// 
//                 iluGetImageInfo(&NativeILImageInfo);
// 
//                 if (NativeILImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
//                 {
//                     iluFlipImage();
//                 }
// 
//                 pTextureData = ilGetData();
// 
//                 glTexSubImage3D(GL_TEXTURE_CUBE_MAP, 0, 0, 0, IndexOfFace, ImageWidth, ImageHeight, 1, GLFormat, GLType, pTextureData);
            }
        }

        // -----------------------------------------------------------------------------
        // Create mip maps depending on uploaded data
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }

        // -----------------------------------------------------------------------------
        // Unbind
        // -----------------------------------------------------------------------------
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CTexture2DPtr Texture2DPtr = static_cast<CTexture2DPtr>(m_Textures2D.Allocate());

        assert(NumberOfBytes > 0);

        try
        {
            CInternTexture2D& rTexture = *static_cast<CInternTexture2D*>(Texture2DPtr.GetPtr());
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != 0) rTexture.m_FileName = _rDescriptor.m_pFileName;

            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Gfx::CTextureBase::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Gfx::CTextureBase::BPixels>(ImageHeight);
            rTexture.m_Hash              = 0;
            
            rTexture.m_Info.m_Access            = _rDescriptor.m_Access;
            rTexture.m_Info.m_Binding           = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension         = CTextureBase::Dim2D;
            rTexture.m_Info.m_Format            = _rDescriptor.m_Format;
            rTexture.m_Info.m_IsCubeTexture     = true;
            rTexture.m_Info.m_IsDeletable       = _IsDeleteable;
            rTexture.m_Info.m_IsDummyTexture    = false;
            rTexture.m_Info.m_NumberOfTextures  = _rDescriptor.m_NumberOfTextures;
            rTexture.m_Info.m_NumberOfMipLevels = NumberOfMipmaps;
            rTexture.m_Info.m_CurrentMipLevel   = 0;
            rTexture.m_Info.m_Semantic          = _rDescriptor.m_Semantic;
            rTexture.m_Info.m_Usage             = _rDescriptor.m_Usage;
            
            rTexture.m_NativeTexture        = NativeTextureHandle;
            rTexture.m_NativeUsage          = GLUsage;
            rTexture.m_NativeInternalFormat = GLInternalFormat;
            rTexture.m_NativeDimension      = GL_TEXTURE_2D;
            
            // -----------------------------------------------------------------------------
            // Check the behavior.
            // -----------------------------------------------------------------------------
            if (_Behavior == SDataBehavior::Copy || _Behavior == SDataBehavior::CopyAndDelete)
            {
                pBytes = Base::CMemory::Allocate(NumberOfBytes);
                
                if (pTextureData)
                {
                    pBytes = Base::CMemory::Copy(NumberOfBytes, pTextureData);
                }
                else if (_rDescriptor.m_pPixels)
                {
                    pBytes = Base::CMemory::Copy(NumberOfBytes, _rDescriptor.m_pPixels);
                }
            }
            
            switch (_Behavior)
            {
                case SDataBehavior::LeftAlone:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                    rTexture.m_pPixels             = nullptr;
                }
                break;
                    
                case SDataBehavior::DeleteAfterUpload:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                    rTexture.m_pPixels             = nullptr;
                    
                    Base::CMemory::Free(_rDescriptor.m_pPixels);
                }
                break;
                    
                case SDataBehavior::TakeOwnerShip:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = _rDescriptor.m_pPixels;
                }
                break;
                    
                case SDataBehavior::Copy:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = pBytes;
                }
                break;
                    
                case SDataBehavior::CopyAndDelete:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = pBytes;
                    
                    Base::CMemory::Free(_rDescriptor.m_pPixels);
                }
                break;
                    
                default:
                    BASE_CONSOLE_STREAMWARNING("Undefined texture data behavior while creating an texture.");
                    break;
            }

            // -----------------------------------------------------------------------------
            // Delete image on DevIL because it isn't needed anymore
            // -----------------------------------------------------------------------------
            if (ImageIsLoaded)
            {
//                 ilDeleteImage(NativeImageName);
// 
//                 ilBindImage(0);
            }
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }
        
        return Texture2DPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxTextureManager::ConvertGLFormatToBytesPerPixel(Gfx::CTextureBase::EFormat _Format) const
    {
        static int s_NativeFormat[] =
        {
            1,
            2,
            3,
            4,
            1,
            2,
            3,
            4,
            1,
            2,
            3,
            4,
            1,
            2,
            3,
            4,
            1,
            2,
            3,
            4,
            1,
            2,
            3,
            4,
            
            2,
            4,
            6,
            8,
            2,
            4,
            6,
            8,
            2,
            4,
            6,
            8,
            2,
            4,
            6,
            8,
            2,
            4,
            6,
            8,
            2,
            4,
            6,
            8,
            2,
            4,
            6,
            8,
            
            4,
            8,
            12,
            16,
            4,
            8,
            12,
            16,
            4,
            8,
            12,
            16,
            
            1,
            2,
            2,
            4,
        };
        
        return s_NativeFormat[_Format];
    }

    // -----------------------------------------------------------------------------

    int CGfxTextureManager::ConvertGLImageUsage(Gfx::CTextureBase::EUsage _Usage) const
    {
        static int s_NativeUsage[] =
        {
            GL_READ_WRITE,
            GL_READ_ONLY,
            GL_READ_ONLY,
            GL_READ_ONLY,
        };

        return s_NativeUsage[_Usage];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxTextureManager::ConvertGLInternalImageFormat(Gfx::CTextureBase::EFormat _Format) const
    {
        static int s_NativeFormat[] =
        {
            GL_R8,
            GL_RG8,
            GL_RGB8,
            GL_RGBA8,
            GL_R8,
            GL_RG8,
            GL_RGB8,
            GL_RGBA8,
            GL_R8,
            GL_RG8,
            GL_RGB8,
            GL_RGBA8,
            GL_R8,
            GL_RG8,
            GL_RGB8,
            GL_RGBA8,
            GL_R8I,
            GL_RG8I,
            GL_RGB8I,
            GL_RGBA8I,
            GL_R8UI,
            GL_RG8UI,
            GL_RGB8UI,
            GL_RGBA8UI,
            
            GL_NONE, //GL_R16,
            GL_NONE, //GL_RG16,
            GL_NONE, //GL_RGB16,
            GL_NONE, //GL_RGBA16,
            GL_NONE, //GL_R16,
            GL_NONE, //GL_RG16,
            GL_NONE, //GL_RGB16,
            GL_NONE, //GL_RGBA16,
            GL_NONE, //GL_R16,
            GL_NONE, //GL_RG16,
            GL_NONE, //GL_RGB16,
            GL_NONE, //GL_RGBA16,
            GL_NONE, //GL_R16,
            GL_NONE, //GL_RG16,
            GL_NONE, //GL_RGB16,
            GL_NONE, //GL_RGBA16,
            GL_R16I,
            GL_RG16I,
            GL_RGB16I,
            GL_RGBA16I,
            GL_R16UI,
            GL_RG16UI,
            GL_RGB16UI,
            GL_RGBA16UI,
            GL_R16F,
            GL_RG16F,
            GL_RGB16F,
            GL_RGBA16F,
            
            GL_R32I,
            GL_RG32I,
            GL_RGB32I,
            GL_RGBA32I,
            GL_R32UI,
            GL_RG32UI,
            GL_RGB32UI,
            GL_RGBA32UI,
            GL_R32F,
            GL_RG32F,
            GL_RGB32F,
            GL_RGBA32F,
            
            GL_NONE, //GL_R3_G3_B2,
            GL_RGBA4,
            GL_RGB5_A1,
            GL_RGB10_A2,
        };
        
        return s_NativeFormat[_Format];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxTextureManager::ConvertGLImageFormat(Gfx::CTextureBase::EFormat _Format) const
    {
        static int s_NativeFormat[] =
        {
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            
            GL_RGB,
            GL_RGBA,
            GL_RGBA,
            GL_RGBA,
        };
        
        return s_NativeFormat[_Format];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxTextureManager::ConvertGLImageType(Gfx::CTextureBase::EFormat _Format) const
    {
        static int s_NativeType[] =
        {
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_FLOAT,
            GL_FLOAT,
            GL_FLOAT,
            GL_FLOAT,
            
            GL_INT,
            GL_INT,
            GL_INT,
            GL_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_UNSIGNED_INT,
            GL_FLOAT,
            GL_FLOAT,
            GL_FLOAT,
            GL_FLOAT,
            
            GL_NONE, //GL_UNSIGNED_BYTE_3_3_2,
            GL_UNSIGNED_SHORT_4_4_4_4,
            GL_UNSIGNED_SHORT_5_5_5_1,
            GL_NONE, //GL_UNSIGNED_INT_10_10_10_2,
        };
        
        return s_NativeType[_Format];
    }

    // -----------------------------------------------------------------------------

//     ILenum CGfxTextureManager::ConvertILImageFormat(Gfx::CTextureBase::EFormat _Format) const
//     {
//         static ILenum s_NativeFormat[] =
//         {
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
// 
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
// 
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
//             IL_LUMINANCE,
//             0,
//             IL_RGB,
//             IL_RGBA,
// 
//             IL_RGB,
//             IL_RGBA,
//             IL_RGBA,
//             IL_RGBA,
//         };
// 
//         return s_NativeFormat[_Format];
//     }
// 
//     // -----------------------------------------------------------------------------
// 
//     ILenum CGfxTextureManager::ConvertILImageType(Gfx::CTextureBase::EFormat _Format) const
//     {
//         static int s_NativeType[] =
//         {
//             IL_BYTE,
//             IL_BYTE,
//             IL_BYTE,
//             IL_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_SHORT,
//             IL_SHORT,
//             IL_SHORT,
//             IL_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_INT,
//             IL_INT,
//             IL_INT,
//             IL_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
// 
//             IL_BYTE,
//             IL_BYTE,
//             IL_BYTE,
//             IL_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_UNSIGNED_BYTE,
//             IL_SHORT,
//             IL_SHORT,
//             IL_SHORT,
//             IL_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_UNSIGNED_SHORT,
//             IL_INT,
//             IL_INT,
//             IL_INT,
//             IL_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_FLOAT,
//             IL_FLOAT,
//             IL_FLOAT,
//             IL_FLOAT,
// 
//             IL_INT,
//             IL_INT,
//             IL_INT,
//             IL_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_UNSIGNED_INT,
//             IL_FLOAT,
//             IL_FLOAT,
//             IL_FLOAT,
//             IL_FLOAT,
// 
//             0,
//             0,
//             0,
//             0,
//         };
// 
//         return s_NativeType[_Format];
//     }

    // -----------------------------------------------------------------------------

    Gfx::CTextureBase::EDimension CGfxTextureManager::ConvertDataDimension(Dt::CTextureBase::EDimension _Dimension)
    {
        static Gfx::CTextureBase::EDimension s_Types[] =
        {
            Gfx::CTextureBase::Dim1D,
            Gfx::CTextureBase::Dim2D,
            Gfx::CTextureBase::Dim3D,
        };

        return s_Types[_Dimension];
    }

    // -----------------------------------------------------------------------------

    Gfx::CTextureBase::EFormat CGfxTextureManager::ConvertDataFormat(Dt::CTextureBase::EFormat _Format)
    {
        static Gfx::CTextureBase::EFormat s_Types[] =
        {
            Gfx::CTextureBase::R8_BYTE,
            Gfx::CTextureBase::R8G8_BYTE,
            Gfx::CTextureBase::R8G8B8_BYTE,
            Gfx::CTextureBase::R8G8B8A8_BYTE,
            Gfx::CTextureBase::R8_UBYTE,
            Gfx::CTextureBase::R8G8_UBYTE,
            Gfx::CTextureBase::R8G8B8_UBYTE,
            Gfx::CTextureBase::R8G8B8A8_UBYTE,
            Gfx::CTextureBase::R8_SHORT,
            Gfx::CTextureBase::R8G8_SHORT,
            Gfx::CTextureBase::R8G8B8_SHORT,
            Gfx::CTextureBase::R8G8B8A8_SHORT,
            Gfx::CTextureBase::R8_USHORT,
            Gfx::CTextureBase::R8G8_USHORT,
            Gfx::CTextureBase::R8G8B8_USHORT,
            Gfx::CTextureBase::R8G8B8A8_USHORT,
            Gfx::CTextureBase::R8_INT,
            Gfx::CTextureBase::R8G8_INT,
            Gfx::CTextureBase::R8G8B8_INT,
            Gfx::CTextureBase::R8G8B8A8_INT,
            Gfx::CTextureBase::R8_UINT,
            Gfx::CTextureBase::R8G8_UINT,
            Gfx::CTextureBase::R8G8B8_UINT,
            Gfx::CTextureBase::R8G8B8A8_UINT,

            Gfx::CTextureBase::R16_BYTE,
            Gfx::CTextureBase::R16G16_BYTE,
            Gfx::CTextureBase::R16G16B16_BYTE,
            Gfx::CTextureBase::R16G16B16A16_BYTE,
            Gfx::CTextureBase::R16_UBYTE,
            Gfx::CTextureBase::R16G16_UBYTE,
            Gfx::CTextureBase::R16G16B16_UBYTE,
            Gfx::CTextureBase::R16G16B16A16_UBYTE,
            Gfx::CTextureBase::R16_SHORT,
            Gfx::CTextureBase::R16G16_SHORT,
            Gfx::CTextureBase::R16G16B16_SHORT,
            Gfx::CTextureBase::R16G16B16A16_SHORT,
            Gfx::CTextureBase::R16_USHORT,
            Gfx::CTextureBase::R16G16_USHORT,
            Gfx::CTextureBase::R16G16B16_USHORT,
            Gfx::CTextureBase::R16G16B16A16_USHORT,
            Gfx::CTextureBase::R16_INT,
            Gfx::CTextureBase::R16G16_INT,
            Gfx::CTextureBase::R16G16B16_INT,
            Gfx::CTextureBase::R16G16B16A16_INT,
            Gfx::CTextureBase::R16_UINT,
            Gfx::CTextureBase::R16G16_UINT,
            Gfx::CTextureBase::R16G16B16_UINT,
            Gfx::CTextureBase::R16G16B16A16_UINT,
            Gfx::CTextureBase::R16_FLOAT,
            Gfx::CTextureBase::R16G16_FLOAT,
            Gfx::CTextureBase::R16G16B16_FLOAT,
            Gfx::CTextureBase::R16G16B16A16_FLOAT,

            Gfx::CTextureBase::R32_INT,
            Gfx::CTextureBase::R32G32_INT,
            Gfx::CTextureBase::R32G32B32_INT,
            Gfx::CTextureBase::R32G32B32A32_INT,
            Gfx::CTextureBase::R32_UINT,
            Gfx::CTextureBase::R32G32_UINT,
            Gfx::CTextureBase::R32G32B32_UINT,
            Gfx::CTextureBase::R32G32B32A32_UINT,
            Gfx::CTextureBase::R32_FLOAT,
            Gfx::CTextureBase::R32G32_FLOAT,
            Gfx::CTextureBase::R32G32B32_FLOAT,
            Gfx::CTextureBase::R32G32B32A32_FLOAT,

            Gfx::CTextureBase::R3G3B2_UBYTE,
            Gfx::CTextureBase::R4G4B4A4_USHORT,
            Gfx::CTextureBase::R5G5G5A1_USHORT,
            Gfx::CTextureBase::R10G10B10A2_UINT,
        };

        return s_Types[_Format];
    }

    // -----------------------------------------------------------------------------

    Gfx::CTextureBase::ESemantic CGfxTextureManager::ConvertDataSemantic(Dt::CTextureBase::ESemantic _Semantic)
    {
        static Gfx::CTextureBase::ESemantic s_Types[] =
        {
            Gfx::CTextureBase::Diffuse,
            Gfx::CTextureBase::Normal,
            Gfx::CTextureBase::Height,
            Gfx::CTextureBase::Diffuse,
        };

        return s_Types[_Semantic];
    }

    // -----------------------------------------------------------------------------

    unsigned int CGfxTextureManager::ConvertDataBinding(unsigned int _Binding)
    {
        unsigned int Binding = 0;

        if ((_Binding & Dt::CTextureBase::ShaderResource) != 0)
        {
            Binding |= Gfx::CTextureBase::ShaderResource;
        }
        
        if ((_Binding & Dt::CTextureBase::RenderTarget) != 0)
        {
            Binding |= Gfx::CTextureBase::RenderTarget;
        }

        if ((_Binding & Dt::CTextureBase::DepthStencilTarget) != 0)
        {
            Binding |= Gfx::CTextureBase::DepthStencilTarget;
        }

        return Binding;
    }
} // namespace

namespace
{
    CGfxTextureManager::CInternTexture1D::CInternTexture1D()
        : CTexture1D()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxTextureManager::CInternTexture1D::~CInternTexture1D()
    {
    }
} // namespace

namespace
{
    CGfxTextureManager::CInternTexture2D::CInternTexture2D()
        : CNativeTexture2D()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxTextureManager::CInternTexture2D::~CInternTexture2D()
    {
        if (m_Info.m_IsDeletable)
        {
            glDeleteTextures(1, &m_NativeTexture);
        }
    }
} // namespace

namespace
{
    CGfxTextureManager::CInternTexture3D::CInternTexture3D()
        : CNativeTexture3D()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxTextureManager::CInternTexture3D::~CInternTexture3D()
    {
        if (m_Info.m_IsDeletable)
        {
            glDeleteTextures(1, &m_NativeTexture);
        }
    }
} // namespace

namespace
{
    CGfxTextureManager::CInternTextureSet::CInternTextureSet()
        : CTextureSet()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxTextureManager::CInternTextureSet::~CInternTextureSet()
    {
        unsigned int IndexOfTexture;

        for (IndexOfTexture = 0; IndexOfTexture < m_NumberOfTextures; ++IndexOfTexture)
        {
            m_TexturePtrs[IndexOfTexture] = 0;
        }
    }

    // -----------------------------------------------------------------------------

    bool CGfxTextureManager::CInternTextureSet::operator == (const CInternTextureSet& _rTextureSet) const
    {
        unsigned int IndexOfTexture;

        if (m_NumberOfTextures != _rTextureSet.m_NumberOfTextures)
        {
            return false;
        }

        // -----------------------------------------------------------------------------
        // Important not to use the native interfaces here, because maybe we want to
        // exchange those without the need to rehash the set.
        // -----------------------------------------------------------------------------
        for (IndexOfTexture = 0; IndexOfTexture < m_NumberOfTextures; ++ IndexOfTexture)
        {
            if (m_TexturePtrs[IndexOfTexture] != _rTextureSet.m_TexturePtrs[IndexOfTexture])
            {
                return false;
            }
        }

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CGfxTextureManager::CInternTextureSet::operator != (const CInternTextureSet& _rTextureSet) const
    {
        Base::Size IndexOfTextures;

        if (m_NumberOfTextures != _rTextureSet.m_NumberOfTextures)
        {
            return true;
        }

        // -----------------------------------------------------------------------------
        // Important not to use the native interfaces here, because maybe we want to
        // exchange those without the need to rehash the set.
        // -----------------------------------------------------------------------------
        for (IndexOfTextures = 0; IndexOfTextures < m_NumberOfTextures; ++ IndexOfTextures)
        {
            if (m_TexturePtrs[IndexOfTextures] != _rTextureSet.m_TexturePtrs[IndexOfTextures])
            {
                return true;
            }
        }

        return false;
    }
} // namespace

namespace Gfx
{
namespace TextureManager
{
    void OnStart()
    {
        CGfxTextureManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxTextureManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    CTexture1DPtr GetDummyTexture1D()
    {
        return CGfxTextureManager::GetInstance().GetDummyTexture1D();
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr GetDummyTexture2D()
    {
        return CGfxTextureManager::GetInstance().GetDummyTexture2D();
    }

    // -----------------------------------------------------------------------------

    CTexture3DPtr GetDummyTexture3D()
    {
        return CGfxTextureManager::GetInstance().GetDummyTexture3D();
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr GetDummyCubeTexture()
    {
        return CGfxTextureManager::GetInstance().GetDummyCubeTexture();
    }

    // -----------------------------------------------------------------------------

    CTexture1DPtr CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CGfxTextureManager::GetInstance().CreateTexture1D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CGfxTextureManager::GetInstance().CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexture3DPtr CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CGfxTextureManager::GetInstance().CreateTexture3D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CGfxTextureManager::GetInstance().CreateCubeTexture(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr)
    {
        CTextureBasePtr TexturePtrs[] = { _Texture1Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 1);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr, CTextureBasePtr _Texture2Ptr)
    {
        CTextureBasePtr TexturePtrs[] = { _Texture1Ptr, _Texture2Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 2);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr, CTextureBasePtr _Texture2Ptr, CTextureBasePtr _Texture3Ptr)
    {
        CTextureBasePtr TexturePtrs[] = { _Texture1Ptr, _Texture2Ptr, _Texture3Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 3);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr, CTextureBasePtr _Texture2Ptr, CTextureBasePtr _Texture3Ptr, CTextureBasePtr _Texture4Ptr)
    {
        CTextureBasePtr TexturePtrs[] = { _Texture1Ptr, _Texture2Ptr, _Texture3Ptr, _Texture4Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 4);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTextureBasePtr* _pTexturePtrs, unsigned int _NumberOfTextures)
    {
        return CGfxTextureManager::GetInstance().CreateTextureSet(_pTexturePtrs, _NumberOfTextures);
    }

    // -----------------------------------------------------------------------------

    CTexture1DPtr GetTexture1DByHash(unsigned int _Hash)
    {
        return CGfxTextureManager::GetInstance().GetTexture1DByHash(_Hash);
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr GetTexture2DByHash(unsigned int _Hash)
    {
        return CGfxTextureManager::GetInstance().GetTexture2DByHash(_Hash);
    }

    // -----------------------------------------------------------------------------

    CTexture3DPtr GetTexture3DByHash(unsigned int _Hash)
    {
        return CGfxTextureManager::GetInstance().GetTexture3DByHash(_Hash);
    }
    
    // -----------------------------------------------------------------------------

    void ClearTexture1D(CTexture1DPtr _TexturePtr, const Base::Float4& _rColor)
    {
        CGfxTextureManager::GetInstance().ClearTexture1D(_TexturePtr, _rColor);
    }

    // -----------------------------------------------------------------------------

    void ClearTexture2D(CTexture2DPtr _TexturePtr, const Base::Float4& _rColor)
    {
        CGfxTextureManager::GetInstance().ClearTexture2D(_TexturePtr, _rColor);
    }

    // -----------------------------------------------------------------------------

    void ClearTexture3D(CTexture3DPtr _TexturePtr, const Base::Float4& _rColor)
    {
        CGfxTextureManager::GetInstance().ClearTexture3D(_TexturePtr, _rColor);
    }

    // -----------------------------------------------------------------------------

    void CopyToTexture2D(CTexture2DPtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        CGfxTextureManager::GetInstance().CopyToTexture2D(_TexturePtr, _rTargetRect, _NumberOfBytesPerLine, _pBytes, _UpdateMipLevels);
    }

    // -----------------------------------------------------------------------------

    void CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        CGfxTextureManager::GetInstance().CopyToTextureArray2D(_TextureArrayPtr, _IndexOfSlice, _rTargetRect, _NumberOfBytesPerLine, _pBytes, _UpdateMipLevels);
    }

    // -----------------------------------------------------------------------------

    void CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexture2DPtr _TexturePtr, bool _UpdateMipLevels)
    {
        CGfxTextureManager::GetInstance().CopyToTextureArray2D(_TextureArrayPtr, _IndexOfSlice, _TexturePtr, _UpdateMipLevels);
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr GetMipmapFromTexture2D(CTexture2DPtr _TexturePtr, unsigned int _Mipmap)
    {
        return CGfxTextureManager::GetInstance().GetMipmapFromTexture2D(_TexturePtr, _Mipmap);
    }
    
    // -----------------------------------------------------------------------------
    
    void UpdateMipmap(CTexture2DPtr _TexturePtr)
    {
        CGfxTextureManager::GetInstance().UpdateMipmap(_TexturePtr);
    }

    // -----------------------------------------------------------------------------

    void SetTexture2DLabel(CTexture2DPtr _TexturePtr, const char* _pLabel)
    {
        CGfxTextureManager::GetInstance().SetTexture2DLabel(_TexturePtr, _pLabel);
    }

    // -----------------------------------------------------------------------------

    void SetTexture3DLabel(CTexture3DPtr _TexturePtr, const char* _pLabel)
    {
        CGfxTextureManager::GetInstance().SetTexture3DLabel(_TexturePtr, _pLabel);
    }
} // namespace TextureManager
} // namespace Gfx
