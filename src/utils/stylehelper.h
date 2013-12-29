#ifndef UTILS_STYLEHELPER_H
#define UTILS_STYLEHELPER_H

class QSize;
class QRect;
class QPainter;
class QPixmap;
class QColor;
class QString;
class QPolygon;
class QFont;
class QIcon;

namespace Utils {
class StyleHelper
{
public:
    enum TextState {
        Normal,
        Active
    };

    enum BadgeType {
        BadgeNormal,
        BadgeEncryted,
    };

    static void initPainterByDevice(QPainter* p);
    static QPixmap pixmapFromDevice(const QSize& sz);
    static int lineSpacing();
    static int leading();
    static int margin();
    static int thumbnailHeight();

    static QString themeName();
    static QIcon loadIcon(const QString& strName);

    static QColor listViewSeperator();
    static QColor listViewBackground(int stat);
    static QColor listViewTitle(int stat);
    static QIcon listViewBadge(int type);
    static QPolygon bubbleFromSize(const QSize& sz, int nAngle = 10, bool bAlignLeft = true);
    static QRect drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
                        int nFlags, const QColor& color, const QFont& font);

    static void drawListViewSeperator(QPainter* p, const QRect& rc);
    static void drawListViewBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect);
    static QRect drawThumbnailPixmap(QPainter* p, const QRect& rc, const QPixmap& pm);
    static QRect drawBadgeIcon(QPainter* p, const QRect& rc, int height, int type, bool bFocus, bool bSelect);

    static int avatarHeight();
    static QSize avatarSize();
    static QRect drawAvatar(QPainter* p, const QRect& rc, const QPixmap& pm);

    static QFont fontHead();
    static QFont fontNormal();
};
} // namespace Utils

#endif // UTILS_STYLEHELPER_H
