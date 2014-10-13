#ifndef WIZSHADOWWINDOW_H
#define WIZSHADOWWINDOW_H



#include <QWidget>
#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>
#include <QDebug>
#include <QBoxLayout>
#include <QTimer>
#include "wiztitlebar.h"
#include "wizshadoweffect.h"

template <class Base>
class CWizShadowWindow
        : public Base
{
public:
    explicit CWizShadowWindow(QWidget *parent = 0)
        : Base(parent)
        , m_oldHitCode(wizClient)
        , m_mousePressed(false)
        , m_rootWidget(NULL)
        , m_shadowWidget(NULL)
        , m_clientWidget(NULL)
        , m_clientLayout(NULL)
        , m_canResize(true)
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
        m_rootWidget = new CWizShadowWidget(this);
        m_rootWidget->setContentsMargins(10, 10, 10, 10);
        windowLayout->addWidget(m_rootWidget);
        //
        QLayout* rootLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_rootWidget->setLayout(rootLayout);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        m_shadowWidget = new QWidget(m_rootWidget);
        rootLayout->addWidget(m_shadowWidget);
        //
        QLayout* shadowLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        shadowLayout->setContentsMargins(0, 0, 0, 0);
        shadowLayout->setSpacing(0);
        m_shadowWidget->setLayout(shadowLayout);
        m_shadowWidget->setAutoFillBackground(true);
        //don't use QGraphicsEffect
        //CWizShadowEffect* effect = new CWizShadowEffect();
        //m_shadowWidget->setGraphicsEffect(effect);
        m_shadowWidget->setCursor(QCursor(Qt::ArrowCursor));
        //
        m_titleBar = new CWizTitleBar(m_shadowWidget, this, m_rootWidget);
        shadowLayout->addWidget(m_titleBar);
        //
        m_clientWidget = new QWidget(m_shadowWidget);
        shadowLayout->addWidget(m_clientWidget);

        //
        m_clientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_clientWidget->setLayout(m_clientLayout);
        //
        m_clientLayout->setSpacing(0);
        m_clientLayout->setContentsMargins(0, 0, 0, 0);
        //
        //
        pT->setMouseTracking(true);
    }
public:
    QWidget* rootWidget() const { return m_rootWidget; }
    QWidget *clientWidget() const { return m_clientWidget; }
    QLayout* clientLayout() const { return m_clientLayout; }
    CWizTitleBar* titleBar() const { return m_titleBar; }
    bool canResize() const { return m_canResize; }
    void setCanResize(bool b) { m_canResize = b; m_titleBar->setCanResize(b); }
    void setTitleText(QString title) { m_titleBar->setText(title); }
protected:
    enum WizWindowHitTestResult {wizTopLeft, wizTop, wizTopRight, wizLeft, wizClient, wizRight, wizBottomLeft, wizBottom, wizBottomRight};
private:
    WizWindowHitTestResult m_oldHitCode;
    QPoint m_oldPressPos;
    QRect m_oldGeometry;
    bool m_mousePressed;
    QWidget* m_rootWidget;
    QWidget* m_shadowWidget;
    QWidget* m_clientWidget;
    QLayout* m_clientLayout;
    CWizTitleBar* m_titleBar;
    bool m_canResize;
protected:
    virtual WizWindowHitTestResult hitTest(const QPoint& posOfWindow)
    {
        if (!m_canResize)
            return wizClient;
        //
        Base* pT = this;
        QPoint globalPos = pT->mapToGlobal(posOfWindow);
        QPoint pos = m_shadowWidget->mapFromGlobal(globalPos);
        if (pos.x() < 0)
        {
            if (pos.y() < 0)
            {
                return wizTopLeft;
            }
            else if (pos.y() >= m_shadowWidget->height())
            {
                return wizBottomLeft;
            }
            else
            {
                return wizLeft;
            }
        }
        else if (pos.x() > m_shadowWidget->width())
        {
            if (pos.y() < 0)
            {
                return wizTopRight;
            }
            else if (pos.y() >= m_shadowWidget->height())
            {
                return wizBottomRight;
            }
            else
            {
                return wizRight;
            }
        }
        else if (pos.y() < 0)
        {
            return wizTop;
        }
        else if (pos.y() > m_shadowWidget->height())
        {
            return wizBottom;
        }
        else
        {
            return wizClient;
        }
    }

    virtual void mouseMoveEvent(QMouseEvent *event)
    {
        Base* pT = this;
        //
        if (m_mousePressed)
        {
            if (m_oldHitCode == wizClient)
                return;
            //
            QPoint pos = event->globalPos();
            int offsetX = pos.x() - m_oldPressPos.x();
            int offsetY = pos.y() - m_oldPressPos.y();
            //
            QRect rc = m_oldGeometry;
            //
            switch (m_oldHitCode)
            {
            case wizTopLeft:
                rc.adjust(offsetX, offsetY, 0, 0);
                break;
            case wizTop:
                rc.adjust(0, offsetY, 0, 0);
                break;
            case wizTopRight:
                rc.adjust(0, offsetY, offsetX, 0);
                break;
            case wizLeft:
                rc.adjust(offsetX, 0, 0, 0);
                break;
            case wizRight:
                rc.adjust(0, 0, offsetX, 0);
                break;
            case wizBottomLeft:
                rc.adjust(offsetX, 0, 0, offsetY);
                break;
            case wizBottom:
                rc.adjust(0, 0, 0, offsetY);
                break;
            case wizBottomRight:
                rc.adjust(0, 0, offsetX, offsetY);
                break;
            default:
                Q_ASSERT(false);
                break;
            }
            //
            pT->setGeometry(rc);
        }
        else
        {
            QPoint pos = event->pos();
            WizWindowHitTestResult hit = hitTest(pos);
            if (hit != wizClient)
            {
                event->accept();
            }
            //
            switch (hit)
            {
            case wizTopLeft:
                pT->setCursor(QCursor(Qt::SizeFDiagCursor));
                break;
            case wizTop:
                pT->setCursor(QCursor(Qt::SizeVerCursor));
                break;
            case wizTopRight:
                pT->setCursor(QCursor(Qt::SizeBDiagCursor));
                break;
            case wizLeft:
                pT->setCursor(QCursor(Qt::SizeHorCursor));
                break;
            case wizClient:
                pT->setCursor(QCursor(Qt::ArrowCursor));
                break;
            case wizRight:
                pT->setCursor(QCursor(Qt::SizeHorCursor));
                break;
            case wizBottomLeft:
                pT->setCursor(QCursor(Qt::SizeBDiagCursor));
                break;
            case wizBottom:
                pT->setCursor(QCursor(Qt::SizeVerCursor));
                break;
            case wizBottomRight:
                pT->setCursor(QCursor(Qt::SizeFDiagCursor));
                break;
            }
        }
    }
    virtual void mousePressEvent(QMouseEvent *event)
    {
        Base* pT = this;
        //
        QPoint pos = event->pos();
        WizWindowHitTestResult hit = hitTest(pos);
        //
        m_oldHitCode = hit;
        m_oldPressPos = event->globalPos();
        m_mousePressed = true;
        m_oldGeometry = pT->geometry();
        //
        Base::mousePressEvent(event);
    }
    virtual void mouseReleaseEvent(QMouseEvent *event)
    {
        m_mousePressed = false;
        //
        Base::mouseReleaseEvent(event);
    }
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
