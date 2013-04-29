/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qwebviewkineticscroller.h"

#include <QApplication>
#include <QGraphicsView>
#include <QWebView>
#include <QWebFrame>
#include <QScrollBar>
#include <QPointF>
#include <QPointer>
#include <QMouseEvent>
#include <QTouchEvent>
#if QT_VERSION < 0x040700
#  include <QTime>
#else
#  include <QElapsedTimer>
#endif

#include <QtDebug>

QT_BEGIN_NAMESPACE

//bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event);

class QWebViewKineticScrollerPrivate
{
public:
    QWebViewKineticScroller *q_ptr;

    QWebView *web;
    bool ignoreEvents;
    bool touchActive;
    bool pressed;
    QPointF fractionalPosition;
    QPointer<QWebFrame> scrollFrame;
    Qt::ScrollBarPolicy oldHorizontalSBP;
    Qt::ScrollBarPolicy oldVerticalSBP;

    QEvent::Type pressType;
    QPoint pressPos;
    QPoint pressGlobalPos;
    Qt::MouseButton pressButton;
    Qt::MouseButtons pressButtons;
    Qt::KeyboardModifiers pressModifiers;

#if QT_VERSION < 0x040700
    QTime timer;
#else
    QElapsedTimer timer;
#endif

    QWebViewKineticScrollerPrivate()
        : q_ptr(0)
        , web(0)
        , ignoreEvents(false)
        , touchActive(false)
        , pressed(false)
        , pressType(QEvent::None)
    {}

    void init()
    {
        timer.start();
    }

    void sendEvent(QWidget *w, QEvent *e)
    {
        ignoreEvents = true;
        sendEvent(w, e);
        //qt_sendSpontaneousEvent(w, e);
        ignoreEvents = false;
    }

    QWebFrame *currentFrame() const;
    QWebFrame *scrollingFrameAt(const QPointF &pos) const;
};

/*!
 * The QWebViewKineticScroller class implements the QKineticScroller for the QWebView
 */
QWebViewKineticScroller::QWebViewKineticScroller()
    : QObject()
    , QKineticScroller()
    , d_ptr(new QWebViewKineticScrollerPrivate())
{
    Q_D(QWebViewKineticScroller);
    d->q_ptr = this;
    d->init();
}

/*!
    Destroys the scroller.
*/
QWebViewKineticScroller::~QWebViewKineticScroller()
{ }

void QWebViewKineticScroller::setWidget(QWebView *widget)
{
    Q_D(QWebViewKineticScroller);

    if (d->web) {
        d->web->removeEventFilter(this);
        d->web->setAttribute(Qt::WA_AcceptTouchEvents, false);

        QWebFrame *mainFrame = d->web->page()->mainFrame();
        mainFrame->setScrollBarPolicy(Qt::Vertical, d->oldVerticalSBP);
        mainFrame->setScrollBarPolicy(Qt::Horizontal, d->oldHorizontalSBP);
    }

    d->scrollFrame = 0;
    reset();
    d->web = widget;
    d->fractionalPosition = QPointF();
    setParent(d->web);

    if (d->web) {
        QWebFrame *mainFrame = d->web->page()->mainFrame();
        d->oldVerticalSBP = mainFrame->scrollBarPolicy(Qt::Vertical);
        d->oldHorizontalSBP = mainFrame->scrollBarPolicy(Qt::Horizontal);
        mainFrame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        mainFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        d->web->setAttribute(Qt::WA_AcceptTouchEvents, true);
        d->web->installEventFilter(this);
    }
}

bool QWebViewKineticScroller::eventFilter(QObject *o, QEvent *e)
{
    Q_D(QWebViewKineticScroller);

    qint64 timestamp = d->timer.elapsed();
    bool res = false;

    if (d->web && (o == d->web) && !d->ignoreEvents && d->web->isEnabled() && isEnabled()) {
        switch (e->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd: {
                qWarning("TOUCH%s", e->type() == QEvent::TouchBegin ? "BEGIN" : (e->type() == QEvent::TouchEnd ? "END" : "UPDATE"));
            Input type = (e->type() == QEvent::TouchBegin) ? InputPress :
                        ((e->type() == QEvent::TouchEnd) ? InputRelease : InputMove);
            QTouchEvent *te = static_cast<QTouchEvent *>(e);
            int idx = (te->deviceType() == QTouchEvent::TouchScreen) ? 0 : 1;

            if (te->touchPoints().count() == (idx + 1)) {
                QPointF p = te->touchPoints().at(idx).pos();
                if (type == InputPress) {
                    // remember the frame where the button was pressed
                    QWebFrame *hitFrame = d->scrollingFrameAt(p);
                    if (hitFrame)
                        d->scrollFrame = hitFrame;
                }

                res = handleInput(type, p, timestamp);
                if (type == InputPress) {
                    d->touchActive = true;
                    res = true; // we need to swallow this event, since WebKit would reject it otherwise
                } else if (type == InputRelease) {
                    d->touchActive = false;
                }
                qWarning("   TOUCH%s (%d): %d", type == InputPress ? "BEGIN" : (type == InputRelease ? "END" : "UPDATE"), d->touchActive, res);
            }
            break;
        }
        case QEvent::Wheel:
            // the two-finger gesture on the Mac is mapped to wheel events by default
            qWarning("WHEEL");
            res = d->touchActive;
            break;

        case QEvent::MouseButtonPress:
            // re-install the event filter so that we get the mouse release before all other filters.
            // this is an evil hack, but hard to work around without prioritized event filters.
            d->web->removeEventFilter(this);
            d->web->installEventFilter(this);            
            // fall through

        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::MouseButtonRelease:
            if (!d->touchActive) {
                Input type = (e->type() == QEvent::MouseButtonRelease) ? InputRelease :
                            ((e->type() == QEvent::MouseMove) ? InputMove : InputPress);
                QPointF p = static_cast<QMouseEvent *>(e)->posF();

                if (type == InputPress) {
                    // remember the frame where the button was pressed
                    QWebFrame *hitFrame = d->scrollingFrameAt(p);
                    if (hitFrame)
                        d->scrollFrame = hitFrame;
                    d->pressed = true;
                } else if (type == InputRelease) {
                    d->pressed = false;
                }

                if (type != InputMove || d->pressed)
                    res = handleInput(type, p, timestamp);

                if (e->type() == QEvent::MouseButtonPress && !res) {
                    // eat the event but store it for further reference
                    QMouseEvent *me = reinterpret_cast<QMouseEvent *>(e);

                    d->pressType = me->type();
                    d->pressPos = me->pos();
                    d->pressGlobalPos = me->globalPos();
                    d->pressButton = me->button();
                    d->pressButtons = me->buttons();
                    d->pressModifiers = me->modifiers();

                    res = true;
                }

                if (type == InputRelease && !res) {
                    if (d->pressType == QEvent::MouseButtonPress) {
                        // replay the mouse press
                        QMouseEvent me(d->pressType, d->pressPos, d->pressGlobalPos,
                                       d->pressButton, d->pressButtons, d->pressModifiers);
                        d->sendEvent(d->web, &me);
                        d->pressType = QEvent::None;
                    }
                }
            }
            break;

        default:
            break;
        }
    }

    if (res)
        e->accept();
    return res;
}


bool QWebViewKineticScroller::canStartScrollingAt(const QPointF & /*pos*/) const
{
    return true;
}

void QWebViewKineticScroller::cancelPress(const QPointF & /*pressPos*/)
{
    Q_D(QWebViewKineticScroller);

    d->pressType = QEvent::None;
}

QWebFrame *QWebViewKineticScrollerPrivate::currentFrame() const
{
    if (web && scrollFrame)
        return scrollFrame;
    else if (web)
        return web->page()->mainFrame();
    else
        return 0;
}

// returns the innermost frame at the given position that can be scrolled
QWebFrame *QWebViewKineticScrollerPrivate::scrollingFrameAt(const QPointF &pos) const
{
    QWebFrame *hitFrame = 0;
    if (web) {
        QWebFrame *mainFrame = web->page()->mainFrame();
        hitFrame = mainFrame->hitTestContent(pos.toPoint()).frame();
        QSize range = hitFrame->contentsSize() - hitFrame->geometry().size();
        
        while (hitFrame && range.width() <= 1 && range.height() <= 1)
            hitFrame = hitFrame->parentFrame();
    }
    return hitFrame;
}

QSizeF QWebViewKineticScroller::viewportSize() const
{
    Q_D(const QWebViewKineticScroller);

    return d->web ? d->web->page()->viewportSize() : QSize();
}

QPointF QWebViewKineticScroller::maximumContentPosition() const
{
    Q_D(const QWebViewKineticScroller);
    
    QWebFrame *frame = d->currentFrame();
    QSize s = frame ? frame->contentsSize() - frame->geometry().size() : QSize();
    return QPointF(qMax(0, s.width()), qMax(0, s.height()));
}

QPointF QWebViewKineticScroller::contentPosition() const
{
    Q_D(const QWebViewKineticScroller);

    QWebFrame *frame = d->currentFrame();
    return frame ? QPointF(frame->scrollPosition()) + d->fractionalPosition : QPointF();
}

void QWebViewKineticScroller::setContentPosition(const QPointF &p, const QPointF & /*overshoot*/)
{
    Q_D(QWebViewKineticScroller);
    
    QWebFrame *frame = d->currentFrame();
    if (frame) {
        QPoint pint(int(p.x()), int(p.y()));
        frame->setScrollPosition(pint);
        d->fractionalPosition = p - QPointF(pint);
    }
}

QT_END_NAMESPACE
