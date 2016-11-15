
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_material_manager.h"
#include "data/data_texture_manager.h"

#include "editor/edit_material_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

namespace
{
    class CMaterialHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMaterialHelper)

    public:

        CMaterialHelper();
        ~CMaterialHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnNewMaterial(Edit::CMessage& _rMessage);

        void OnRequestMaterialInfo(Edit::CMessage& _rMessage);

        void OnMaterialInfo(Edit::CMessage& _rMessage);

        void OnDirtyMaterial(Dt::CMaterial* _pMaterial);

        std::string CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile);
    };
} // namespace

namespace
{
    CMaterialHelper::CMaterialHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CMaterialHelper::~CMaterialHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::MaterialManager::RegisterDirtyMaterialHandler(DATA_DIRTY_MATERIAL_METHOD(&CMaterialHelper::OnDirtyMaterial));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewMaterial, EDIT_RECEIVE_MESSAGE(&CMaterialHelper::OnNewMaterial));

        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestMaterialInfo, EDIT_RECEIVE_MESSAGE(&CMaterialHelper::OnRequestMaterialInfo));

        Edit::MessageManager::Register(Edit::SGUIMessageType::MaterialInfo, EDIT_RECEIVE_MESSAGE(&CMaterialHelper::OnMaterialInfo));
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnNewMaterial(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Model
        // -----------------------------------------------------------------------------
        Dt::SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_pMaterialName   = 0;
        MaterialDescriptor.m_pColorMap       = 0;
        MaterialDescriptor.m_pNormalMap      = 0;
        MaterialDescriptor.m_pRoughnessMap   = 0;
        MaterialDescriptor.m_pReflectanceMap = 0;
        MaterialDescriptor.m_pMetalMaskMap   = 0;
        MaterialDescriptor.m_pAOMap          = 0;
        MaterialDescriptor.m_pBumpMap        = 0;
        MaterialDescriptor.m_Roughness       = 1.0f;
        MaterialDescriptor.m_Reflectance     = 0.0f;
        MaterialDescriptor.m_MetalMask       = 0.0f;
        MaterialDescriptor.m_AlbedoColor     = Base::Float3(1.0f);
        MaterialDescriptor.m_TilingOffset    = Base::Float4(1.0f, 1.0f, 0.0f, 0.0f);
        MaterialDescriptor.m_pFileName       = 0;
        
        Dt::CMaterial& rNewMaterial = Dt::MaterialManager::CreateMaterial(MaterialDescriptor);

        // -----------------------------------------------------------------------------
        // Add model to map
        // -----------------------------------------------------------------------------
        Dt::MaterialManager::MarkMaterialAsDirty(rNewMaterial, Dt::CMaterial::DirtyCreate);
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnRequestMaterialInfo(Edit::CMessage& _rMessage)
    {
        int MaterialHash = _rMessage.GetInt();

        Dt::CMaterial& rMaterial = Dt::MaterialManager::GetMaterialByHash(static_cast<unsigned int>(MaterialHash));

        Edit::CMessage NewMessage;

        NewMessage.PutInt(rMaterial.GetHash());

        NewMessage.PutFloat(rMaterial.GetColor()[0]);
        NewMessage.PutFloat(rMaterial.GetColor()[1]);
        NewMessage.PutFloat(rMaterial.GetColor()[2]);

        NewMessage.PutFloat(rMaterial.GetTilingOffset()[0]);
        NewMessage.PutFloat(rMaterial.GetTilingOffset()[1]);
        NewMessage.PutFloat(rMaterial.GetTilingOffset()[2]);
        NewMessage.PutFloat(rMaterial.GetTilingOffset()[3]);

        NewMessage.PutFloat(rMaterial.GetRoughness());
        NewMessage.PutFloat(rMaterial.GetReflectance());
        NewMessage.PutFloat(rMaterial.GetMetalness());

        if (rMaterial.GetColorTexture())
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(rMaterial.GetColorTexture()->GetFileName());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (rMaterial.GetNormalTexture())
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(rMaterial.GetNormalTexture()->GetFileName());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (rMaterial.GetRoughnessTexture())
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(rMaterial.GetRoughnessTexture()->GetFileName());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (rMaterial.GetMetalTexture())
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(rMaterial.GetMetalTexture()->GetFileName());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::MaterialInfo, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnMaterialInfo(Edit::CMessage& _rMessage)
    {
        int MaterialHash = _rMessage.GetInt();

        Dt::CMaterial& rMaterial = Dt::MaterialManager::GetMaterialByHash(static_cast<unsigned int>(MaterialHash));

        float X, Y, Z, W;

        bool HasColorMap     = false;
        bool HasNormalMap    = false;
        bool HasRoughnessMap = false;
        bool HasMetalnessMap = false;

        char ColorMapName[256];
        char NormalMapName[256];
        char RoughnessMapName[256];
        char MetalMapName[256];

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();

        Base::Float3 Color = Base::Float3(X, Y, Z);

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();
        W = _rMessage.GetFloat();

        Base::Float4 TilingOffset = Base::Float4(X, Y, Z, W);

        float Roughness = _rMessage.GetFloat();

        float Reflectance = _rMessage.GetFloat();

        float Metalness = _rMessage.GetFloat();

        HasColorMap = _rMessage.GetBool();

        if (HasColorMap)
        {
            _rMessage.GetString(ColorMapName, 256);
        }

        HasNormalMap = _rMessage.GetBool();

        if (HasNormalMap)
        {
            _rMessage.GetString(NormalMapName, 256);
        }

        HasRoughnessMap = _rMessage.GetBool();

        if (HasRoughnessMap)
        {
            _rMessage.GetString(RoughnessMapName, 256);
        }

        HasMetalnessMap = _rMessage.GetBool();

        if (HasMetalnessMap)
        {
            _rMessage.GetString(MetalMapName, 256);
        }

        rMaterial.SetColor       (Color);
        rMaterial.SetTilingOffset(TilingOffset);
        rMaterial.SetRoughness   (Roughness);
        rMaterial.SetReflectance (Reflectance);
        rMaterial.SetMetalness   (Metalness);

        Dt::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_Binding          = Dt::CTextureBase::ShaderResource;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = 0;

        if (HasColorMap)
        {
            Dt::CTexture2D* pTexture = rMaterial.GetColorTexture();

            if (pTexture != nullptr && strcmp(pTexture->GetFileName(), ColorMapName))
            {
                Dt::TextureManager::CopyToTexture2D(pTexture, ColorMapName);

                Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyFile);
            }
            else
            {
                TextureDescriptor.m_pFileName = ColorMapName;

                pTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

                if (pTexture != nullptr)
                {
                    rMaterial.SetColorTexture(pTexture);

                    Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyCreate);
                }
            }
        }
        else if (HasColorMap == false)
        {
            rMaterial.SetColorTexture(0);
        }

        if (HasNormalMap)
        {
            Dt::CTexture2D* pTexture = rMaterial.GetNormalTexture();

            if (pTexture != nullptr && strcmp(pTexture->GetFileName(), NormalMapName))
            {
                Dt::TextureManager::CopyToTexture2D(pTexture, NormalMapName);

                Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyFile);
            }
            else
            {
                TextureDescriptor.m_pFileName = NormalMapName;

                pTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

                if (pTexture != nullptr)
                {
                    rMaterial.SetNormalTexture(pTexture);

                    Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyCreate);
                }
            }
        }
        else if (HasNormalMap == false)
        {
            rMaterial.SetNormalTexture(0);
        }

        if (HasRoughnessMap)
        {
            Dt::CTexture2D* pTexture = rMaterial.GetRoughnessTexture();

            if (pTexture != nullptr && strcmp(pTexture->GetFileName(), RoughnessMapName))
            {
                Dt::TextureManager::CopyToTexture2D(pTexture, RoughnessMapName);

                Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyFile);
            }
            else
            {
                TextureDescriptor.m_pFileName = RoughnessMapName;

                pTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

                if (pTexture != nullptr)
                {
                    rMaterial.SetRoughnessTexture(pTexture);

                    Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyCreate);
                }
            }
        }
        else if (HasRoughnessMap == false)
        {
            rMaterial.SetRoughnessTexture(0);
        }

        if (HasMetalnessMap)
        {
            Dt::CTexture2D* pTexture = rMaterial.GetMetalTexture();

            if (pTexture != nullptr && strcmp(pTexture->GetFileName(), MetalMapName))
            {
                Dt::TextureManager::CopyToTexture2D(pTexture, MetalMapName);

                Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyFile);
            }
            else
            {
                TextureDescriptor.m_pFileName = MetalMapName;

                pTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

                if (pTexture != nullptr)
                {
                    rMaterial.SetMetalTexture(pTexture);

                    Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyCreate);
                }
            }
        }
        else if (HasMetalnessMap == false)
        {
            rMaterial.SetMetalTexture(0);
        }

        Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyData);
    }


    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnDirtyMaterial(Dt::CMaterial* _pMaterial)
    {
        assert(_pMaterial != nullptr);

        if ((_pMaterial->GetDirtyFlags() & Dt::CMaterial::DirtyCreate) != 0)
        {
            
        }
    }

    // -----------------------------------------------------------------------------

    std::string CMaterialHelper::CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile)
    {
        char pDrive[4];
        char pDirectory[512];
        char pFilename[32];
        char pExtension[12];

        std::string FileExtension;
        std::string RelativePathToModel;

        _splitpath_s(_pPathToFile, pDrive, 4, pDirectory, 512, pFilename, 32, pExtension, 12);

        FileExtension = std::string(pFilename) + std::string(pExtension);
        RelativePathToModel = std::string(_pAssetFolder) + FileExtension;

        CopyFileA(_pPathToFile, RelativePathToModel.c_str(), true);

        return FileExtension.c_str();
    };
} // namespace

namespace Edit
{
namespace Helper
{
namespace Material
{
    void OnStart()
    {
        CMaterialHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMaterialHelper::GetInstance().OnExit();
    }
} // namespace Material
} // namespace Helper
} // namespace Edit