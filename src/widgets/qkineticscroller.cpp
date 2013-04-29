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
#include <QMap>
#include <QApplication>
#include <QDesktopWidget>
#include <QtCore/qmath.h>

#include <QtDebug>

#include "qkineticscroller.h"
#include "qkineticscroller_p.h"

QT_BEGIN_NAMESPACE

#define KINETIC_SCROLLER_DEBUG

#ifdef KINETIC_SCROLLER_DEBUG
#  define qKSDebug  qDebug
#else
#  define qKSDebug  while (false) qDebug
#endif


inline bool operator<=(const QPointF &p, qreal f)
{
    return (qAbs(p.x()) <= f) && (qAbs(p.y()) <= f);
}
inline bool operator<(const QPointF &p, qreal f)
{
    return (qAbs(p.x()) < f) && (qAbs(p.y()) < f);
}

inline bool operator>=(const QPointF &p, qreal f)
{
    return (qAbs(p.x()) >= f) || (qAbs(p.y()) >= f);
}
inline bool operator>(const QPointF &p, qreal f)
{
    return (qAbs(p.x()) > f) || (qAbs(p.y()) > f);
}

inline QPointF qAbs(const QPointF &p)
{
    return QPointF(qAbs(p.x()), qAbs(p.y()));
}

inline int qSign(qreal r)
{
    return (r < 0) ? -1 : ((r > 0) ? 1 : 0);
}


/*!
    \class QKineticScroller
    \brief The QKineticScroller class enables kinetic scrolling for any scrolling widget or graphics item.
    \ingroup qtmaemo5
    \since 4.6
    \preliminary

    With kinetic scrolling, the user can push the widget in a given
    direction and it will continue to scroll in this direction until it is
    stopped either by the user or by friction.  Aspects of inertia, friction
    and other physical concepts can be changed in order to fine-tune an
    intuitive user experience.

    To enable kinetic scrolling for a widget or graphics item, you need to
    derive from this class and implement at least all the pure-virtual
    functions.

    Qt for Maemo 5 already comes with two implementations for
    QScrollArea and QWebView, and those kinetic scrollers are
    automatically instantiated and attached to these widgets on creation.
    In the QScrollArea case, the kinetic scroller is initially
    disabled. However, for QItemView and QScrollArea derived classes
    it is enabled by default.  You can obtain these automatically created
    objects via a dynamic property:

    \code
    // disable the kinetic scroller on scrollArea
    QKineticScroller *scroller = scrollArea->property("kineticScroller")
                                             .value<QKineticScroller *>();
    if (scroller)
        scroller->setEnabled(false);
    \endcode

    In addition there is also an example on how you would add kinetic
    scrolling to a QGraphicsView based application in \c maemobrowser
    examples in the \c maemo5 examples directory.

    The kinetic scroller installs an event filter on the widget to handle mouse
    presses and moves on the widget \mdash presses and moves on a device's touch screen
    are also handled by this mechanism. These events will be interpreted as scroll actions
    depending on the current state() of the scroller.

    Even though this kinetic scroller has a huge number of settings, we
    recommend that you leave them all at their default values.  In case you
    really want to change them you can try out the \c kineticscroller
    example in the \c maemo5 examples directory.

    \sa QWidget
*/


/*!
    Constructs a new kinetic scroller.
*/
QKineticScroller::QKineticScroller()
    : d_ptr(new QKineticScrollerPrivate())
{
    Q_D(QKineticScroller);
    d->q_ptr = this;
    d->init();
}

/*! \internal
*/
QKineticScroller::QKineticScroller(QKineticScrollerPrivate &dd)
    : d_ptr(&dd)
{
    Q_D(QKineticScroller);
    d->q_ptr = this;
    d->init();
}

/*!
    Destroys the scroller.
*/
QKineticScroller::~QKineticScroller()
{
}

/*!
    \enum QKineticScroller::State

    This enum describes the possible states the kinetic scroller can be in.

    \value Inactive  The scroller is inactive. It may also have been disabled.
    \value Pressed   The user has pressed the mouse button (or pressed the
                     the touch screen).
    \value Dragging  The user is dragging the mouse cursor (or other input
                     point) over the scroll area.
    \value Scrolling Scrolling is occurring without direct user input.
*/

QKineticScrollerPrivate::QKineticScrollerPrivate()
    : enabled(true)
    , state(QKineticScroller::StateInactive)
    , hOvershootPolicy(QKineticScroller::OvershootWhenScrollable)
    , vOvershootPolicy(QKineticScroller::OvershootWhenScrollable)
    , pressTimestamp(0)
    , lastTimestamp(0)
    , scrollToX(false)
    , scrollToY(false)
    , overshootX(false)
    , overshootY(false)
    , cancelPress(false)
    , debugHook(0)
    , debugHookUser(0)
{ }

QKineticScrollerPrivate::~QKineticScrollerPrivate()
{ }

void QKineticScrollerPrivate::init()
{
    Q_Q(QKineticScroller);
    q->setDpiFromWidget(0);
    q->resetScrollMetrics();
}

void QKineticScroller::registerDebugHook(void (*callback)(void *user, const QPointF &releaseVelocity, const QPointF &position, const QPointF &overshootPosition), void *user)
{
    Q_D(QKineticScroller);
    d->debugHook = callback;
    d->debugHookUser = user;
}

void QKineticScroller::resetScrollMetrics()
{
    static QMap<ScrollMetric, QVariant> metrics;

#ifdef Q_WS_MAEMO_5
    metrics.insert(DragVelocitySmoothingFactor, qreal(0.15));
    metrics.insert(ExponentialDecelerationBase, qreal(0.38)); // 0.85^20
    metrics.insert(LinearDecelerationFactor, qreal(0));
    metrics.insert(OvershootSpringConstant, qreal(80.56));
    metrics.insert(OvershootDragResistanceFactor, qreal(1));
    metrics.insert(OvershootMaximumDistance, QPointF(qreal(15.0 / 1000), qreal(15.0 / 1000)));
    metrics.insert(DragStartDistance, qreal(2.5 / 1000));
    metrics.insert(DragStartDirectionErrorMargin, qreal(1.0 / 1000));
    metrics.insert(MaximumVelocity, qreal(6.84));
    metrics.insert(MinimumVelocity, qreal(0.0195));
    metrics.insert(MaximumNonAcceleratedVelocity, qreal(5.6));
    metrics.insert(MaximumClickThroughVelocity, qreal(0.0684));
    metrics.insert(AxisLockThreshold, qreal(0));
    metrics.insert(FastSwipeBaseVelocity, qreal(5.6 * 27));
    metrics.insert(FastSwipeMinimumVelocity, qreal(0.078));
    metrics.insert(FastSwipeMaximumTime, qreal(0.125));
    metrics.insert(FramesPerSecond, qreal(20));
#else
    metrics.insert(DragVelocitySmoothingFactor, qreal(0.02));
    metrics.insert(ExponentialDecelerationBase, qreal(1));
    metrics.insert(LinearDecelerationFactor, qreal(0.38));
    metrics.insert(OvershootSpringConstant, qreal(15.0));
    metrics.insert(OvershootDragResistanceFactor, qreal(0.5));
    metrics.insert(OvershootMaximumDistance, QPointF(0,0)); // QPointF(qreal(14.25 / 1000), qreal(14.25 / 1000)));
    metrics.insert(DragStartDistance, qreal(2.5 / 1000));
    metrics.insert(DragStartDirectionErrorMargin, qreal(1.0 / 1000));
    metrics.insert(MaximumVelocity, qreal(6650.0 / 1000));
    metrics.insert(MinimumVelocity, qreal(30.0 / 1000));
    metrics.insert(MaximumNonAcceleratedVelocity, qreal(532.0 / 1000));
    metrics.insert(MaximumClickThroughVelocity, qreal(66.5 / 1000));
    metrics.insert(AxisLockThreshold, qreal(0));
    metrics.insert(FastSwipeBaseVelocity, qreal(51.3 / 1000));
    metrics.insert(FastSwipeMinimumVelocity, qreal(76.0 / 1000));
    metrics.insert(FastSwipeMaximumTime, qreal(0.125));
    metrics.insert(FramesPerSecond, qreal(60));
#endif

    if (!metrics.isEmpty()) {
        for (QMap<ScrollMetric, QVariant>::const_iterator it = metrics.constBegin(); it != metrics.constEnd(); ++it)
            setScrollMetric(it.key(), it.value());
        if (metrics.count() != ScrollMetricCount)
            qWarning("QKineticScroller::resetAllMetrics(): scroll metrics parameter set did not contain all metrics.");
    } else {
        qWarning("QKineticScroller::resetAllMetrics(): no platform default parameter set available.");
    }
}


const char *QKineticScrollerPrivate::stateName(QKineticScroller::State state)
{
    switch (state) {
    case QKineticScroller::StateInactive:     return "inactive";
    case QKineticScroller::StatePressed:      return "pressed";
    case QKineticScroller::StateDragging:     return "dragging";
    case QKineticScroller::StateScrolling:    return "scrolling";
    default:                                  return "(invalid)";
    }
}

const char *QKineticScrollerPrivate::inputName(QKineticScroller::Input input)
{
    switch (input) {
    case QKineticScroller::InputPress:   return "press";
    case QKineticScroller::InputMove:    return "move";
    case QKineticScroller::InputRelease: return "release";
    default:                             return "(invalid)";
    }
}




void QKineticScrollerPrivate::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId) {
        QObject::timerEvent(e);
        return;
    }

    struct timerevent {
        QKineticScroller::State state;
        typedef void (QKineticScrollerPrivate::*timerhandler_t)();
        timerhandler_t handler;
    };

    timerevent timerevents[] = {
        { QKineticScroller::StateDragging, &QKineticScrollerPrivate::timerEventWhileDragging },
        { QKineticScroller::StateScrolling, &QKineticScrollerPrivate::timerEventWhileScrolling },
    };

    for (int i = 0; i < int(sizeof(timerevents) / sizeof(*timerevents)); ++i) {
        timerevent *te = timerevents + i;

        if (state == te->state) {
            (this->*te->handler)();
            return;
        }
    }

    if (timerId) {
        qWarning() << "Unhandled timer event, while in state " << stateName(state);
        killTimer(timerId);
        timerId = 0;
    }
    // otherwise this is a timer event that was already queued when the
    // timer was killed, so just ignore it
}

bool QKineticScroller::handleInput(Input input, const QPointF &position, qint64 timestamp)
{
    Q_D(QKineticScroller);


    qKSDebug() << "QKS::handleInput(" << input << ", " << position << ", " << timestamp << ")";
    struct statechange {
        State state;
        Input input;
        typedef bool (QKineticScrollerPrivate::*inputhandler_t)(Input input, const QPointF &position, qint64 timestamp);
        inputhandler_t handler;
    };

    statechange statechanges[] = {
        { StateInactive,  InputPress,   &QKineticScrollerPrivate::pressWhileInactive },
        { StatePressed,   InputMove,    &QKineticScrollerPrivate::moveWhilePressed },
        { StatePressed,   InputRelease, &QKineticScrollerPrivate::releaseWhilePressed },
        { StateDragging,  InputMove,    &QKineticScrollerPrivate::moveWhileDragging },
        { StateDragging,  InputRelease, &QKineticScrollerPrivate::releaseWhileDragging },
        { StateScrolling, InputPress,   &QKineticScrollerPrivate::pressWhileScrolling }
    };

    for (int i = 0; i < int(sizeof(statechanges) / sizeof(*statechanges)); ++i) {
        statechange *sc = statechanges + i;

         if (d->state == sc->state && input == sc->input)
             return (d->*sc->handler)(input, position, timestamp);
    }

    qWarning() << "Unhandled input: got input " << d->inputName(input) << " while in state " << d->stateName(d->state);
    return false;
}

bool QKineticScroller::isEnabled() const
{
    Q_D(const QKineticScroller);
    return d->enabled;
}

void QKineticScroller::setEnabled(bool b)
{
    Q_D(QKineticScroller);
    d->enabled = b;
}

QKineticScroller::State QKineticScroller::state() const
{
    Q_D(const QKineticScroller);
    return d->state;
}

/*!
    Resets the internal state of the kinetic scroller. This function is not
    needed for normal use.  This function only needs to be called if the
    kinetic scroller is being re-attached to a different widget.
*/
void QKineticScroller::reset()
{
    Q_D(QKineticScroller);

    d->setState(StateInactive);
}

QKineticScroller::OvershootPolicy QKineticScroller::horizontalOvershootPolicy() const
{
    Q_D(const QKineticScroller);
    return d->hOvershootPolicy;
}

void QKineticScroller::setHorizontalOvershootPolicy(QKineticScroller::OvershootPolicy policy)
{
    Q_D(QKineticScroller);
    d->hOvershootPolicy = policy;
}

QKineticScroller::OvershootPolicy QKineticScroller::verticalOvershootPolicy() const
{
    Q_D(const QKineticScroller);
    return d->vOvershootPolicy;
}

void QKineticScroller::setVerticalOvershootPolicy(QKineticScroller::OvershootPolicy policy)
{
    Q_D(QKineticScroller);
    d->vOvershootPolicy = policy;
}

QVariant QKineticScroller::scrollMetric(ScrollMetric metric) const
{
    Q_D(const QKineticScroller);

    switch (metric) {
    case DragVelocitySmoothingFactor:   return d->dragVelocitySmoothingFactor;
    case LinearDecelerationFactor:      return d->linearDecelerationFactor;
    case ExponentialDecelerationBase:   return d->exponentialDecelerationBase;
    case OvershootSpringConstant:       return d->overshootSpringConstantRoot * d->overshootSpringConstantRoot;
    case OvershootDragResistanceFactor: return d->overshootDragResistanceFactor;
    case OvershootMaximumDistance:      return d->overshootMaximumDistance;
    case DragStartDistance:             return d->dragStartDistance;
    case DragStartDirectionErrorMargin: return d->dragStartDirectionErrorMargin;
    case MinimumVelocity:               return d->minimumVelocity;
    case MaximumVelocity:               return d->maximumVelocity;
    case MaximumNonAcceleratedVelocity: return d->maximumNonAcceleratedVelocity;
    case MaximumClickThroughVelocity:   return d->maximumClickThroughVelocity;
    case AxisLockThreshold:             return d->axisLockThreshold;
    case FramesPerSecond:               return d->framesPerSecond;
    case FastSwipeMaximumTime:          return d->fastSwipeMaximumTime;
    case FastSwipeMinimumVelocity:      return d->fastSwipeMinimumVelocity;
    case FastSwipeBaseVelocity:         return d->fastSwipeBaseVelocity;
    case ScrollMetricCount:             break;
    }
    return QVariant();
}


void QKineticScroller::setScrollMetric(ScrollMetric metric, const QVariant &value)
{
    Q_D(QKineticScroller);

    switch (metric) {
    case DragVelocitySmoothingFactor:   d->dragVelocitySmoothingFactor = qBound(qreal(0), value.toReal(), qreal(1)); break;
    case LinearDecelerationFactor:      d->linearDecelerationFactor = qBound(qreal(0), value.toReal(), qreal(1)); break;
    case ExponentialDecelerationBase:   d->exponentialDecelerationBase = qBound(qreal(0), value.toReal(), qreal(1)); break;
    case OvershootSpringConstant:       d->overshootSpringConstantRoot = qSqrt(value.toReal()); break;
    case OvershootDragResistanceFactor: d->overshootDragResistanceFactor = value.toReal(); break;
    case OvershootMaximumDistance:      d->overshootMaximumDistance = value.toPointF(); break;
    case DragStartDistance:             d->dragStartDistance = value.toReal(); break;
    case DragStartDirectionErrorMargin: d->dragStartDirectionErrorMargin = value.toReal(); break;
    case MinimumVelocity:               d->minimumVelocity = value.toReal(); break;
    case MaximumVelocity:               d->maximumVelocity = value.toReal(); break;
    case MaximumNonAcceleratedVelocity: d->maximumNonAcceleratedVelocity = value.toReal(); break;
    case MaximumClickThroughVelocity:   d->maximumClickThroughVelocity = value.toReal(); break;
    case AxisLockThreshold:             d->axisLockThreshold = qBound(qreal(0), value.toReal(), qreal(1)); break;
    case FramesPerSecond:               d->framesPerSecond = qBound(1, value.toInt(), 100); break;
    case FastSwipeMaximumTime:          d->fastSwipeMaximumTime = value.toReal(); break;
    case FastSwipeMinimumVelocity:      d->fastSwipeMinimumVelocity = value.toReal(); break;
    case FastSwipeBaseVelocity:         d->fastSwipeBaseVelocity = value.toReal(); break;
    case ScrollMetricCount:             break;
    }
}

qreal QKineticScroller::dpi() const
{
    Q_D(const QKineticScroller);
    return d->pixelPerMeter / qreal(39.3700787);
}


void QKineticScroller::setDpi(qreal dpi)
{
    Q_D(QKineticScroller);
    d->pixelPerMeter = dpi * qreal(39.3700787);
}

void QKineticScroller::setDpiFromWidget(QWidget *widget)
{
    Q_D(QKineticScroller);

    QDesktopWidget *dw = QApplication::desktop();
    QPointF dpi = d->realDpi(widget ? dw->screenNumber(widget) : dw->primaryScreen());
    setDpi((dpi.x() + dpi.y()) / qreal(2));
}

#if !defined(Q_WS_MAEMO_5) && !defined(Q_WS_MAC)

QPointF QKineticScrollerPrivate::realDpi(int screen)
{
    QWidget *w = QApplication::desktop()->screen(screen);
    return QPointF(w->physicalDpiX(), w->physicalDpiY());
}

#endif


void QKineticScrollerPrivate::updateVelocity(const QPointF &deltaPixelRaw, qint64 deltaTime)
{
    qKSDebug() << "QKS::updateVelocity(" << deltaPixelRaw << " [delta pix], " << deltaTime << " [delta ms])";

    QPointF deltaPixel = deltaPixelRaw;

    // faster than 2.5mm/ms seems bogus (that would be a screen height in ~20 ms)
    if (((deltaPixelRaw / qreal(deltaTime)).manhattanLength() / pixelPerMeter * 1000) > qreal(2.5))
        deltaPixel = deltaPixelRaw * qreal(2.5) * pixelPerMeter / 1000 / (deltaPixelRaw / qreal(deltaTime)).manhattanLength();

    qreal inversSmoothingFactor = ((qreal(1) - dragVelocitySmoothingFactor) * qreal(deltaTime) / qreal(1000));
    QPointF newv = -deltaPixel / qreal(deltaTime) * qreal(1000) / pixelPerMeter;
    newv = newv * (qreal(1) - inversSmoothingFactor) + releaseVelocity * inversSmoothingFactor;

    // newv = newv * dragVelocitySmoothingFactor + velocity * (qreal(1) - dragVelocitySmoothingFactor);

    if (deltaPixel.x())
        releaseVelocity.setX(qBound(-maximumVelocity, newv.x(), maximumVelocity));
    if (deltaPixel.y())
        releaseVelocity.setY(qBound(-maximumVelocity, newv.y(), maximumVelocity));

    qKSDebug() << "  --> new velocity:" << releaseVelocity;
}

qreal QKineticScrollerPrivate::decelerate(qreal v, qreal t)
{
    qreal result = v * qPow(exponentialDecelerationBase, t);
    qreal linear = linearDecelerationFactor * t;
    if (qAbs(result) > linear)
        return result + (result < 0 ? linear : -linear);
    else
        return 0;
}

/*! Calculates the current velocity during scrolling
 */
QPointF QKineticScrollerPrivate::calculateVelocity(qreal time)
{
    QPointF velocity;

    // -- x coordinate
    if (overshootX) {
        if (overshootSpringConstantRoot * (time-overshootStartTimeX) < M_PI) // prevent swinging around
            velocity.setX(overshootVelocity.x() * qCos(overshootSpringConstantRoot * (time - overshootStartTimeX)));
        else
            velocity.setX(-overshootVelocity.x());

    } else {
        qreal newVelocity = decelerate(releaseVelocity.x(), time);

        if (scrollToX) {
            if (qAbs(newVelocity) < qreal(30.0 / 1000) /* 30mm/s */)
                newVelocity = qreal(30.0 / 1000) * qSign(releaseVelocity.x());

        } else {
            if (qAbs(newVelocity) < qreal(0.5) * qreal(framesPerSecond) / pixelPerMeter /* 0.5 [pix/frame] */)
                newVelocity = 0;
        }

        velocity.setX(newVelocity);
    }

    // -- y coordinate
    if (overshootY) {
        if (overshootSpringConstantRoot * (time-overshootStartTimeY) < M_PI) // prevent swinging around
            velocity.setY(overshootVelocity.y() * qCos(overshootSpringConstantRoot * (time - overshootStartTimeY)));
        else
            velocity.setY(-overshootVelocity.y());

    } else {
        qreal newVelocity = decelerate(releaseVelocity.y(), time);

        if (scrollToY) {
            if (qAbs(newVelocity) < qreal(30.0 / 1000) /* 30mm/s */)
                newVelocity = qreal(30.0 / 1000) * qSign(releaseVelocity.y());

        } else {
            if (qAbs(newVelocity) < qreal(0.5) * qreal(framesPerSecond) / pixelPerMeter /* 0.5 [pix/frame] */)
                newVelocity = 0;
        }

        velocity.setY(newVelocity);
    }

    return velocity;
}


void QKineticScrollerPrivate::handleDrag(const QPointF &position, qint64 timestamp)
{
    Q_Q(QKineticScroller);

    QPointF deltaPixel = position - lastPosition;
    qint64 deltaTime = timestamp - lastTimestamp;

    if (axisLockThreshold) {
        int dx = qAbs(deltaPixel.x());
        int dy = qAbs(deltaPixel.y());
        if (dx || dy) {
            bool vertical = (dy > dx);
            qreal alpha = qreal(vertical ? dx : dy) / qreal(vertical ? dy : dx);
            //qKSDebug() << "QKS::handleDrag() -- axis lock:" << alpha << " / " << axisLockThreshold << "- isvertical:" << vertical << "- dx:" << dx << "- dy:" << dy;
            if (alpha <= axisLockThreshold) {
                if (vertical)
                    deltaPixel.setX(0);
                else
                    deltaPixel.setY(0);
            }
        }
    }

    // calculate velocity (if the user would release the mouse NOW)
    updateVelocity(deltaPixel, deltaTime);

    // restrict velocity, if content is not scrollable
    QPointF maxPos = q->maximumContentPosition();
    bool canScrollX = maxPos.x() || (hOvershootPolicy == QKineticScroller::OvershootAlwaysOn);
    bool canScrollY = maxPos.y() || (vOvershootPolicy == QKineticScroller::OvershootAlwaysOn);

    if (!canScrollX) {
        deltaPixel.setX(0);
        releaseVelocity.setX(0);
    }
    if (!canScrollY) {
        deltaPixel.setY(0);
        releaseVelocity.setY(0);
    }

//    if (firstDrag) {
//        // Do not delay the first drag
//        setContentPositionHelper(q->contentPosition() - overshootDistance - deltaPixel);
//        dragDistance = QPointF(0, 0);
//    } else {
    dragDistance += deltaPixel;
//    }

    if (canScrollX)
        lastPosition.setX(position.x());
    if (canScrollY)
        lastPosition.setY(position.y());
    lastTimestamp = timestamp;
}



bool QKineticScrollerPrivate::pressWhileInactive(QKineticScroller::Input, const QPointF &position, qint64 timestamp)
{
    Q_Q(QKineticScroller);

    if ((q->maximumContentPosition() > qreal(0)) ||
        (hOvershootPolicy == QKineticScroller::OvershootAlwaysOn) ||
        (vOvershootPolicy == QKineticScroller::OvershootAlwaysOn)) {
        if (q->canStartScrollingAt(position)) {
            lastPosition = pressPosition = position;
            lastTimestamp = pressTimestamp = timestamp;
            cancelPress = true;
            setState(QKineticScroller::StatePressed);
        }
    }
    return false;
}

bool QKineticScrollerPrivate::releaseWhilePressed(QKineticScroller::Input, const QPointF &, qint64)
{
    if (overshootX || overshootY)
        setState(QKineticScroller::StateScrolling);
    else
        setState(QKineticScroller::StateInactive);
    return false;
}

bool QKineticScrollerPrivate::moveWhilePressed(QKineticScroller::Input, const QPointF &position, qint64 timestamp)
{
    Q_Q(QKineticScroller);

    QPointF deltaPixel = position - pressPosition;

    bool moveStarted = ((deltaPixel.manhattanLength() / pixelPerMeter) > dragStartDistance);

    if (moveStarted) {
        qreal deltaXtoY = qAbs(pressPosition.x() - position.x()) - qAbs(pressPosition.y() - position.y());
        deltaXtoY /= pixelPerMeter;

        QPointF maxPos = q->maximumContentPosition();
        bool canScrollX = (maxPos.x() > 0);
        bool canScrollY = (maxPos.y() > 0);

        if (hOvershootPolicy == QKineticScroller::OvershootAlwaysOn)
            canScrollX = true;
        if (vOvershootPolicy == QKineticScroller::OvershootAlwaysOn)
            canScrollY = true;

        if (deltaXtoY < 0) {
            if (!canScrollY && (!canScrollX || (-deltaXtoY >= dragStartDirectionErrorMargin)))
                moveStarted = false;
        } else {
            if (!canScrollX && (!canScrollY || (deltaXtoY >= dragStartDirectionErrorMargin)))
                moveStarted = false;
        }
    }

    if (moveStarted) {
        if (cancelPress)
            q->cancelPress(pressPosition);
        setState(QKineticScroller::StateDragging);

        // subtract the dragStartDistance
        deltaPixel = deltaPixel - deltaPixel * (dragStartDistance / deltaPixel.manhattanLength());

        if (!deltaPixel.isNull()) {
            // handleDrag updates lastPosition, lastTimestamp and velocity
            handleDrag(pressPosition + deltaPixel, timestamp);
        }
    }
    return moveStarted;
}

bool QKineticScrollerPrivate::moveWhileDragging(QKineticScroller::Input, const QPointF &position, qint64 timestamp)
{
    // handleDrag updates lastPosition, lastTimestamp and velocity
    handleDrag(position, timestamp);
    return true;
}

void QKineticScrollerPrivate::timerEventWhileDragging()
{
    if (!dragDistance.isNull()) {
        qKSDebug() << "QKS::timerEventWhileDragging() -- dragDistance:" << dragDistance;

        setContentPositionHelper(-dragDistance);
        dragDistance = QPointF(0, 0);
    }
}

bool QKineticScrollerPrivate::releaseWhileDragging(QKineticScroller::Input, const QPointF &, qint64 timestamp)
{
    Q_Q(QKineticScroller);

    // calculate the fastSwipe velocity
    QPointF maxPos = q->maximumContentPosition();
    QPointF fastSwipeVelocity  = QPoint(0, 0);
    QSizeF size = q->viewportSize();
    if (size.width())
        fastSwipeVelocity.setX(qMin(maximumVelocity, maxPos.x() / size.width() * fastSwipeBaseVelocity));
    if (size.height())
        fastSwipeVelocity.setY(qMin(maximumVelocity, maxPos.y() / size.height() * fastSwipeBaseVelocity));

    if (fastSwipeMaximumTime &&
        ((timestamp - pressTimestamp) < qint64(fastSwipeMaximumTime * 1000)) &&
        (oldVelocity > fastSwipeMinimumVelocity)) {

        // more than one fast swipe in a row: add fastSwipeVelocity
        int signX = 0, signY = 0;
        if (releaseVelocity.x())
            signX = (releaseVelocity.x() > 0) == (oldVelocity.x() > 0) ? 1 : -1;
        if (releaseVelocity.y())
            signY = (releaseVelocity.y() > 0) == (oldVelocity.y() > 0) ? 1 : -1;

        releaseVelocity.setX(signX * (oldVelocity.x() + (oldVelocity.x() > 0 ? fastSwipeVelocity.x() : -fastSwipeVelocity.x())));
        releaseVelocity.setY(signY * (oldVelocity.y() + (oldVelocity.y() > 0 ? fastSwipeVelocity.y() : -fastSwipeVelocity.y())));

    } else if (releaseVelocity >= minimumVelocity) {

        // if we have a fast swipe, accelerate it to the fastSwipe velocity
        if ((qAbs(releaseVelocity.x()) > maximumNonAcceleratedVelocity) &&
            (fastSwipeVelocity.x() > maximumNonAcceleratedVelocity)) {
            releaseVelocity.setX(releaseVelocity.x() > 0 ? fastSwipeVelocity.x() : -fastSwipeVelocity.x());
        }
        if ((qAbs(releaseVelocity.y()) > maximumNonAcceleratedVelocity) &&
            (fastSwipeVelocity.y() > maximumNonAcceleratedVelocity)) {
            releaseVelocity.setY(releaseVelocity.y() > 0 ? fastSwipeVelocity.y() : -fastSwipeVelocity.y());
        }

    }

    qKSDebug() << "QKS::releaseWhileDragging() -- velocity:" << releaseVelocity << "-- minimum velocity:" << minimumVelocity;
    if (overshootX || overshootY)
        setState(QKineticScroller::StateScrolling);
    else if (releaseVelocity >= minimumVelocity)
        setState(QKineticScroller::StateScrolling);
    else
        setState(QKineticScroller::StateInactive);

    return true;
}

void QKineticScrollerPrivate::timerEventWhileScrolling()
{
    qreal deltaTime = qreal(scrollRelativeTimer.restart()) / 1000;
    qreal time = qreal(scrollAbsoluteTimer.elapsed()) / 1000;

    // calculate the velocity for the passed interval deltatime.
    // using the midpoint of the interval gives a better precision than using just time.
    QPointF newVelocity = calculateVelocity(time - deltaTime / 2);
    QPointF deltaPos = newVelocity * deltaTime * pixelPerMeter;

    // -- move (convert from [m/s] to [pix/frame]
    if (!deltaPos.isNull())
        setContentPositionHelper(deltaPos);

    qKSDebug() << "QKS::timerEventWhileScrolling() -- DeltaPos:" << deltaPos << "- NewVel:" <<  newVelocity  << "- Time:" <<  time;

    if (newVelocity.isNull() ||
            (releaseVelocity.isNull() && !scrollToX && !scrollToY && !overshootX && !overshootY))
    // if (newVelocity.isNull())
        setState(QKineticScroller::StateInactive);
}

bool QKineticScrollerPrivate::pressWhileScrolling(QKineticScroller::Input, const QPointF &position, qint64 timestamp)
{
    lastPosition = pressPosition = position;
    lastTimestamp = pressTimestamp = timestamp;
    cancelPress = false;
    setState(QKineticScroller::StatePressed);
    return true;
}

void QKineticScrollerPrivate::setState(QKineticScroller::State newstate)
{
    Q_Q(QKineticScroller);

    if (state == newstate)
        return;

    qKSDebug() << "QKS::setState(" << stateName(newstate) << ")";

    switch (newstate) {
    case QKineticScroller::StateInactive:
        if (state == QKineticScroller::StateScrolling) {
            if (timerId) {
                killTimer(timerId);
                timerId = 0;
            } else {
                qKSDebug() << "  --> state change from " << stateName(state) << " to " << stateName(newstate) << ", but timer is not active.";
            }
        }
        releaseVelocity = QPointF(0, 0);
        break;

    case QKineticScroller::StatePressed:
        if (timerId) {
            killTimer(timerId);
            timerId = 0;
        }
        scrollToX = false;
        scrollToY = false;
        oldVelocity = releaseVelocity;
        // releaseVelocity = QPointF(0, 0);
        break;

    case QKineticScroller::StateDragging:

        dragDistance = QPointF(0, 0);
        if (state == QKineticScroller::StatePressed) {
            if (!timerId) {
                timerId = startTimer(1000 / framesPerSecond);
            } else {
                qKSDebug() << "  --> state change from " << stateName(state) << " to " << stateName(newstate) << ", but timer is already active.";
            }
        }

        break;

    case QKineticScroller::StateScrolling:
        if (!timerId) {
            timerId = startTimer(1000 / framesPerSecond);
        }
        scrollRelativeTimer.start();
        scrollAbsoluteTimer.start();

        if (state == QKineticScroller::StateDragging) {
            // TODO: better calculate StartTime using the current releaseVelocity
            overshootStartTimeX = overshootStartTimeY = qreal(scrollAbsoluteTimer.elapsed()) / 1000 - M_PI / (overshootSpringConstantRoot * 2);
            overshootVelocity = overshootPosition / pixelPerMeter * overshootSpringConstantRoot;
        }

        break;
    }

    qSwap(state, newstate);
    q->stateChanged(newstate);
}

/*!
    Starts scrolling the widget so that the point \a pos is visible inside
    the viewport.

    If the specified point cannot be reached, the contents are scrolled to the
    nearest valid position (in this case the scroller might or might not overshoot).

    The scrolling speed will be calculated so that the given position will
    be reached after a platform-defined time span (1 second for Maemo 5).
    The final speed at the end position is not guaranteed to be zero.

    \sa ensureVisible(), maximumContentPosition()
*/
void QKineticScroller::scrollTo(const QPointF &pos, int scrollTime)
{
    Q_D(QKineticScroller);

    if (scrollTime <= 0)
        scrollTime = 1;

    qKSDebug() << "QKS::scrollTo(" << pos << " [pix], " << scrollTime << " [ms])";

    qreal time = qreal(scrollTime) / 1000;

    if ((pos == contentPosition()) ||
            (d->state == QKineticScroller::StatePressed) ||
            (d->state == QKineticScroller::StateDragging)) {
        return;
    }

    // estimate the minimal start velocity
    // if the start velocity is below that then the scrolling would stop before scrollTime.
    //qreal vMin = d->linearDecelerationFactor * time / qPow(d->exponentialDecelerationBase, time);

    /*
    // estimate of the distance passed within the vMin time during scrollTime
    qreal distMin = qreal(scrollTime) * vMin / 2.0;

    QPointF v = QPointF((pos.x()-contentPosition().x()) / distMin * vMin,
                        (pos.y()-contentPosition().y()) / distMin * vMin);

    */

    // v(t) = vstart * exponentialDecelerationBase ^ t - linearDecelerationFactor * t
    // pos(t) = integrate(v(t) * dt)
    // pos(t) = vstart * (eDB ^ t / ln(eDB) + C) -  lDF / 2 * t ^ 2
    //
    // pos(time) = pos - contentsPos()
    // vstart = ((lDF / 2) * time ^ 2 + (pos - contentPos())) / (eDB ^ time / ln(eDB) + C)
    // (for C = -1/ln(eDB) )

    QPointF scrollDir(qSign(pos.x() - contentPosition().x()),
                      qSign(pos.y() - contentPosition().y()));

    QPointF v = (scrollDir * (d->linearDecelerationFactor / qreal(2)) * qreal(time) * qreal(time) + (pos - contentPosition()) / d->pixelPerMeter);
    if (d->exponentialDecelerationBase != qreal(1))
        v /= (qPow(d->exponentialDecelerationBase, time) - 1) / qLn(d->exponentialDecelerationBase);
    else
        v /= time;

    d->scrollToPosition = pos;
    d->scrollToX = true;
    d->scrollToY = true;
    d->releaseVelocity = v;
    d->scrollRelativeTimer.restart();
    d->scrollAbsoluteTimer.restart();
    d->setState(QKineticScroller::StateScrolling);
}

/*!
    Starts scrolling the widget so that the point \a pos is visible inside the
    viewport with margins specified in pixels by \a xmargin and \a ymargin.

    If the specified point cannot be reached, the contents are scrolled to the
    nearest valid position.  The default value for both margins is 50 pixels.

    This function performs the actual scrolling by calling scrollTo().

    \sa maximumContentPosition()
*/
void QKineticScroller::ensureVisible(const QPointF &pos, int xmargin, int ymargin, int scrollTime)
{
    QSizeF visible = viewportSize();
    QPointF currentPos = contentPosition();

    qKSDebug() << "QKS::ensureVisible(" << pos << " [pix], " << xmargin << " [pix], " << ymargin << " [pix], " << scrollTime << "[ms])";
    qKSDebug() << "  --> content position:" << contentPosition();

    QRectF posRect(pos.x() - xmargin, pos.y() - ymargin, 2 * xmargin, 2 * ymargin);
    QRectF visibleRect(currentPos, visible);

    if (visibleRect.contains(posRect))
        return;

    QPointF newPos = currentPos;
    if (posRect.top() < visibleRect.top())
        newPos.setY(posRect.top());
    else if (posRect.bottom() > visibleRect.bottom())
        newPos.setY(posRect.bottom() - visible.height());
    if (posRect.left() < visibleRect.left())
        newPos.setX(posRect.left());
    else if (posRect.right() > visibleRect.right())
        newPos.setY(posRect.right() - visible.width());

    scrollTo(newPos, scrollTime);
}


/*! \internal
    Helps when setting the content position.
    It will try to move the content by the requested delta but stop in case
    when we are coming back from an overshoot or a scrollTo.
    It will also indicate a new overshooting condition by the overshootX and oversthootY flags.

    In this cases it will reset the velocity variables and other flags.

    Also keeps track of the current over-shooting value in overshootPosition.

    \deltaPos is the amout of pixels the current content position should be moved
*/
void QKineticScrollerPrivate::setContentPositionHelper(const QPointF &deltaPos)
{
    Q_Q(QKineticScroller);

    if (state == QKineticScroller::StateDragging && overshootDragResistanceFactor)
        overshootPosition /= overshootDragResistanceFactor;

    QPointF oldPos = q->contentPosition() + overshootPosition;
    QPointF newPos = oldPos + deltaPos;
    QPointF maxPos = q->maximumContentPosition();

    QPointF oldScrollToDist = scrollToPosition - oldPos;
    QPointF newScrollToDist = scrollToPosition - newPos;

    qKSDebug() << "QKS::setContentPositionHelper(" << deltaPos << " [pix])";
    qKSDebug() << "  --> overshoot:" << overshootPosition << "- old pos:" << oldPos << "- new pos:" << newPos;

    QPointF oldClampedPos;
    oldClampedPos.setX(qBound(qreal(0), oldPos.x(), maxPos.x()));
    oldClampedPos.setY(qBound(qreal(0), oldPos.y(), maxPos.y()));

    QPointF newClampedPos;
    newClampedPos.setX(qBound(qreal(0), newPos.x(), maxPos.x()));
    newClampedPos.setY(qBound(qreal(0), newPos.y(), maxPos.y()));

    // --- handle overshooting and stop if the coordinate is going back inside the normal area
    bool alwaysOvershootX = (hOvershootPolicy == QKineticScroller::OvershootAlwaysOn);
    bool alwaysOvershootY = (vOvershootPolicy == QKineticScroller::OvershootAlwaysOn);
    bool noOvershootX = (hOvershootPolicy == QKineticScroller::OvershootAlwaysOff) ||
                        ((state == QKineticScroller::StateDragging) && !overshootDragResistanceFactor);
    bool noOvershootY = (vOvershootPolicy == QKineticScroller::OvershootAlwaysOff) ||
                        ((state == QKineticScroller::StateDragging) && !overshootDragResistanceFactor);
    bool canOvershootX = !noOvershootX && (alwaysOvershootX || maxPos.x());
    bool canOvershootY = !noOvershootY && (alwaysOvershootY || maxPos.y());

    qreal oldOvershootX = (canOvershootX) ? oldPos.x() - oldClampedPos.x() : 0;
    qreal oldOvershootY = (canOvershootY) ? oldPos.y() - oldClampedPos.y() : 0;

    qreal newOvershootX = (canOvershootX) ? newPos.x() - newClampedPos.x() : 0;
    qreal newOvershootY = (canOvershootY) ? newPos.y() - newClampedPos.y() : 0;

    if (state == QKineticScroller::StateDragging && overshootDragResistanceFactor) {
        oldOvershootX *= overshootDragResistanceFactor;
        oldOvershootY *= overshootDragResistanceFactor;
        newOvershootX *= overshootDragResistanceFactor;
        newOvershootY *= overshootDragResistanceFactor;
    }

    // -- stop at the maximum overshoot distance (if set)
    if (!overshootMaximumDistance.isNull()) {
        newOvershootX = qBound(-overshootMaximumDistance.x() * pixelPerMeter, newOvershootX, overshootMaximumDistance.x() * pixelPerMeter);

        newOvershootY = qBound(-overshootMaximumDistance.y() * pixelPerMeter, newOvershootY, overshootMaximumDistance.y() * pixelPerMeter);
    }

    // --- sanity check for scrollTo in case we can't even scroll that direction
    if (!(maxPos.x() || alwaysOvershootX))
        scrollToX = false;
    if (!(maxPos.y() || alwaysOvershootY))
        scrollToY = false;

    // --- handle crossing over borders (scrollTo and overshoot)
    qKSDebug() << "  --> old overshoot Y:" << oldOvershootY << "- new overshoot Y:" << newOvershootY;
    // -- x axis
    if (scrollToX && qSign(oldScrollToDist.x()) != qSign(newScrollToDist.x())) {
        newClampedPos.setX(scrollToPosition.x());
        newOvershootX = 0;
        releaseVelocity.setX(0);
        scrollToX = false;

    } else if (oldOvershootX && (qSign(oldOvershootX) != qSign(newOvershootX))) {
        newClampedPos.setX((oldOvershootX < 0) ? 0 : maxPos.x());
        newOvershootX = 0;
        releaseVelocity.setX(0);
        overshootVelocity.setX(0);
        overshootX = false;

    } else if (!oldOvershootX && newOvershootX) {
        overshootStartTimeX = qreal(scrollAbsoluteTimer.elapsed()) / 1000;
        overshootVelocity.setX(calculateVelocity(overshootStartTimeX).x());

        // restrict the overshoot to overshootMaximumDistance
        qreal maxOvershootVelocity = overshootMaximumDistance.x() * overshootSpringConstantRoot;
        if (overshootMaximumDistance.x() && qAbs(overshootVelocity.x()) > maxOvershootVelocity)
            overshootVelocity.setX(maxOvershootVelocity * qSign(newOvershootX));

        // -- prevent going into overshoot too far
        if (state != QKineticScroller::StateDragging)
            newOvershootX = qSign(newOvershootX) * qreal(0.0001);

        scrollToX = false;
        overshootX = true;
    }

    // -- y axis
    if (scrollToY && qSign(oldScrollToDist.y()) != qSign(newScrollToDist.y())) {
        newClampedPos.setY(scrollToPosition.y());
        newOvershootY = 0;
        releaseVelocity.setY(0);
        scrollToY = false;

    } else if (oldOvershootY && (qSign(oldOvershootY) != qSign(newOvershootY))) {
        newClampedPos.setY((oldOvershootY < 0) ? 0 : maxPos.y());
        newOvershootY = 0;
        releaseVelocity.setY(0);
        overshootVelocity.setY(0);
        overshootY = false;

    } else if (!oldOvershootY && newOvershootY) {
        overshootStartTimeY = qreal(scrollAbsoluteTimer.elapsed()) / 1000;
        overshootVelocity.setY(calculateVelocity(overshootStartTimeY).y());

        // -- restrict the overshoot to overshootMaximumDistance
        qreal maxOvershootVelocity = overshootMaximumDistance.y() * overshootSpringConstantRoot;
        if (overshootMaximumDistance.y() && (qAbs(overshootVelocity.y()) > maxOvershootVelocity))
            overshootVelocity.setY(maxOvershootVelocity * qSign(newOvershootY));

        // -- prevent going into overshoot too far
        if (state != QKineticScroller::StateDragging)
            newOvershootY = qSign(newOvershootY) * qreal(0.0001);

        scrollToY = false;
        overshootY = true;
    }

    overshootPosition.setX(newOvershootX);
    overshootPosition.setY(newOvershootY);

    q->setContentPosition(newClampedPos, overshootPosition);

    if (debugHook)
        debugHook(debugHookUser, calculateVelocity(qreal(scrollAbsoluteTimer.elapsed()) / 1000), newClampedPos, overshootPosition);

    qKSDebug() << "  --> new position:" << newClampedPos << "- new overshoot:" << overshootPosition <<
                  "- overshoot x/y?:" << overshootX << "/" << overshootY << "- scrollto x/y?:" << scrollToX << "/" << scrollToY;
}


/*!
    \enum QKineticScroller::OvershootPolicy

    This enum describes the various modes of overshooting.

    \value OvershootWhenScrollable Overshooting is when the content is scrollable. This is the default.

    \value OvershootAlwaysOff Overshooting is never enabled (even when the content is scrollable).

    \value OvershootAlwaysOn Overshooting is always enabled (even when the content is not scrollable).
*/


/*!
    If kinetic scrolling can be started at the given content's \a position,
    this function needs to return true; otherwise it needs to return false.

    The default value is true, regardless of \a position.
*/
bool QKineticScroller::canStartScrollingAt(const QPointF &position) const
{
    Q_UNUSED(position);
    return true;
}

/*!
    Since a mouse press is always delivered normally when the scroller is in
    the StateInactive state, we may need to cancel it as soon as the user
    has moved the mouse far enough to actually start a kinetic scroll
    operation.

    The \a pressPosition parameter can be used to find out which widget (or
    graphics item) received the mouse press in the first place.

    Subclasses may choose to simulate a fake mouse release event for that
    widget (or graphics item), preferably \bold not within its boundaries.
    The default implementation does nothing.
*/
void QKineticScroller::cancelPress(const QPointF &pressPosition)
{
    Q_UNUSED(pressPosition);
}


/*!
    This function get called whenever the state of the kinetic scroller changes.
    The old state is supplied as \a oldState, while the new state is returned by
    calling state().

    The default implementation does nothing.

    \sa state()
*/
void QKineticScroller::stateChanged(State oldState)
{
    Q_UNUSED(oldState);
}

/*!
    \fn QPointF QKineticScroller::maximumContentPosition() const

    Returns the maximum valid content position. The minimum is always \c
    (0,0).

    \sa scrollTo()
*/

/*!
    \fn QSizeF QKineticScroller::viewportSize() const

    Returns the size of the currently visible content positions.  In the
    case where an QAbstractScrollArea is used, this is equivalent to the
    viewport() size.

    \sa scrollTo()
*/

/*!
    \fn QPointF QKineticScroller::contentPosition() const

    \brief Returns the current position of the content.

    Note that overshooting is not considered to be "real" scrolling so the
    position might be (0,0) even if the user is currently dragging the
    widget outside the "normal" maximumContentPosition().

    \sa maximumContentPosition()
*/


/*!
    \fn void QKineticScroller::setContentPosition(const QPointF &position, const QPointF &overshoot)

    Set the content's \a position. This parameter will always be in the
    valid range QPointF(0, 0) and maximumContentPosition().

    In the case where overshooting is required, the \a overshoot parameter
    will give the direction and the absolute distance to overshoot.

    \sa maximumContentPosition()
*/

QT_END_NAMESPACE
