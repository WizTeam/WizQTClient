#ifndef UTILS_STYLEHELPER_H
#define UTILS_STYLEHELPER_H

class QSize;
class QRect;
class QPainter;
class QPixmap;

namespace Utils {
class StyleHelper
{
public:
    static void initPainterByDevice(QPainter* p);
    static QPixmap pixmapFromDevice(const QSize& sz);
    static QSize avatarSize();
    static int avatarHeight();
    static int lineSpacing();
    static int margin();
    static int thumbnailHeight();
};
} // namespace Utils

#endif // UTILS_STYLEHELPER_H
