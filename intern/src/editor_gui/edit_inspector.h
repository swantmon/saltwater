#pragma once

#include <QWidget>

namespace Edit
{
    class CInspector : public QWidget
    {
        Q_OBJECT

    public:

        CInspector(QWidget* _pParent = Q_NULLPTR);
        ~CInspector();
    };
} // namespace Edit