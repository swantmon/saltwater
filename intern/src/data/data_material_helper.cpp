
#include "data/data_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_importer.h"
#include "core/core_asset_manager.h"

#include "data/data_component.h"
#include "data/data_material_component.h"
#include "data/data_material_helper.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "tinyxml2.h"

#include <unordered_map>

using namespace Dt;
using namespace Dt::MaterialHelper;

namespace
{
    class CDtMaterialManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtMaterialManager)
        
    public:
        
        CDtMaterialManager();
        ~CDtMaterialManager();
        
    public:

        CMaterialComponent* CreateMaterial(const Core::MaterialImporter::SMaterialDescriptor _rDescriptor) const;

        const CMaterialComponent* GetDefaultMaterial() const;
        
    private:

        CMaterialComponent* m_pDefaultMaterial;
    };
} // namespace

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_pDefaultMaterial(0)
    {
        m_pDefaultMaterial = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMaterialManager::~CDtMaterialManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterialComponent* CDtMaterialManager::CreateMaterial(const Core::MaterialImporter::SMaterialDescriptor _rDescriptor) const
    {
        // -----------------------------------------------------------------------------
        // Create hash value
        // -----------------------------------------------------------------------------
        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();

        pComponent->SetMaterialname(_rDescriptor.m_MaterialName);
        pComponent->SetColorTexture(_rDescriptor.m_ColorTexture);
        pComponent->SetNormalTexture(_rDescriptor.m_NormalTexture);
        pComponent->SetRoughnessTexture(_rDescriptor.m_RoughnessTexture);
        pComponent->SetMetalTexture(_rDescriptor.m_MetalTexture);
        pComponent->SetAmbientOcclusionTexture(_rDescriptor.m_AmbientOcclusionTexture);
        pComponent->SetBumpTexture(_rDescriptor.m_BumpTexture);
        pComponent->SetRoughness(_rDescriptor.m_Roughness);
        pComponent->SetReflectance(_rDescriptor.m_Reflectance);
        pComponent->SetMetalness(_rDescriptor.m_MetalMask);
        pComponent->SetDisplacement(_rDescriptor.m_Displacement);
        pComponent->SetColor(_rDescriptor.m_AlbedoColor);
        pComponent->SetTilingOffset(_rDescriptor.m_TilingOffset);

        Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, CMaterialComponent::DirtyCreate);

        return pComponent;
    }

    // -----------------------------------------------------------------------------

    const CMaterialComponent* CDtMaterialManager::GetDefaultMaterial() const
    {
        return m_pDefaultMaterial;
    }
} // namespace

namespace Dt
{
namespace MaterialHelper
{
    CMaterialComponent* CreateMaterial(const Core::MaterialImporter::SMaterialDescriptor _rDescriptor)
    {
        return CDtMaterialManager::GetInstance().CreateMaterial(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    const CMaterialComponent* GetDefaultMaterial()
    {
        return CDtMaterialManager::GetInstance().GetDefaultMaterial();
    }
} // namespace MaterialHelper
} // namespace Dt