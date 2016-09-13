#include "WizSegmentedButton.h"

#include <QStylePainter>
#include <QStyleOptionButton>

WizSegmentedButton::WizSegmentedButton(QWidget *parent)
    : QPushButton(parent)
    , m_nMargin(5)
    , m_nArcWidth(8)
    , m_nSpace(8)
{
    setIconSize(QSize(16, 16));
}

QSize WizSegmentedButton::sizeHint() const
{
    // add 2 pixel for drawing
    return QSize(m_cells.size() * (iconSize().width() + m_nSpace * 2) + m_nArcWidth * 2 + 2,
                 iconSize().height() + m_nMargin * 2);
}

void WizSegmentedButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionButton opt;
    initStyleOption(&opt);

    // FIXME
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen("#747474");

    // draw border first
    QRect arcRect(opt.rect.topLeft(), QSize(opt.rect.height(), opt.rect.height()));
    p.drawArc(arcRect, 115 * 16, 130 * 16);

    QLine line(opt.rect.left() + m_nArcWidth, opt.rect.top(),
               opt.rect.right() - m_nArcWidth, opt.rect.top());
    p.drawLine(line);
    p.drawLine(line.translated(0, opt.iconSize.height() + m_nMargin * 2));

    arcRect.moveRight((opt.iconSize.width() + m_nSpace * 2) * m_cells.size() + m_nArcWidth * 2);
    p.drawArc(arcRect, 65 * 16, -130 * 16);

    QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    if (mode == QIcon::Normal && opt.state & QStyle::State_Sunken)
        mode = QIcon::Active;
    QIcon::State state = QIcon::Off;
    if (opt.state & QStyle::State_On)
        state = QIcon::On;

    QRect iconsRect = opt.rect.adjusted(m_nArcWidth, 0, -m_nArcWidth, 0);
    for (int i = 0; i < m_cells.size(); i++) {
        QRect iconRect = iconsRect;
        int x1 = iconRect.left() + i * (opt.iconSize.width() + m_nSpace * 2) + m_nSpace;
        int y1 = iconRect.top() + m_nMargin;
        int x2 = x1 + opt.iconSize.width();
        int y2 = iconRect.bottom() - m_nMargin;
        iconRect.setCoords(x1, y1, x2, y2);

        QPixmap pixmap = m_cells.at(i).pixmap(opt.iconSize, mode, state);
        p.drawPixmap(iconRect, pixmap);

        // draw line
        if (i < m_cells.size() - 1) {
            QLine line(iconRect.topRight(), iconRect.bottomRight());
            p.drawLine(line.translated(m_nSpace, 0));
        }
    }

    //QColor dark;
    //QColor button  = opt.palette.button().color();
    //dark.setHsv(button.hue(),
    //            qMin(255, (int)(button.saturation()*1.9)),
    //            qMin(255, (int)(button.value()*0.7)));

    //if (opt.state & QStyle::State_Sunken) {
    //    QRect rect = iconsRect;
    //    p.setBackgroundMode(Qt::TransparentMode);
    //    p.setBrush(QBrush(dark.darker(120), Qt::Dense4Pattern));
    //    p.setBrushOrigin(rect.topLeft());
    //    p.setPen(Qt::NoPen);
    //    const QRect rects[4] = {
    //        QRect(rect.left(), rect.top(), rect.width(), 1),    // Top
    //        QRect(rect.left(), rect.bottom(), rect.width(), 1), // Bottom
    //        QRect(rect.left(), rect.top(), 1, rect.height()),   // Left
    //        QRect(rect.right(), rect.top(), 1, rect.height())   // Right
    //    };
    //    p.drawRects(rects, 4);
    //}
}

void WizSegmentedButton::setCellSpacing(int nSpacing)
{
    m_nSpace = nSpacing;
}

void WizSegmentedButton::addCell(const QIcon& icon)
{
    m_cells.append(icon);
    update();
}
