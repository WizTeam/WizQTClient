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

#include <QApplication>
#include <QGraphicsView>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QItemSelection>
#include <QPointF>
#include <QPointer>
#include <QMouseEvent>
#if QT_VERSION < 0x040700
#  include <QTime>
#else
#  include <QElapsedTimer>
#endif

#include "qscrollareakineticscroller.h"
#include "qabstractitemview.h"
#include "qgraphicsview.h"
#include "qgraphicsitem.h"

#include <QtDebug>

QT_BEGIN_NAMESPACE

//extern bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event);

class QScrollAreaKineticScrollerPrivate
{
public:
    QScrollAreaKineticScroller *q_ptr;

    QAbstractScrollArea *area;
    QItemSelection oldSelection; // the selection before the first mouse down
    bool ignoreEvents;
    bool touchActive;
    QPoint lastOvershoot; // don't change the type to QPointF or we might never shoot completely back.
    QPointer<QWidget> childWidget; // the widget where the mouse was pressed
    QPointF fractionalPosition;

#if QT_VERSION < 0x040700
    QTime timer;
#else
    QElapsedTimer timer;
#endif

    QScrollAreaKineticScrollerPrivate()
        : q_ptr(0)
        , area(0)
        , ignoreEvents(false)
        , touchActive(false)
    {}

    void init()
    {
        timer.start();
    }

    static QWidget *mouseTransparentChildAtGlobalPos(QWidget *parent, const QPoint &gp)
    {
        foreach (QWidget *w, parent->findChildren<QWidget *>()) {
            if (w && !w->isWindow() && w->isVisible() && (w->rect().contains(w->mapFromGlobal(gp)))) {
                if (QWidget *t = mouseTransparentChildAtGlobalPos(w, gp))
                    return t;
                else
                    return w;
            }
        }
        return 0;
    }

    void sendEvent(QWidget *w, QEvent *e)
    {
        ignoreEvents = true;
        sendEvent(w, e);
        //qt_sendSpontaneousEvent(w, e);
        ignoreEvents = false;
    }
};

/*!
 * The QScrollAreaKineticScroller class implements the QKineticScroller for the AbstractScrollArea
 */
QScrollAreaKineticScroller::QScrollAreaKineticScroller()
    : QObject()
    , QKineticScroller()
    , d_ptr(new QScrollAreaKineticScrollerPrivate())
{
    Q_D(QScrollAreaKineticScroller);
    d->q_ptr = this;
    d->init();
}

/*!
    Destroys the scroller.
*/
QScrollAreaKineticScroller::~QScrollAreaKineticScroller()
{}

void QScrollAreaKineticScroller::setWidget(QAbstractScrollArea *widget)
{
    Q_D(QScrollAreaKineticScroller);

    if (d->area) {
        d->area->viewport()->removeEventFilter(this);
        d->area->viewport()->move(d->area->viewport()->pos() + d->lastOvershoot);
        d->area->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false);
    }

    reset();
    d->area = widget;
    d->lastOvershoot = QPoint();
    d->fractionalPosition = QPointF();

    setParent(d->area);
    if (d->area) {
        d->area->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
        d->area->viewport()->installEventFilter(this);
        if (QAbstractItemView *view = qobject_cast<QAbstractItemView *>(d->area)) {
            view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        }
    }
}

bool QScrollAreaKineticScroller::eventFilter(QObject *o, QEvent *e)
{
    Q_D(QScrollAreaKineticScroller);

    qint64 timestamp = d->timer.elapsed();
    bool res = false;
    bool isMouseEvent = false;
    bool isMouseDown = false;

    qDebug("QScrollAreaKineticScroller::eventFilter: %d", e->type());
    if (d->area && (o == d->area->viewport()) &&
        !d->ignoreEvents && d->area->isEnabled() && isEnabled()) {
        switch (e->type()) {
            case QEvent::TouchBegin:
            case QEvent::TouchUpdate:
            case QEvent::TouchEnd: {
                Input type = (e->type() == QEvent::TouchBegin) ? InputPress :
                            ((e->type() == QEvent::TouchEnd) ? InputRelease : InputMove);
                QTouchEvent *te = static_cast<QTouchEvent *>(e);
                int idx = (te->deviceType() == QTouchEvent::TouchScreen) ? 0 : 1;

                if (te->touchPoints().count() == (idx + 1)) {
                    res = handleInput(type, te->touchPoints().at(idx).pos() - d->lastOvershoot, timestamp);
                    if (type == InputPress) {
                        d->touchActive = true;
                        e->accept();
                    } else if (type == InputRelease) {
                        d->touchActive = false;
                    }
                }
                break;
            }
            case QEvent::Wheel:
                // the two-finger gesture on the Mac is mapped to wheel events by default
                res = d->touchActive;
                break;

            case QEvent::MouseButtonPress:
                // re-install the event filter so that we get the mouse release before all other filters.
                // this is an evil hack, but hard to work around without prioritized event filters.
                d->area->viewport()->removeEventFilter(this);
                d->area->viewport()->installEventFilter(this);
                // fall through

            case QEvent::MouseButtonDblClick:
                isMouseDown = true;
                // fall through

            case QEvent::MouseMove:
            case QEvent::MouseButtonRelease:
                if (!d->touchActive) {
                    Input type = (e->type() == QEvent::MouseButtonRelease) ? InputRelease :
                                ((e->type() == QEvent::MouseMove) ? InputMove : InputPress);
                    QMouseEvent *me = static_cast<QMouseEvent *>(e);
                    isMouseEvent = true;
                    res = handleInput(type, me->posF() - d->lastOvershoot, timestamp);
                }
                break;

            case QEvent::ChildAdded:
            case QEvent::ChildRemoved: {
                QChildEvent *ce = static_cast<QChildEvent *>(e);
                if (ce->child()->isWidgetType()) {
                    static_cast<QWidget *>(ce->child())->setAttribute(Qt::WA_TransparentForMouseEvents, ce->added());
                    if ((e->type() == QEvent::ChildRemoved) && (ce->child() == d->childWidget))
                        d->childWidget = 0;
                }
                break;
            }
            default:
                break;
        }
    }

    if (res)
        e->accept();
    res |= QObject::eventFilter(o, e);

    // all child widgets get the WA_TransparentForMouseEvents attribute, so
    // we have to find the "real" widget to forward this event to...
    if (!res && isMouseEvent) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);

        if (isMouseDown)
            d->childWidget = d->mouseTransparentChildAtGlobalPos(d->area->viewport(), me->globalPos());

        if (d->childWidget) {
            QMouseEvent cme(me->type(), me->pos(),
                    me->globalPos(), me->button(), me->buttons(), me->modifiers());
            d->sendEvent(d->childWidget, &cme);
            res = cme.isAccepted();
        }
    }
    return res;
}

void QScrollAreaKineticScroller::stateChanged(State oldState)
{
    Q_D(QScrollAreaKineticScroller);

    // hack to remove the current selection as soon as we start to scroll
    if (QAbstractItemView *view = qobject_cast<QAbstractItemView *>(d->area)) {
        if (oldState == StateInactive)
            d->oldSelection = view->selectionModel()->selection(); // store the selection
    }
}

bool QScrollAreaKineticScroller::canStartScrollingAt(const QPointF &pos) const
{
    Q_D(const QScrollAreaKineticScroller);

    // don't start scrolling when a drag mode has been set.
    // don't start scrolling on a movable item.
    if (QGraphicsView *view = qobject_cast<QGraphicsView *>(d->area)) {
        if (view->dragMode() != QGraphicsView::NoDrag)
            return false;

        QGraphicsItem *childItem = view->itemAt(pos.toPoint());

        if (childItem && (childItem->flags() & QGraphicsItem::ItemIsMovable))
            return false;
    }

    // don't start scrolling on a QSlider
    if (qobject_cast<QSlider *>(d->mouseTransparentChildAtGlobalPos(d->area->viewport(), d->area->viewport()->mapToGlobal(pos.toPoint())))) {
        return false;
    }

    return true;
}

void QScrollAreaKineticScroller::cancelPress(const QPointF &pressPos)
{
    Q_D(QScrollAreaKineticScroller);

    QPoint pos = pressPos.toPoint();

    if (d->childWidget) {
        if (d->touchActive) {
            //TODO
            // TouchUpdate + TouchEnd?
        } else {
            // simulate a mouse-move and mouse-release far, far away
            QPoint faraway(-1000, -1000);
            QMouseEvent cmem(QEvent::MouseMove, faraway, d->childWidget->mapToGlobal(faraway),
                    Qt::LeftButton, QApplication::mouseButtons() | Qt::LeftButton,
                    QApplication::keyboardModifiers());
            d->sendEvent(d->childWidget, &cmem);

            QMouseEvent cmer(QEvent::MouseButtonRelease, faraway, d->childWidget->mapToGlobal(faraway),
                    Qt::LeftButton, QApplication::mouseButtons() & ~Qt::LeftButton,
                    QApplication::keyboardModifiers());
            d->sendEvent(d->childWidget, &cmer);
        }
    }

    if (QAbstractItemView *view = qobject_cast<QAbstractItemView *>(d->area))
        view->selectionModel()->select(d->oldSelection, QItemSelectionModel::ClearAndSelect);
}

QSizeF QScrollAreaKineticScroller::viewportSize() const
{
    Q_D(const QScrollAreaKineticScroller);

    return d->area ? QSizeF(d->area->viewport()->size()) : QSizeF();
}

QPointF QScrollAreaKineticScroller::maximumContentPosition() const
{
    Q_D(const QScrollAreaKineticScroller);

    QPointF p;
    if (d->area) {
        if (QScrollBar *s = d->area->horizontalScrollBar())
            p.setX(s->maximum());
        if (QScrollBar *s = d->area->verticalScrollBar())
            p.setY(s->maximum());
    }
    return p;
}

QPointF QScrollAreaKineticScroller::contentPosition() const
{
    Q_D(const QScrollAreaKineticScroller);

    QPointF p;
    if (d->area) {
        if (QScrollBar *s = d->area->horizontalScrollBar())
            p.setX(s->value());
        if (QScrollBar *s = d->area->verticalScrollBar())
            p.setY(s->value());
        p += d->fractionalPosition;
    }
    return p;
}

void QScrollAreaKineticScroller::setContentPosition(const QPointF &p, const QPointF &overshootDelta)
{
    Q_D(QScrollAreaKineticScroller);

    if (d->area) {
        if (QScrollBar *s = d->area->horizontalScrollBar())
            s->setValue(p.x());
        if (QScrollBar *s = d->area->verticalScrollBar())
            s->setValue(p.y());

        QPoint delta = d->lastOvershoot - overshootDelta.toPoint();
        if (!delta.isNull())
            d->area->viewport()->move(d->area->viewport()->pos() + delta);
        d->lastOvershoot -= delta;

        d->fractionalPosition = QPointF(p.x() - int(p.x()), p.y() - int(p.y()));
    }
}

QT_END_NAMESPACE
