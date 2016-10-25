#pragma once

#include <QTreeWidget>

namespace Edit
{
    class CSceneGraph : public QTreeWidget
    {
	    Q_OBJECT

    public:
	    CSceneGraph(QWidget* _pParent = Q_NULLPTR);
	    ~CSceneGraph();
    };
} // namespace Edit
