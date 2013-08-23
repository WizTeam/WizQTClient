#include <QtGui/QIcon>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QStyleOption>
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>
#include <QtGui/QPixmapCache>
#include <QStyle>

#include "widgets/qsegmentcontrol.h"

//#ifndef Q_WS_MAC
//#include <QMacStyle>
//#include <Carbon/Carbon.h>
//
//extern CGContextRef qt_mac_cg_context(const QPaintDevice *);
//
//static ThemeDrawState getDrawState(QStyle::State flags)
//{
//    ThemeDrawState tds = kThemeStateActive;
//    if (flags & QStyle::State_Sunken) {
//        tds = kThemeStatePressed;
//    } else if (flags & QStyle::State_Active) {
//        if (!(flags & QStyle::State_Enabled))
//            tds = kThemeStateUnavailable;
//    } else {
//        if (flags & QStyle::State_Enabled)
//            tds = kThemeStateInactive;
//        else
//            tds = kThemeStateUnavailableInactive;
//    }
//    return tds;
//}
//#endif


class QtStyleOptionSegmentControlSegment : public QStyleOption
{
public:
    enum StyleOptionType { Type = 100000  };
    enum StyleOptionVersion { Version = 1 };

    enum SegmentPosition { Beginning, Middle, End, OnlyOneSegment };
    enum SelectedPosition { NotAdjacent, NextIsSelected, PreviousIsSelected };

    QString text;
    QIcon icon;
    QSize iconSize;
    int iconMargin;
    SegmentPosition position;
    SelectedPosition selectedPositions;

    QtStyleOptionSegmentControlSegment()
        : position(OnlyOneSegment), selectedPositions(NotAdjacent), iconMargin(5) { }
    QtStyleOptionSegmentControlSegment(const QtStyleOptionSegmentControlSegment &other)
        : QStyleOption(Version, Type) { *this = other; }

protected:
    QtStyleOptionSegmentControlSegment(int version);
};



static void drawSegmentControlSegmentSegment(const QStyleOption *option, QPainter *painter, QWidget *widget)
{
    // ### Change to qstyleoption_cast!
    if (const QtStyleOptionSegmentControlSegment *segment
            = static_cast<const QtStyleOptionSegmentControlSegment *>(option)) {
//#ifndef Q_WS_MAC
//        if (qobject_cast<QMacStyle *>(widget->style())) {
//            CGContextRef cg = qt_mac_cg_context(painter->device());
//            HIThemeSegmentDrawInfo sgi;
//            bool selected = (segment->state & QStyle::State_Selected);
//            sgi.version = 0;
//            // Things look the same regardless of enabled.
//            sgi.state = getDrawState(segment->state | QStyle::State_Enabled);
//            sgi.value = selected ? kThemeButtonOn : kThemeButtonOff;
//            sgi.size = kHIThemeSegmentSizeNormal;
//            sgi.kind = kHIThemeSegmentKindNormal;
//            sgi.adornment = kHIThemeSegmentAdornmentNone;
//            switch (segment->position) {
//            case QtStyleOptionSegmentControlSegment::Beginning:
//                sgi.position = kHIThemeSegmentPositionFirst;
//                if (segment->selectedPositions == QtStyleOptionSegmentControlSegment::NotAdjacent
//                    || selected)
//                    sgi.adornment |= kHIThemeSegmentAdornmentTrailingSeparator;
//                break;
//            case QtStyleOptionSegmentControlSegment::Middle:
//                sgi.position = kHIThemeSegmentPositionMiddle;
//                if (selected && !(segment->selectedPositions & QtStyleOptionSegmentControlSegment::PreviousIsSelected))
//                    sgi.adornment |= kHIThemeSegmentAdornmentLeadingSeparator;
//                if (selected || !(segment->selectedPositions & QtStyleOptionSegmentControlSegment::NextIsSelected)) // Also when we're selected.
//                    sgi.adornment |= kHIThemeSegmentAdornmentTrailingSeparator;
//                break;
//            case QStyleOptionTab::End:
//                sgi.position = kHIThemeSegmentPositionLast;
//                if (selected && !(segment->selectedPositions & QtStyleOptionSegmentControlSegment::PreviousIsSelected))
//                    sgi.adornment |= kHIThemeSegmentAdornmentLeadingSeparator;
//                break;
//            case QStyleOptionTab::OnlyOneTab:
//                sgi.position = kHIThemeSegmentPositionOnly;
//                break;
//            }
//
//            HIRect hirect = CGRectMake(segment->rect.x(), segment->rect.y(),
//                                       segment->rect.width(), segment->rect.height());
//            HIThemeDrawSegment(&hirect, &sgi, cg, kHIThemeOrientationNormal);
//            CGContextRelease(cg);
//        } else
//#endif
        {
            Q_UNUSED(widget);
            painter->save();

            bool selected = (segment->state & QStyle::State_Selected);

            QPixmap pm;

            int nIconMargin = segment->iconMargin;

            QSize buttonSize = widget->rect().size();
            QString key = QString("qt_segment %0 %1 %2").arg(option->state).arg(buttonSize.width()).arg(buttonSize.height());

            if (!QPixmapCache::find(key, pm)) {
                pm = QPixmap(buttonSize);
                pm.fill(Qt::transparent);
                QPainter pmPainter(&pm);
                QStyleOptionButton btnOpt;
                btnOpt.QStyleOption::operator =(*option);
                btnOpt.state &= ~QStyle::State_HasFocus;
                btnOpt.rect = QRect(QPoint(0, 0), buttonSize);
                btnOpt.state = option->state;

                if (selected)
                    btnOpt.state |= QStyle::State_Sunken;
                else
                    btnOpt.state |= QStyle::State_Raised;

                // draw border
                //QRect arcRect(btnOpt.rect.topLeft(), QSize(btnOpt.rect.height(), btnOpt.rect.height()));
                //pmPainter.drawArc(arcRect, 115 * 16, 130 * 16);

                //QLine line(btnOpt.rect.left() + nArcWidth, btnOpt.rect.top(),
                //           btnOpt.rect.right() - nArcWidth, btnOpt.rect.top());
                //pmPainter.drawLine(line);
                //pmPainter.drawLine(line.translated(0, btnOpt.iconSize.height() + nMargin * 2));

                //arcRect.moveRight((btnOpt.iconSize.width() + nSpace * 2) * m_cells.size() + nArcWidth * 2);
                //p.drawArc(arcRect, 65 * 16, -130 * 16);

                widget->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &btnOpt, &pmPainter, widget);

                pmPainter.end();
                QPixmapCache::insert(key, pm);
            }

            //int margin = widget->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, widget);
            switch (segment->position) {
            case QtStyleOptionSegmentControlSegment::Beginning:
                painter->setClipRect(option->rect);
                painter->drawPixmap(0, 0, pm);
                painter->setOpacity(0.6);
                painter->setPen(option->palette.dark().color());
                //painter->drawLine(option->rect.topRight() + QPoint(-1, margin), option->rect.bottomRight() + QPoint(-1, -margin));
                painter->drawLine(option->rect.topRight() + QPoint(-1, nIconMargin), option->rect.bottomRight() + QPoint(-1, -nIconMargin));
                break;
            case QtStyleOptionSegmentControlSegment::Middle:
                painter->setClipRect(option->rect);
                painter->drawPixmap(0, 0, pm);
                painter->setPen(option->palette.dark().color());
                painter->drawLine(option->rect.topRight() + QPoint(-1, nIconMargin), option->rect.bottomRight() + QPoint(-1, -nIconMargin));
                //painter->drawLine(option->rect.topRight() + QPoint(-1, margin), option->rect.bottomRight() + QPoint(-1, -margin));
                break;
            case QStyleOptionTab::End:
                painter->setClipRect(option->rect);
                painter->drawPixmap(0, 0, pm);
                break;
            case QStyleOptionTab::OnlyOneTab:
                painter->setClipRect(option->rect);
                painter->drawPixmap(0, 0, pm);
                break;
            }
            painter->restore();
        }
    }
}

static QSize segmentSizeFromContents(const QStyleOption *option, const QSize &contentSize)
{
    QSize ret = contentSize;
    if (const QtStyleOptionSegmentControlSegment *segment
            = static_cast<const QtStyleOptionSegmentControlSegment *>(option)) {
        if (!segment->icon.isNull()) {
            ret.rwidth() += (segment->iconMargin * 2);
            ret.rheight() += (segment->iconMargin * 2);
        }
    }
    return ret;
}

static void drawSegmentControlSegmentLabel(const QStyleOption *option, QPainter *painter, QWidget *widget)
{
    if (const QtStyleOptionSegmentControlSegment *segment
            = static_cast<const QtStyleOptionSegmentControlSegment *>(option)) {
//#ifndef Q_WS_MAC
//        if (qobject_cast<QMacStyle *>(widget->style())) {
//            QRect retRect = option->rect;
//            retRect.adjust(+11, +4, -11, -6);
//            switch (segment->position) {
//            default:
//            case QtStyleOptionSegmentControlSegment::Middle:
//                break;
//            case QtStyleOptionSegmentControlSegment::Beginning:
//            case QtStyleOptionSegmentControlSegment::End:
//                retRect.adjust(+1, 0, -1, 0);
//                break;
//            case QtStyleOptionSegmentControlSegment::OnlyOneSegment:
//                retRect.adjust(+2, 0, -2, 0);
//                break;
//            }
//        }
//#endif
        QStyleOptionButton btn;
        btn.QStyleOption::operator=(*option);
        btn.text = segment->text;
        btn.icon = segment->icon;
        btn.iconSize = segment->iconSize;

        QStyleOptionButton *button = &btn;

        uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
        QRect textRect = button->rect;
        if (!button->icon.isNull()) {
            //Center both icon and text
            QRect iconRect;
            QIcon::Mode mode = button->state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
            if (mode == QIcon::Normal && button->state & QStyle::State_Sunken)
                mode = QIcon::Active;
            QIcon::State state = QIcon::Off;
            if (button->state & QStyle::State_On)
                state = QIcon::On;

            QPixmap pixmap = button->icon.pixmap(button->iconSize, mode, state);
            int labelWidth = pixmap.width();
            int labelHeight = pixmap.height();
            int iconSpacing = 0; //### 4 is currently hardcoded in QPushButton::sizeHint()
            int textWidth = button->fontMetrics.boundingRect(button->rect, tf, button->text).width();
            if (!button->text.isEmpty())
                labelWidth += (textWidth + iconSpacing);

            iconRect = QRect(textRect.x() + (textRect.width() - labelWidth) / 2,
                             textRect.y() + (textRect.height() - labelHeight) / 2,
                             pixmap.width(), pixmap.height());

            iconRect = QStyle::visualRect(button->direction, textRect, iconRect);

            tf |= Qt::AlignLeft; //left align, we adjust the text-rect instead

            if (button->direction == Qt::RightToLeft)
                textRect.setRight(iconRect.left() - iconSpacing);
            else
                textRect.setLeft(iconRect.left() + iconRect.width() + iconSpacing);

            if (button->state & (QStyle::State_On | QStyle::State_Sunken))
                iconRect.translate(widget->style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, button, widget),
                                   widget->style()->pixelMetric(QStyle::PM_ButtonShiftVertical, button, widget));
            painter->drawPixmap(iconRect, pixmap);
        } else {
            tf |= Qt::AlignHCenter;
        }

        if (button->state & (QStyle::State_On | QStyle::State_Sunken))
            textRect.translate(widget->style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, button, widget),
                         widget->style()->pixelMetric(QStyle::PM_ButtonShiftVertical, button, widget));

        if (button->features & QStyleOptionButton::HasMenu) {
            int indicatorSize = widget->style()->pixelMetric(QStyle::PM_MenuButtonIndicator, button, widget);
            if (button->direction == Qt::LeftToRight)
                textRect = textRect.adjusted(0, 0, -indicatorSize, 0);
            else
                textRect = textRect.adjusted(indicatorSize, 0, 0, 0);
        }
        widget->style()->drawItemText(painter, textRect, tf, button->palette, (button->state & QStyle::State_Enabled),
                                      button->text, QPalette::ButtonText);

        //widget->style()->drawControl(QStyle::CE_PushButtonLabel, button, painter, widget);
    }

}

static void drawSegmentControlFocusRect(const QStyleOption *option, QPainter *painter, QWidget *widget)
{
    QStyleOptionFocusRect focusOpt;
    focusOpt.QStyleOption::operator =(*option);
    focusOpt.rect.adjust(2, 2, -2, -2); //use subcontrolrect for this
    widget->style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpt, painter, widget);
}

static void drawSegmentControlSegment(const QStyleOption *option,
                                      QPainter *painter, QWidget *widget)
{
    drawSegmentControlSegmentSegment(option, painter, widget);
    drawSegmentControlSegmentLabel(option, painter, widget);
    //if (option->state & QStyle::State_HasFocus)
    //    drawSegmentControlFocusRect(option, painter, widget);
}


struct SegmentInfo {
    SegmentInfo() : menu(0), selected(false), enabled(true) {}
    ~SegmentInfo() { delete menu; }
    QString text;
    QString toolTip;
    QString whatsThis;
    QIcon icon;
    QMenu *menu;
    bool selected;
    bool enabled;
    QRect rect;
};

class QtSegmentControlPrivate {
public:
    QtSegmentControlPrivate(QtSegmentControl *myQ)
        : q(myQ), lastSelected(-1), layoutDirty(true), pressedIndex(-1), wasPressed(-1), focusIndex(-1) {}
    ~QtSegmentControlPrivate() {}

    void layoutSegments();
    void postUpdate(int index = -1, bool geoToo = false);

    QtSegmentControl *q;
    QtSegmentControl::SelectionBehavior selectionBehavior;
    QSize iconSize;
    QVector<SegmentInfo> segments;
    int lastSelected;
    bool layoutDirty;
    int pressedIndex;
    int wasPressed;
    int focusIndex;
    inline bool validIndex(int index) { return index >= 0 && index < segments.count(); }
};

void QtSegmentControlPrivate::layoutSegments()
{
    if (!layoutDirty)
        return;
    const int segmentCount = segments.count();
    QRect rect;
    for (int i = 0; i < segmentCount; ++i) {
        QSize ssh = q->segmentSizeHint(i);
        rect.setSize(ssh);
        segments[i].rect = rect;
        rect.setLeft(rect.left() + ssh.width());
    }
    layoutDirty = false;
}

void QtSegmentControlPrivate::postUpdate(int /*index*/, bool geoToo)
{
    if (geoToo) {
        layoutDirty = true;
        q->updateGeometry();
    }
    q->update();
}

QtSegmentControl::QtSegmentControl(QWidget *parent)
    : QWidget(parent), d(new QtSegmentControlPrivate(this))
{
    setFocusPolicy(Qt::TabFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

QtSegmentControl::~QtSegmentControl()
{
    delete d;
}

int QtSegmentControl::count() const
{
    return d->segments.count();
}

void QtSegmentControl::setCount(int newCount)
{
    d->segments.resize(newCount);

    // If current index is not valid, make it the first valid index
    if (!d->validIndex(d->focusIndex)) {
        for (int i = 0; i < newCount; ++i) {
            if (d->validIndex(i) && d->segments[i].enabled) {
                d->focusIndex = i;
                break;
            }
        }
    }
}

bool QtSegmentControl::isSegmentSelected(int index) const
{
    if (!d->validIndex(index))
        return false;

    return d->segments.at(index).selected;
}

int QtSegmentControl::selectedSegment() const
{
    return d->lastSelected;
}

void QtSegmentControl::setSegmentSelected(int index, bool selected)
{
    if (!d->validIndex(index))
        return;

    if (d->segments[index].selected != selected) {
        d->segments[index].selected = selected;
        d->lastSelected = index;
        if (d->selectionBehavior == SelectOne) {
            const int segmentCount = d->segments.count();
            for (int i = 0; i < segmentCount; ++i) {
                SegmentInfo &info = d->segments[i];
                if (i != index && info.selected) {
                    info.selected = false;
                    d->postUpdate(i);
                }
            }
        }
        d->postUpdate(index);
        emit segmentSelected(index);
    }
}

void QtSegmentControl::setSegmentEnabled(int index, bool enabled)
{
    if (!d->validIndex(index))
        return;

    if (d->segments[index].enabled != enabled) {
        d->segments[index].enabled = enabled;
        d->postUpdate(index);
    }
}

void QtSegmentControl::setSelectionBehavior(SelectionBehavior behavior)
{
    if (d->selectionBehavior == behavior)
        return;
    d->selectionBehavior = behavior;
    if (behavior == SelectOne) {
        // This call will do the right thing.
        setSegmentSelected(d->lastSelected, true);
    } else if (behavior == SelectNone) {
        d->lastSelected = -1;
        const int segmentCount = d->segments.count();
        for (int i = 0; i < segmentCount; ++i) {
            d->segments[i].selected = false;
        }
        d->postUpdate(-1);
    }
}

QtSegmentControl::SelectionBehavior QtSegmentControl::selectionBehavior() const
{
    return d->selectionBehavior;
}

void QtSegmentControl::setSegmentText(int index, const QString &text)
{
    if (!d->validIndex(index))
        return;

    if (d->segments[index].text != text) {
        d->segments[index].text = text;
        d->postUpdate(index, true);
    }
}

bool QtSegmentControl::segmentEnabled(int index) const
{
    if (d->validIndex(index))
        return d->segments[index].enabled;
    return false;
}

QString QtSegmentControl::segmentText(int index) const
{
    return d->validIndex(index) ? d->segments.at(index).text : QString();
}

void QtSegmentControl::setSegmentIcon(int index, const QIcon &icon)
{
    if (!d->validIndex(index))
        return;

    d->segments[index].icon = icon;
    d->postUpdate(index, true);
}

QIcon QtSegmentControl::segmentIcon(int index) const
{
    return d->validIndex(index) ? d->segments.at(index).icon : QIcon();
}

void QtSegmentControl::setIconSize(const QSize &size)
{
    if (d->iconSize == size)
        return;

    d->iconSize = size;
    d->postUpdate(-1, true);
}

QSize QtSegmentControl::iconSize() const
{
    return d->iconSize;
}

void QtSegmentControl::setSegmentMenu(int index, QMenu *menu)
{
    if (!d->validIndex(index))
        return;

    if (menu != d->segments[index].menu) {
        QMenu *oldMenu = d->segments[index].menu;
        d->segments[index].menu = menu;
        delete oldMenu;
        d->postUpdate(index, true);
    }
}

QMenu *QtSegmentControl::segmentMenu(int index) const
{
    return d->validIndex(index) ? d->segments.at(index).menu : 0;
}

void QtSegmentControl::setSegmentToolTip(int segment, const QString &tipText)
{
    if (!d->validIndex(segment))
        return;

    d->segments[segment].toolTip = tipText;
}

QString QtSegmentControl::segmentToolTip(int segment) const
{
    return d->validIndex(segment) ? d->segments.at(segment).toolTip : QString();
}

void QtSegmentControl::setSegmentWhatsThis(int segment, const QString &whatsThisText)
{
    if (!d->validIndex(segment))
        return;

    d->segments[segment].whatsThis = whatsThisText;
}

QString QtSegmentControl::segmentWhatsThis(int segment) const
{
    return d->validIndex(segment) ? d->segments.at(segment).whatsThis : QString();
}

QSize QtSegmentControl::segmentSizeHint(int segment) const
{
    QSize size;
    const SegmentInfo &segmentInfo = d->segments[segment];
    QFontMetrics fm(font());
    size = fm.size(0, segmentInfo.text);
    if (!segmentInfo.icon.isNull()) {
        QSize size2 = segmentInfo.icon.actualSize(iconSize());
        size.rwidth() += size2.width();
        size.rheight() = qMax(size.height(), size2.height());
    }
    QtStyleOptionSegmentControlSegment opt;
    opt.initFrom(this);
    opt.text = segmentInfo.text;
    opt.icon = segmentInfo.icon;
    opt.iconSize = d->iconSize;
    size = segmentSizeFromContents(&opt, size);
    return size;
}

QSize QtSegmentControl::sizeHint() const
{
    d->layoutSegments();

    QRect rect;
    const int segmentCount = d->segments.count();
    for (int i = 0; i < segmentCount; ++i) {
        rect = rect.united(segmentRect(i));
    }
    return rect.size();
}

QRect QtSegmentControl::segmentRect(int index) const
{
    return d->validIndex(index) ? d->segments[index].rect : QRect();
}

int QtSegmentControl::segmentAt(const QPoint &pos) const
{
    const int segmentCount = d->segments.count();
    for (int i = 0; i < segmentCount; ++i) {
        QRect rect = segmentRect(i);
        if (rect.contains(pos))
            return i;
    }
    return -1;
}

void QtSegmentControl::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Left && event->key() != Qt::Key_Right
        && event->key() != Qt::Key_Space) {
        event->ignore();
        return;
    }

    if (event->key() == Qt::Key_Space) {
        d->pressedIndex = d->focusIndex = d->focusIndex;
        d->postUpdate(d->wasPressed);
    } else {
        int dx = event->key() == (isRightToLeft() ? Qt::Key_Right : Qt::Key_Left) ? -1 : 1;
        for (int index = d->focusIndex + dx; d->validIndex(index); index += dx) {
            if (d->segments[index].enabled) {
                d->focusIndex = index;
                update();
                break;
            }
        }
    }
}

void QtSegmentControl::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        int index = d->pressedIndex;
        if (d->selectionBehavior != SelectNone) {
            if (d->selectionBehavior == SelectAll) {
                setSegmentSelected(index, !d->segments[index].selected);
            } else {
                setSegmentSelected(index, true);
            }
        }
        d->postUpdate(index);
        d->pressedIndex = -1;
    }
    QWidget::keyReleaseEvent(event);
}


void QtSegmentControl::paintEvent(QPaintEvent *)
{
    d->layoutSegments();
    QPainter p(this);
    QtStyleOptionSegmentControlSegment segmentInfo;
    const int segmentCount = d->segments.count();
    for (int i = 0; i < segmentCount; ++i) {
        initStyleOption(i, &segmentInfo);
        drawSegmentControlSegment(&segmentInfo, &p, this);
    }
}

void QtSegmentControl::mousePressEvent(QMouseEvent *event)
{
    int index = segmentAt(event->pos());
    if (segmentEnabled(index)) {
        d->wasPressed = d->focusIndex = d->pressedIndex = segmentAt(event->pos());
        d->postUpdate(d->pressedIndex);

        Q_EMIT segmentClicked(index);
    }
}

void QtSegmentControl::mouseMoveEvent(QMouseEvent *event)
{
    int index = segmentAt(event->pos());
    if (index != d->wasPressed) {
        d->pressedIndex = -1;
        d->postUpdate(d->wasPressed);
    } else if (index == d->wasPressed && d->pressedIndex == -1) {
        d->pressedIndex = d->wasPressed;
        d->postUpdate(d->wasPressed);
    }
}

void QtSegmentControl::mouseReleaseEvent(QMouseEvent *event)
{
    int index = segmentAt(event->pos());
    // This order of reset is important.
    d->pressedIndex = -1;
    if (index == d->wasPressed && d->selectionBehavior != SelectNone) {
        if (d->selectionBehavior == SelectAll) {
            setSegmentSelected(index, !d->segments[index].selected);
        } else {
            setSegmentSelected(index, true);
        }
    }
    d->postUpdate(index);
    d->wasPressed = -1;
}

bool QtSegmentControl::event(QEvent *event)
{
    return QWidget::event(event);
}

void QtSegmentControl::initStyleOption(int segment, QStyleOption *option) const
{
    if (!option || !d->validIndex(segment))
        return;

    option->initFrom(this);

    if (segment == d->pressedIndex)
        option->state |= QStyle::State_Sunken;

    // ## Change to qstyleoption_cast
    if (QtStyleOptionSegmentControlSegment *sgi = static_cast<QtStyleOptionSegmentControlSegment *>(option)) {
        sgi->iconSize = d->iconSize;
        const SegmentInfo &segmentInfo = d->segments[segment];
        if (d->segments.count() == 1) {
            sgi->position = QtStyleOptionSegmentControlSegment::OnlyOneSegment;
        } else if (segment == 0) {
            sgi->position = QtStyleOptionSegmentControlSegment::Beginning;
        } else if (segment == d->segments.count() - 1) {
            sgi->position = QtStyleOptionSegmentControlSegment::End;
        } else {
            sgi->position = QtStyleOptionSegmentControlSegment::Middle;
        }

        if (hasFocus() && segment == d->focusIndex)
            sgi->state |= QStyle::State_HasFocus;
        else
            sgi->state &= ~QStyle::State_HasFocus;

        if (segmentInfo.enabled && isEnabled())
            sgi->state |= QStyle::State_Enabled;
        else
            sgi->state &= ~QStyle::State_Enabled;

        if (segmentInfo.selected) {
            sgi->state |= QStyle::State_Selected;
        } else {
            if (d->validIndex(segment - 1) && d->segments[segment - 1].selected) {
                sgi->selectedPositions = QtStyleOptionSegmentControlSegment::PreviousIsSelected;
            } else if (d->validIndex(segment + 1) && d->segments[segment + 1].selected) {
                sgi->selectedPositions = QtStyleOptionSegmentControlSegment::NextIsSelected;
            } else {
                sgi->selectedPositions = QtStyleOptionSegmentControlSegment::NotAdjacent;
            }
        }
        sgi->rect = segmentInfo.rect;
        sgi->text = segmentInfo.text;
        sgi->icon = segmentInfo.icon;
    }
}
