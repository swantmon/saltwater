#pragma once

#include <QListView>

namespace Edit
{
    class CSceneGraph : public QListView 
    {
	    Q_OBJECT

    public:
	    CSceneGraph(QWidget* _pParent = Q_NULLPTR);
	    ~CSceneGraph();
    };
} // namespace Edit
