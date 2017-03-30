#pragma once

#include "editor_gui/ui_edit_inspector_slam.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorSLAM : public QWidget, public Ui::InspectorSLAM
    {
	    Q_OBJECT

    public:
        CInspectorSLAM(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorSLAM();

    public Q_SLOTS:

        void reset();
        void pause();
        void volumeResolutionChanged(int);

    private:

        bool m_IsPaused;

        static const QString s_PauseText;
        static const QString s_ResumeText;
    };
} // namespace Edit
