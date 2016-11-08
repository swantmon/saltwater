
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"

#include "editor/edit_map_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

namespace
{
    class CMapHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMapHelper)

    public:

        CMapHelper();
        ~CMapHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        Dt::CEntity* m_pLastRequestedEntity;

    private:

        void OnNewMap(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CMapHelper::CMapHelper()
        : m_pLastRequestedEntity(nullptr)
    {
        
    }

    // -----------------------------------------------------------------------------

    CMapHelper::~CMapHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMapHelper::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewMap, EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewMap));
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewMap(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        int MapX = _rMessage.GetInt();
        int MapY = _rMessage.GetInt();

        Dt::Map::AllocateMap(MapX, MapY);
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if ((_pEntity->GetDirtyFlags() & Dt::CEntity::DirtyAdd) == Dt::CEntity::DirtyAdd)
        {
            
        }
    }
} // namespace

namespace Edit
{
namespace Helper
{
namespace Map
{
    void OnStart()
    {
        CMapHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMapHelper::GetInstance().OnExit();
    }
} // namespace Map
} // namespace Helper
} // namespace Edit