
#include "data/data_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_manager.h"

#include "data/data_component_manager.h"
#include "data/data_material_component.h"
#include "data/data_material_helper.h"

#include <unordered_map>
#include <vector>

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

        CMaterialComponent* CreateMaterialFromFile(const std::string& _rFilename);

        const CMaterialComponent* GetDefaultMaterial() const;

        
    private:

        typedef std::unordered_map<Base::BHash, CMaterialComponent*> CMaterialByHashs;
        
    private:
        
        CMaterialByHashs m_MaterialByHashs;

        CMaterialComponent* m_pDefaultMaterial;
    };
} // namespace

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_MaterialByHashs ()
        , m_pDefaultMaterial(0)
    {
        m_pDefaultMaterial = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMaterialManager::~CDtMaterialManager()
    {
        m_MaterialByHashs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterialComponent* CDtMaterialManager::CreateMaterialFromFile(const std::string& _rFilename)
    {
        if (_rFilename.length() == 0)
        {
            BASE_THROWM("No material filename was given to load material.")
        }

        // -----------------------------------------------------------------------------
        // Create hash value
        // -----------------------------------------------------------------------------
        int NumberOfBytes = static_cast<unsigned int>(_rFilename.length() * sizeof(char));
        const void* pData = static_cast<const void*>(_rFilename.c_str());

        Base::BHash Hash = Base::CRC32(pData, NumberOfBytes);

        if (m_MaterialByHashs.find(Hash) != m_MaterialByHashs.end())
        {
            return m_MaterialByHashs.at(Hash);
        }

        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();
        
        pComponent->SetFileName(_rFilename);

        m_MaterialByHashs[Hash] = pComponent;

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
    CMaterialComponent* CreateMaterialFromFile(const std::string& _rFilename)
    {
        return CDtMaterialManager::GetInstance().CreateMaterialFromFile(_rFilename);
    }

    // -----------------------------------------------------------------------------

    const CMaterialComponent* GetDefaultMaterial()
    {
        return CDtMaterialManager::GetInstance().GetDefaultMaterial();
    }
} // namespace MaterialHelper
} // namespace Dt