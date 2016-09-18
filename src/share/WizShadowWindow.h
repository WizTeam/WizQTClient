#ifndef WIZSHADOWWINDOW_H
#define WIZSHADOWWINDOW_H



#include <QWidget>
#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>
#include <QDebug>
#include <QBoxLayout>
#include <QTimer>
#include "WizWindowTitleBar.h"
#include "WizShadowEffect.h"

template <class Base>
class WizShadowWindow
        : public Base
{
public:
    explicit WizShadowWindow(QWidget *parent, bool canResize)
        : Base(parent)
        , m_shadowWidget(NULL)
        , m_clientWidget(NULL)
        , m_clientLayout(NULL)
    {
        Base* pT = this;
        //
        pT->setAttribute(Qt::WA_TranslucentBackground); //enable MainWindow to be transparent
        pT->setWindowFlags(Qt::FramelessWindowHint);
        pT->setContentsMargins(0, 0, 0, 0);
        //
        QLayout* windowLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        pT->setLayout(windowLayout);
        windowLayout->setContentsMargins(0, 0, 0, 0);
        windowLayout->setSpacing(0);
        //
        int shadowSize = WizSmartScaleUI(20);
        m_shadowWidget = new WizShadowWidget(this, shadowSize, canResize);
        m_shadowWidget->setContentsMargins(shadowSize, shadowSize, shadowSize, shadowSize);
        windowLayout->addWidget(m_shadowWidget);
        //
        QLayout* rootLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_shadowWidget->setLayout(rootLayout);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        QWidget* shadowClientWidget = new QWidget(m_shadowWidget);
        rootLayout->addWidget(shadowClientWidget);
        //
        QLayout* shadowClientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        shadowClientLayout->setContentsMargins(0, 0, 0, 0);
        shadowClientLayout->setSpacing(0);
        shadowClientWidget->setLayout(shadowClientLayout);
        shadowClientWidget->setAutoFillBackground(true);
        shadowClientWidget->setCursor(QCursor(Qt::ArrowCursor));
        //
        m_titleBar = new WizWindowTitleBar(shadowClientWidget, this, m_shadowWidget, canResize);
        shadowClientLayout->addWidget(m_titleBar);
        //
        m_clientWidget = new QWidget(shadowClientWidget);
        shadowClientLayout->addWidget(m_clientWidget);

        //
        m_clientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_clientWidget->setLayout(m_clientLayout);
        //
        m_clientLayout->setSpacing(0);
        m_clientLayout->setContentsMargins(0, 0, 0, 0);
    }
public:
    QWidget* rootWidget() const { return m_shadowWidget; }
    QWidget *clientWidget() const { return m_clientWidget; }
    QLayout* clientLayout() const { return m_clientLayout; }
    WizWindowTitleBar* titleBar() const { return m_titleBar; }
    void setTitleText(QString title) { m_titleBar->setText(title); }
private:
    WizShadowWidget* m_shadowWidget;
    QWidget* m_clientWidget;
    QLayout* m_clientLayout;
    WizWindowTitleBar* m_titleBar;
protected:
    virtual void changeEvent ( QEvent * event )
    {
        if (event->type() == QEvent::WindowStateChange)
        {
            m_titleBar->windowStateChanged();
        }
        //
        Base::changeEvent(event);
    }
    //
    virtual void layoutTitleBar()
    {
        m_titleBar->layoutTitleBar();
    }
};

#endif // WIZSHADOWWINDOW_H
