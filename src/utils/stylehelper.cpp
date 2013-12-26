#include "stylehelper.h"

#include <QFontMetrics>
#include <QSize>

namespace Utils {

QSize StyleHelper::avatarSize()
{
    return QSize(avatarHeight(), avatarHeight());
}

int StyleHelper::avatarHeight()
{
    int nHeight = lineSpacing() * 3;

    QFont f;
    f.setPixelSize(13);
    f.setBold(true);
    nHeight += QFontMetrics(f).height();
    f.setPixelSize(12);
    f.setBold(false);
    nHeight += QFontMetrics(f).height();

    return nHeight;
}

int StyleHelper::lineSpacing()
{
    return 5;
}

int StyleHelper::margin()
{
    return 5;
}

int StyleHelper::thumbnailHeight()
{
    QFont f;
    int nExtra = QFontMetrics(f).height() * 2 + margin() * 2;
    return margin() * 2 + avatarHeight() + nExtra;
}


} // namespace Utils
