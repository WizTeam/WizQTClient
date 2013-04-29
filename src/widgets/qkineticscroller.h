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

#ifndef QKINETICSCROLLER_H
#define QKINETICSCROLLER_H

#include <QtCore/qmetatype.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QKineticScrollerPrivate;

class QKineticScroller
{
public:
    virtual ~QKineticScroller();

    bool isEnabled() const;
    void setEnabled(bool b);

    enum State
    {
        StateInactive,
        StatePressed,
        StateDragging,
        StateScrolling,
    };

    State state() const;
    void reset();

    enum OvershootPolicy
    {
        OvershootWhenScrollable,
        OvershootAlwaysOff,
        OvershootAlwaysOn,
    };

    OvershootPolicy horizontalOvershootPolicy() const;
    void setHorizontalOvershootPolicy(OvershootPolicy policy);
    OvershootPolicy verticalOvershootPolicy() const;
    void setVerticalOvershootPolicy(OvershootPolicy policy);

    enum ScrollMetric
    {
        DragVelocitySmoothingFactor,             // qreal [0..1/s]  (complex calculation involving time) v = v_new* DASF + v_old * (1-DASF)

        LinearDecelerationFactor,                // qreal [m/s^2]
        ExponentialDecelerationBase,             // qreal
        OvershootSpringConstant,                 // qreal [kg/s^2]
        OvershootDragResistanceFactor,           // qreal [0..1]
        OvershootMaximumDistance,                // QPointF([m], [m])

        DragStartDistance,                       // qreal [m]
        DragStartDirectionErrorMargin,           // qreal [m]

        MinimumVelocity,                         // qreal [m/s]
        MaximumVelocity,                         // qreal [m/s]
        MaximumNonAcceleratedVelocity,           // qreal [m/s]

        MaximumClickThroughVelocity,             // qreal [m/s]
        AxisLockThreshold,                       // qreal [0..1] atan(|min(dx,dy)|/|max(dx,dy)|)

        FramesPerSecond,                         // int [frames/s]

        FastSwipeMaximumTime,                    // qreal [s]
        FastSwipeMinimumVelocity,                // qreal [m/s]
        FastSwipeBaseVelocity,                   // qreal [m/s]

        ScrollMetricCount
    };

    QVariant scrollMetric(ScrollMetric metric) const;
    void setScrollMetric(ScrollMetric metric, const QVariant &value);
    void resetScrollMetrics();

    void scrollTo(const QPointF &pos, int scrollTime = 1000);
    void ensureVisible(const QPointF &pos, int xmargin, int ymargin, int scrollTime = 1000);

    qreal dpi() const;
    void setDpi(qreal dpi);
    void setDpiFromWidget(QWidget *widget);

    void registerDebugHook(void (*callback)(void *user, const QPointF &releaseVelocity, const QPointF &position, const QPointF &overshootPosition), void *user);

protected:
    explicit QKineticScroller();

    virtual QSizeF viewportSize() const = 0;
    virtual QPointF maximumContentPosition() const = 0;
    virtual QPointF contentPosition() const = 0;
    virtual void setContentPosition(const QPointF &pos, const QPointF &overshootDelta) = 0;

    virtual void stateChanged(State oldState);
    virtual bool canStartScrollingAt(const QPointF &pos) const;
    virtual void cancelPress(const QPointF &pressPos);

    enum Input {
        InputPress,
        InputMove,
        InputRelease
    };

    bool handleInput(Input input, const QPointF &position, qint64 timestamp);

    QKineticScroller(QKineticScrollerPrivate &dd);
    QScopedPointer<QKineticScrollerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QKineticScroller)
    Q_DECLARE_PRIVATE(QKineticScroller)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QKineticScroller *)

QT_END_HEADER

#endif // QKINETICSCROLLER_H
