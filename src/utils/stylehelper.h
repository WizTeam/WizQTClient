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
        Active,
        Selected
    };

    enum BadgeType {
        BadgeNormal,
        BadgeEncryted
    };

    enum ListViewType {
        ListTypeThumb,
        ListTypeTwoLine,
        ListTypeOneLine
    };

    static void initPainterByDevice(QPainter* p);
    static QPixmap pixmapFromDevice(const QSize& sz);
    static int lineSpacing();
    static int leading();
    static int margin();
    static int thumbnailHeight();

    static QString themeName();
    static QIcon loadIcon(const QString& strName);

    static QColor treeViewBackground();
    static QColor treeViewItemBackground(int stat);
    static QColor treeViewItemCategoryBackground();
    static QColor treeViewItemCategoryText();
    static QColor treeViewItemText(bool bSelected);
    static QColor treeViewItemTextExtend(bool bSelected);
    static QColor treeViewItemLinkText();

    static void drawTreeViewItemBackground(QPainter* p, const QRect& rc, bool bFocused);
    static void drawTreeViewItemIcon(QPainter* p, const QRect& rc, const QIcon& icn, bool bSelected);
    static void drawTreeViewBadge(QPainter* p, const QRect& rc, const QString& str);

    static int listViewItemHeight(int nType);
    static QColor listViewBackground();
    static QColor listViewItemSeperator();
    static QColor listViewItemBackground(int stat);
    static QColor listViewItemTitle(bool bSelected, bool bFocused);
    static QColor listViewItemLead(bool bSelected, bool bFocused);
    static QColor listViewItemSummary(bool bSelected, bool bFocused);

    static QRect initListViewItemPainter(QPainter* p, QPixmap* pm, const QRect& lrc, bool bFocused, bool bSelected);
    static void drawListViewItemThumb(QPainter* p, const QRect& rc, int nBadgeType,
                                      const QString& title, const QString& lead, const QString& abs,
                                      bool bFocused, bool bSelected);

    //static void drawListViewItem(QPainter* p, const QRect& rc);

    static QIcon listViewBadge(int type);
    static QPolygon bubbleFromSize(const QSize& sz, int nAngle = 10, bool bAlignLeft = true);
    static QRect drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
                        int nFlags, const QColor& color, const QFont& font, bool bElided = true);
    static int drawSingleLineText(QPainter* p, const QRect& rc, QString& str, int nFlags, const QColor& color, const QFont& font);

    static void drawListViewItemSeperator(QPainter* p, const QRect& rc);
    static void drawListViewItemBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect);
    static QRect drawThumbnailPixmap(QPainter* p, const QRect& rc, const QPixmap& pm);
    static QRect drawBadgeIcon(QPainter* p, const QRect& rc, int height, int type, bool bFocus, bool bSelect);

    static int avatarHeight();
    static QSize avatarSize();
    static QRect drawAvatar(QPainter* p, const QRect& rc, const QPixmap& pm);

    static int fontHead(QFont& f);
    static int fontNormal(QFont& f);
    static int fontExtend(QFont& f);

};
} // namespace Utils

#endif // UTILS_STYLEHELPER_H
