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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QTime>
#include <QPointer>
#include <QObject>
#include <QEvent>
#if QT_VERSION < 0x040700
#  include <QTime>
#else
#  include <QElapsedTimer>
#endif

#include "qkineticscroller.h"

QT_BEGIN_NAMESPACE

class QKineticScrollerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QKineticScroller)

public:
    QKineticScrollerPrivate();
    virtual ~QKineticScrollerPrivate();
    void init();

    void setState(QKineticScroller::State s);

    bool pressWhileInactive(QKineticScroller::Input input, const QPointF &position, qint64 timestamp);
    bool moveWhilePressed(QKineticScroller::Input input, const QPointF &position, qint64 timestamp);
    bool releaseWhilePressed(QKineticScroller::Input input, const QPointF &position, qint64 timestamp);
    bool moveWhileDragging(QKineticScroller::Input input, const QPointF &position, qint64 timestamp);
    bool releaseWhileDragging(QKineticScroller::Input input, const QPointF &position, qint64 timestamp);
    bool pressWhileScrolling(QKineticScroller::Input input, const QPointF &position, qint64 timestamp);

    void timerEvent(QTimerEvent *);
    void timerEventWhileDragging();
    void timerEventWhileScrolling();

    void handleDrag(const QPointF &position, qint64 timestamp);
    void updateVelocity(const QPointF &deltaPixel, qint64 deltaTime);

    qreal decelerate(qreal v, qreal t);
    QPointF calculateVelocity(qreal time);
    void setContentPositionHelper(const QPointF &deltaPos);

    QPointF realDpi(int screen);


    static const char *stateName(QKineticScroller::State state);
    static const char *inputName(QKineticScroller::Input input);

    // metrics

    qreal dragVelocitySmoothingFactor;
    qreal linearDecelerationFactor;
    qreal exponentialDecelerationBase;
    qreal overshootSpringConstantRoot;
    QPointF overshootMaximumDistance;
    qreal overshootDragResistanceFactor;
    qreal dragStartDistance;
    qreal dragStartDirectionErrorMargin;
    qreal maximumVelocity;
    qreal minimumVelocity;
    qreal maximumNonAcceleratedVelocity;
    qreal maximumClickThroughVelocity;
    qreal axisLockThreshold;
    qreal fastSwipeBaseVelocity;
    qreal fastSwipeMinimumVelocity;
    qreal fastSwipeMaximumTime;
    int framesPerSecond;

    // state

    bool enabled;
    QKineticScroller::State state;
    QKineticScroller::OvershootPolicy hOvershootPolicy;
    QKineticScroller::OvershootPolicy vOvershootPolicy;
    QPointF oldVelocity;

    QPointF pressPosition;
    QPointF lastPosition;
    qint64  pressTimestamp;
    qint64  lastTimestamp;

    QPointF dragDistance; // the distance we should move during the next drag timer event

    QPointF scrollToPosition;
    bool    scrollToX;
    bool    scrollToY;

    QPointF overshootPosition; // the number of pixels we are overshooting (before overshootDragResistanceFactor)
    bool    overshootX;
    bool    overshootY;

    qreal pixelPerMeter;

#if QT_VERSION < 0x040700
    QTime scrollRelativeTimer;
    QTime scrollAbsoluteTimer;
#else
    QElapsedTimer scrollRelativeTimer;
    QElapsedTimer scrollAbsoluteTimer;
#endif
    QPointF releaseVelocity; // the starting velocity of the scrolling state
    QPointF overshootVelocity; // the starting velocity when going into overshoot
    qreal overshootStartTimeX;
    qreal overshootStartTimeY;
    int timerId;

    bool cancelPress;

    void (*debugHook)(void *user, const QPointF &releaseVelocity, const QPointF &position, const QPointF &overshootPosition);
    void *debugHookUser;

    QKineticScroller *q_ptr;
};

QT_END_NAMESPACE
