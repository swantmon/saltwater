
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
        QTreeWidgetItem* pNewItem = new QTreeWidgetItem();

        // -----------------------------------------------------------------------------
        // ID
        // -----------------------------------------------------------------------------
        pNewItem->setText(0, QString(_rMessage.GetInt()));

        // -----------------------------------------------------------------------------
        // Name
        // -----------------------------------------------------------------------------
        pNewItem->setText(1, "New entity");

        // -----------------------------------------------------------------------------
        // Category
        // -----------------------------------------------------------------------------
        pNewItem->setText(2, QString(_rMessage.GetInt()));

        addTopLevelItem(pNewItem);
    }
} // namespace Edit