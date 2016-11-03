
#include "editor_gui/edit_scenegraph.h"

#include <QDropEvent>
#include <QHeaderView>

namespace Edit
{
    CSceneGraph::CSceneGraph(QWidget* _pParent)
        : QTreeWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Special GUI behavior
        // -----------------------------------------------------------------------------
        header()->hideSection(1);

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

    void CSceneGraph::itemSelected(QTreeWidgetItem* _pItem)
    {
        int EntityID = _pItem->text(1).toInt();

        emit entitySelected(EntityID);
    }

    // -----------------------------------------------------------------------------

    void CSceneGraph::dropEvent(QDropEvent* _pEvent)
    {
        QList<QTreeWidgetItem*> SelectedItems = this->selectedItems();

        if (SelectedItems.size() == 0) return;

        QTreeWidgetItem* pSource = SelectedItems.at(0);

        // -----------------------------------------------------------------------------
        // Perform the default drag & drop implementation
        // -----------------------------------------------------------------------------
        QTreeWidget::dropEvent(_pEvent);

        QTreeWidgetItem* pDestination = SelectedItems.at(0)->parent();

        int EntityIDSource      = -1;
        int EntityIDDestination = -1;

        if (pSource == Q_NULLPTR)
        {
            _pEvent->setDropAction(Qt::IgnoreAction);

            return;
        }

        EntityIDSource = pSource->text(1).toInt();

        if (pDestination != Q_NULLPTR)
        {
            EntityIDDestination = pDestination->text(1).toInt();
        }

        Edit::CMessage NewMessage;

        NewMessage.PutInt(EntityIDSource);
        NewMessage.PutInt(EntityIDDestination);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoHierarchie, NewMessage);

        // -----------------------------------------------------------------------------
        // Emit signal
        // -----------------------------------------------------------------------------
        emit childDragedAndDroped(pSource, pDestination);
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
        pNewItem->setText(1, QString::number(_rMessage.GetInt()));

        // -----------------------------------------------------------------------------
        // Name
        // -----------------------------------------------------------------------------
        pNewItem->setText(0, "Entity (" + QString(GetCategoryName(_rMessage.GetInt())) + ")");

        addTopLevelItem(pNewItem);

        // -----------------------------------------------------------------------------
        // hide columns
        // -----------------------------------------------------------------------------
        header()->hideSection(1);
    }
} // namespace Edit