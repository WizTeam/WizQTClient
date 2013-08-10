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

protected:
    virtual void enterEvent(QEvent* event);

private:
    CWizExplorerApp& m_app;
    QPointer<QPropertyAnimation> m_animation;
    QTimer m_animationTimer;
    QTimer m_hideTimer;
    QLabel m_label;
    bool m_bIsVisible;

    bool isCursorInside();

private Q_SLOTS:
    void on_animation_finished();
    void on_animationTimer_timeout();
    void on_hideTimer_timeout();
};

#endif // WIZSTATUSBAR_H
