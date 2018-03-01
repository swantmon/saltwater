
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_manager.h"

#include "data/data_component.h"
#include "data/data_material_component.h"
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
        std::string PathToMaterial = Core::AssetManager::GetPathToAssets() + "/" + PathToFile;

        auto pComponent = Dt::MaterialManager::CreateMaterialFromXML(PathToMaterial);

        // -----------------------------------------------------------------------------
        // Set result as ID
        // -----------------------------------------------------------------------------
        if (pComponent != nullptr)
        {
            _rMessage.SetResult(static_cast<int>(pComponent->GetHash()));
        }
        else
        {
            _rMessage.SetResult(-1);
        }
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnRequestMaterialInfo(Edit::CMessage& _rMessage)
    {
        Base::BHash MaterialHash = _rMessage.Get<Base::BHash>();

        auto pMaterial = Dt::MaterialManager::GetMaterialByHash(MaterialHash);

        Edit::CMessage NewMessage;

        NewMessage.Put(pMaterial->GetHash());

        NewMessage.Put(pMaterial->GetColor());
        NewMessage.Put(pMaterial->GetTilingOffset());

        NewMessage.Put(pMaterial->GetRoughness());
        NewMessage.Put(pMaterial->GetReflectance());
        NewMessage.Put(pMaterial->GetMetalness());
        NewMessage.Put(pMaterial->GetDisplacement());

        NewMessage.Put(pMaterial->GetColorTexture());
        NewMessage.Put(pMaterial->GetNormalTexture());
        NewMessage.Put(pMaterial->GetRoughnessTexture());
        NewMessage.Put(pMaterial->GetMetalTexture());
        NewMessage.Put(pMaterial->GetBumpTexture());
        NewMessage.Put(pMaterial->GetAmbientOcclusionTexture());

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CMaterialHelper::OnMaterialUpdate(Edit::CMessage& _rMessage)
    {
        Base::BHash MaterialHash = _rMessage.Get<Base::BHash>();

        auto pMaterial = Dt::MaterialManager::GetMaterialByHash(MaterialHash);

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        glm::vec3 Color = _rMessage.Get<glm::vec3>();

        glm::vec4 TilingOffset = _rMessage.Get<glm::vec4>();

        float Roughness    = _rMessage.Get<float>();
        float Reflectance  = _rMessage.Get<float>();
        float Metalness    = _rMessage.Get<float>();
        float Displacement = _rMessage.Get<float>();

        std::string ColorMapName     = _rMessage.Get<std::string>();
        std::string NormalMapName    = _rMessage.Get<std::string>();
        std::string RoughnessMapName = _rMessage.Get<std::string>();
        std::string MetalMapName     = _rMessage.Get<std::string>();
        std::string BumpMapName      = _rMessage.Get<std::string>();
        std::string AOMapName        = _rMessage.Get<std::string>();
 
        pMaterial->SetColor(Color);
        pMaterial->SetTilingOffset(TilingOffset);
        pMaterial->SetRoughness(Roughness);
        pMaterial->SetReflectance(Reflectance);
        pMaterial->SetMetalness(Metalness);
        pMaterial->SetDisplacement(Displacement);

        pMaterial->SetColorTexture(ColorMapName);
        pMaterial->SetNormalTexture(NormalMapName);
        pMaterial->SetRoughnessTexture(RoughnessMapName);
        pMaterial->SetMetalTexture(MetalMapName);
        pMaterial->SetBumpTexture(BumpMapName);
        pMaterial->SetAmbientOcclusionTexture(AOMapName);

        // -----------------------------------------------------------------------------
        // Mark all material components related to this material as dirty
        // -----------------------------------------------------------------------------
        auto MaterialComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CMaterialComponent>();

        for (auto Component : MaterialComponents)
        {
            auto MaterialComponent = static_cast<Dt::CMaterialComponent*>(Component);

            if (MaterialComponent->GetMaterial()->GetHash() == MaterialHash)
            {
                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(MaterialComponent, Dt::CMaterialComponent::DirtyInfo);
            }
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