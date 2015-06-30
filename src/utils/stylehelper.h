#ifndef UTILS_STYLEHELPER_H
#define UTILS_STYLEHELPER_H

class QSize;
class QRect;
class QPainter;
class QPixmap;
class QColor;
class QString;
class QPolygonF;
class QFont;
class QIcon;
class CWizSettings;

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
        BadgeEncryted,
        BadgeAlwaysOnTop
    };

    enum ListViewType {
        ListTypeThumb,
        ListTypeTwoLine,
        ListTypeOneLine
    };

    enum ListViewBGType {
        ListBGTypeNone,
        ListBGTypeActive,
        ListBGTypeHalfActive,
        ListBGTypeUnread
    };

    static void initPainterByDevice(QPainter* p);
    static QPixmap pixmapFromDevice(const QSize& sz);
    static int lineSpacing();
    static int leading();
    static int margin();
    static int thumbnailHeight();


    static QString themeName();
    static QString skinResourceFileName(const QString& strName, bool need2x = false);
    static QIcon loadIcon(const QString& strName);

    static int treeViewItemHeight();
    static QColor treeViewBackground();
    static QColor treeViewItemBackground(int stat);
    static QColor treeViewItemCategoryBackground();
    static QColor treeViewItemCategoryText();
    static QColor treeViewItemText(bool bSelected);
    static QColor treeViewItemTextExtend(bool bSelected);
    static QColor treeViewItemLinkText();
    static QColor treeViewItemBottomLine();
    static QColor treeViewItemMessageBackground();
    static QColor treeViewItemMessageText();

    static void drawTreeViewItemBackground(QPainter* p, const QRect& rc, bool bFocused);
    static void drawTreeViewItemIcon(QPainter* p, const QRect& rc, const QIcon& icn, bool bSelected);
    static void drawTreeViewBadge(QPainter* p, const QRect& rc, const QString& str);

    static void drawPixmapWithScreenScaleFactor(QPainter* p, const QRect& rcOrign, const QPixmap& pix);

    static int listViewSortControlWidgetHeight();

    static int listViewItemHeight(int nType);
    static QColor listViewBackground();
    static QColor listViewItemSeperator();
    static QColor listViewItemBackground(int stat);
    static QColor listViewItemTitle(bool bSelected, bool bFocused);
    static QColor listViewItemLead(bool bSelected, bool bFocused);
    static QColor listViewItemSummary(bool bSelected, bool bFocused);
    static QColor listViewMultiLineFirstLine(bool bSelected);
    static QColor listViewMultiLineOtherLine(bool bSelected);

    static QRect initListViewItemPainter(QPainter* p, const QRect& lrc, ListViewBGType bgType);
    static void drawListViewItemThumb(QPainter* p, const QRect& rc, int nBadgeType,
                                      const QString& title, const QString& lead, const QString& abs,
                                      bool bFocused, bool bSelected, bool bContainsAttach);

    //static void drawListViewItem(QPainter* p, const QRect& rc);

    static QIcon listViewBadge(int type);
    static QPolygonF bubbleFromSize(const QSize& sz, int nAngle = 10, bool bAlignLeft = true);
    static QRect drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
                        int nFlags, const QColor& color, const QFont& font, bool bElided = true);
    static int drawSingleLineText(QPainter* p, const QRect& rc, QString& str, int nFlags, const QColor& color, const QFont& font);

    static void drawListViewItemSeperator(QPainter* p, const QRect& rc);
    static void drawListViewItemBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect);
    static void drawListViewItemSeperator(QPainter* p, const QRect& rc, ListViewBGType bgType);
    static void drawListViewItemBackground(QPainter* p, const QRect& rc, ListViewBGType bgType);
    static QRect drawThumbnailPixmap(QPainter* p, const QRect& rc, const QPixmap& pm);
    static QRect drawBadgeIcon(QPainter* p, const QRect& rc, int height, int type, bool bFocus, bool bSelect);
    static QRect drawAttachIcon(QPainter* p, const QRect& rc, bool bFocus, bool bSelect);

    static int avatarHeight(bool bNoScreenFactor = false);
    static QSize avatarSize(bool bNoScreenFactor = false);
    static QRect drawAvatar(QPainter* p, const QRect& rc, const QPixmap& pm);

    static int fontHead(QFont& f);
    static int fontNormal(QFont& f);
    static int fontExtend(QFont& f);

    static int titleEditorHeight();
    static int editToolBarHeight();
    static int infoBarHeight();
    static int tagBarHeight();
    //
    static int notifyBarHeight();

private:
    static CWizSettings* m_settings;
};
} // namespace Utils

#endif // UTILS_STYLEHELPER_H
