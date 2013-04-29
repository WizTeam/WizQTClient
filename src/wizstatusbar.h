#ifndef WIZSTATUSBAR_H
#define WIZSTATUSBAR_H

#include <QtGui>

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

class CWizExplorerApp;

class CWizStatusBar : public QLabel
{
    Q_OBJECT

public:
    explicit CWizStatusBar(CWizExplorerApp& app, QWidget *parent = 0);

    void autoShow(const QString& strMsg = QString());

private:
    CWizExplorerApp& m_app;
    QLabel m_label;
    QTimer m_timer;
};

#endif // WIZSTATUSBAR_H
