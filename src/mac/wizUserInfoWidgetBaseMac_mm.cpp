#include "wizUserInfoWidgetBaseMac_mm.h"
#ifdef USECOCOATOOLBAR

#include <QPainter>
#include "sync/avataruploader.h"
#include "sync/avatar.h"
#include "utils/stylehelper.h"


using namespace WizService;
using namespace WizService::Internal;

QString CWizUserInfoWidgetBaseMac::text() const
{
    return m_text;
}
void CWizUserInfoWidgetBaseMac::setText(QString val)
{
    m_text = val;
    m_textWidth = 0;
    calTextSize();
}

void CWizToolButtonWidget::buttonClicked()
{
    emit triggered(false);
}

QSize CWizToolButtonWidget::sizeHint() const
{
    //FIXME: hardcode
    return QSize(106, 36);
}

QPixmap CWizToolButtonWidget::getBackgroundImage(bool selected)
{
//    if (selected)
//    {
//        if (!m_backgroundSelected.isNull())
//            return m_backgroundSelected;

//        QPixmap left = Utils::StyleHelper::skinResourceFileName("toolButtonBackground_left", false);
//        QPixmap mid = Utils::StyleHelper::skinResourceFileName("toolButtonBackground_mid", false);
//        QPixmap right = Utils::StyleHelper::skinResourceFileName("toolButtonBackground_right", false);
//        QPixmap pix(sizeHint().width() - 4, 26);
//        QPainter pt(&pix);
//        pix.fill(QColor(213, 214, 214));
//        pt.setPen(Qt::NoPen);
//        pt.setBrush(Qt::NoBrush);
//        pt.drawPixmap(QRect(0, 0, left.width(), 26), left);
//        pt.drawPixmap(QRect(left.width(), 0, sizeHint().width() - left.width() - right.width(), 26), mid);
//        pt.drawPixmap(QRect(sizeHint().width() - right.width(), 0, right.width(), 26), right);
//        m_backgroundSelected = pix;
//        return m_backgroundSelected;
//    }
//    else
//    {
//        if (!m_backgroundNormal.isNull())
//            return m_backgroundNormal;

//        QPixmap left = Utils::StyleHelper::skinResourceFileName("toolButtonBackground_left", false);
//        QPixmap mid = Utils::StyleHelper::skinResourceFileName("toolButtonBackground_mid", false);
//        QPixmap right = Utils::StyleHelper::skinResourceFileName("toolButtonBackground_right", false);
//        int nPixHeight = 26;
//        QPixmap pix(sizeHint().width() - 4, nPixHeight);
//        QPainter pt(&pix);
//        pix.fill(QColor(213, 214, 214));
//        pt.setPen(Qt::NoPen);
//        pt.setBrush(Qt::NoBrush);
//        pt.drawPixmap(QRect(0, 0, left.width(), nPixHeight), left);
//        pt.drawPixmap(QRect(left.width(), 0, pix.width() - left.width() - right.width(), nPixHeight), mid);
//        pt.drawPixmap(QRect(pix.width() - right.width() - 1, 0, right.width(), nPixHeight), right);
//        m_backgroundNormal = pix;
//        return m_backgroundNormal;
//    }
//    return QPixmap();

    return Utils::StyleHelper::skinResourceFileName("toolBarButtonBackground");
}

#endif
