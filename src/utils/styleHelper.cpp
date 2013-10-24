#include "styleHelper.h"

#include <QFontMetrics>


namespace Utils {

int styleHelper::avatarHeight()
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

int styleHelper::lineSpacing()
{
    return 5;
}


} // namespace Utils
