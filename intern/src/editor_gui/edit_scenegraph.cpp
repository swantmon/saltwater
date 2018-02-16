
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Entity_Hierarchy_Info, EDIT_RECEIVE_MESSAGE(&CSceneGraph::OnSceneGraphChanged));
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Entity_Selected, EDIT_RECEIVE_MESSAGE(&CSceneGraph::OnEntitySelected));
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

    void CSceneGraph::changeNameOfSelectedItem(QString _Name)
    {
        QList<QTreeWidgetItem*> SelectedItems = this->selectedItems();

        if (SelectedItems.size() == 0) return;

        SelectedItems.at(0)->setText(0, _Name);
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
        Base::ID EntityIDDestination = static_cast<Base::ID>(-1);

        QTreeWidgetItem* pDestination = SelectedItems.at(0)->parent();

        if (pDestination != Q_NULLPTR)
        {
            EntityIDDestination = pDestination->text(1).toInt();
        }

        // -----------------------------------------------------------------------------
        // Move all selected items
        // -----------------------------------------------------------------------------
        for (int IndexOfSelectedItem = 0; IndexOfSelectedItem < SelectedItems.length(); ++IndexOfSelectedItem)
        {
            QTreeWidgetItem* pSource = SelectedItems.at(IndexOfSelectedItem);

            Base::ID EntityIDSource = static_cast<Base::ID>(-1);
            
            if (pSource == Q_NULLPTR)
            {
                _pEvent->setDropAction(Qt::IgnoreAction);

                continue;
            }

            EntityIDSource = pSource->text(1).toInt();

            Edit::CMessage NewMessage;

            NewMessage.Put(EntityIDSource);
            NewMessage.Put(EntityIDDestination);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Entity_Hierarchy_Update, NewMessage);

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

            for (int IndexOfSelectedItem = 0; IndexOfSelectedItem < SelectedItems.length(); ++IndexOfSelectedItem)
            {
                QTreeWidgetItem* pSource = SelectedItems.at(IndexOfSelectedItem);

                Base::ID EntityID = pSource->text(1).toInt();

                Edit::CMessage NewMessage;

                NewMessage.Put(EntityID);

                NewMessage.Reset();

                int Result = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Entity_Destroy, NewMessage);

                if (Result == 1)
                {
                    delete pSource;
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CSceneGraph::OnSceneGraphChanged(Edit::CMessage& _rMessage)
    {
        QTreeWidgetItem* pNewItem = new QTreeWidgetItem();

        // -----------------------------------------------------------------------------
        // ID
        // -----------------------------------------------------------------------------
        pNewItem->setText(1, QString::number(_rMessage.Get<Base::ID>()));

        // -----------------------------------------------------------------------------
        // Name
        // -----------------------------------------------------------------------------
        bool HasName = _rMessage.Get<bool>();

        if (HasName)
        {
            char pEntityName[256];

            _rMessage.GetString(pEntityName, 256);

            pNewItem->setText(0, QString(pEntityName));
        }
        else
        {
            pNewItem->setText(0, "Unnamed entity");
        }

        // -----------------------------------------------------------------------------
        // Hierarchy
        // -----------------------------------------------------------------------------
        bool HasHierachy = _rMessage.Get<bool>();

        if (HasHierachy)
        {
            bool HasParent = _rMessage.Get<bool>();

            if (HasParent)
            {
                int ParentID = _rMessage.Get<Base::ID>();

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

    // -----------------------------------------------------------------------------

    void CSceneGraph::OnEntitySelected(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        QList<QTreeWidgetItem*> ListOfEntities = findItems(QString::number(EntityID), Qt::MatchExactly | Qt::MatchRecursive, 1);

        foreach(QTreeWidgetItem* pEntity, ListOfEntities)
        {
            setCurrentItem(pEntity);

            itemSelected(pEntity);
        }
    }
} // namespace Edit