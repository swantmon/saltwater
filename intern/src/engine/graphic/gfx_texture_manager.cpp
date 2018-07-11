
#include "engine/engine_precompiled.h"

#include "app_droid/app_application.h"

#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_asset_manager.h"

#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_native_texture.h"
#include "engine/graphic/gfx_texture_manager.h"

#include "IL/il.h"
#include "IL/ilu.h"

#include <unordered_map>

using namespace Gfx;

namespace
{
    std::string g_PathToDataTextures = "/graphic/textures/";
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

        CTexturePtr GetDummyTexture2D();
        CTexturePtr GetDummyTexture3D();
        CTexturePtr GetDummyCubeTexture();

        CTexturePtr CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);
        CTexturePtr CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        CTexturePtr CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        CTexturePtr CreateExternalTexture();

        CTextureSetPtr CreateTextureSet(CTexturePtr* _pTexturePtrs, unsigned int _NumberOfTextures);

        CTexturePtr GetTextureByHash(unsigned int _Hash);

        void ClearTextureLayer(CTexturePtr _TexturePtr, const void* _pData, int _Layer);
        void ClearTexture(CTexturePtr _TexturePtr, const void* _pData);

        void CopyToTexture2D(CTexturePtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels);
        void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels);
        void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexturePtr _TexturePtr, bool _UpdateMipLevels);

        CTexturePtr GetMipmapFromTexture2D(CTexturePtr _TexturePtr, unsigned int _Mipmap);
        
        void UpdateMipmap(CTexturePtr _TexturePtr);

        void SaveTexture(CTexturePtr _TexturePtr, const std::string& _rPathToFile);

        void SetTextureLabel(CTexturePtr _TexturePtr, const char* _pLabel);

    private:

        // -----------------------------------------------------------------------------
        // Represents a 2D texture.
        // -----------------------------------------------------------------------------
        class CInternTexture : public CNativeTexture
        {
            public:

                CInternTexture();
               ~CInternTexture();

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
        // There are way more 2D textures than 3D ones, so use bigger pages here.
        // -----------------------------------------------------------------------------
        typedef Base::CManagedPool<CInternTexture, 256, 0> CTextures;
        
        typedef std::unordered_map<unsigned int, CTexturePtr> CTextureByHashs;

    private:

        CTextures       m_Textures;
        CTextureByHashs m_TexturesByHash;
        CTextureSets    m_TextureSets;
        CTexturePtr     m_Texture2DPtr;

    private:

        CTexturePtr InternCreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);
        CTexturePtr InternCreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        CTexturePtr InternCreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior);

        CTexturePtr InternCreateExternalTexture();

        int ConvertGLFormatToBytesPerPixel(Gfx::CTexture::EFormat _Format) const;
        int ConvertGLImageUsage(Gfx::CTexture::EUsage _Usage) const;
        int ConvertGLInternalImageFormat(Gfx::CTexture::EFormat _Format) const;
        int ConvertGLImageFormat(Gfx::CTexture::EFormat _Format) const;
        int ConvertGLImageType(Gfx::CTexture::EFormat _Format) const;

        ILenum ConvertILImageFormat(Gfx::CTexture::EFormat _Format) const;
        ILenum ConvertILImageType(Gfx::CTexture::EFormat _Format) const;

    private:

        friend class CInternTextureSet;
        friend class CInternTargetSet;
    };
} // namespace

namespace
{
    CGfxTextureManager::CGfxTextureManager()
        : m_Textures      ()
        , m_TexturesByHash()
        , m_TextureSets   ()
        , m_Texture2DPtr  ()
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
        ilInit();

        // -----------------------------------------------------------------------------
        // Create 2x2 dummy texture
        // -----------------------------------------------------------------------------
        glm::tvec4<char> Pixels[4];
        Pixels[0] = glm::tvec4<char>(1, 0, 0, 1);
        Pixels[1] = glm::tvec4<char>(0, 1, 0, 1);
        Pixels[2] = glm::tvec4<char>(0, 0, 1, 1);
        Pixels[3] = glm::tvec4<char>(1, 1, 0, 1);

        Gfx::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = 2;
        TextureDescriptor.m_NumberOfPixelsV  = 2;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
        TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
        TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
        TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
        TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = nullptr;
        TextureDescriptor.m_pPixels          = Pixels;
        TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;

        m_Texture2DPtr = CreateTexture2D(TextureDescriptor, true, SDataBehavior::LeftAlone);

        SetTextureLabel(m_Texture2DPtr, "Dummy Texture 2D");
        
        // -----------------------------------------------------------------------------
        // Setup default settings in OpenGL
        // -----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif // PLATFORM_WINDOWS
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::OnExit()
    {
        m_Texture2DPtr = 0;

        // -----------------------------------------------------------------------------
        // Clear all the pools with the textures.
        // -----------------------------------------------------------------------------
        m_TexturesByHash.clear();

        m_Textures.Clear();

        // -----------------------------------------------------------------------------
        // Clear all the sets.
        // -----------------------------------------------------------------------------
        m_TextureSets.Clear();
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::GetDummyTexture2D()
    {
        return m_Texture2DPtr;
    }
    
    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::GetDummyTexture3D()
    {
        assert(false); // TODO: create dummy texture

        return nullptr;
    }
    
    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::GetDummyCubeTexture()
    {
        assert(false); // TODO: create dummy texture

        return nullptr;
    }
    
    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
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
            
            if (m_TexturesByHash.find(Hash) != m_TexturesByHash.end())
            {
                return m_TexturesByHash.at(Hash);
            }
        }            

        // -----------------------------------------------------------------------------
        // Texture
        // -----------------------------------------------------------------------------
        CTexturePtr Texture2DPtr = InternCreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior);

        CInternTexture* pInternTexture2D = static_cast<CInternTexture*>(Texture2DPtr.GetPtr());

        if (pInternTexture2D == nullptr)
        {
            return GetDummyTexture2D();
        }

        if (Hash != 0)
        {
            pInternTexture2D->m_Hash = Hash;

            m_TexturesByHash[Hash] = Texture2DPtr;
        }

        return Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return InternCreateTexture3D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return InternCreateCubeTexture(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::CreateExternalTexture()
    {
        return InternCreateExternalTexture();
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGfxTextureManager::CreateTextureSet(CTexturePtr* _pTexturePtrs, unsigned int _NumberOfTextures)
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

    CTexturePtr CGfxTextureManager::GetTextureByHash(unsigned int _Hash)
    {
        if (m_TexturesByHash.find(_Hash) != m_TexturesByHash.end())
        {
            return m_TexturesByHash.at(_Hash);
        }

        return nullptr;
    }
    
    // -----------------------------------------------------------------------------

    void CGfxTextureManager::ClearTextureLayer(CTexturePtr _TexturePtr, const void* _pData, int _Layer)
    {
#ifdef PLATFORM_ANDROID
        ENGINE_CONSOLE_ERROR("Clearing textures is currently not supported on Android");
        assert(false); // TODO: implement
#else
        // TODO: Remove dummy
        // Renderdoc crashes when _pData is nullptr but OpenGL allows NULL
        // Therefore we create a dummy value until Renderdoc is fixed

        glm::ivec4 Dummy = glm::ivec4(0);

        if (_pData == nullptr)
        {
            _pData = &Dummy;
        }

        CInternTexture* pInternTexture = static_cast<CInternTexture*>(_TexturePtr.GetPtr());
        Gfx::CNativeTextureHandle TextureHandle = pInternTexture->m_NativeTexture;
        
        assert(_Layer <= static_cast<int>(pInternTexture->GetNumberOfTextures()));

        const int Format = ConvertGLImageFormat(_TexturePtr->GetFormat());
        const int Type = ConvertGLImageType(_TexturePtr->GetFormat());
        
        const int Width = pInternTexture->GetNumberOfPixelsU();
        const int Height = pInternTexture->GetNumberOfPixelsV();

        const int MipLevels = pInternTexture->GetNumberOfMipLevels();

        for (int MipIndex = 0; MipIndex < MipLevels; ++ MipIndex)
        {
            glClearTexSubImage(TextureHandle, MipIndex, 0, 0, _Layer, Width >> MipIndex, Height >> MipIndex, 1, Format, Type, _pData);
        }
#endif // PLATFORM_ANDROID
    }
    
    // -----------------------------------------------------------------------------

    void CGfxTextureManager::ClearTexture(CTexturePtr _TexturePtr, const void* _pData)
    {
#ifdef PLATFORM_ANDROID
        ENGINE_CONSOLE_ERROR("Clearing textures is currently not supported on Android");
        assert(false); // TODO: implement
#else
        // TODO: Remove dummy
        // Renderdoc crashes when _pData is nullptr but OpenGL allows NULL
        // Therefore we create a dummy value until Renderdoc is fixed

        glm::ivec4 Dummy = glm::ivec4(0);

        if (_pData == nullptr)
        {
            _pData = &Dummy;
        }

        CInternTexture* pInternTexture = static_cast<CInternTexture*>(_TexturePtr.GetPtr());
        Gfx::CNativeTextureHandle TextureHandle = pInternTexture->m_NativeTexture;

        const int Format = ConvertGLImageFormat(_TexturePtr->GetFormat());
        const int Type = ConvertGLImageType(_TexturePtr->GetFormat());

        const int Width = pInternTexture->GetNumberOfPixelsU();
        const int Height = pInternTexture->GetNumberOfPixelsV();
        const int LayerCount = pInternTexture->GetNumberOfPixelsW();

        const int MipLevels = pInternTexture->GetNumberOfMipLevels();

        for (int MipIndex = 0; MipIndex < MipLevels; ++MipIndex)
        {
            glClearTexSubImage(TextureHandle, MipIndex, 0, 0, 0, Width >> MipIndex, Height >> MipIndex, LayerCount, Format, Type, _pData);
        }
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------
    
    void CGfxTextureManager::CopyToTexture2D(CTexturePtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        BASE_UNUSED(_NumberOfBytesPerLine);

        // -----------------------------------------------------------------------------
        // Get informations
        // -----------------------------------------------------------------------------
        assert(_TexturePtr.IsValid());
        assert(_pBytes != 0);
        
        glm::uvec2 Offset     = _rTargetRect[0];
        glm::uvec2 UpdateSize = _rTargetRect[1] - _rTargetRect[0];
        
        assert(_TexturePtr->GetNumberOfPixelsU() <= UpdateSize[0] + Offset[0]);
        assert(_TexturePtr->GetNumberOfPixelsV() <= UpdateSize[1] + Offset[1]);
        
        CInternTexture* pInternTexture = static_cast<CInternTexture*>(_TexturePtr.GetPtr());
        
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

    void CGfxTextureManager::CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        BASE_UNUSED(_UpdateMipLevels);
        BASE_UNUSED(_NumberOfBytesPerLine);
                
        // -----------------------------------------------------------------------------
        // Get information
        // -----------------------------------------------------------------------------
        glm::uvec2 Size = _rTargetRect.Size();

        assert(_pBytes != 0);
        assert(_TextureArrayPtr.IsValid());
        assert(_TextureArrayPtr->GetNumberOfPixelsU() == Size[0] && _TextureArrayPtr->GetNumberOfPixelsV() == Size[1]);
        
        glm::uvec2 Offset     = glm::uvec2(0);
        glm::uvec2 UpdateSize = glm::uvec2(_TextureArrayPtr->GetNumberOfPixelsU(), _TextureArrayPtr->GetNumberOfPixelsV());
        
        assert(Size[0] <= UpdateSize[0] + Offset[0]);
        assert(Size[1] <= UpdateSize[1] + Offset[1]);
        
        CInternTexture* pInternTextureArray = static_cast<CInternTexture*>(_TextureArrayPtr.GetPtr());
        
        int Format = ConvertGLImageFormat(pInternTextureArray->GetFormat());
        int Type   = ConvertGLImageType  (pInternTextureArray->GetFormat());
        
        // -----------------------------------------------------------------------------
        // Upload data to texture
        // -----------------------------------------------------------------------------
        if (pInternTextureArray->m_Info.m_IsCubeTexture)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, _TextureArrayPtr->GetNativeHandle());

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + _IndexOfSlice, 0, Offset[0], Offset[1], UpdateSize[0], UpdateSize[1], Format, Type, _pBytes);
        }
        else
        {
            // TODO by tschwandt
            // Do this not only for cubemaps!

            ENGINE_CONSOLE_STREAMWARNING("Copy to texture array is actually not supported.");
        }
    }

    // -----------------------------------------------------------------------------
    
    void CGfxTextureManager::CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexturePtr _TexturePtr, bool _UpdateMipLevels)
    {
        BASE_UNUSED(_UpdateMipLevels);

        // -----------------------------------------------------------------------------
        // Get informations
        // -----------------------------------------------------------------------------
        assert(_TextureArrayPtr.IsValid());
        assert(_TexturePtr     .IsValid());
        assert(_TextureArrayPtr->GetNumberOfPixelsU() == _TexturePtr->GetNumberOfPixelsU() && _TextureArrayPtr->GetNumberOfPixelsV() == _TexturePtr->GetNumberOfPixelsV());
        
        glm::uvec2 Offset     = glm::uvec2(0);
        glm::uvec2 UpdateSize = glm::uvec2(_TextureArrayPtr->GetNumberOfPixelsU(), _TextureArrayPtr->GetNumberOfPixelsV());
        
        assert(_TexturePtr->GetNumberOfPixelsU() <= UpdateSize[0] + Offset[0]);
        assert(_TexturePtr->GetNumberOfPixelsV() <= UpdateSize[1] + Offset[1]);

        int Format = ConvertGLImageFormat(_TextureArrayPtr->GetFormat());
        int Type   = ConvertGLImageType(_TextureArrayPtr->GetFormat());
        
        // -----------------------------------------------------------------------------
        // Upload data to texture
        // -----------------------------------------------------------------------------
        if (_TextureArrayPtr->IsCube())
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, _TextureArrayPtr->GetNativeHandle());

            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + _IndexOfSlice, 0, Offset[0], Offset[1], UpdateSize[0], UpdateSize[1], Format, Type, _TexturePtr->GetPixels());
        }
        else
        {
            // TODO by tschwandt
            // Do this not only for cubemaps!

            ENGINE_CONSOLE_STREAMWARNING("Copy to texture array is actually not supported.");
        }
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::GetMipmapFromTexture2D(CTexturePtr _TexturePtr, unsigned int _Mipmap)
    {
        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CTextures::CPtr Texture2DPtr;

        assert(_Mipmap < _TexturePtr->GetNumberOfMipLevels());

        try
        {
            Texture2DPtr = m_Textures.Allocate();
            
            CInternTexture& rTexture = *Texture2DPtr;
            
            int MipmapPow = static_cast<int>(glm::pow(2, static_cast<int>(_Mipmap)));

            rTexture.m_FileName          = _TexturePtr->GetFileName();
            rTexture.m_pPixels           = _TexturePtr->GetPixels();
            rTexture.m_NumberOfPixels[0] = static_cast<Gfx::CTexture::BPixels>(glm::max(static_cast<int>(_TexturePtr->GetNumberOfPixelsU()) / MipmapPow, 1));
            rTexture.m_NumberOfPixels[1] = static_cast<Gfx::CTexture::BPixels>(glm::max(static_cast<int>(_TexturePtr->GetNumberOfPixelsV()) / MipmapPow, 1));
            
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
            
            CInternTexture* pInternalTexture = static_cast<CInternTexture*>(_TexturePtr.GetPtr());

            rTexture.m_NativeTexture        = pInternalTexture->m_NativeTexture;
            rTexture.m_NativeBinding        = pInternalTexture->m_NativeBinding;
            rTexture.m_NativeInternalFormat = pInternalTexture->m_NativeInternalFormat;
            rTexture.m_NativeUsage          = pInternalTexture->m_NativeUsage;
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }
        
        return CTexturePtr(Texture2DPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxTextureManager::UpdateMipmap(CTexturePtr _TexturePtr)
    {
        assert(_TexturePtr != 0);
        
        CInternTexture* pInternTexture = static_cast<CInternTexture*>(_TexturePtr.GetPtr());

        assert(pInternTexture);

        if (pInternTexture->IsCube())
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, pInternTexture->m_NativeTexture);

            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, pInternTexture->m_NativeTexture);

            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::SaveTexture(CTexturePtr _TexturePtr, const std::string& _rPathToFile)
    {
        // -----------------------------------------------------------------------------
        // Get data
        // -----------------------------------------------------------------------------
        assert(_TexturePtr != 0);

        CInternTexture* pInternTexture = static_cast<CInternTexture*>(_TexturePtr.GetPtr());

        assert(pInternTexture);

        // -----------------------------------------------------------------------------
        // Handle error cases
        // -----------------------------------------------------------------------------
        if (pInternTexture->GetNumberOfPixelsW() > 1)
        {
            ENGINE_CONSOLE_WARNING("Saving 3D textures is not suported. Saving aborted!");

            return;
        }

        if (_rPathToFile.find_last_of('.') == -1 || _rPathToFile.substr(_rPathToFile.find_last_of('.')) != ".ppm")
        {
            ENGINE_CONSOLE_WARNING("No or unsupported image extension found. Use Portable Pixmap (.ppm) to save textures.");

            return;
        }

#if PLATFORM_ANDROID
        static const int s_NumberOfChannels = 4;
#else
        static const int s_NumberOfChannels = 3;
#endif

        // -----------------------------------------------------------------------------
        // Save data to PPM function
        // -----------------------------------------------------------------------------
        auto SaveBytesToPPM = [](const std::string& _rPathToFile, int _Width, int _Height, void* _pBytes)
        {
            std::ofstream PPMOutput;

            PPMOutput.open(_rPathToFile, std::ofstream::out);

            PPMOutput << "P3" << std::endl;

            PPMOutput << _Width << " " << _Height << std::endl;

            PPMOutput << "255" << std::endl;

            for (int IndexOfPixel = 0; IndexOfPixel < _Width * _Height; ++IndexOfPixel)
            {
                int Index = IndexOfPixel * s_NumberOfChannels;

                PPMOutput << ((char*)_pBytes)[Index + 0] % 255 << " ";
                PPMOutput << ((char*)_pBytes)[Index + 1] % 255 << " ";
                PPMOutput << ((char*)_pBytes)[Index + 2] % 255 << " ";
            }

            PPMOutput.close();
        };

        // -----------------------------------------------------------------------------
        // Allocate memory
        // -----------------------------------------------------------------------------
        int NumberOfPixel = pInternTexture->GetNumberOfPixelsU() * pInternTexture->GetNumberOfPixelsV();
        int NumberOfBytes = NumberOfPixel * s_NumberOfChannels * sizeof(char);

        void* pBytes = Base::CMemory::Allocate(NumberOfBytes);

        // -----------------------------------------------------------------------------
        // Get data from GPU
        // -----------------------------------------------------------------------------
        glBindTexture(pInternTexture->m_NativeBinding, pInternTexture->m_NativeTexture);

        if (pInternTexture->IsCube())
        {
#if PLATFORM_ANDROID
            GLuint Framebuffer;

            glGenFramebuffers(1, &Framebuffer);

            glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
#endif

            for (int i = 0; i < 6; i++)
            {
#if PLATFORM_ANDROID
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, pInternTexture->m_NativeTexture, 0);

                GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

                glReadPixels(0, 0, pInternTexture->GetNumberOfPixelsU(), pInternTexture->GetNumberOfPixelsV(), GL_RGBA, GL_UNSIGNED_BYTE, pBytes);
#else
                glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_BYTE, pBytes);
#endif

                std::string PathToFilePerFace = _rPathToFile.substr(0, _rPathToFile.find_last_of('.'));

                PathToFilePerFace += "_" + std::to_string(i) + _rPathToFile.substr(_rPathToFile.find_last_of('.'));

                SaveBytesToPPM(PathToFilePerFace, pInternTexture->GetNumberOfPixelsU(), pInternTexture->GetNumberOfPixelsV(), pBytes);
            }

#if PLATFORM_ANDROID
            glDeleteFramebuffers(1, &Framebuffer);
#endif
        }
        else
        {
#if PLATFORM_ANDROID
            GLuint Framebuffer;

            glGenFramebuffers(1, &Framebuffer);

            glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);

            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pInternTexture->m_NativeTexture, 0);

            glReadPixels(0, 0, pInternTexture->GetNumberOfPixelsU(), pInternTexture->GetNumberOfPixelsV(), GL_RGB, GL_BYTE, pBytes);

            glDeleteFramebuffers(1, &Framebuffer);
#else
            glGetTexImage(pInternTexture->m_NativeBinding, 0, GL_RGB, GL_BYTE, pBytes);
#endif

            SaveBytesToPPM(_rPathToFile, pInternTexture->GetNumberOfPixelsU(), pInternTexture->GetNumberOfPixelsV(), pBytes);
        }

        // -----------------------------------------------------------------------------
        // Release memory
        // -----------------------------------------------------------------------------
        Base::CMemory::Free(pBytes);
    }

    // -----------------------------------------------------------------------------

    void CGfxTextureManager::SetTextureLabel(CTexturePtr _TexturePtr, const char* _pLabel)
    {
        assert(_pLabel != nullptr);

        CInternTexture* pInternTexture = static_cast<CInternTexture*>(_TexturePtr.GetPtr());

        assert(pInternTexture != nullptr);
        
        glObjectLabel(GL_TEXTURE, pInternTexture->m_NativeTexture, -1, _pLabel);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::InternCreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
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
        ILuint       NativeImageName;
        ILenum       NativeILFormat;
        ILenum       NativeILType;

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

        NativeILFormat  = ConvertILImageFormat(_rDescriptor.m_Format);
        NativeILType    = ConvertILImageType(_rDescriptor.m_Format);

        NativeImageName = 0;

        // -----------------------------------------------------------------------------
        // Load texture from file if one is set in descriptor
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr)
        {
            // -----------------------------------------------------------------------------
            // Create and bin texture on DevIL
            // -----------------------------------------------------------------------------
            NativeImageName = ilGenImage();

            ilBindImage(NativeImageName);

            // -----------------------------------------------------------------------------
            // Load texture from file (either in assets or data)
            // -----------------------------------------------------------------------------
            std::string PathToTexture;

            PathToTexture = Core::AssetManager::GetPathToAssets() + "/" + _rDescriptor.m_pFileName;

#ifdef PLATFORM_ANDROID
            const char* pPathToTexture = 0;

            pPathToTexture = PathToTexture.c_str();
#else
            const wchar_t* pPathToTexture = 0;

            pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
#endif

            Result = ilLoadImage(pPathToTexture) == IL_TRUE;

            if (!Result)
            {
                PathToTexture = Core::AssetManager::GetPathToData() + g_PathToDataTextures + _rDescriptor.m_pFileName;

#ifdef PLATFORM_ANDROID
                pPathToTexture = PathToTexture.c_str();
#else
                pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
#endif

                Result = ilLoadImage(pPathToTexture) == IL_TRUE;
            }

            if (Result)
            {
                ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
                ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);

                if (_rDescriptor.m_Format != STextureDescriptor::s_FormatFromSource)
                {
                    if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
                    {
                        ilConvertImage(NativeILFormat, NativeILType);
                    }
                }
                else
                {
                    NativeILFormat = CheckILFormat;
                    NativeILType   = CheckILType;

                    GLInternalFormat = ilGetInteger(IL_IMAGE_CHANNELS) == 4 ? GL_RGBA8 : GL_RGB8;
                    GLFormat         = NativeILFormat;
                    GLType           = NativeILType;
                }

                pTextureData = ilGetData();

                ImageWidth    = ilGetInteger(IL_IMAGE_WIDTH);
                ImageHeight   = ilGetInteger(IL_IMAGE_HEIGHT);
                NumberOfBytes = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);

                assert(ImageWidth    > 0);
                assert(ImageHeight   > 0);
                assert(NumberOfBytes > 0);
            }
            else
            {
                ENGINE_CONSOLE_ERRORV("Failed loading image '%s' from file.", PathToTexture.c_str());

                ilDeleteImage(NativeImageName);

                return nullptr;
            }
        }

        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            NumberOfMipmaps = static_cast<int>(glm::log2(static_cast<float>(glm::max(ImageWidth, ImageHeight)))) + 1;
        }
        else if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_NumberOfMipMapsFromSource)
        {
            NumberOfMipmaps = ilGetInteger(IL_NUM_MIPMAPS);
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
        if (_rDescriptor.m_Binding & Gfx::CTexture::DepthStencilTarget)
        {
            GLInternalFormat = GL_DEPTH_COMPONENT32F;
        }

        glTexStorage2D(GL_TEXTURE_2D, NumberOfMipmaps, GLInternalFormat, ImageWidth, ImageHeight);

        // -----------------------------------------------------------------------------
        // Is data available, then upload it to graphic card
        // -----------------------------------------------------------------------------
        if (pTextureData != 0)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ImageWidth, ImageHeight, GLFormat, GLType, pTextureData);

            if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_NumberOfMipMapsFromSource && NumberOfMipmaps > 1 && _rDescriptor.m_pFileName != nullptr)
            {
                for (unsigned int IndexOfMipMap = 1; IndexOfMipMap < NumberOfMipmaps; ++IndexOfMipMap)
                {
                    ilBindImage(NativeImageName);

                    ilActiveMipmap(IndexOfMipMap);

                    ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
                    ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);

                    if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
                    {
                        ilConvertImage(NativeILFormat, NativeILType);
                    }

                    ImageWidth   = ilGetInteger(IL_IMAGE_WIDTH);
                    ImageHeight  = ilGetInteger(IL_IMAGE_HEIGHT);
                    pTextureData = ilGetData();

                    glTexSubImage2D(GL_TEXTURE_2D, IndexOfMipMap, 0, 0, ImageWidth, ImageHeight, GLFormat, GLType, pTextureData);
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
        CTexturePtr Texture2DPtr = static_cast<CTexturePtr>(m_Textures.Allocate());
        
        assert(NumberOfBytes > 0);
        
        try
        {
            CInternTexture& rTexture = *static_cast<CInternTexture*>(Texture2DPtr.GetPtr());
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != 0) rTexture.m_FileName = _rDescriptor.m_pFileName;

            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Gfx::CTexture::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Gfx::CTexture::BPixels>(ImageHeight);
            rTexture.m_Hash              = 0;
            
            rTexture.m_Info.m_Access            = _rDescriptor.m_Access;
            rTexture.m_Info.m_Binding           = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension         = CTexture::Dim2D;
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
            rTexture.m_NativeBinding        = GL_TEXTURE_2D;
            
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
                    ENGINE_CONSOLE_STREAMWARNING("Undefined texture data behavior while creating an texture.");
                    break;
            }

            // -----------------------------------------------------------------------------
            // Delete image on DevIL because it isn't needed anymore
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr)
            {
                ilDeleteImage(NativeImageName);

                ilBindImage(0);
            }
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }
        
        return Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::InternCreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
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
        ILenum       NativeILFormat;
        ILenum       NativeILType;

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

        NativeILFormat  = ConvertILImageFormat(_rDescriptor.m_Format);
        NativeILType    = ConvertILImageType(_rDescriptor.m_Format);

        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            NumberOfMipmaps = static_cast<int>(glm::log2(static_cast<float>(glm::max(ImageWidth, ImageHeight)))) + 1;
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
        if (_rDescriptor.m_Binding & Gfx::CTexture::DepthStencilTarget)
        {
            glTexStorage3D(GL_TEXTURE_3D, NumberOfMipmaps, GL_DEPTH_COMPONENT32F, ImageWidth, ImageHeight, ImageDepth);
        }
        else if (_rDescriptor.m_Binding & Gfx::CTexture::RenderTarget)
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
        CTexturePtr Texture3DPtr = static_cast<CTexturePtr>(m_Textures.Allocate());

        assert(NumberOfBytes > 0);

        try
        {
            CInternTexture& rTexture = *static_cast<CInternTexture*>(Texture3DPtr.GetPtr());
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != 0) rTexture.m_FileName = _rDescriptor.m_pFileName;

            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Gfx::CTexture::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Gfx::CTexture::BPixels>(ImageHeight);
            rTexture.m_NumberOfPixels[2] = static_cast<Gfx::CTexture::BPixels>(ImageDepth);
            rTexture.m_Hash              = 0;
            
            rTexture.m_Info.m_Access            = _rDescriptor.m_Access;
            rTexture.m_Info.m_Binding           = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension         = CTexture::Dim3D;
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
            rTexture.m_NativeBinding        = GL_TEXTURE_3D;
            
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
                    ENGINE_CONSOLE_STREAMWARNING("Undefined texture data behavior while creating an texture.");
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

    CTexturePtr CGfxTextureManager::InternCreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
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
        ILuint       NumberOfFaces;
        ILuint       NativeImageName;
        ILenum       NativeILFormat;
        ILenum       NativeILType;
        ILinfo       NativeILImageInfo;

        assert(_rDescriptor.m_NumberOfTextures == 6 && _rDescriptor.m_NumberOfPixelsW == 1 && _rDescriptor.m_pPixels == 0);

        ImageIsLoaded = false;
        NumberOfFaces = 1;
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

        NativeILFormat  = ConvertILImageFormat(_rDescriptor.m_Format);
        NativeILType    = ConvertILImageType(_rDescriptor.m_Format);

        NativeImageName = 0;

        // -----------------------------------------------------------------------------
        // Load texture from file if one is set in descriptor
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr)
        {
            // -----------------------------------------------------------------------------
            // Create and bind texture on DevIL
            // -----------------------------------------------------------------------------
            NativeImageName = ilGenImage();

            ilBindImage(NativeImageName);

            // -----------------------------------------------------------------------------
            // Load texture from file (either in assets or data)
            // -----------------------------------------------------------------------------
            std::string PathToTexture;

            PathToTexture = Core::AssetManager::GetPathToAssets() + "/" + _rDescriptor.m_pFileName;

#ifdef PLATFORM_ANDROID
            const char* pPathToTexture = 0;

            pPathToTexture = PathToTexture.c_str();
#else
            const wchar_t* pPathToTexture = 0;

            pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
#endif

            ImageIsLoaded = ilLoadImage(pPathToTexture) == IL_TRUE;

            if (!ImageIsLoaded)
            {
                PathToTexture = Core::AssetManager::GetPathToData() + g_PathToDataTextures + _rDescriptor.m_pFileName;

#ifdef PLATFORM_ANDROID
                pPathToTexture = PathToTexture.c_str();
#else
                pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
#endif

                ImageIsLoaded = ilLoadImage(pPathToTexture) == IL_TRUE;
            }

            if (ImageIsLoaded)
            {
                NumberOfFaces = ilGetInteger(IL_NUM_FACES);
            }

            if (ImageIsLoaded && NumberOfFaces == 5)
            {
                ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
                ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);

                if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
                {
                    ilConvertImage(NativeILFormat, NativeILType);
                }

                pTextureData = ilGetData();

                ImageWidth    = ilGetInteger(IL_IMAGE_WIDTH);
                ImageHeight   = ilGetInteger(IL_IMAGE_HEIGHT);
                NumberOfBytes = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);

                assert(ImageWidth    > 0);
                assert(ImageHeight   > 0);
                assert(NumberOfBytes > 0);
            }
            else
            {
                ENGINE_CONSOLE_STREAMERROR("Failed loading cubemap '" << PathToTexture.c_str() << "' from file.");

                ilDeleteImage(NativeImageName);

                ilBindImage(0);

                return nullptr;
            }
        }

        if (_rDescriptor.m_NumberOfMipMaps == STextureDescriptor::s_GenerateAllMipMaps)
        {
            NumberOfMipmaps = static_cast<int>(glm::log2(static_cast<float>(glm::max(ImageWidth, ImageHeight)))) + 1;
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
        if (_rDescriptor.m_Binding & Gfx::CTexture::DepthStencilTarget)
        {
            glTexStorage2D(GL_TEXTURE_CUBE_MAP, NumberOfMipmaps, GL_DEPTH_COMPONENT32F, ImageWidth, ImageHeight);
        }
        else if (_rDescriptor.m_Binding & Gfx::CTexture::RenderTarget)
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
            for (unsigned int IndexOfFace = 0; IndexOfFace <= NumberOfFaces; ++IndexOfFace)
            {
                ilBindImage(NativeImageName);

                ilActiveFace(IndexOfFace);

                ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
                ILenum CheckILType   = ilGetInteger(IL_IMAGE_TYPE);

                if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
                {
                    ilConvertImage(NativeILFormat, NativeILType);
                }

                iluGetImageInfo(&NativeILImageInfo);

                if (NativeILImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
                {
                    iluFlipImage();
                }

                pTextureData = ilGetData();

                glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + IndexOfFace, 0, 0, 0, ImageWidth, ImageHeight, GLFormat, GLType, pTextureData);
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
        CTexturePtr Texture2DPtr = static_cast<CTexturePtr>(m_Textures.Allocate());

        assert(NumberOfBytes > 0);

        try
        {
            CInternTexture& rTexture = *static_cast<CInternTexture*>(Texture2DPtr.GetPtr());
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName != 0) rTexture.m_FileName = _rDescriptor.m_pFileName;

            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Gfx::CTexture::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Gfx::CTexture::BPixels>(ImageHeight);
            rTexture.m_NumberOfPixels[2] = static_cast<Gfx::CTexture::BPixels>(1);
            rTexture.m_Hash              = 0;
            
            rTexture.m_Info.m_Access            = _rDescriptor.m_Access;
            rTexture.m_Info.m_Binding           = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension         = CTexture::Dim2D;
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
            rTexture.m_NativeBinding        = GL_TEXTURE_CUBE_MAP;
            
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
                    ENGINE_CONSOLE_STREAMWARNING("Undefined texture data behavior while creating an texture.");
                    break;
            }

            // -----------------------------------------------------------------------------
            // Delete image on DevIL because it isn't needed anymore
            // -----------------------------------------------------------------------------
            if (ImageIsLoaded)
            {
                ilDeleteImage(NativeImageName);

                ilBindImage(0);
            }
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }
        
        return Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CGfxTextureManager::InternCreateExternalTexture()
    {
        GLuint NativeTextureHandle;

        // -----------------------------------------------------------------------------
        // Generate OpenGL external texture
        // -----------------------------------------------------------------------------
        glGenTextures(1, &NativeTextureHandle);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, NativeTextureHandle);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CTexturePtr Texture2DPtr = static_cast<CTexturePtr>(m_Textures.Allocate());

        try
        {
            CInternTexture& rTexture = *static_cast<CInternTexture*>(Texture2DPtr.GetPtr());

            rTexture.m_pPixels           = 0;
            rTexture.m_NumberOfPixels[0] = 0;
            rTexture.m_NumberOfPixels[1] = 0;
            rTexture.m_Hash              = 0;

            rTexture.m_Info.m_Access            = CTexture::CPUWrite;
            rTexture.m_Info.m_Binding           = CTexture::ShaderResource;
            rTexture.m_Info.m_Dimension         = CTexture::External;
            rTexture.m_Info.m_Format            = 255; // TODO: should be CTexture::Unknown but since m_Format is 8 bit unsigned it cannot store -1
            rTexture.m_Info.m_IsCubeTexture     = true;
            rTexture.m_Info.m_IsDeletable       = Base::U32(false);
            rTexture.m_Info.m_IsDummyTexture    = false;
            rTexture.m_Info.m_NumberOfTextures  = 1;
            rTexture.m_Info.m_NumberOfMipLevels = 1;
            rTexture.m_Info.m_CurrentMipLevel   = 0;
            rTexture.m_Info.m_Semantic          = CTexture::Diffuse;
            rTexture.m_Info.m_Usage             = CTexture::GPUReadWrite;

            rTexture.m_NativeTexture        = NativeTextureHandle;
            rTexture.m_NativeUsage          = GL_READ_ONLY;
            rTexture.m_NativeInternalFormat = GL_NONE;
            rTexture.m_NativeBinding        = GL_TEXTURE_EXTERNAL_OES;
        }
        catch (...)
        {
            BASE_THROWM("Error creating texture in texture manager.");
        }

        return Texture2DPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxTextureManager::ConvertGLFormatToBytesPerPixel(Gfx::CTexture::EFormat _Format) const
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
            3
        };
        
        return s_NativeFormat[_Format];
    }

    // -----------------------------------------------------------------------------

    int CGfxTextureManager::ConvertGLImageUsage(Gfx::CTexture::EUsage _Usage) const
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
    
    int CGfxTextureManager::ConvertGLInternalImageFormat(Gfx::CTexture::EFormat _Format) const
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
            GL_RGB8,
        };
        
        return s_NativeFormat[_Format];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxTextureManager::ConvertGLImageFormat(Gfx::CTexture::EFormat _Format) const
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
            GL_RED_INTEGER,
            GL_RG_INTEGER,
            GL_RGB_INTEGER,
            GL_RGBA_INTEGER,
            GL_RED_INTEGER,
            GL_RG_INTEGER,
            GL_RGB_INTEGER,
            GL_RGBA_INTEGER,
            
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
            GL_RED_INTEGER,
            GL_RG_INTEGER,
            GL_RGB_INTEGER,
            GL_RGBA_INTEGER,
            GL_RED_INTEGER,
            GL_RG_INTEGER,
            GL_RGB_INTEGER,
            GL_RGBA_INTEGER,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            
            GL_RED_INTEGER,
            GL_RG_INTEGER,
            GL_RGB_INTEGER,
            GL_RGBA_INTEGER,
            GL_RED_INTEGER,
            GL_RG_INTEGER,
            GL_RGB_INTEGER,
            GL_RGBA_INTEGER,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
            
            GL_RGB,
            GL_RGBA,
            GL_RGBA,
            GL_RGBA,
#ifdef PLATFORM_WINDOWS
            GL_BGR
#else
            GL_RGB
#endif // PLATFORM_WINDOWS
        };
        
        return s_NativeFormat[_Format];
    }
    
    // -----------------------------------------------------------------------------
    
    int CGfxTextureManager::ConvertGLImageType(Gfx::CTexture::EFormat _Format) const
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
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_SHORT,
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
            GL_UNSIGNED_BYTE
        };
        
        return s_NativeType[_Format];
    }

    // -----------------------------------------------------------------------------

    ILenum CGfxTextureManager::ConvertILImageFormat(Gfx::CTexture::EFormat _Format) const
    {
        static ILenum s_NativeFormat[] =
        {
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,

            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,

            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,
            IL_LUMINANCE,
            0,
            IL_RGB,
            IL_RGBA,

            IL_RGB,
            IL_RGBA,
            IL_RGBA,
            IL_RGBA,
            IL_BGR
        };

        return s_NativeFormat[_Format];
    }

    // -----------------------------------------------------------------------------

    ILenum CGfxTextureManager::ConvertILImageType(Gfx::CTexture::EFormat _Format) const
    {
        static int s_NativeType[] =
        {
            IL_BYTE,
            IL_BYTE,
            IL_BYTE,
            IL_BYTE,
            IL_UNSIGNED_BYTE,
            IL_UNSIGNED_BYTE,
            IL_UNSIGNED_BYTE,
            IL_UNSIGNED_BYTE,
            IL_SHORT,
            IL_SHORT,
            IL_SHORT,
            IL_SHORT,
            IL_UNSIGNED_SHORT,
            IL_UNSIGNED_SHORT,
            IL_UNSIGNED_SHORT,
            IL_UNSIGNED_SHORT,
            IL_INT,
            IL_INT,
            IL_INT,
            IL_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,

            IL_BYTE,
            IL_BYTE,
            IL_BYTE,
            IL_BYTE,
            IL_UNSIGNED_BYTE,
            IL_UNSIGNED_BYTE,
            IL_UNSIGNED_BYTE,
            IL_UNSIGNED_BYTE,
            IL_SHORT,
            IL_SHORT,
            IL_SHORT,
            IL_SHORT,
            IL_UNSIGNED_SHORT,
            IL_UNSIGNED_SHORT,
            IL_UNSIGNED_SHORT,
            IL_UNSIGNED_SHORT,
            IL_INT,
            IL_INT,
            IL_INT,
            IL_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_FLOAT,
            IL_FLOAT,
            IL_FLOAT,
            IL_FLOAT,

            IL_INT,
            IL_INT,
            IL_INT,
            IL_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_UNSIGNED_INT,
            IL_FLOAT,
            IL_FLOAT,
            IL_FLOAT,
            IL_FLOAT,

            0,
            0,
            0,
            0,
            IL_UNSIGNED_BYTE
        };

        return s_NativeType[_Format];
    }
} // namespace

namespace
{
    CGfxTextureManager::CInternTexture::CInternTexture()
        : CNativeTexture()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxTextureManager::CInternTexture::~CInternTexture()
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

    CTexturePtr GetDummyTexture2D()
    {
        return CGfxTextureManager::GetInstance().GetDummyTexture2D();
    }

    // -----------------------------------------------------------------------------

    CTexturePtr GetDummyTexture3D()
    {
        return CGfxTextureManager::GetInstance().GetDummyTexture3D();
    }

    // -----------------------------------------------------------------------------

    CTexturePtr GetDummyCubeTexture()
    {
        return CGfxTextureManager::GetInstance().GetDummyCubeTexture();
    }
    
    // -----------------------------------------------------------------------------

    CTexturePtr CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CGfxTextureManager::GetInstance().CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CGfxTextureManager::GetInstance().CreateTexture3D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CGfxTextureManager::GetInstance().CreateCubeTexture(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CreateExternalTexture()
    {
        return CGfxTextureManager::GetInstance().CreateExternalTexture();
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr)
    {
        CTexturePtr TexturePtrs[] = { _Texture1Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 1);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr)
    {
        CTexturePtr TexturePtrs[] = { _Texture1Ptr, _Texture2Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 2);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr, CTexturePtr _Texture3Ptr)
    {
        CTexturePtr TexturePtrs[] = { _Texture1Ptr, _Texture2Ptr, _Texture3Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 3);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr, CTexturePtr _Texture3Ptr, CTexturePtr _Texture4Ptr)
    {
        CTexturePtr TexturePtrs[] = { _Texture1Ptr, _Texture2Ptr, _Texture3Ptr, _Texture4Ptr, };

        return CGfxTextureManager::GetInstance().CreateTextureSet(TexturePtrs, 4);
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CreateTextureSet(CTexturePtr* _pTexturePtrs, unsigned int _NumberOfTextures)
    {
        return CGfxTextureManager::GetInstance().CreateTextureSet(_pTexturePtrs, _NumberOfTextures);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr GetTextureByHash(unsigned int _Hash)
    {
        return CGfxTextureManager::GetInstance().GetTextureByHash(_Hash);
    }
    
    // -----------------------------------------------------------------------------

    void ClearTextureLayer(CTexturePtr _TexturePtr, const void* _pData, int _Layer)
    {
        CGfxTextureManager::GetInstance().ClearTextureLayer(_TexturePtr, _pData, _Layer);
    }

    // -----------------------------------------------------------------------------

    void ClearTexture(CTexturePtr _TexturePtr, const void* _pData)
    {
        CGfxTextureManager::GetInstance().ClearTexture(_TexturePtr, _pData);
    }
    
    // -----------------------------------------------------------------------------

    void CopyToTexture2D(CTexturePtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        CGfxTextureManager::GetInstance().CopyToTexture2D(_TexturePtr, _rTargetRect, _NumberOfBytesPerLine, _pBytes, _UpdateMipLevels);
    }

    // -----------------------------------------------------------------------------

    void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels)
    {
        CGfxTextureManager::GetInstance().CopyToTextureArray2D(_TextureArrayPtr, _IndexOfSlice, _rTargetRect, _NumberOfBytesPerLine, _pBytes, _UpdateMipLevels);
    }

    // -----------------------------------------------------------------------------

    void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexturePtr _TexturePtr, bool _UpdateMipLevels)
    {
        CGfxTextureManager::GetInstance().CopyToTextureArray2D(_TextureArrayPtr, _IndexOfSlice, _TexturePtr, _UpdateMipLevels);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr GetMipmapFromTexture2D(CTexturePtr _TexturePtr, unsigned int _Mipmap)
    {
        return CGfxTextureManager::GetInstance().GetMipmapFromTexture2D(_TexturePtr, _Mipmap);
    }
    
    // -----------------------------------------------------------------------------
    
    void UpdateMipmap(CTexturePtr _TexturePtr)
    {
        CGfxTextureManager::GetInstance().UpdateMipmap(_TexturePtr);
    }

    // -----------------------------------------------------------------------------

    void SaveTexture(CTexturePtr _TexturePtr, const std::string& _rPathToFile)
    {
        CGfxTextureManager::GetInstance().SaveTexture(_TexturePtr, _rPathToFile);
    }

    // -----------------------------------------------------------------------------

    void SetTextureLabel(CTexturePtr _TexturePtr, const char* _pLabel)
    {
        CGfxTextureManager::GetInstance().SetTextureLabel(_TexturePtr, _pLabel);
    }
} // namespace TextureManager
} // namespace Gfx
