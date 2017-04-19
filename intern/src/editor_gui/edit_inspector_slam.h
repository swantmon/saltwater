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
        void pauseIntegration();
        void pauseTracking();
        void volumeResolutionChanged(int);
        void volumeSizeChanged(int);

    private:

        bool m_IsIntegrationPaused;
        bool m_IsTrackingPaused;

        static const QString s_PauseIntegrationText;
        static const QString s_ResumeIntegrationText;
        
        static const QString s_PauseTrackingText;
        static const QString s_ResumeTrackingText;
    };
} // namespace Edit
