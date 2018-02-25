
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_component_manager.h"
#include "data/data_material_helper.h"
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
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnLoadMaterial(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        std::string PathToFile = _rMessage.Get<std::string>();

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

        // -----------------------------------------------------------------------------
        // Set result as hash
        // -----------------------------------------------------------------------------
        _rMessage.SetResult(pComponent->GetID());
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnRequestMaterialInfo(Edit::CMessage& _rMessage)
    {
        Base::ID MaterialHash = _rMessage.Get<int>();

        auto pMaterial = Dt::CComponentManager::GetInstance().GetComponent<Dt::CMaterialComponent>(MaterialHash);

        Edit::CMessage NewMessage;

        NewMessage.Put(pMaterial->GetID());

        NewMessage.Put(pMaterial->GetColor()[0]);
        NewMessage.Put(pMaterial->GetColor()[1]);
        NewMessage.Put(pMaterial->GetColor()[2]);

        NewMessage.Put(pMaterial->GetTilingOffset()[0]);
        NewMessage.Put(pMaterial->GetTilingOffset()[1]);
        NewMessage.Put(pMaterial->GetTilingOffset()[2]);
        NewMessage.Put(pMaterial->GetTilingOffset()[3]);

        NewMessage.Put(pMaterial->GetRoughness());
        NewMessage.Put(pMaterial->GetReflectance());
        NewMessage.Put(pMaterial->GetMetalness());
        NewMessage.Put(pMaterial->GetDisplacement());

        NewMessage.Put(true);

        NewMessage.Put(pMaterial->GetColorTexture());

        NewMessage.Put(true);

        NewMessage.Put(pMaterial->GetNormalTexture());

        NewMessage.Put(true);

        NewMessage.Put(pMaterial->GetRoughnessTexture());

        NewMessage.Put(true);

        NewMessage.Put(pMaterial->GetMetalTexture());

        NewMessage.Put(true);

        NewMessage.Put(pMaterial->GetBumpTexture());

        NewMessage.Put(true);

        NewMessage.Put(pMaterial->GetAmbientOcclusionTexture());

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnMaterialUpdate(Edit::CMessage& _rMessage)
    {
        Base::ID MaterialHash = _rMessage.Get<int>();

        auto pMaterial = Dt::CComponentManager::GetInstance().GetComponent<Dt::CMaterialComponent>(MaterialHash);

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
            ColorMapName = _rMessage.Get<unsigned int>();
        }

        HasNormalMap = _rMessage.Get<bool>();

        if (HasNormalMap)
        {
            NormalMapName = _rMessage.Get<unsigned int>();
        }

        HasRoughnessMap = _rMessage.Get<bool>();

        if (HasRoughnessMap)
        {
            RoughnessMapName = _rMessage.Get<unsigned int>();
        }

        HasMetalnessMap = _rMessage.Get<bool>();

        if (HasMetalnessMap)
        {
            MetalMapName = _rMessage.Get<unsigned int>();
        }

        HasBumpMap = _rMessage.Get<bool>();

        if (HasBumpMap)
        {
            BumpMapName = _rMessage.Get<unsigned int>();
        }

        HasAOMap = _rMessage.Get<bool>();

        if (HasAOMap)
        {
            AOMapName = _rMessage.Get<unsigned int>();
        }

        pMaterial->SetColor       (Color);
        pMaterial->SetTilingOffset(TilingOffset);
        pMaterial->SetRoughness   (Roughness);
        pMaterial->SetReflectance (Reflectance);
        pMaterial->SetMetalness   (Metalness);
        pMaterial->SetDisplacement(Displacement);

        /*
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
        */

        Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pMaterial, Dt::CMaterialComponent::DirtyInfo);
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