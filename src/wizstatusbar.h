#ifndef WIZSTATUSBAR_H
#define WIZSTATUSBAR_H

#include <QLabel>
#include <QTimer>
#include <QPointer>
#include <QPropertyAnimation>

class CWizExplorerApp;

class CWizStatusBar : public QLabel
{
    Q_OBJECT

public:
    explicit CWizStatusBar(CWizExplorerApp& app, QWidget *parent = 0);

    void adjustPosition();
    void showText(const QString& strText = QString());

private:
    CWizExplorerApp& m_app;

    bool isCursorInside();

};

#endif // WIZSTATUSBAR_H
