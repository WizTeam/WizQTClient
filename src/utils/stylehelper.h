#ifndef UTILS_STYLEHELPER_H
#define UTILS_STYLEHELPER_H

#include <QVariant>
#include <QPixmap>
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
class QMargins;

#define  DocTypeNormal                         0x0000
#define  DocTypeEncrytedInTitle             0x0001
#define  DocTypeEncrytedInSummary   0x0002
#define  DocTypeAlwaysOnTop               0x0004
#define  DocTypeContainsAttachment    0x0008


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
        BadgeAttachment,
        BadgeEncryptedInTitle,
        BadgeEncryptedInSummary
    };


    enum ListViewType {
        ListTypeThumb,
        ListTypeTwoLine,
        ListTypeOneLine,
        ListTypeSection
    };

    enum ListViewBGType {
        ListBGTypeNone,
        ListBGTypeActive,
        ListBGTypeHalfActive,
        ListBGTypeUnread
    };

    static void initPainterByDevice(QPainter* p);
    static QPixmap pixmapFromDevice(const QSize& sz);
    static QSize applyScreenScaleFactor(const QSize& sz);
    static int lineSpacing();
    static int leading();
    static int margin();
    static int thumbnailHeight();


    static QString themeName();
    static QString skinResourceFileName(const QString& strName, bool need2x = false);
    static QIcon loadIcon(const QString& strName);

    static QRegion borderRadiusRegion(const QRect& rect);
    static QRegion borderRadiusRegionWithTriangle(const QRect& rect, bool triangleAlginLeft,
                                                  int nTriangleMargin, int nTriangleWidth, int nTriangleHeight);

    static QColor splitterLineColor();

    static QString wizCommonListViewStyleSheet();
    static QString wizCommonStyleSheet();
    static QString wizCommonScrollBarStyleSheet(int marginTop = 0);

    static QSize treeViewItemIconSize();
    static int treeViewItemHeight();
    static QColor treeViewBackground();
    static QColor treeViewItemBackground(int stat);
    static QColor treeViewItemCategoryBackground();
    static QColor treeViewItemCategoryText();
    static QColor treeViewItemText(bool bSelected, bool bSecondLevel);
    static QColor treeViewItemTextExtend(bool bSelected);
    static QColor treeViewSectionItemText();
    static QColor treeViewItemLinkText();
    static QColor treeViewItemBottomLine();
    static QColor treeViewItemMessageBackground();
    static QColor treeViewItemMessageText();

    static void drawTreeViewItemBackground(QPainter* p, const QRect& rc, bool bFocused);
    static void drawTreeViewItemIcon(QPainter* p, const QRect& rc, const QIcon& icn, bool bSelected);
    static void drawTreeViewBadge(QPainter* p, const QRect& rc, const QString& str);

    static void drawPixmapWithScreenScaleFactor(QPainter* p, const QRect& rcOrign, const QPixmap& pix);

    static int listViewSortControlWidgetHeight();

    static int messageViewItemHeight();

    static int listViewItemHeight(int nType);
    static QColor listViewBackground();
    static int listViewItemHorizontalPadding();
    static QColor listViewItemSeperator();
    static QColor listViewSectionItemText();
    static QColor listViewSectionItemBackground();
    static QColor listViewItemBackground(int stat);
    static QColor listViewItemType(bool bSelected, bool bFocused);
    static QColor listViewItemTitle(bool bSelected, bool bFocused);
    static QColor listViewItemLead(bool bSelected, bool bFocused);
    static QColor listViewItemLocation(bool bSelected, bool bFocused);
    static QColor listViewItemSummary(bool bSelected, bool bFocused);
    static QColor listViewMultiLineFirstLine(bool bSelected);
    static QColor listViewMultiLineOtherLine(bool bSelected);

    static QRect initListViewItemPainter(QPainter* p, const QRect& lrc, ListViewBGType bgType, bool useFullSeperatorLine = true);
    static void drawListViewItemThumb(QPainter* p, const QRect& rc, int nBadgeType,
                                      const QString& title, const QStringList& lead, const QString& location,
                                      const QString& abs, bool bFocused, bool bSelected, QPixmap thumbPix = QPixmap());

    //static void drawListViewItem(QPainter* p, const QRect& rc);

    static QIcon listViewBadge(int type);
    static QPolygonF bubbleFromSize(const QSize& sz, int nAngle = 10, bool bAlignLeft = true);
    static QRect drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
                        int nFlags, const QColor& color, const QFont& font, bool bElided = true,
                          Qt::TextElideMode elidedMode = Qt::ElideRight);
    static int drawSingleLineText(QPainter* p, const QRect& rc, QString& str, int nFlags, const QColor& color, const QFont& font);

    static void drawListViewItemSeperator(QPainter* p, const QRect& rc);
    static void drawListViewItemBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect);
    static void drawListViewItemSeperator(QPainter* p, const QRect& rc, ListViewBGType bgType, bool useFullSeperatorLine);
    static void drawListViewItemBackground(QPainter* p, const QRect& rc, ListViewBGType bgType);
    static QRect drawThumbnailPixmap(QPainter* p, const QRect& rc, const QPixmap& pm);
    static QRect drawBadgeIcon(QPainter* p, const QRect& rc, int height, int type, bool bFocus, bool bSelect);
    static QRect drawBadgeIcon(QPainter* p, const QRect& rc, BadgeType nType, bool bFocus, bool bSelect);

    static int avatarHeight(bool bNoScreenFactor = false);
    static QSize avatarSize(bool bNoScreenFactor = false);
    static QRect drawAvatar(QPainter* p, const QRect& rc, const QPixmap& pm);

    static int fontHead(QFont& f);
    static int fontNormal(QFont& f);
    static int fontThumb(QFont& f);
    static int fontExtend(QFont& f);
    static int fontCategoryItem(QFont& f);
    static int fontSection(QFont& f);

    static int editorButtonHeight();
    static QMargins editorBarMargins();
    static int titleEditorHeight();
    static int editToolBarHeight();
    static int infoBarHeight();
    static int tagBarHeight();
    //
    static int notifyBarHeight();

private:
    static CWizSettings* m_settings;

    static QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant());
};
} // namespace Utils

#endif // UTILS_STYLEHELPER_H
