#ifndef WIZSHADOWWINDOW_H
#define WIZSHADOWWINDOW_H



#include <QWidget>
#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>
#include <QDebug>
#include <QBoxLayout>
#include "wiztitlebar.h"
#include "wizshadoweffect.h"




template <class Base>
class CWizShadowWindow : public Base
{
public:
    explicit CWizShadowWindow(QWidget *parent = 0)
        : Base(parent)
        , m_oldHitCode(wizClient)
        , m_mousePressed(false)
        , m_mainWidget(NULL)
        , m_mainLayout(NULL)
    {
        Base* pT = this;
        pT->setContentsMargins(10, 10, 10, 10);
        pT->setGeometry(100, 100, 400, 300);
        //
        pT->setAttribute(Qt::WA_TranslucentBackground); //enable MainWindow to be transparent
        //
        pT->setWindowFlags(Qt::FramelessWindowHint);
        //
        m_mainWidget = new QWidget(this);
        m_mainWidget->setAutoFillBackground(true);

        CWizShadowEffect* effect = new CWizShadowEffect();
        m_mainWidget->setGraphicsEffect(effect);
        m_mainWidget->setCursor(QCursor(Qt::ArrowCursor));
        //
        m_mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_mainWidget->setLayout(m_mainLayout);
        //
        m_mainLayout->setSpacing(0);
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        //
        m_titleBar = new CWizTitleBar(m_mainWidget, this);
        m_titleBar->setFixedHeight(40);
        m_mainLayout->addWidget(m_titleBar);
        //
        pT->setMouseTracking(true);
    }
public:
    QWidget *mainWidget() const { return m_mainWidget; }
    QLayout* mainLayout() const { return m_mainLayout; }
protected:
    enum WizWindowHitTestResult {wizTopLeft, wizTop, wizTopRight, wizLeft, wizClient, wizRight, wizBottomLeft, wizBottom, wizBottomRight};
private:
    WizWindowHitTestResult m_oldHitCode;
    QPoint m_oldPressPos;
    QRect m_oldGeometry;
    bool m_mousePressed;
    QWidget* m_mainWidget;
    QLayout* m_mainLayout;
    CWizTitleBar* m_titleBar;
protected:
    virtual WizWindowHitTestResult hitTest(const QPoint& posOfWindow)
    {
        Base* pT = this;
        QPoint globalPos = pT->mapToGlobal(posOfWindow);
        QPoint pos = m_mainWidget->mapFromGlobal(globalPos);
        if (pos.x() < 0)
        {
            if (pos.y() < 0)
            {
                return wizTopLeft;
            }
            else if (pos.y() >= m_mainWidget->height())
            {
                return wizBottomLeft;
            }
            else
            {
                return wizLeft;
            }
        }
        else if (pos.x() > m_mainWidget->width())
        {
            if (pos.y() < 0)
            {
                return wizTopRight;
            }
            else if (pos.y() >= m_mainWidget->height())
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
        else if (pos.y() > m_mainWidget->height())
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

};

#endif // WIZSHADOWWINDOW_H
