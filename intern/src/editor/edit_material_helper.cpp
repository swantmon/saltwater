
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_material_manager.h"
#include "data/data_texture_manager.h"

#include "editor/edit_material_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

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

        void OnLoadMaterial(Edit::CMessage& _rMessage);

        void OnRequestMaterialInfo(Edit::CMessage& _rMessage);

        void OnMaterialUpdate(Edit::CMessage& _rMessage);

        void OnDirtyMaterial(Dt::CMaterial* _pMaterial);
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
        Edit::MessageManager::Register(Edit::SGUIMessageType::Material_New, EDIT_RECEIVE_MESSAGE(&CMaterialHelper::OnNewMaterial));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Material_Load, EDIT_RECEIVE_MESSAGE(&CMaterialHelper::OnLoadMaterial));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Material_Info, EDIT_RECEIVE_MESSAGE(&CMaterialHelper::OnRequestMaterialInfo));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Material_Update, EDIT_RECEIVE_MESSAGE(&CMaterialHelper::OnMaterialUpdate));
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnNewMaterial(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        Dt::SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_pMaterialName   = 0;
        MaterialDescriptor.m_pColorMap       = 0;
        MaterialDescriptor.m_pNormalMap      = 0;
        MaterialDescriptor.m_pRoughnessMap   = 0;
        MaterialDescriptor.m_pMetalMaskMap   = 0;
        MaterialDescriptor.m_pAOMap          = 0;
        MaterialDescriptor.m_pBumpMap        = 0;
        MaterialDescriptor.m_Roughness       = 1.0f;
        MaterialDescriptor.m_Reflectance     = 0.0f;
        MaterialDescriptor.m_MetalMask       = 0.0f;
        MaterialDescriptor.m_Displacement    = 0.0f;
        MaterialDescriptor.m_AlbedoColor     = glm::vec3(1.0f);
        MaterialDescriptor.m_TilingOffset    = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
        MaterialDescriptor.m_pFileName       = 0;
        
        Dt::CMaterial& rNewMaterial = Dt::MaterialManager::CreateMaterial(MaterialDescriptor);

        // -----------------------------------------------------------------------------
        // Add model to map
        // -----------------------------------------------------------------------------
        Dt::MaterialManager::MarkMaterialAsDirty(rNewMaterial, Dt::CMaterial::DirtyCreate);
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnLoadMaterial(Edit::CMessage& _rMessage)
    {
        char pTmp[512];

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        const char* pPathToFile = _rMessage.GetString(pTmp, 512);

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        Dt::SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_pMaterialName   = 0;
        MaterialDescriptor.m_pColorMap       = 0;
        MaterialDescriptor.m_pNormalMap      = 0;
        MaterialDescriptor.m_pRoughnessMap   = 0;
        MaterialDescriptor.m_pMetalMaskMap   = 0;
        MaterialDescriptor.m_pAOMap          = 0;
        MaterialDescriptor.m_pBumpMap        = 0;
        MaterialDescriptor.m_Roughness       = 1.0f;
        MaterialDescriptor.m_Reflectance     = 0.0f;
        MaterialDescriptor.m_MetalMask       = 0.0f;
        MaterialDescriptor.m_Displacement    = 0.0f;
        MaterialDescriptor.m_AlbedoColor     = glm::vec3(1.0f);
        MaterialDescriptor.m_TilingOffset    = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
        MaterialDescriptor.m_pFileName       = pPathToFile;
        
        Dt::CMaterial& rNewMaterial = Dt::MaterialManager::CreateMaterial(MaterialDescriptor);

        // -----------------------------------------------------------------------------
        // Set result as hash
        // -----------------------------------------------------------------------------
        _rMessage.SetResult(rNewMaterial.GetHash());

        // -----------------------------------------------------------------------------
        // Mark material as dirty
        // -----------------------------------------------------------------------------
        Dt::MaterialManager::MarkMaterialAsDirty(rNewMaterial, Dt::CMaterial::DirtyCreate);
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnRequestMaterialInfo(Edit::CMessage& _rMessage)
    {
        int MaterialHash = _rMessage.Get<int>();

        Dt::CMaterial& rMaterial = Dt::MaterialManager::GetMaterialByHash(static_cast<unsigned int>(MaterialHash));

        Edit::CMessage NewMessage;

        NewMessage.Put(rMaterial.GetHash());

        NewMessage.Put(rMaterial.GetColor()[0]);
        NewMessage.Put(rMaterial.GetColor()[1]);
        NewMessage.Put(rMaterial.GetColor()[2]);

        NewMessage.Put(rMaterial.GetTilingOffset()[0]);
        NewMessage.Put(rMaterial.GetTilingOffset()[1]);
        NewMessage.Put(rMaterial.GetTilingOffset()[2]);
        NewMessage.Put(rMaterial.GetTilingOffset()[3]);

        NewMessage.Put(rMaterial.GetRoughness());
        NewMessage.Put(rMaterial.GetReflectance());
        NewMessage.Put(rMaterial.GetMetalness());
        NewMessage.Put(rMaterial.GetDisplacement());

        if (rMaterial.GetColorTexture())
        {
            NewMessage.Put(true);

            NewMessage.PutString(rMaterial.GetColorTexture()->GetFileName().c_str());
        }
        else
        {
            NewMessage.Put(false);
        }

        if (rMaterial.GetNormalTexture())
        {
            NewMessage.Put(true);

            NewMessage.PutString(rMaterial.GetNormalTexture()->GetFileName().c_str());
        }
        else
        {
            NewMessage.Put(false);
        }

        if (rMaterial.GetRoughnessTexture())
        {
            NewMessage.Put(true);

            NewMessage.PutString(rMaterial.GetRoughnessTexture()->GetFileName().c_str());
        }
        else
        {
            NewMessage.Put(false);
        }

        if (rMaterial.GetMetalTexture())
        {
            NewMessage.Put(true);

            NewMessage.PutString(rMaterial.GetMetalTexture()->GetFileName().c_str());
        }
        else
        {
            NewMessage.Put(false);
        }

        if (rMaterial.GetBumpTexture())
        {
            NewMessage.Put(true);

            NewMessage.PutString(rMaterial.GetBumpTexture()->GetFileName().c_str());
        }
        else
        {
            NewMessage.Put(false);
        }

        if (rMaterial.GetAmbientOcclusionTexture())
        {
            NewMessage.Put(true);

            NewMessage.PutString(rMaterial.GetAmbientOcclusionTexture()->GetFileName().c_str());
        }
        else
        {
            NewMessage.Put(false);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnMaterialUpdate(Edit::CMessage& _rMessage)
    {
        unsigned int MaterialHash = _rMessage.Get<int>();

        Dt::CMaterial& rMaterial = Dt::MaterialManager::GetMaterialByHash(MaterialHash);

        float X, Y, Z, W;

        bool HasColorMap     = false;
        bool HasNormalMap    = false;
        bool HasRoughnessMap = false;
        bool HasMetalnessMap = false;
        bool HasBumpMap      = false;
        bool HasAOMap        = false;

        unsigned int ColorMapName = 0;
        unsigned int NormalMapName = 0;
        unsigned int RoughnessMapName = 0;
        unsigned int MetalMapName = 0;
        unsigned int BumpMapName = 0;
        unsigned int AOMapName = 0;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        X = _rMessage.Get<float>();
        Y = _rMessage.Get<float>();
        Z = _rMessage.Get<float>();

        glm::vec3 Color = glm::vec3(X, Y, Z);

        X = _rMessage.Get<float>();
        Y = _rMessage.Get<float>();
        Z = _rMessage.Get<float>();
        W = _rMessage.Get<float>();

        glm::vec4 TilingOffset = glm::vec4(X, Y, Z, W);

        float Roughness = _rMessage.Get<float>();

        float Reflectance = _rMessage.Get<float>();

        float Metalness = _rMessage.Get<float>();

        float Displacement = _rMessage.Get<float>();

        HasColorMap = _rMessage.Get<bool>();

        if (HasColorMap)
        {
            ColorMapName = _rMessage.Get<int>();
        }

        HasNormalMap = _rMessage.Get<bool>();

        if (HasNormalMap)
        {
            NormalMapName = _rMessage.Get<int>();
        }

        HasRoughnessMap = _rMessage.Get<bool>();

        if (HasRoughnessMap)
        {
            RoughnessMapName = _rMessage.Get<int>();
        }

        HasMetalnessMap = _rMessage.Get<bool>();

        if (HasMetalnessMap)
        {
            MetalMapName = _rMessage.Get<int>();
        }

        HasBumpMap = _rMessage.Get<bool>();

        if (HasBumpMap)
        {
            BumpMapName = _rMessage.Get<int>();
        }

        HasAOMap = _rMessage.Get<bool>();

        if (HasAOMap)
        {
            AOMapName = _rMessage.Get<int>();
        }

        rMaterial.SetColor       (Color);
        rMaterial.SetTilingOffset(TilingOffset);
        rMaterial.SetRoughness   (Roughness);
        rMaterial.SetReflectance (Reflectance);
        rMaterial.SetMetalness   (Metalness);
        rMaterial.SetDisplacement(Displacement);

        if (HasColorMap)
        {
            Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(ColorMapName);

            if (pTexture != nullptr)
            {
                rMaterial.SetColorTexture(pTexture);
            }
        }
        else
        {
            rMaterial.SetColorTexture(0);
        }

        if (HasNormalMap)
        {
            Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(NormalMapName);

            if (pTexture != nullptr)
            {
                rMaterial.SetNormalTexture(pTexture);
            }
        }
        else
        {
            rMaterial.SetNormalTexture(0);
        }

        if (HasRoughnessMap)
        {
            Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(RoughnessMapName);

            if (pTexture != nullptr)
            {
                rMaterial.SetRoughnessTexture(pTexture);
            }
        }
        else
        {
            rMaterial.SetRoughnessTexture(0);
        }

        if (HasMetalnessMap)
        {
            Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(MetalMapName);

            if (pTexture != nullptr)
            {
                rMaterial.SetMetalTexture(pTexture);
            }
        }
        else
        {
            rMaterial.SetMetalTexture(0);
        }

        if (HasBumpMap)
        {
            Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(BumpMapName);

            if (pTexture != nullptr)
            {
                rMaterial.SetBumpTexture(pTexture);
            }
        }
        else
        {
            rMaterial.SetBumpTexture(0);
        }

        if (HasAOMap)
        {
            Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(AOMapName);

            if (pTexture != nullptr)
            {
                rMaterial.SetAmbientOcclusionTexture(pTexture);
            }
        }
        else
        {
            rMaterial.SetAmbientOcclusionTexture(0);
        }

        Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyData | Dt::CMaterial::DirtyTexture);
    }


    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnDirtyMaterial(Dt::CMaterial* _pMaterial)
    {
        assert(_pMaterial != nullptr);

        if ((_pMaterial->GetDirtyFlags() & Dt::CMaterial::DirtyCreate) != 0)
        {
            
        }
    }
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