#include "wizUserInfoWidgetBaseMac_mm.h"
#include <QPainter>
#include "sync/avataruploader.h"
#include "sync/avatar.h"

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

/*
QPixmap corpAvatar(const QPixmap& org)
{
    if (org.isNull())
        return org;
    //
    QSize sz = org.size();
    //
    int width = sz.width();
    int height = sz.height();
    if (width == height)
        return org;
    //
    if (width > height)
    {
        int xOffset = (width - height) / 2;
        return org.copy(xOffset, 0, height, height);
    }
    else
    {
        int yOffset = (height - width) / 2;
        return org.copy(0, yOffset, width, width);
    }
}

QPixmap CWizUserInfoWidgetBaseMac::getCircleAvatar(int width, int height)
{
    if (width <= 0 || height <= 0)
        return QPixmap();
    //
    if (!m_circleAvatar.isNull())
    {
        if (QSize(width, height) == m_circleAvatar.size())
            return m_circleAvatar;
        //
        m_circleAvatar = QPixmap();
    }

    QPixmap org = AvatarHost::orgAvatar(userId());
    if (org.isNull())
        return org;
    //
    org = corpAvatar(org);
    //
    int largeWidth = width * 8;
    int largeHeight = height * 8;
    //
    QPixmap orgResized = org.scaled(QSize(largeWidth, largeHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    //
    QPixmap largePixmap(QSize(largeWidth, largeHeight));
    largePixmap.fill(QColor(Qt::transparent));
    //
    QPainter painter(&largePixmap);
    //
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    QPainterPath path;
    path.addEllipse(0, 0, largeWidth, largeHeight);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, orgResized);
    //
    m_circleAvatar = largePixmap.scaled(QSize(width, height), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    //
    return m_circleAvatar;
}
*/
