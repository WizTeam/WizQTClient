#ifndef WIZSTATUSBAR_H
#define WIZSTATUSBAR_H

#include <QLabel>
#include <QTimer>
#include <QPointer>
#include <QPropertyAnimation>

class WizExplorerApp;

class WizStatusBar : public QLabel
{
    Q_OBJECT

public:
    explicit WizStatusBar(WizExplorerApp& app, QWidget *parent = 0);

    void adjustPosition();
    void showText(const QString& strText = QString());

private:
    WizExplorerApp& m_app;

    bool isCursorInside();

};

#endif // WIZSTATUSBAR_H
