
#include "editor/edit_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_facet.h"
#include "data/data_light_manager.h"
#include "data/data_fx_facet.h"
#include "data/data_fx_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_model_manager.h"
#include "data/data_plugin_facet.h"
#include "data/data_plugin_manager.h"
#include "data/data_script_facet.h"
#include "data/data_script_manager.h"
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_map_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace
{
    class CMapHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMapHelper)

    public:

        CMapHelper();
        ~CMapHelper();

    public:

        void Refresh();

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CMapHelper::CMapHelper()
    {
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMapHelper::OnDirtyEntity));
    }

    // -----------------------------------------------------------------------------

    CMapHelper::~CMapHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CMapHelper::Refresh()
    {

    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        Edit::CMessage NewMessage;

        Dt::CEntity& rCurrentEntity = *_pEntity;

        NewMessage.PutInt(rCurrentEntity.GetID());
        NewMessage.PutInt(rCurrentEntity.GetCategory());

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::SceneGraphChanged, NewMessage);
    }
} // namespace

namespace Edit
{
namespace Helper
{
namespace Map
{
    void Refresh()
    {
        CMapHelper::GetInstance().Refresh();
    }
} // namespace Map
} // namespace Helper
} // namespace Edit