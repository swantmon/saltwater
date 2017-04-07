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
        void pauseDepthIntegration();
        void pauseColorIntegration();
        void pauseTracking();
        void volumeResolutionChanged(int);

    private:

        bool m_IsDepthPaused;
        bool m_IsColorPaused;
        bool m_IsTrackingPaused;

        static const QString s_PauseDepthText;
        static const QString s_ResumeDepthText;

        static const QString s_PauseColorText;
        static const QString s_ResumeColorText;

        static const QString s_PauseTrackingText;
        static const QString s_ResumeTrackingText;
    };
} // namespace Edit
