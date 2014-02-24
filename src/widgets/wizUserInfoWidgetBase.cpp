#include "wizUserInfoWidgetBase.h"

#ifndef Q_OS_MAC

#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>

CWizUserInfoWidgetBase::CWizUserInfoWidgetBase(QWidget *parent)
    : QToolButton(parent)
{
    setPopupMode(QToolButton::MenuButtonPopup);
}


void CWizUserInfoWidgetBase::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    int nAvatarWidth = 32;
    int nArrawWidth = 10;
    int nMargin = 4;

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setClipRect(opt.rect);

    // draw user avatar
    QRect rectIcon = opt.rect;
    rectIcon.setLeft(rectIcon.left());
    rectIcon.setRight(rectIcon.left() + nAvatarWidth);

    QPixmap pixmap = getAvatar(nAvatarWidth, nAvatarWidth);
    if (!pixmap.isNull())
    {
        p.drawPixmap(rectIcon, pixmap);
    }
    //if (!opt.icon.isNull()) {
    //    opt.icon.paint(&p, rectIcon);
    //}

    // draw vip indicator
    QRect rectVip = rectIcon;
    rectVip.setLeft(rectVip.right() + nMargin);
    rectVip.setRight(rectVip.left() + fontMetrics().width(opt.text));
    //rectVip.setBottom(rectVip.top() + rectVip.height()/2);
    //if (!m_iconVipIndicator.isNull()) {
    //    m_iconVipIndicator.paint(&p, rectVip, Qt::AlignLeft|Qt::AlignTop);
    //}

    // draw display name
    QRect rectText = rectVip;
    //rectText.setBottom(rectText.bottom() + rectVip.height());
    //rectText.setTop(rectText.bottom() - rectVip.height());
    if (!opt.text.isEmpty()) {
        if (opt.state & QStyle::State_MouseOver) {
            QFont font = p.font();
            font.setUnderline(true);
            p.setFont(font);
        }

        p.setPen("#787878"); // FIXME
        p.drawText(rectText, Qt::AlignLeft|Qt::AlignVCenter, opt.text);
    }

    // draw arraw
    QRect rectArrow = rectText;
    rectArrow.setLeft(rectArrow.right() + nMargin);
    rectArrow.setRight(rectArrow.left() + nArrawWidth);
    QIcon arrow = getArrow();
    if (!arrow.isNull()) {
        arrow.paint(&p, rectArrow, Qt::AlignVCenter, QIcon::Normal);
    }
}

void CWizUserInfoWidgetBase::mousePressEvent(QMouseEvent* event)
{
    // show menu at proper position
    if (hitButton(event->pos())) {
        QPoint pos(event->pos().x(), sizeHint().height());
        menu()->popup(mapToGlobal(pos), defaultAction());
    }
}

bool CWizUserInfoWidgetBase::hitButton(const QPoint& pos) const
{
    // FIXME
    QRect rectArrow(32 + 8, 32 - fontMetrics().height(), sizeHint().width() - 32 - 4, fontMetrics().height());
    return rectArrow.contains(pos) ? true : false;
}

int CWizUserInfoWidgetBase::textWidth() const
{
    return fontMetrics().width(text());
}
void CWizUserInfoWidgetBase::updateUI()
{
}

#endif //Q_OS_MAC

