
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

        // -----------------------------------------------------------------------------
        // Perform the default drag & drop implementation
        // -----------------------------------------------------------------------------
        QTreeWidget::dropEvent(_pEvent);

        // -----------------------------------------------------------------------------
        // Destination (this could be only one single element)
        // -----------------------------------------------------------------------------
        int EntityIDDestination = -1;

        QTreeWidgetItem* pDestination = SelectedItems.at(0)->parent();

        if (pDestination != Q_NULLPTR)
        {
            EntityIDDestination = pDestination->text(1).toInt();
        }

        // -----------------------------------------------------------------------------
        // Move all selected items
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfSelectedItem = 0; IndexOfSelectedItem < SelectedItems.length(); ++IndexOfSelectedItem)
        {
            QTreeWidgetItem* pSource = SelectedItems.at(IndexOfSelectedItem);

            int EntityIDSource = -1;
            
            if (pSource == Q_NULLPTR)
            {
                _pEvent->setDropAction(Qt::IgnoreAction);

                continue;
            }

            EntityIDSource = pSource->text(1).toInt();

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
    }

    // -----------------------------------------------------------------------------

    void CSceneGraph::keyReleaseEvent(QKeyEvent* _pEvent)
    {
        if (_pEvent->key() == Qt::Key_Delete)
        {
            QList<QTreeWidgetItem*> SelectedItems = this->selectedItems();

            if (SelectedItems.size() == 0) return;

            for (unsigned int IndexOfSelectedItem = 0; IndexOfSelectedItem < SelectedItems.length(); ++IndexOfSelectedItem)
            {
                QTreeWidgetItem* pSource = SelectedItems.at(IndexOfSelectedItem);

                int EntityID = pSource->text(1).toInt();

                Edit::CMessage NewMessage;

                NewMessage.PutInt(EntityID);

                NewMessage.Reset();

                int Result = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::RemoveEntity, NewMessage);

                if (Result == 100)
                {
                    delete pSource;
                }
            }
        }
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

        // -----------------------------------------------------------------------------
        // Hierarchy
        // -----------------------------------------------------------------------------
        bool HasHierachy = _rMessage.GetBool();

        if (HasHierachy)
        {
            bool HasParent = _rMessage.GetBool();

            if (HasParent)
            {
                int ParentID = _rMessage.GetInt();

                QList<QTreeWidgetItem*> ListOfEntities = findItems(QString::number(ParentID), Qt::MatchContains | Qt::MatchRecursive, 1);

                if (ListOfEntities.size() > 0)
                {
                    ListOfEntities[0]->addChild(pNewItem);
                }
                else
                {
                    addTopLevelItem(pNewItem);
                }
            }
            else
            {
                addTopLevelItem(pNewItem);
            }
        }
        else
        {
            addTopLevelItem(pNewItem);
        }

        // -----------------------------------------------------------------------------
        // hide columns
        // -----------------------------------------------------------------------------
        header()->hideSection(1);
    }
} // namespace Edit