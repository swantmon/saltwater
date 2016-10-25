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

    private:

        void OnSceneGraphChanged(Edit::CMessage& _rMessage);
    };
} // namespace Edit
