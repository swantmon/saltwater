
#include "edit_scenegraph.h"

namespace Edit
{
    CSceneGraph::CSceneGraph(QWidget* _pParent)
        : QTreeWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::SceneGraphChanged, EDIT_RECEIVE_MESSAGE(&CSceneGraph::OnSceneGraphChanged));
    }

    // -----------------------------------------------------------------------------

    CSceneGraph::~CSceneGraph() 
    {
	
    }

    // -----------------------------------------------------------------------------

    void CSceneGraph::OnSceneGraphChanged(Edit::CMessage& _rMessage)
    {
        auto GetCategoryName = [&](unsigned int _Category)->const char*
        {
            const char* pCategoryStrings[]
            {
                "Actor" ,
                "Light" ,
                "FX"    ,
                "Plugin",
            };

            return pCategoryStrings[_Category];
        };


        QTreeWidgetItem* pNewItem = new QTreeWidgetItem();

        // -----------------------------------------------------------------------------
        // ID
        // -----------------------------------------------------------------------------
        pNewItem->setText(0, QString::number(_rMessage.GetInt()));

        // -----------------------------------------------------------------------------
        // Name
        // -----------------------------------------------------------------------------
        pNewItem->setText(1, "New entity");

        // -----------------------------------------------------------------------------
        // Category
        // -----------------------------------------------------------------------------
        pNewItem->setText(2, QString(GetCategoryName(_rMessage.GetInt())));

        addTopLevelItem(pNewItem);
    }

    // -----------------------------------------------------------------------------

    void CSceneGraph::entitySelected(QTreeWidgetItem* _pItem)
    {
        int EntityID = _pItem->text(0).toInt();

        CMessage NewMessage;

        NewMessage.PutInt(EntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoTransformation, NewMessage);
    }
} // namespace Edit