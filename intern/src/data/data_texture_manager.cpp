
#include "data/data_precompiled.h"

#include "base/base_crc.h"
#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_manager.h"
#include "core/core_time.h"

#include "data/data_texture_manager.h"

#include "IL/il.h"

#include <unordered_map>

using namespace Dt;
using namespace Dt::TextureManager;

namespace
{
    class CDtTextureManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtTextureManager)
        
    public:

        CDtTextureManager();
       ~CDtTextureManager();

    public:

        void OnStart();
        void OnExit();

    public:

        CTextureBase* CreateTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::Listen, bool _IsInternal = false);
        CTexture1D* CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior, bool _IsInternal = false);
        CTexture2D* CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior, bool _IsInternal = false);

        CTextureCube* CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior, bool _IsInternal = false);

        CTextureBase* GetTextureByHash(unsigned int _Hash);
        CTexture2D* GetTexture2DByHash(unsigned int _Hash);
        CTextureCube* GetTextureCubeByHash(unsigned int _Hash);

        void CopyToTexture2D(CTexture2D* _pTexture2D, const Base::Char* _pFile);
        void CopyToTexture2D(CTexture2D* _pTexture2D, void* _pPixels);
        void CopyToTexture2D(CTexture2D* _pTexture2D, CTexture2D* _pTexture);

        void CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, void* _pPixels);
        void CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, CTexture2D* _pTexture);

        void SaveTexture2DToFile(CTexture2D* _pTexture2D, const Base::Char* _pPathToFile);

        void MarkTextureAsDirty(CTextureBase* _pTextureBase, unsigned int _DirtyFlags);

        void RegisterDirtyTextureHandler(CTextureDelegate _NewDelegate);

    private:

        // -----------------------------------------------------------------------------
        // Represents a base texture.
        // -----------------------------------------------------------------------------
        class CInternTextureBase : public CTextureBase
        {
        public:

            CInternTextureBase();
            ~CInternTextureBase();

        private:

            friend class  CDtTextureManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a 1D texture.
        // -----------------------------------------------------------------------------
        class CInternTexture1D : public CTexture1D
        {
            public:

                CInternTexture1D();
               ~CInternTexture1D();

            private:

                friend class  CDtTextureManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a 2D texture.
        // -----------------------------------------------------------------------------
        class CInternTexture2D : public CTexture2D
        {
            public:

                CInternTexture2D();
               ~CInternTexture2D();

            private:

                friend class  CDtTextureManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a cube texture.
        // -----------------------------------------------------------------------------
        class CInternTextureCube : public CTextureCube
        {
            public:

                CInternTextureCube();
                ~CInternTextureCube();

            private:

                friend class  CDtTextureManager;
        };

        // -----------------------------------------------------------------------------
        // There are way more 2D textures than 1D or 3D ones, so use bigger pages here.
        // -----------------------------------------------------------------------------
        typedef Base::CPool<CInternTexture1D,    16> CTexture1Ds;
        typedef Base::CPool<CInternTexture2D,   256> CTexture2Ds;
        typedef Base::CPool<CInternTextureCube,  16> CTextureCubes;

        typedef CTexture1Ds::CIterator   CTexture1DIterator;
        typedef CTexture2Ds::CIterator   CTexture2DIterator;
        typedef CTextureCubes::CIterator CTextureCubeIterator;
        
        typedef std::unordered_map<unsigned int, CInternTexture1D*> CTexture1DByHashs;
        typedef std::unordered_map<unsigned int, CInternTexture2D*> CTexture2DByHashs;
        typedef std::unordered_map<unsigned int, CInternTextureCube*> CTextureCubeByHashs;

        typedef std::vector<CTextureDelegate> CTextureDelegates;

    private:

        CTexture1Ds          m_Textures1D;
        CTexture2Ds          m_Textures2D;
        CTextureCubes        m_TexturesCube;
        CTexture1DByHashs    m_Textures1DByHash;
        CTexture2DByHashs    m_Textures2DByHash;
        CTextureCubeByHashs  m_TexturesCubeByHash;
        CTextureDelegates    m_TextureDelegates;

    private:

        int ConvertFormatToBytesPerPixel(Dt::CTextureBase::EFormat _Format) const;

        Dt::CTextureBase::EFormat ConvertImageFormat(ILenum _ILFormat, ILenum _ILType) const;

        ILenum ConvertILImageFormat(Dt::CTextureBase::EFormat _Format) const;
        ILenum ConvertILImageType(Dt::CTextureBase::EFormat _Format) const;
        ILubyte ConvertILImageChannels(Dt::CTextureBase::EFormat _Format) const;
    };
} // namespace

namespace
{
    CDtTextureManager::CDtTextureManager()
        : m_Textures1D        ()
        , m_Textures2D        ()
        , m_TexturesCube      ()
        , m_Textures1DByHash  ()
        , m_Textures2DByHash  ()
        , m_TexturesCubeByHash()
        , m_TextureDelegates  ()
    {
    }

    // -----------------------------------------------------------------------------

    CDtTextureManager::~CDtTextureManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Initialize devil image engine. But we only initialize core part.
        // -----------------------------------------------------------------------------
//         ilInit();
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::OnExit()
    {
        // -----------------------------------------------------------------------------
        // Clear all the pools with the textures.
        // -----------------------------------------------------------------------------
        m_Textures1D  .Clear();
        m_Textures2D  .Clear();
        m_TexturesCube.Clear();

        m_Textures1DByHash  .clear();
        m_Textures2DByHash  .clear();
        m_TexturesCubeByHash.clear();

        m_TextureDelegates.clear();
    }

    // -----------------------------------------------------------------------------

    CTextureBase* CDtTextureManager::CreateTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior, bool _IsInternal)
    {
        const Base::Char* pHashIdentifier;
        void*             pBytes;
        void*             pTextureData;
        unsigned int      Hash;
        unsigned int      NativeImageName;
        int               NumberOfPixelsU;
        int               NumberOfPixelsV;
        int               NumberOfPixelsW;
        int               NumberOfFaces;
        unsigned int      NumberOfBytes;
        bool              Result;
        ILenum            NativeILFormat;
        ILenum            NativeILType;

        pHashIdentifier = nullptr;
        pBytes          = nullptr;
        pTextureData    = nullptr;
        Hash            = 0;

        // -----------------------------------------------------------------------------
        // Create hash value over user identifier or filename
        // -----------------------------------------------------------------------------
        pHashIdentifier = _rDescriptor.m_pIdentifier != 0 ? _rDescriptor.m_pIdentifier : _rDescriptor.m_pFileName;

        if (pHashIdentifier != nullptr && _IsInternal == false)
        {
            const void* pData;

            NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
            pData = static_cast<const void*>(pHashIdentifier);

            Hash = Base::CRC32(pData, NumberOfBytes);

            if (m_Textures1DByHash.find(Hash) != m_Textures1DByHash.end())
            {
                return m_Textures1DByHash.at(Hash);
            }

            if (m_Textures2DByHash.find(Hash) != m_Textures2DByHash.end())
            {
                return m_Textures2DByHash.at(Hash);
            }

            if (m_TexturesCubeByHash.find(Hash) != m_TexturesCubeByHash.end())
            {
                return m_TexturesCubeByHash.at(Hash);
            }
        }

        // -----------------------------------------------------------------------------
        // If format is undefined and texture is set, then we extract information from
        // file and call internal function.
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_Format == CTextureBase::UndefinedFormat && _rDescriptor.m_pFileName != nullptr)
        {
            Dt::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = _rDescriptor.m_NumberOfPixelsU;
            TextureDescriptor.m_NumberOfPixelsV  = _rDescriptor.m_NumberOfPixelsV;
            TextureDescriptor.m_NumberOfPixelsW  = _rDescriptor.m_NumberOfPixelsW;
            TextureDescriptor.m_Format           = _rDescriptor.m_Format;
            TextureDescriptor.m_Semantic         = _rDescriptor.m_Semantic;
            TextureDescriptor.m_Binding          = _rDescriptor.m_Binding;
            TextureDescriptor.m_pPixels          = 0;
            TextureDescriptor.m_pFileName        = _rDescriptor.m_pFileName;
            TextureDescriptor.m_pIdentifier      = _rDescriptor.m_pIdentifier;

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

#if __ANDROID__
            const char* pPathToTexture = PathToTexture.c_str();
#else
            const wchar_t* pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
#endif

            Result = ilLoadImage(pPathToTexture) == IL_TRUE;

            if (Result)
            {
                NativeILFormat  = ilGetInteger(IL_IMAGE_FORMAT);
                NativeILType    = ilGetInteger(IL_IMAGE_TYPE);
                NumberOfPixelsU = ilGetInteger(IL_IMAGE_WIDTH);
                NumberOfPixelsV = ilGetInteger(IL_IMAGE_HEIGHT);
                NumberOfPixelsW = ilGetInteger(IL_IMAGE_DEPTH);
                NumberOfFaces   = ilGetInteger(IL_NUM_FACES);

                CTextureBase::EFormat Format = ConvertImageFormat(NativeILFormat, NativeILType);

                TextureDescriptor.m_NumberOfPixelsU  = NumberOfPixelsU;
                TextureDescriptor.m_NumberOfPixelsV  = NumberOfPixelsV;
                TextureDescriptor.m_NumberOfPixelsW  = NumberOfPixelsW;
                TextureDescriptor.m_Format           = Format;

                ilDeleteImage(NativeImageName);

                ilBindImage(0);

                if (NumberOfFaces == 5)
                {
                    return CreateCubeTexture(TextureDescriptor, _IsDeleteable, _Behavior, _IsInternal);
                }
                else if (NumberOfPixelsW > 1)
                {
                    // return CreateTexture3D(TextureDescriptor, _IsDeleteable, _Behavior, _IsInternal);
                }
                else if (NumberOfPixelsV > 1)
                {
                    return CreateTexture2D(TextureDescriptor, _IsDeleteable, _Behavior, _IsInternal);
                }
                else if (NumberOfPixelsU > 1)
                {
                    return CreateTexture1D(TextureDescriptor, _IsDeleteable, _Behavior, _IsInternal);
                }
            }
            else
            {
                BASE_CONSOLE_STREAMERROR("Failed loading image '" << PathToTexture.c_str() << "' from file.");

                ilDeleteImage(NativeImageName);

                return nullptr;
            }
        }

        // -----------------------------------------------------------------------------
        // Else, we create the texture depending on specified number of pixels.
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_NumberOfPixelsW > 1 && _rDescriptor.m_NumberOfPixelsW != Dt::STextureDescriptor::s_NumberOfPixelsFromSource)
        {
            // return CreateTexture3D(_rDescriptor, _IsDeleteable, _Behavior, _IsInternal);
        }
        else if (_rDescriptor.m_NumberOfPixelsV > 1 && _rDescriptor.m_NumberOfPixelsV != Dt::STextureDescriptor::s_NumberOfPixelsFromSource)
        {
            return CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior, _IsInternal);
        }
        else if (_rDescriptor.m_NumberOfPixelsU > 1 && _rDescriptor.m_NumberOfPixelsU != Dt::STextureDescriptor::s_NumberOfPixelsFromSource)
        {
            return CreateTexture1D(_rDescriptor, _IsDeleteable, _Behavior, _IsInternal);
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTexture1D* CDtTextureManager::CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior, bool _IsInternal)
    {
        BASE_UNUSED(_rDescriptor);
        BASE_UNUSED(_IsDeleteable);
        BASE_UNUSED(_Behavior);
        BASE_UNUSED(_IsInternal);

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTexture2D* CDtTextureManager::CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior, bool _IsInternal)
    {
        const Base::Char* pHashIdentifier;
        void*             pBytes;
        void*             pTextureData;
        unsigned int      Hash;
        unsigned int      NativeImageName;
        int               ImageWidth;
        int               ImageHeight;
        int               NumberOfPixel;
        int               NumberOfBytes;
        bool              Result;
        ILenum            NativeILFormat;
        ILenum            NativeILType;

        pHashIdentifier = nullptr;
        pBytes          = nullptr;
        pTextureData    = nullptr;
        Hash            = 0;
        
        // -----------------------------------------------------------------------------
        // Create hash value over user identifier or filename
        // -----------------------------------------------------------------------------
        pHashIdentifier = _rDescriptor.m_pIdentifier != 0 ? _rDescriptor.m_pIdentifier : _rDescriptor.m_pFileName;

        if (pHashIdentifier != nullptr && _IsInternal == false)
        {
            const void* pData;

            NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
            pData         = static_cast<const void*>(pHashIdentifier);
            
            Hash = Base::CRC32(pData, NumberOfBytes);
            
            if (m_Textures2DByHash.find(Hash) != m_Textures2DByHash.end())
            {
                return m_Textures2DByHash.at(Hash);
            }
        }
        
        // -----------------------------------------------------------------------------
        // Setup variables
        // -----------------------------------------------------------------------------
        ImageWidth      = _rDescriptor.m_NumberOfPixelsU;
        ImageHeight     = _rDescriptor.m_NumberOfPixelsV;
        pBytes          = nullptr;
        pTextureData    = _rDescriptor.m_pPixels;
        NumberOfPixel   = ImageWidth * ImageHeight;
        NumberOfBytes   = ConvertFormatToBytesPerPixel(_rDescriptor.m_Format) * NumberOfPixel;

        NativeImageName = 0;

        // -----------------------------------------------------------------------------
        // Load texture from file if one is set in descriptor
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr && _Behavior != SDataBehavior::Listen && _IsInternal == false)
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

            NativeILFormat = ConvertILImageFormat(_rDescriptor.m_Format);
            NativeILType   = ConvertILImageType(_rDescriptor.m_Format);

#if __ANDROID__
            const char* pPathToTexture = PathToTexture.c_str();
#else
            const wchar_t* pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
#endif
            
            Result = ilLoadImage(pPathToTexture) == IL_TRUE;
            
            if (Result)
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
                BASE_CONSOLE_STREAMERROR("Failed loading image '" << PathToTexture.c_str() << "' from file.");

                ilDeleteImage(NativeImageName);
                
                return nullptr;
            }
        }

        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CInternTexture2D* pTexture = nullptr;

        assert(NumberOfBytes > 0);
        
        try
        {
            CInternTexture2D& rTexture = m_Textures2D.Allocate();

            pTexture = &rTexture;
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName)   rTexture.m_FileName   = _rDescriptor.m_pFileName;
            if (_rDescriptor.m_pIdentifier) rTexture.m_Identifier = _rDescriptor.m_pIdentifier;

            rTexture.m_Hash              = Hash;
            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Dt::CTextureBase::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Dt::CTextureBase::BPixels>(ImageHeight);
            
            rTexture.m_Info.m_Binding          = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension        = CTextureBase::Dim2D;
            rTexture.m_Info.m_Format           = _rDescriptor.m_Format;
            rTexture.m_Info.m_IsCubeTexture    = false;
            rTexture.m_Info.m_IsDeletable      = _IsDeleteable;
            rTexture.m_Info.m_IsDummyTexture   = false;
            rTexture.m_Info.m_Semantic         = _rDescriptor.m_Semantic;
            rTexture.m_Info.m_NumberOfTextures = 1;
            
            // -----------------------------------------------------------------------------
            // Set hash to map
            // -----------------------------------------------------------------------------
            if (Hash != 0)
            {
                m_Textures2DByHash[Hash] = &rTexture;
            }
            
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
                case SDataBehavior::Listen:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                }
                break;

                case SDataBehavior::TakeOwnerShip:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = _rDescriptor.m_pPixels != 0 ? _rDescriptor.m_pPixels : pTextureData;
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
        
        return pTexture;
    }

    // -----------------------------------------------------------------------------

    CTextureCube* CDtTextureManager::CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior, bool _IsInternal)
    {
        const Base::Char* pHashIdentifier;
        void*             pBytes;
        void*             pTextureData;
        unsigned int      Hash;
        int               ImageWidth;
        int               ImageHeight;
        int               NumberOfPixel;
        int               NumberOfBytes;
        unsigned int      NativeImageName;
        bool              ImageIsLoaded;
        ILenum            NativeILFormat;
        ILenum            NativeILType;
        unsigned int      NumberOfFaces;

        NumberOfFaces   = 1;
        ImageIsLoaded   = false;
        pHashIdentifier = nullptr;
        pBytes          = nullptr;
        pTextureData    = nullptr;
        Hash            = 0;
        NativeImageName = 0;
        NativeILFormat  = 0;
        NativeILType    = 0;
        
        // -----------------------------------------------------------------------------
        // Create hash value over user identifier or filename
        // -----------------------------------------------------------------------------
        pHashIdentifier = _rDescriptor.m_pIdentifier != 0 ? _rDescriptor.m_pIdentifier : _rDescriptor.m_pFileName;

        if (pHashIdentifier != nullptr && _IsInternal == false)
        {
            const void* pData;

            NumberOfBytes = static_cast<unsigned int>(strlen(pHashIdentifier) * sizeof(char));
            pData         = static_cast<const void*>(pHashIdentifier);
            
            Hash = Base::CRC32(pData, NumberOfBytes);
            
            if (m_TexturesCubeByHash.find(Hash) != m_TexturesCubeByHash.end())
            {
                return m_TexturesCubeByHash.at(Hash);
            }
        }
        
        // -----------------------------------------------------------------------------
        // Setup variables
        // -----------------------------------------------------------------------------
        ImageWidth      = _rDescriptor.m_NumberOfPixelsU;
        ImageHeight     = _rDescriptor.m_NumberOfPixelsV;
        pBytes          = nullptr;
        pTextureData    = _rDescriptor.m_pPixels;
        NumberOfPixel   = ImageWidth * ImageHeight;
        NumberOfBytes   = ConvertFormatToBytesPerPixel(_rDescriptor.m_Format) * NumberOfPixel;

        // -----------------------------------------------------------------------------
        // Load texture from file if one is set in descriptor
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && _rDescriptor.m_pPixels == nullptr && _Behavior != SDataBehavior::Listen && _IsInternal == false)
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

            NativeILFormat = ConvertILImageFormat(_rDescriptor.m_Format);
            NativeILType   = ConvertILImageType(_rDescriptor.m_Format);

#if __ANDROID__
            const char* pPathToTexture = PathToTexture.c_str();
#else
            const wchar_t* pPathToTexture = reinterpret_cast<const wchar_t*>(PathToTexture.c_str());
#endif

            ImageIsLoaded = ilLoadImage(pPathToTexture) == IL_TRUE;

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

                assert(ImageWidth > 0);
                assert(ImageHeight > 0);
                assert(NumberOfBytes > 0);
            }
            else
            {
                BASE_CONSOLE_STREAMERROR("Failed loading cubemap '" << PathToTexture.c_str() << "' from file.");

                ilDeleteImage(NativeImageName);

                ilBindImage(0);

                return nullptr;
            }
        }

        // -----------------------------------------------------------------------------
        // Generate texture inside texture manager
        // -----------------------------------------------------------------------------
        CInternTextureCube* pTexture = nullptr;

        assert(NumberOfBytes > 0);
        
        try
        {
            CInternTextureCube& rTexture = m_TexturesCube.Allocate();

            pTexture = &rTexture;
            
            // -----------------------------------------------------------------------------
            // Setup the new texture inside manager
            // -----------------------------------------------------------------------------
            if (_rDescriptor.m_pFileName)   rTexture.m_FileName   = _rDescriptor.m_pFileName;
            if (_rDescriptor.m_pIdentifier) rTexture.m_Identifier = _rDescriptor.m_pIdentifier;

            rTexture.m_Hash              = Hash;
            rTexture.m_pPixels           = _rDescriptor.m_pPixels;
            rTexture.m_NumberOfPixels[0] = static_cast<Dt::CTextureBase::BPixels>(ImageWidth);
            rTexture.m_NumberOfPixels[1] = static_cast<Dt::CTextureBase::BPixels>(ImageHeight);
            
            rTexture.m_Info.m_Binding          = _rDescriptor.m_Binding;
            rTexture.m_Info.m_Dimension        = CTextureBase::Dim2D;
            rTexture.m_Info.m_Format           = _rDescriptor.m_Format;
            rTexture.m_Info.m_IsCubeTexture    = true;
            rTexture.m_Info.m_IsDeletable      = _IsDeleteable;
            rTexture.m_Info.m_IsDummyTexture   = false;
            rTexture.m_Info.m_Semantic         = _rDescriptor.m_Semantic;
            rTexture.m_Info.m_NumberOfTextures = 6;
            
            // -----------------------------------------------------------------------------
            // Set hash to map
            // -----------------------------------------------------------------------------
            if (Hash != 0)
            {
                m_TexturesCubeByHash[Hash] = &rTexture;
            }   
            
            // -----------------------------------------------------------------------------
            // Check the behavior.
            // -----------------------------------------------------------------------------
            if (_Behavior == SDataBehavior::Copy || _Behavior == SDataBehavior::CopyAndDelete)
            {
                pBytes = Base::CMemory::Allocate(NumberOfBytes);
                
                if (pTextureData)
                {
                    memcpy(pBytes, pTextureData, NumberOfBytes);
                }
                else if (_rDescriptor.m_pPixels)
                {
                    memcpy(pBytes, _rDescriptor.m_pPixels, NumberOfBytes);
                }
            }
            
            switch (_Behavior)
            {   
                case SDataBehavior::Listen:
                {
                    rTexture.m_Info.m_IsPixelOwner = false;
                }
                break;

                case SDataBehavior::TakeOwnerShip:
                {
                    rTexture.m_Info.m_IsPixelOwner = true;
                    rTexture.m_pPixels             = _rDescriptor.m_pPixels != 0 ? _rDescriptor.m_pPixels : pTextureData;
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
            // Create faces
            // -----------------------------------------------------------------------------
            rTexture.m_pFaces[Dt::CTextureCube::Right ] = CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior, true);
            rTexture.m_pFaces[Dt::CTextureCube::Left  ] = CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior, true);
            rTexture.m_pFaces[Dt::CTextureCube::Top   ] = CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior, true);
            rTexture.m_pFaces[Dt::CTextureCube::Bottom] = CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior, true);
            rTexture.m_pFaces[Dt::CTextureCube::Front ] = CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior, true);
            rTexture.m_pFaces[Dt::CTextureCube::Back  ] = CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior, true);

            // -----------------------------------------------------------------------------
            // Copy data if image is loaded from file
            // -----------------------------------------------------------------------------
            if (ImageIsLoaded)
            {
                for (unsigned int IndexOfFace = 0; IndexOfFace < NumberOfFaces; IndexOfFace++)
                {
                    ilBindImage(NativeImageName);

                    ilActiveFace(IndexOfFace);

                    ILenum CheckILFormat = ilGetInteger(IL_IMAGE_FORMAT);
                    ILenum CheckILType = ilGetInteger(IL_IMAGE_TYPE);

                    if (CheckILFormat != NativeILFormat || CheckILType != NativeILType)
                    {
                        ilConvertImage(NativeILFormat, NativeILType);
                    }

                    pTextureData = ilGetData();

                    CopyToTexture2D(rTexture.m_pFaces[IndexOfFace], pTextureData);
                }
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
        
        return pTexture;
    }

    // -----------------------------------------------------------------------------

    CTextureBase* CDtTextureManager::GetTextureByHash(unsigned int _Hash)
    {
        if (m_Textures1DByHash.find(_Hash) != m_Textures1DByHash.end())
        {
            return m_Textures1DByHash.at(_Hash);
        }

        if (m_Textures2DByHash.find(_Hash) != m_Textures2DByHash.end())
        {
            return m_Textures2DByHash.at(_Hash);
        }

        if (m_TexturesCubeByHash.find(_Hash) != m_TexturesCubeByHash.end())
        {
            return m_TexturesCubeByHash.at(_Hash);
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTexture2D* CDtTextureManager::GetTexture2DByHash(unsigned int _Hash)
    {
        if (m_Textures2DByHash.find(_Hash) != m_Textures2DByHash.end())
        {
            return m_Textures2DByHash.at(_Hash);
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CTextureCube* CDtTextureManager::GetTextureCubeByHash(unsigned int _Hash)
    {
        if (m_TexturesCubeByHash.find(_Hash) != m_TexturesCubeByHash.end())
        {
            return m_TexturesCubeByHash.at(_Hash);
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::CopyToTexture2D(CTexture2D* _pTexture2D, const Base::Char* _pFile)
    {
        assert(_pTexture2D != 0);
        assert(_pFile != 0);

        unsigned int      NumberOfBytes;
        const Base::Char* pHashIdentifier;
        unsigned int      Hash;
        const void*       pData;

        // -----------------------------------------------------------------------------
        // Get internal texture
        // -----------------------------------------------------------------------------
        CInternTexture2D* pInternTexture2D = static_cast<CInternTexture2D*>(_pTexture2D);

        // -----------------------------------------------------------------------------
        // Prepare
        // -----------------------------------------------------------------------------
        pHashIdentifier = nullptr;
        Hash            = pInternTexture2D->m_Hash;

        // -----------------------------------------------------------------------------
        // Create hash value over filename if no identifier is set
        // -----------------------------------------------------------------------------
        if (pInternTexture2D->m_Identifier.length() == 0)
        {
            NumberOfBytes = static_cast<unsigned int>(strlen(_pFile) * sizeof(char));
            pData = static_cast<const void*>(_pFile);

            Hash = Base::CRC32(pData, NumberOfBytes);

            if (m_Textures2DByHash.find(Hash) != m_Textures2DByHash.end())
            {
                _pTexture2D = m_Textures2DByHash.at(Hash);
            }
        }

        // -----------------------------------------------------------------------------
        // Set texture
        // -----------------------------------------------------------------------------
        if (pInternTexture2D->m_Info.m_IsPixelOwner)
        {
            // TODO by tschwandt
            // Load file and copy data
        }
        else
        {
            pInternTexture2D->m_FileName = _pFile;

            pInternTexture2D->m_Hash = Hash;
        }
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::CopyToTexture2D(CTexture2D* _pTexture2D, void* _pPixels)
    {
        assert(_pTexture2D != 0);

        unsigned int NumberOfPixel;
        unsigned int NumberOfBytes;

        // -----------------------------------------------------------------------------
        // Get internal texture
        // -----------------------------------------------------------------------------
        CInternTexture2D* pInternTexture2D = static_cast<CInternTexture2D*>(_pTexture2D);

        // -----------------------------------------------------------------------------
        // Set texture
        // -----------------------------------------------------------------------------
        if (pInternTexture2D->m_Info.m_IsPixelOwner)
        {
            NumberOfPixel = pInternTexture2D->GetNumberOfPixelsU() * pInternTexture2D->GetNumberOfPixelsV();
            NumberOfBytes = ConvertFormatToBytesPerPixel(pInternTexture2D->GetFormat()) * NumberOfPixel;

            pInternTexture2D->m_pPixels = Base::CMemory::Copy(NumberOfBytes, _pPixels);
        }
        else
        {
            pInternTexture2D->m_pPixels = _pPixels;
        }
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::CopyToTexture2D(CTexture2D* _pTexture2D, CTexture2D* _pTexture)
    {
        assert(_pTexture2D != 0);

        unsigned int NumberOfPixel;
        unsigned int NumberOfBytes;

        // -----------------------------------------------------------------------------
        // Get internal texture
        // -----------------------------------------------------------------------------
        CInternTexture2D* pInternTexture2D = static_cast<CInternTexture2D*>(_pTexture2D);

        // -----------------------------------------------------------------------------
        // Set texture
        // -----------------------------------------------------------------------------
        if (pInternTexture2D->m_Info.m_IsPixelOwner)
        {
            NumberOfPixel = pInternTexture2D->GetNumberOfPixelsU() * pInternTexture2D->GetNumberOfPixelsV();
            NumberOfBytes = ConvertFormatToBytesPerPixel(pInternTexture2D->GetFormat()) * NumberOfPixel;

            pInternTexture2D->m_pPixels = Base::CMemory::Copy(NumberOfBytes, _pTexture->GetPixels());
        }
        else
        {
            pInternTexture2D->m_pPixels = _pTexture->GetPixels();
        }
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, void* _pPixels)
    {
        assert(_pTextureCube != 0);

        // -----------------------------------------------------------------------------
        // Get internal texture
        // -----------------------------------------------------------------------------
        CInternTextureCube* pInternTextureCube = static_cast<CInternTextureCube*>(_pTextureCube);

        // -----------------------------------------------------------------------------
        // Copy texture
        // -----------------------------------------------------------------------------
        CopyToTexture2D(pInternTextureCube->m_pFaces[_Face], _pPixels);
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, CTexture2D* _pTexture)
    {
        assert(_pTextureCube != 0);

        // -----------------------------------------------------------------------------
        // Get internal texture
        // -----------------------------------------------------------------------------
        CInternTextureCube* pInternTextureCube = static_cast<CInternTextureCube*>(_pTextureCube);

        // -----------------------------------------------------------------------------
        // Copy texture
        // -----------------------------------------------------------------------------
        CopyToTexture2D(pInternTextureCube->m_pFaces[_Face], _pTexture);
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::SaveTexture2DToFile(CTexture2D* _pTexture2D, const Base::Char* _pPathToFile)
    {
        assert(_pTexture2D && _pPathToFile);

        CInternTexture2D* pInternTexture2D = static_cast<CInternTexture2D*>(_pTexture2D);

        // -----------------------------------------------------------------------------
        // Variables
        // -----------------------------------------------------------------------------
        ILuint  NativeImageHandle;
        ILenum  NativeFormat;
        ILenum  NativeImageType;
        ILubyte NativeNumberOfChannels;

        // -----------------------------------------------------------------------------
        // Create devIL image
        // -----------------------------------------------------------------------------
        ilEnable(IL_FILE_OVERWRITE);
        ilGenImages(1, &NativeImageHandle);
        ilBindImage(NativeImageHandle);

        NativeFormat           = ConvertILImageFormat(pInternTexture2D->GetFormat());
        NativeImageType        = ConvertILImageType(pInternTexture2D->GetFormat());
        NativeNumberOfChannels = ConvertILImageChannels(pInternTexture2D->GetFormat());

        // -----------------------------------------------------------------------------
        // Save date to image
        // -----------------------------------------------------------------------------
        ilTexImage(pInternTexture2D->GetNumberOfPixelsU(), pInternTexture2D->GetNumberOfPixelsV(), 1, NativeNumberOfChannels, NativeFormat, NativeImageType, pInternTexture2D->GetPixels());

        // -----------------------------------------------------------------------------
        // Save image on file system
        // -----------------------------------------------------------------------------
        ilSave(IL_PNG, (const ILstring)_pPathToFile);
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::MarkTextureAsDirty(CTextureBase* _pTextureBase, unsigned int _DirtyFlags)
    {
        CInternTextureBase* pInternTextureBase = static_cast<CInternTextureBase*>(_pTextureBase);

        // -----------------------------------------------------------------------------
        // Flag
        // -----------------------------------------------------------------------------
        pInternTextureBase->m_DirtyFlags = _DirtyFlags;

        // -----------------------------------------------------------------------------
        // Dirty time
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        pInternTextureBase->m_DirtyTime = FrameTime;

        // -----------------------------------------------------------------------------
        // Send new dirty entity to all handler
        // -----------------------------------------------------------------------------
        CTextureDelegates::iterator CurrentDirtyDelegate = m_TextureDelegates.begin();
        CTextureDelegates::iterator EndOfDirtyDelegates  = m_TextureDelegates.end();

        for (; CurrentDirtyDelegate != EndOfDirtyDelegates; ++CurrentDirtyDelegate)
        {
            (*CurrentDirtyDelegate)(pInternTextureBase);
        }
    }

    // -----------------------------------------------------------------------------

    void CDtTextureManager::RegisterDirtyTextureHandler(CTextureDelegate _NewDelegate)
    {
        m_TextureDelegates.push_back(_NewDelegate);
    }
    
    // -----------------------------------------------------------------------------
    
    int CDtTextureManager::ConvertFormatToBytesPerPixel(Dt::CTextureBase::EFormat _Format) const
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

    Dt::CTextureBase::EFormat CDtTextureManager::ConvertImageFormat(ILenum _ILFormat, ILenum _ILType) const
    {
        // -----------------------------------------------------------------------------
        // Format
        // -----------------------------------------------------------------------------
        unsigned int Index = 0;

        if (_ILFormat == IL_LUMINANCE)
        {
            Index = 0;
        }
        else if (_ILFormat == IL_LUMINANCE_ALPHA)
        {
            Index = 1;
        }
        else if (_ILFormat == IL_RGB)
        {
            Index = 2;
        }
        else // (_ILFormat == IL_RGBA)
        {
            Index = 3;
        }

        // -----------------------------------------------------------------------------
        // Type
        // -----------------------------------------------------------------------------
        if (_ILType == IL_BYTE)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R8_BYTE,
                Dt::CTextureBase::R8G8_BYTE,
                Dt::CTextureBase::R8G8B8_BYTE,
                Dt::CTextureBase::R8G8B8A8_BYTE,
            };
            
            return s_Formats[Index];
        }
        else if (_ILType == IL_UNSIGNED_BYTE)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R8_UBYTE,
                Dt::CTextureBase::R8G8_UBYTE,
                Dt::CTextureBase::R8G8B8_UBYTE,
                Dt::CTextureBase::R8G8B8A8_UBYTE,
            };

            return s_Formats[Index];
        }
        else if (_ILType == IL_SHORT)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R8_SHORT,
                Dt::CTextureBase::R8G8_SHORT,
                Dt::CTextureBase::R8G8B8_SHORT,
                Dt::CTextureBase::R8G8B8A8_SHORT,
            };

            return s_Formats[Index];
        }
        else if (_ILType == IL_UNSIGNED_SHORT)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R8_USHORT,
                Dt::CTextureBase::R8G8_USHORT,
                Dt::CTextureBase::R8G8B8_USHORT,
                Dt::CTextureBase::R8G8B8A8_USHORT,
            };

            return s_Formats[Index];
        }
        else if (_ILType == IL_INT)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R8_INT,
                Dt::CTextureBase::R8G8_INT,
                Dt::CTextureBase::R8G8B8_INT,
                Dt::CTextureBase::R8G8B8A8_INT,
            };

            return s_Formats[Index];
        }
        else if (_ILType == IL_UNSIGNED_INT)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R8_UINT,
                Dt::CTextureBase::R8G8_UINT,
                Dt::CTextureBase::R8G8B8_UINT,
                Dt::CTextureBase::R8G8B8A8_UINT,
            };

            return s_Formats[Index];
        }
        else if (_ILType == IL_HALF)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R16_FLOAT,
                Dt::CTextureBase::R16G16_FLOAT,
                Dt::CTextureBase::R16G16B16_FLOAT,
                Dt::CTextureBase::R16G16B16A16_FLOAT,
            };

            return s_Formats[Index];
        }
        else if (_ILType == IL_FLOAT || _ILType == IL_DOUBLE)
        {
            static Dt::CTextureBase::EFormat s_Formats[] =
            {
                Dt::CTextureBase::R32_FLOAT,
                Dt::CTextureBase::R32G32_FLOAT,
                Dt::CTextureBase::R32G32B32_FLOAT,
                Dt::CTextureBase::R32G32B32A32_FLOAT,
            };

            return s_Formats[Index];
        }

        return Dt::CTextureBase::UndefinedFormat;
    }

    // -----------------------------------------------------------------------------

    ILenum CDtTextureManager::ConvertILImageFormat(Dt::CTextureBase::EFormat _Format) const
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
        };

        return s_NativeFormat[_Format];
    }

    // -----------------------------------------------------------------------------

    ILenum CDtTextureManager::ConvertILImageType(Dt::CTextureBase::EFormat _Format) const
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
        };

        return s_NativeType[_Format];
    }

    // -----------------------------------------------------------------------------

    ILubyte CDtTextureManager::ConvertILImageChannels(Dt::CTextureBase::EFormat _Format) const
    {
        static ILubyte s_NativeFormat[] =
        {
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,

            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,

            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,
            1,
            0,
            3,
            4,

            3,
            4,
            4,
            4,
        };

        return s_NativeFormat[_Format];
    }
} // namespace

namespace
{
    CDtTextureManager::CInternTextureBase::CInternTextureBase()
        : CTextureBase()
    {
    }

    // -----------------------------------------------------------------------------

    CDtTextureManager::CInternTextureBase::~CInternTextureBase()
    {
    }
} // namespace

namespace
{
    CDtTextureManager::CInternTexture1D::CInternTexture1D()
        : CTexture1D()
    {
    }

    // -----------------------------------------------------------------------------

    CDtTextureManager::CInternTexture1D::~CInternTexture1D()
    {
        if (m_Info.m_IsDeletable)
        {
            m_FileName  .clear();
            m_Identifier.clear();

            if (m_Info.m_IsPixelOwner)
            {
                Base::CMemory::Free(m_pPixels);
            }
        }
    }
} // namespace

namespace
{
    CDtTextureManager::CInternTexture2D::CInternTexture2D()
        : CTexture2D()
    {
    }

    // -----------------------------------------------------------------------------

    CDtTextureManager::CInternTexture2D::~CInternTexture2D()
    {
        if (m_Info.m_IsDeletable)
        {
            m_FileName  .clear();
            m_Identifier.clear();

            if (m_Info.m_IsPixelOwner)
            {
                Base::CMemory::Free(m_pPixels);
            }
        }
    }
} // namespace

namespace
{
    CDtTextureManager::CInternTextureCube::CInternTextureCube()
        : CTextureCube()
    {
    }

    // -----------------------------------------------------------------------------

    CDtTextureManager::CInternTextureCube::~CInternTextureCube()
    {
        if (m_Info.m_IsDeletable)
        {
            m_pFaces[0] = 0;
            m_pFaces[1] = 0;
            m_pFaces[2] = 0;
            m_pFaces[3] = 0;
            m_pFaces[4] = 0;
            m_pFaces[5] = 0;

            m_FileName  .clear();
            m_Identifier.clear();

            if (m_Info.m_IsPixelOwner)
            {
                if (m_pPixels)
                {
                    Base::CMemory::Free(m_pPixels);
                }
            }
        }
    }
} // namespace

namespace Dt
{
namespace TextureManager
{
    void OnStart()
    {
        CDtTextureManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtTextureManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    CTextureBase* CreateTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CDtTextureManager::GetInstance().CreateTexture(_rDescriptor, _IsDeleteable, _Behavior);
    }
    
    // -----------------------------------------------------------------------------

    CTexture1D* CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CDtTextureManager::GetInstance().CreateTexture1D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTexture2D* CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CDtTextureManager::GetInstance().CreateTexture2D(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTextureCube* CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable, SDataBehavior::Enum _Behavior)
    {
        return CDtTextureManager::GetInstance().CreateCubeTexture(_rDescriptor, _IsDeleteable, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CTextureBase* GetTextureByHash(unsigned int _Hash)
    {
        return CDtTextureManager::GetInstance().GetTextureByHash(_Hash);
    }

    // -----------------------------------------------------------------------------

    CTexture2D* GetTexture2DByHash(unsigned int _Hash)
    {
        return CDtTextureManager::GetInstance().GetTexture2DByHash(_Hash);
    }

    // -----------------------------------------------------------------------------

    CTextureCube* GetTextureCubeByHash(unsigned int _Hash)
    {
        return CDtTextureManager::GetInstance().GetTextureCubeByHash(_Hash);
    }

    // -----------------------------------------------------------------------------

    void CopyToTexture2D(CTexture2D* _pTexture2D, const Base::Char* _pFile)
    {
        CDtTextureManager::GetInstance().CopyToTexture2D(_pTexture2D, _pFile);
    }

    // -----------------------------------------------------------------------------

    void CopyToTexture2D(CTexture2D* _pTexture2D, void* _pPixels)
    {
        CDtTextureManager::GetInstance().CopyToTexture2D(_pTexture2D, _pPixels);
    }

    // -----------------------------------------------------------------------------

    void CopyToTexture2D(CTexture2D* _pTexture2D, CTexture2D* _pTexture)
    {
        CDtTextureManager::GetInstance().CopyToTexture2D(_pTexture2D, _pTexture);
    }

    // -----------------------------------------------------------------------------

    void CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, void* _pPixels)
    {
        CDtTextureManager::GetInstance().CopyToTextureCube(_pTextureCube, _Face, _pPixels);
    }

    // -----------------------------------------------------------------------------

    void CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, CTexture2D* _pTexture)
    {
        CDtTextureManager::GetInstance().CopyToTextureCube(_pTextureCube, _Face, _pTexture);
    }

    // -----------------------------------------------------------------------------

    void SaveTexture2DToFile(CTexture2D* _pTexture2D, const Base::Char* _pPathToFile)
    {
        CDtTextureManager::GetInstance().SaveTexture2DToFile(_pTexture2D, _pPathToFile);
    }

    // -----------------------------------------------------------------------------

    void MarkTextureAsDirty(CTextureBase* _pTextureBase, unsigned int _DirtyFlags)
    {
        CDtTextureManager::GetInstance().MarkTextureAsDirty(_pTextureBase, _DirtyFlags);
    }

    // -----------------------------------------------------------------------------

    void RegisterDirtyTextureHandler(CTextureDelegate _NewDelegate)
    {
        CDtTextureManager::GetInstance().RegisterDirtyTextureHandler(_NewDelegate);
    }
} // namespace TextureManager
} // namespace Dt
