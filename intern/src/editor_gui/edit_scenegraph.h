#pragma once

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QTreeWidget>

namespace Edit
{
    class CSceneGraph : public QTreeWidget
    {
	    Q_OBJECT

    public:

	    CSceneGraph(QWidget* _pParent = Q_NULLPTR);
	    ~CSceneGraph();

    public Q_SLOTS:

        void entitySelected(QTreeWidgetItem* _pItem);

    private:

        void OnSceneGraphChanged(Edit::CMessage& _rMessage);
    };
} // namespace Edit
