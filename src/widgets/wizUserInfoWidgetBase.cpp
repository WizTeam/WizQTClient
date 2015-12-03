#include "wizUserInfoWidgetBase.h"

//#ifndef Q_OS_MAC

#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include "utils/stylehelper.h"
#ifdef Q_OS_MAC
#include "mac/wizmachelper.h"
#endif

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
    rectIcon.setTop(rectIcon.top() + (rectIcon.height() - nAvatarWidth) / 2);
    rectIcon.setHeight(nAvatarWidth);

#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0);
    nAvatarWidth *= factor;
#endif
    //
    QPixmap pixmap = getAvatar(nAvatarWidth, nAvatarWidth);
    Utils::StyleHelper::drawPixmapWithScreenScaleFactor(&p, rectIcon, pixmap);    

    // draw display name
    QRect rectText = rectIcon;
    rectText.setLeft(rectText.right() + nMargin);
    rectText.setRight(rectText.left() + fontMetrics().width(opt.text));
//    rectText.setBottom(rectText.top() + rectText.height()/2);
    if (!opt.text.isEmpty()) {
        if (opt.state & QStyle::State_MouseOver) {
            QFont font = p.font();
            font.setUnderline(true);
            p.setFont(font);
        }

        p.setPen("#787878"); // FIXME
        p.drawText(rectText, Qt::AlignLeft|Qt::AlignVCenter, opt.text);
    }

    // draw vip indicator
    QRect rectVip = rectText;
    QIcon iconVip = getVipIcon();
    QSize iconSize(16, 16);
    if (iconVip.availableSizes().size() != 0)
    {
        iconSize = iconVip.availableSizes().first();
    }
    rectVip.setLeft(rectVip.right() + nMargin);
    rectVip.setRight(rectVip.left() + iconSize.width());
//    rectVip.setBottom(rectVip.top() + rectVip.height()/2);
//    rectVip.setTop(rectVip.top() + (rectVip.height() - iconSize.height()) / 2);
    if (!iconVip.isNull()) {
        iconVip.paint(&p, rectVip, Qt::AlignLeft|Qt::AlignVCenter);
    }

    // draw arraw
    QRect rectArrow = rectVip;
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
        //QPoint pos(event->pos().x(), sizeHint().height());
        // FIXME
        QPoint pos(32 + 4, 32 - fontMetrics().height() / 2);
        menu()->popup(mapToGlobal(pos), defaultAction());
    }
}

bool CWizUserInfoWidgetBase::hitButton(const QPoint& pos) const
{
    // FIXME
    QRect rectArrow(32 + 8, 32 - fontMetrics().height() - 4, sizeHint().width() - 32 - 4, fontMetrics().height());
    return rectArrow.contains(pos) ? true : false;
}

int CWizUserInfoWidgetBase::textWidth() const
{
    return fontMetrics().width(text());
}
void CWizUserInfoWidgetBase::updateUI()
{
    update();
}

//#endif //Q_OS_MAC

