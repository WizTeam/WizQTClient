#include "WizStyleHelper.h"

#include <QFontMetrics>
#include <QSize>
#include <QRect>
#include <QTransform>
#include <QPainter>
#include <QVector>
#include <QTextLayout>
#include <QDebug>
#include <QApplication>
#include <QTextDocument>

#include "WizPathResolve.h"
#include "../share/WizMisc.h"
#include "../share/WizSettings.h"
#include "../share/WizGlobal.h"


#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#endif

namespace Utils {

WizSettings* WizStyleHelper::m_settings = 0;

void WizStyleHelper::initPainterByDevice(QPainter* p)
{
#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0); // factor == 2 on retina

    QTransform trans;
    trans.scale(factor, factor);
    p->setWorldTransform(trans);
#endif
}

QPixmap WizStyleHelper::pixmapFromDevice(const QSize& sz)
{
#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0); // factor == 2 on retina
    QSize sz2 = sz * factor;
#else
    QSize sz2 = sz;
#endif

    return QPixmap(sz2);
}

QSize WizStyleHelper::applyScreenScaleFactor(const QSize& sz)
{
#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0); // factor == 2 on retina
    QSize sz2 = sz * factor;
#else
    QSize sz2 = sz;
#endif

    return sz2;
}

QString WizStyleHelper::themeName()
{
    QSettings* st = WizGlobal::settings();
    QString strTheme = st->value("Theme/Name").toString();
    if (strTheme.isEmpty()) {
        st->setValue("Theme/Name", "default");
        return "default";
    }

    return strTheme;
}

QString WizStyleHelper::skinResourceFileName(const QString& strName, bool need2x)
{
    bool use2x = need2x && ::WizIsHighPixel();
    return ::WizGetSkinResourceFileName(themeName(),
                                        (use2x ? strName + "@2x" : strName));
}

QIcon WizStyleHelper::loadIcon(const QString& strName)
{
    QString strThemeName = themeName();
    QString strIconNormal = ::WizGetSkinResourceFileName(strThemeName, strName);
    QString strIconActive1 = ::WizGetSkinResourceFileName(strThemeName, strName+ "_on");
    QString strIconActive2 = ::WizGetSkinResourceFileName(strThemeName, strName+ "_selected");

    if (!QFile::exists(strIconNormal)) {
        qWarning() << "load icon failed, filePath:" << strIconNormal << " name : " << strName;
        return QIcon();
    }

    QIcon icon;
    icon.addFile(strIconNormal, QSize(), QIcon::Normal, QIcon::Off);

    // used for check state
    if (QFile::exists(strIconActive1)) {
        icon.addFile(strIconActive1, QSize(), QIcon::Active, QIcon::On);
    }

    // used for sunken state
    if (QFile::exists(strIconActive2)) {
        icon.addFile(strIconActive2, QSize(), QIcon::Active, QIcon::Off);
    }

    return icon;
}

QRegion WizStyleHelper::borderRadiusRegion(const QRect& rect)
{
    QVector<QPoint> points;
    int nBorderInterval = 2;
    points.append(QPoint(rect.left(), rect.top() + nBorderInterval));
    points.append(QPoint(rect.left() + 1, rect.top() + 1));
    points.append(QPoint(rect.left() + nBorderInterval, rect.top()));
    points.append(QPoint(rect.right() - nBorderInterval, rect.top()));
    points.append(QPoint(rect.right() - 1, rect.top() + 1));
    points.append(QPoint(rect.right(), rect.top() + nBorderInterval));
    points.append(QPoint(rect.right(), rect.bottom() - nBorderInterval));
    points.append(QPoint(rect.right() - 1, rect.bottom() - 1));
    points.append(QPoint(rect.right() - nBorderInterval, rect.bottom()));
    points.append(QPoint(rect.left() + nBorderInterval, rect.bottom()));
    points.append(QPoint(rect.left() + 1, rect.bottom() - 1));
    points.append(QPoint(rect.left(), rect.bottom() - nBorderInterval));

    QPolygon polygon(points);

    return QRegion(polygon);
}

QRegion WizStyleHelper::borderRadiusRegionWithTriangle(const QRect& rect, bool triangleAlginLeft,
                                                    int nTriangleMargin, int nTriangleWidth, int nTriangleHeight)
{    
    QVector<QPoint> pointsRegion;

    pointsRegion.push_back(QPoint(rect.left() + 1, nTriangleHeight + rect.top()));
    if (triangleAlginLeft)
    {
        pointsRegion.push_back(QPoint(rect.left() + 1 + nTriangleMargin, nTriangleHeight + rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + 1 + nTriangleMargin + nTriangleWidth / 2, rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + 1 + nTriangleMargin + nTriangleWidth, nTriangleHeight + rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + rect.width() - 1, nTriangleHeight + rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + rect.width(), 1 + nTriangleHeight + rect.top()));        
    }
    else
    {
        pointsRegion.push_back(QPoint(rect.left() + rect.width() - 1 - nTriangleMargin - nTriangleWidth, nTriangleHeight + rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + rect.width() - 1 - nTriangleMargin - nTriangleWidth / 2, rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + rect.width() - 1 - nTriangleMargin, nTriangleHeight + rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + rect.width() - 1, nTriangleHeight + rect.top()));
        pointsRegion.push_back(QPoint(rect.left() + rect.width(), 1 + nTriangleHeight + rect.top()));
    }
    pointsRegion.push_back(QPoint(rect.left() + rect.width(), rect.top() + rect.height() - 3));
    pointsRegion.push_back(QPoint(rect.left() + rect.width() - 1, rect.top() + rect.height() - 2));
    pointsRegion.push_back(QPoint(rect.left() + rect.width() - 2, rect.top() + rect.height() - 1));
    pointsRegion.push_back(QPoint(rect.left() + rect.width() - 3, rect.top() + rect.height()));
    //pointsRegion.push_back(QPoint(rect.left() + rect.width() - 4, rect.top() + rect.height() - 1));
    //pointsRegion.push_back(QPoint(rect.left() + rect.width() - 5, rect.top() + rect.height() - 1));
//    pointsRegion.push_back(QPoint(rect.left() + rect.width() - 4, rect.top() + rect.height() - 2));
//    pointsRegion.push_back(QPoint(rect.left() + rect.width() - , rect.top() + rect.height()));
    pointsRegion.push_back(QPoint(rect.left() + 3, rect.top() + rect.height()));
    pointsRegion.push_back(QPoint(rect.left() + 2, rect.top() + rect.height() - 1));
    pointsRegion.push_back(QPoint(rect.left() + 1, rect.top() + rect.height() - 2));
    pointsRegion.push_back(QPoint(rect.left(), rect.top() + rect.height() - 3));
    pointsRegion.push_back(QPoint(rect.left(), 1 + nTriangleHeight + rect.top()));

    QPolygon polygon(pointsRegion);

    return QRegion(polygon);
}

QColor WizStyleHelper::splitterLineColor()
{
    return QColor("#DBDBDB");
}


QString WizStyleHelper::wizCommonListViewStyleSheet()
{
    return QString("QListView{ border-width: 1px; \
        background-color:#FFFFFF; \
        padding: 1px; \
        border-style: solid; \
        border-color: #ECECEC; \
        border-radius: 5px; \
        border-bottom-color:#E0E0E0;}");
}

QString WizStyleHelper::wizCommonStyleSheet()
{
    QString location = Utils::WizPathResolve::skinResourcesPath(themeName()) + "style.qss";
    QString style;
    QFile qss(location);
    if (qss.exists())
    {
        qss.open(QFile::ReadOnly);
        style = qss.readAll();
        qss.close();

        //
        style.replace("WizComboBoxDownArrow", skinResourceFileName("comboBox_downArrow", true));
        style.replace("WizSpinBoxUpButton", skinResourceFileName("spinbox_upButton"));
        style.replace("WizSpinBoxUpButtonPressed", skinResourceFileName("spinbox_upButton_selected"));
        style.replace("WizSpinBoxDownButton", skinResourceFileName("spinbox_downButton"));
        style.replace("WizSpinBoxDownButtonPressed", skinResourceFileName("spinbox_downButton_selected"));

    }
    return style;
}

QString WizStyleHelper::wizCommonScrollBarStyleSheet(int marginTop)
{
    return QString("QScrollBar {\
            background: #FFFFFF;\
            width: 12px; \
        }\
        QScrollBar::handle:vertical {\
            width: 6px; \
            background:#DADADA; \
            border-radius:3px;\
            min-height:20; \
            margin-top:%1px; \
            margin-right:3px; \
            margin-left:3px; \
        }\
        QScrollBar::add-page, QScrollBar::sub-page {\
            background: transparent;\
        }\
        QScrollBar::up-arrow, QScrollBar::down-arrow, QScrollBar::left-arrow, QScrollBar::right-arrow {\
            background: transparent;\
        }\
        QScrollBar::add-line, QScrollBar::sub-line {\
            height: 0px;\
            width: 0px;\
        }").arg(marginTop);
}

QSize WizStyleHelper::treeViewItemIconSize()
{
    return QSize(WizSmartScaleUI(14), WizSmartScaleUI(14));
}

int WizStyleHelper::treeViewItemHeight()
{
    return WizSmartScaleUI(28);
}

QColor WizStyleHelper::treeViewBackground()
{
    return QColor(getValue("Category/Background", "#F2F0EE").toString());
}

QColor WizStyleHelper::treeViewItemBackground(int stat)
{    
    if (stat == Selected) {
        return QColor(getValue("Category/ItemSelectedNoFocus", "#cecece").toString());
    } else if (stat == Active) {
        return QColor(getValue("Category/ItemSelected", "#5990EF").toString());
    }

    Q_ASSERT(0);
    return QColor();
}

QColor WizStyleHelper::treeViewItemCategoryBackground()
{    
    QColor co(getValue("Category/ItemCategory", "#ffffff").toString());
    co.setAlpha(15);
    return co;
}

QColor WizStyleHelper::treeViewItemCategoryText()
{
    return QColor(getValue("Category/ItemCategoryText", "#777775").toString());
}
QColor WizStyleHelper::treeViewItemLinkText()
{    
    return QColor(getValue("Category/ItemLinkText", "#448aff").toString());
}

QColor WizStyleHelper::treeViewItemBottomLine()
{    
    return QColor(getValue("Category/ItemBottomLine", "#DFDFD7").toString());
}

QColor WizStyleHelper::treeViewItemMessageBackground()
{    
    return QColor(getValue("Category/ItemMessageBackground", "#3498DB").toString());
}

QColor WizStyleHelper::treeViewItemMessageText()
{    
    return QColor(getValue("Category/ItemMessageText", "#FFFFFF").toString());
}

QColor WizStyleHelper::treeViewItemText(bool bSelected, bool bSecondLevel)
{    
    if (bSelected) {
        return QColor(getValue("Category/TextSelected", "#ffffff").toString());
    } else {
//        if (bSecondLevel) {
//            return QColor(m_settings->value("Category/SecondLevelText", "#535353").toString());
//        } else {
            return QColor(getValue("Category/Text", "#111111").toString());
//        }
    }
}

QColor WizStyleHelper::treeViewItemTextExtend(bool bSelected)
{    
    if (bSelected) {
        return QColor(getValue("Category/TextExtendSelected", "#e6e6e6").toString());
    } else {
        return QColor(getValue("Category/TextExtend", "#888888").toString());
    }
}

QColor WizStyleHelper::treeViewSectionItemText()
{
    return QColor(getValue("Category/SectionItemText", "#C1C1C1").toString());
}

void WizStyleHelper::drawTreeViewItemBackground(QPainter* p, const QRect& rc, bool bFocused)
{
    QRect rcd(rc);
    QColor bg1 = treeViewItemBackground(Active);
    QColor bg2 = treeViewItemBackground(Selected);

    if (bFocused) {
        p->fillRect(rcd, bg1);
    } else {
        p->fillRect(rcd, bg2);
    }
}

void WizStyleHelper::drawTreeViewItemIcon(QPainter* p, const QRect& rc, const QIcon& icn, bool bSelected)
{
    if (bSelected) {
        icn.paint(p, rc, Qt::AlignCenter, QIcon::Selected);
    } else {
        icn.paint(p, rc, Qt::AlignCenter, QIcon::Normal);
    }
}

void WizStyleHelper::drawTreeViewBadge(QPainter* p, const QRect& rc, const QString& str)
{
    QFont f;
    f.setPixelSize(11);
    QRect rcd(rc.adjusted(2, 2, -5, -2));
    int nWidth = QFontMetrics(f).width(str);
    int nHeight = QFontMetrics(f).height();
    if (nWidth > rcd.width() || nHeight > rcd.height()) {
        qDebug() << "[WARNING] not enough space for drawing badge string";
    }

    nWidth = (nWidth < rcd.height()) ? rcd.height() : nWidth;

    p->save();

    QColor co = treeViewItemBackground(Active);
    rcd.setLeft(rcd.right() - nWidth);
    p->setRenderHints(QPainter::Antialiasing);
    p->setBrush(co);
    p->setPen(co);
    p->drawEllipse(rcd);

    p->setPen(Qt::white);
    p->drawText(rcd, Qt::AlignCenter, str);

    p->restore();
}

void WizStyleHelper::drawPixmapWithScreenScaleFactor(QPainter* p, const QRect& rcOrign, const QPixmap& pix)
{
    if (pix.isNull())
        return;

#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0);
    QRect rcTarget(rcOrign);
    rcTarget.setWidth(rcTarget.width() * factor);
    rcTarget.setHeight(rcTarget.height() * factor);
    QTransform transRect;
    transRect.scale(factor, factor);
    QPoint leftTop = transRect.map(rcOrign.topLeft());
    rcTarget.moveTopLeft(leftTop);
    //
    float rFactor = 1 / factor;
    QTransform transPainter;
    transPainter.scale(rFactor, rFactor);    
    //
    p->save();
    p->setWorldTransform(transPainter);
    p->drawPixmap(rcTarget, pix);
    p->restore();
#else
    p->drawPixmap(rcOrign, pix);
#endif
}

int WizStyleHelper::listViewSortControlWidgetHeight()
{    
    int val = getValue("Documents/SortControlWidgetHeight", 30).toInt();
    return WizSmartScaleUI(val);
}

int WizStyleHelper::messageViewItemHeight()
{
    return WizSmartScaleUI(83);
}

int WizStyleHelper::listViewItemHeight(int nType)
{
//    QFont f;
    switch (nType) {
    case ListTypeOneLine:
        return WizSmartScaleUI(38);
//        return fontHead(f) + margin() * 4;
    case ListTypeTwoLine:
        return WizSmartScaleUI(68);
//        return fontHead(f) + fontNormal(f) + margin() * 5;
    case ListTypeThumb:
        return WizSmartScaleUI(122);
    case ListTypeSearchResult:
        return WizSmartScaleUI(160);
//        return thumbnailHeight() + margin() * 2;
    case ListTypeSection:
        return WizSmartScaleUI(20);
    default:
        Q_ASSERT(0);
        return 0;
    }
}


QColor WizStyleHelper::listViewBackground()
{    
    return QColor(getValue("Documents/Background", "#ffffff").toString());
}

int WizStyleHelper::listViewItemHorizontalPadding()
{
    return WizSmartScaleUI(6);
}

QColor WizStyleHelper::listViewItemSeperator()
{    
    return QColor(getValue("Documents/Line", "#e7e7e7").toString());
}

QColor WizStyleHelper::listViewSectionItemText()
{    
    return QColor(getValue("Documents/SectionItemText", "#a7a7a7").toString());
}

QColor WizStyleHelper::listViewSectionItemBackground()
{    
    return QColor(getValue("Documents/SectionItemBackground", "#f7f7f7").toString());
}

QColor WizStyleHelper::listViewItemBackground(int stat)
{    
    if (stat == Normal) {
        return QColor(getValue("Documents/ItemLoseFocusBackground", "#D3E4ED").toString());
    } else if (stat == Active) {
        return QColor(getValue("Documents/ItemFocusBackground", "#3498DB").toString());
    } else if (stat == ListBGTypeUnread) {
        return QColor(getValue("Documents/ItemUnreadBackground", "#f7f7f7").toString());
    }


    Q_ASSERT(0);
    return QColor();
}

QColor WizStyleHelper::listViewItemType(bool bSelected, bool bFocused)
{
//    if (bSelected) {
//        if (bFocused) {
//            return QColor(getValue("Documents/TypeFocus", "#3177EE").toString());
//        } else {
//            return QColor(getValue("Documents/TypeLoseFocus", "#3177EE").toString());
//        }
//    } else {
        return QColor(getValue("Documents/Type", "#3177EE").toString());
//    }
}

QColor WizStyleHelper::listViewItemTitle(bool bSelected, bool bFocused)
{    
//    if (bSelected) {
//        if (bFocused) {
//            return QColor(getValue("Documents/TitleFocus", "#ffffff").toString());
//        } else {
//            return QColor(getValue("Documents/TitleLoseFocus", "#6a6a6a").toString());
//        }
//    } else {
        return QColor(getValue("Documents/Title", "#464646").toString());
//    }
}

QColor WizStyleHelper::listViewItemLead(bool bSelected, bool bFocused)
{
//    if (bSelected) {
//        if (bFocused) {
//            return QColor(getValue("Documents/LeadFocus", "#ffffff").toString());
//        } else {
//            return QColor(getValue("Documents/LeadLoseFocus", "#6a6a6a").toString());
//        }
//    } else {
        return QColor(getValue("Documents/Lead", "#6B6B6B").toString());
//    }
}

QColor WizStyleHelper::listViewItemLocation(bool bSelected, bool bFocused)
{   
//    if (bSelected) {
//        if (bFocused) {
//            return QColor(getValue("Documents/LocationFocus", "#ffffff").toString());
//        } else {
//            return QColor(getValue("Documents/LocationLoseFocus", "#6a6a6a").toString());
//        }
//    } else {
        return QColor(getValue("Documents/Location", "#3177EE").toString());
//    }
}

QColor WizStyleHelper::listViewItemSummary(bool bSelected, bool bFocused)
{    
//    if (bSelected) {
//        if (bFocused) {
//            return QColor(getValue("Documents/SummaryFocus", "#ffffff").toString());
//        } else {
//            return QColor(getValue("Documents/SummaryLoseFocus", "#6a6a6a").toString());
//        }
//    } else {
        return QColor(getValue("Documents/Summary", "#8c8c8c").toString());
//    }
}

QColor WizStyleHelper::listViewMultiLineFirstLine(bool bSelected)
{    
    if (bSelected) {
        return QColor(getValue("MultiLineList/FirstSelected", "#000000").toString());
    } else {
        return QColor(getValue("MultiLineList/First", "#000000").toString());
    }
}

QColor WizStyleHelper::listViewMultiLineOtherLine(bool bSelected)
{    
    if (bSelected) {
        return QColor(getValue("MultiLineList/OtherSelected", "#666666").toString());
    } else {
        return QColor(getValue("MultiLineList/Other", "#666666").toString());
    }
}

QIcon WizStyleHelper::listViewBadge(int type)
{
    switch (type) {
    case BadgeAttachment:
        return loadIcon("document_containsattach");
        break;
    case BadgeEncryptedInTitle:
        return loadIcon("document_encryptedInTitle");
        break;        
    case BadgeEncryptedInSummary:
        return loadIcon("document_encryptedInSummary");
        break;
    default:
        break;
    }

    return QIcon();
}

void drawSelectBorder(QPainter* p, const QRect& rc, const QColor& color, int width)
{
    p->save();
//    p->setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(width);
    pen.setColor(color);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->drawRect(rc);
    p->restore();
}

void WizStyleHelper::drawListViewItemBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect)
{
    int borderMargin = WizIsHighPixel() ? 3 : 2;
    QRect rcBg = rc.adjusted(1, 1, -14, -borderMargin);
    if (bSelect) {
        if (bFocus) {
//            p->fillRect(rcBg, listViewItemBackground(Active));
            drawSelectBorder(p, rcBg, QColor("#3177EE"), 2);
        } else
        {
//            p->fillRect(rcBg, listViewItemBackground(Normal));
            drawSelectBorder(p, rcBg, QColor("#9BC0FF"), 2);
        }
    }
}


void WizStyleHelper::drawListViewItemBackground(QPainter* p, const QRect& rc, WizStyleHelper::ListViewBGType bgType, QColor color)
{
    int borderMargin = 2;

    QRect rcBg = rc;
    switch (bgType) {
    case ListBGTypeNone:
        //p->fillRect(rcBg, listViewItemBackground(Normal));
        break;
    case ListBGTypeActive:
    {
        QRect rcBg = rc.adjusted(1, 1, -2, -borderMargin);
        drawSelectBorder(p, rcBg, QColor("#3177EE"), 2);
    }
//        p->fillRect(rcBg, listViewItemBackground(Active));
        break;
    case ListBGTypeHalfActive:
    {
        QRect rcBg = rc.adjusted(1, 1, -2, -borderMargin);
        drawSelectBorder(p, rcBg,QColor("#9BC0FF"), 2);
    }
//        p->fillRect(rcBg, listViewItemBackground(Normal));
        break;
    case ListBGTypeUnread:
        p->fillRect(rcBg, listViewItemBackground(ListBGTypeUnread));
        break;
    case ListBGTypeCustom:
        p->fillRect(rcBg, color);
        break;
    default:
        break;
    }
}

void WizStyleHelper::drawListViewItemSeperator(QPainter* p, const QRect& rc, ListViewBGType bgType,
                                            bool useFullSeperatorLine)
{
    QRect rcLine = rc;
    p->save();

    switch (bgType)
    {
    case ListBGTypeActive:
    case ListBGTypeHalfActive:
        p->setPen(QColor("#d2d8d6"));
    case ListBGTypeUnread:
    case ListBGTypeNone:
    default:
        p->setPen(listViewItemSeperator());
        break;
    }

    QPoint pLeft(rcLine.bottomLeft());
    if (!useFullSeperatorLine)
    {
        const int nSeperatorLeftMargin = WizSmartScaleUI(12);
        pLeft.setX(pLeft.x() + nSeperatorLeftMargin);
    }

    if (WizIsHighPixel())
    {
        QLineF line(pLeft.x(), pLeft.y() + 0.5f, rcLine.bottomRight().x(),
                    rcLine.bottomRight().y() + 0.5f);
        p->drawLine(line);
    }
    else
    {
        p->drawLine(pLeft, rcLine.bottomRight());
    }
    p->restore();
}

QSize WizStyleHelper::avatarSize(bool bNoScreenFactor)
{
    int nHeight = avatarHeight(bNoScreenFactor);
    return QSize(nHeight, nHeight);
}

int WizStyleHelper::avatarHeight(bool bNoScreenFactor)
{
    QFont f;
    int nHeight = WizSmartScaleUI(36);//fontHead(f) + fontNormal(f) + margin() * 3 ;
    if (bNoScreenFactor)
        return nHeight;

#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0); 
    return nHeight * factor;
#else
    return nHeight;
#endif
}

QRect WizStyleHelper::drawAvatar(QPainter* p, const QRect& rc, const QPixmap& pm)
{
    QRect rectAvatar = rc;
    rectAvatar.setSize(avatarSize(true));
    drawPixmapWithScreenScaleFactor(p, rectAvatar, pm);    

    return rectAvatar;
}
int WizStyleHelper::drawSingleLineText(QPainter* p, const QRect& rc, QString& str, int nFlags, const QColor& color, const QFont& font)
{
    QPen oldpen = p->pen();
    QFont oldFont = p->font();
    //
    p->setPen(color);
    p->setFont(font);
    QRect out;
    p->drawText(rc, nFlags | Qt::TextSingleLine, str, &out);
    //
    p->setPen(oldpen);
    p->setFont(oldFont);
    //
    return out.right();
}

void WizStyleHelper::drawListViewItemSeperator(QPainter* p, const QRect& rc)
{
    QRect rcLine = rc;
    rcLine.adjust(WizSmartScaleUI(12), 0, 0, 0);
    p->save();
    p->setPen(listViewItemSeperator());
    p->drawLine(rcLine.bottomLeft(), rcLine.bottomRight());
    p->restore();
}

QRect WizStyleHelper::drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
                          int nFlags, const QColor& color, const QFont& font, bool bElided,
                            Qt::TextElideMode elidedMode)
{
    if (str.isEmpty()) {
        qDebug() << "[WARNING]: the text should not be empty when drawing!";
        return QRect();
    }

    QFontMetrics fm(font);
    if (rc.height() < (fm.height() + fm.leading()) * nLines) {
        qDebug() << "[WARNING]: space is not enough for drawing! text: " << str.left(30) << "...";
    }

    //if (rc.width() * nLines < fm.width(str)) {
    //    qDebug() << "[WARNING]: width should bigger than font metrics when drawing! text:" << str.left(30) << "...";
    //}

    p->save();
    p->setPen(color);
    p->setFont(font);

    int nWidth = 0;
    int nHeight = 0;
    int nHeightLine = p->fontMetrics().height() + leading();

    QRect rcRet(rc.x(), rc.y(), rc.width(), nHeightLine);
    rcRet.adjust(margin(), 0, -margin(), 0);

    QTextLayout textLayout(str, p->font());
    QTextOption opt = textLayout.textOption();
    opt.setWrapMode(QTextOption::WrapAnywhere);
    textLayout.setTextOption(opt);

    textLayout.beginLayout();
    while (nLines) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(rcRet.width());

        QString lineText;
        if (nLines == 1 && bElided) { // the last line
            lineText = p->fontMetrics().elidedText(str, elidedMode, rcRet.width());
            nWidth = qMax<int>(p->fontMetrics().width(lineText), nWidth);
        } else {
            lineText = str.left(line.textLength());
            nWidth = qMax<int>(p->fontMetrics().width(lineText), nWidth);
        }

        str.remove(0, line.textLength());
        p->drawText(rcRet, nFlags, lineText);

        nHeight = nHeight + nHeightLine + lineSpacing();
        rcRet.setRect(rc.x(), rc.y() + nHeight, nWidth, nHeightLine);
        rcRet.adjust(margin(), 0, -margin(), 0);

        nLines--;
    }
    textLayout.endLayout();

    rcRet.setRect(rc.x() + margin(), rc.y(), nWidth + margin(), nHeight);
    //rcRet.adjust(margin(), 0, -margin(), 0);

    p->restore();

    return rcRet;
}

QRect WizStyleHelper::drawThumbnailPixmap(QPainter* p, const QRect& rc, const QPixmap& pm)
{
    if (pm.isNull()) {
        qDebug() << "[WARNING]pixmap is null when drawing thumbnail";
        return QRect(rc.x(), rc.y(), 0, 0);
    }

    QRect rcd(rc.x() + rc.right() - WizSmartScaleUI(66), rc.y() + rc.height() - WizSmartScaleUI(60), WizSmartScaleUI(50), WizSmartScaleUI(50));

    int nWidth = 0, nHeight = 0;
    if (pm.width() > nThumbnailPixmapMaxWidth || pm.height() > nThumbnailPixmapMaxWidth) {
        double fRate = qMin<double>(double(nThumbnailPixmapMaxWidth) / pm.width(), double(nThumbnailPixmapMaxWidth) / pm.height());
        nWidth = int(pm.width() * fRate);
        nHeight = int(pm.height() * fRate);
    } else {
        nWidth = pm.width();
        nHeight = pm.height();
    }

    int adjustX = (rcd.width() - nWidth) / 2;
    int adjustY = (rcd.height() - nHeight) / 2;
    rcd.adjust(adjustX, adjustY, -adjustX, -adjustY);
    p->save();
    QPainterPath path;
    path.addRoundedRect(rcd, WizSmartScaleUI(4), WizSmartScaleUI(4));
    p->setClipPath(path);
    p->drawPixmap(rcd, pm);
    p->restore();
    return rcd;
}

QRect WizStyleHelper::drawBadgeIcon(QPainter* p, const QRect& rc, int height, int type, bool bFocus, bool bSelect)
{
    QIcon badge(listViewBadge(type));
    QRect rcb = rc.adjusted(margin(), margin(), 0, 0);
    rcb.setSize(QSize(height, height));
    if (bSelect && bFocus) {
        badge.paint(p, rcb, Qt::AlignBottom, QIcon::Active, QIcon::Off);
    } else {
        badge.paint(p, rcb, Qt::AlignBottom, QIcon::Normal, QIcon::Off);
    }

    return rcb;
}

QRect WizStyleHelper::drawBadgeIcon(QPainter* p, const QRect& rc, BadgeType nType,  bool bFocus, bool bSelect)
{
    QIcon attachIcon(listViewBadge(nType));
    QList<QSize> sizes = attachIcon.availableSizes();
    QSize iconSize = sizes.first();
    if (WizIsHighPixel() && sizes.count() >=2)
    {
        iconSize = QSize(sizes.last().width() / 2, sizes.last().height() / 2);
    }
//    int nLeftMargin = 2;
//    QRect rcb(rc.x() + (rc.width() - iconSize.width()) / 2, rc.y() + (rc.height() - iconSize.height()) / 2,
//              iconSize.width(), iconSize.height());//
    QRect rcb = rc.adjusted(0, margin(), 0, 0);
    rcb.setSize(iconSize);
    if (bSelect && bFocus) {
        attachIcon.paint(p, rcb, Qt::AlignTop, QIcon::Active, QIcon::On);
    } else {
        attachIcon.paint(p, rcb, Qt::AlignTop, QIcon::Normal, QIcon::Off);
    }

    return rcb;
}

int WizStyleHelper::lineSpacing()
{
    return WizSmartScaleUI(4);
}

int WizStyleHelper::leading()
{
    return WizSmartScaleUI(3);
}

int WizStyleHelper::margin()
{
    return WizSmartScaleUI(5);
}

int WizStyleHelper::thumbnailHeight()
{
    QFont f;
    return fontHead(f) + fontNormal(f) * 3 + margin() * 5;
}

QPolygonF WizStyleHelper::bubbleFromSize(const QSize& sz, int nAngle, bool bAlignLeft)
{
    Q_ASSERT(sz.width() > 31);
    Q_ASSERT(sz.height() > 11);

    float nBottomCorner = WizIsHighPixel() ? 2 : 1.5;

    QVector<QPointF> ps;
    if (bAlignLeft) {
        ps.push_back(QPointF(0, 2.5 + nAngle));
        ps.push_back(QPointF(1, 1 + nAngle));
        ps.push_back(QPointF(2.5, nAngle));
        ps.push_back(QPointF(11, nAngle));
        ps.push_back(QPointF(11 + nAngle, 0));
        ps.push_back(QPointF(11 + nAngle * 2, nAngle));
        ps.push_back(QPointF(sz.width() - 2, nAngle));
        ps.push_back(QPointF(sz.width() - 0.6, nAngle + 0.6));
        ps.push_back(QPointF(sz.width(), nAngle + 2));
        ps.push_back(QPointF(sz.width(), sz.height() - nBottomCorner));
        if (WizIsHighPixel())
        {
            ps.push_back(QPointF(sz.width() - 0.6, sz.height() - 0.6));
        }
        ps.push_back(QPointF(sz.width() - nBottomCorner, sz.height()));
        ps.push_back(QPointF(nBottomCorner, sz.height()));
        ps.push_back(QPointF(0.6, sz.height() - 0.6));
        ps.push_back(QPointF(0, sz.height() - nBottomCorner));
        ps.push_back(QPointF(0, 2.5 + nAngle));
    } else {
        ps.push_back(QPoint(1, 10));
        ps.push_back(QPoint(sz.width() - 11 - nAngle * 2, nAngle));
        ps.push_back(QPoint(sz.width() - 11 - nAngle, 0));
        ps.push_back(QPoint(sz.width() - 11, nAngle));
        ps.push_back(QPoint(sz.width() - 1, nAngle));
        ps.push_back(QPoint(sz.width(), nAngle + 1));
        ps.push_back(QPoint(sz.width(), sz.height()));
        ps.push_back(QPoint(0, sz.height()));
        ps.push_back(QPoint(0, nAngle + 1));
    }

    return QPolygonF(ps);
}

int WizStyleHelper::fontHead(QFont& f)
{

#ifdef Q_OS_MAC
//    QSettings* st = WizGlobal::settings();
//    QString strFont = st->value("Theme/FontFamily").toString();
//    if (strFont.isEmpty()) {
//        st->setValue("Theme/FontFamily", f.family());
//    }

    //f.setFamily(strFont);
    //FIXME: should not use fix font size. but different widget has different default font size.
    f.setPixelSize(14);
//    f.setBold(true);
#endif

    return QFontMetrics(f).height();
}

int WizStyleHelper::fontNormal(QFont& f)
{
#ifdef Q_OS_MAC
//    QSettings* st = WizGlobal::settings();
//    QString strFont = st->value("Theme/FontFamily").toString();
//    if (strFont.isEmpty()) {
//        st->setValue("Theme/FontFamily", f.family());
//    }

//    f.setFamily(strFont);
    //FIXME: should not use fix font size. but different widget has different default font size.
    f.setPixelSize(14);
#endif
    return QFontMetrics(f).height();
}

int WizStyleHelper::fontThumb(QFont& f)
{
#ifdef Q_OS_MAC
    f.setPixelSize(12);
#endif
    return QFontMetrics(f).height();
}

int WizStyleHelper::fontExtend(QFont& f)
{
    QSettings* st = WizGlobal::settings();
    QString strFont = st->value("Theme/FontFamily").toString();
    if (strFont.isEmpty()) {
        st->setValue("Theme/FontFamily", f.family());
    }

    f.setFamily(strFont);
    f.setPixelSize(9);

    return QFontMetrics(f).height();
}

int WizStyleHelper::fontCategoryItem(QFont& f)
{
#ifdef Q_OS_MAC
    f.setPixelSize(13);
#endif
    return QFontMetrics(f).height();
}

int WizStyleHelper::fontSection(QFont& f)
{
#ifdef Q_OS_MAC
    f.setPixelSize(11);
#endif
    return QFontMetrics(f).height();
}

int WizStyleHelper::editorButtonHeight()
{
    return 28;
}

QMargins WizStyleHelper::editorBarMargins()
{
    return QMargins(WizSmartScaleUI(14), 0, WizSmartScaleUI(14), 0);
}

int WizStyleHelper::titleEditorHeight()
{
    return WizSmartScaleUI(30);
}

int WizStyleHelper::editToolBarHeight()
{
    return 30;
}

int WizStyleHelper::infoBarHeight()
{
    return 34;
}

int WizStyleHelper::tagBarHeight()
{
    return editToolBarHeight();
}

int WizStyleHelper::notifyBarHeight()
{
    return WizSmartScaleUI(32);
}

QVariant WizStyleHelper::getValue(const QString& key, const QVariant& defaultValue)
{
    if (!m_settings) {
        m_settings = new WizSettings(WizPathResolve::themePath(themeName()) + "skin.ini");
    }

    return m_settings->value(key, defaultValue);
}

QRect WizStyleHelper::initListViewItemPainter(QPainter* p, const QRect& lrc, ListViewBGType bgType, bool useFullSeperatorLine, QColor color)
{
    QRect rc = lrc;

    Utils::WizStyleHelper::drawListViewItemBackground(p, rc, bgType, color);

    Utils::WizStyleHelper::drawListViewItemSeperator(p, rc, bgType, useFullSeperatorLine);

    int nMargin = Utils::WizStyleHelper::margin();
    return rc.adjusted(nMargin, nMargin, -nMargin, -nMargin);
}

void WizStyleHelper::drawListViewItemThumb(QPainter* p, const QRect& rc, int nBadgeType,
                                        const QString& title, const QStringList& lead, const QString& location,
                                        const QString& abs, bool bFocused, bool bSelected, QPixmap thumbPix,
                                        QColor textColor)
{
    if (bFocused || bSelected) {
        textColor = QColor();
    }

    QRect rcd = rc.adjusted(2, 0, 0, 0); //

    QFont fontTitle = p->font();
    int nFontHeight = Utils::WizStyleHelper::fontHead(fontTitle);

    if (!title.isEmpty()) {
        bool drawEncrpytIconInTitle = nBadgeType & DocTypeEncrytedInTitle;
        bool drawAttachmentInTitle = nBadgeType & DocTypeContainsAttachment;
        int nSpace4AttachIcon = drawEncrpytIconInTitle * 16 + drawAttachmentInTitle * 16;
        QRect rcTitle = rcd.adjusted(0, 4, 0, 0);
        //
        if (nBadgeType & DocTypeAlwaysOnTop)
        {
            QString strBadgeText(QObject::tr("[ Top ]"));
            QColor colorType = Utils::WizStyleHelper::listViewItemType(bSelected, bFocused);
            rcTitle = Utils::WizStyleHelper::drawText(p, rcTitle, strBadgeText, 1, Qt::AlignVCenter | Qt::AlignLeft, textColor.isValid() ? textColor : colorType, fontTitle);
            rcTitle.setCoords(rcTitle.right(), rcTitle.y(), rcd.right(), rcd.bottom());
        }

        //
        QString strTitle(title);
        strTitle.replace("\n", "");
        strTitle.replace("\r", " ");
        rcTitle.adjust(0, 0, - nSpace4AttachIcon, 0);
        QColor colorTitle = Utils::WizStyleHelper::listViewItemTitle(bSelected, bFocused);
        QRect rcAttach = Utils::WizStyleHelper::drawText(p, rcTitle, strTitle, 1, Qt::AlignVCenter, textColor.isValid() ? textColor : colorTitle, fontTitle);
        int titleHeight = rcAttach.height();

        rcAttach.setCoords(rcAttach.right(), rcAttach.top(), rcd.right(), rcTitle.bottom());
        rcAttach.setHeight(nFontHeight);
        if (drawEncrpytIconInTitle) {
            QRect rcEncrypt = Utils::WizStyleHelper::drawBadgeIcon(p, rcAttach, BadgeEncryptedInTitle, bFocused, false);
            rcAttach.setCoords(rcEncrypt.right() + 4, rcAttach.top(), rcd.right(), rcTitle.bottom());
        }

        if (drawAttachmentInTitle) {
            rcAttach = Utils::WizStyleHelper::drawBadgeIcon(p, rcAttach, BadgeAttachment, bFocused, false);
        }

        rcd.adjust(0, titleHeight, 0, 0);
    }

    QFont fontThumb;
    nFontHeight = Utils::WizStyleHelper::fontThumb(fontThumb);
    QPixmap pixGreyPoint(Utils::WizStyleHelper::skinResourceFileName("document_grey_point", true));
    QRect rcLead = rcd;   //排序类型或标签等
    int nLeadHeight;
    if (!lead.isEmpty()) {
        for (int i = 0; i < lead.count(); i++) {
            QString strInfo(lead.at(i));
            if (strInfo.isEmpty())
                continue;

            if (i > 0) {
                QRect rcGreyPoint = rcLead.adjusted(0, 8, 0, 0);
                rcGreyPoint.setWidth(4);
                rcGreyPoint.setHeight(4);
                Utils::WizStyleHelper::drawPixmapWithScreenScaleFactor(p, rcGreyPoint, pixGreyPoint);
                rcLead.adjust(6, 0, 0, 0);
            }

            QColor colorDate = Utils::WizStyleHelper::listViewItemLead(bSelected, bFocused);
            rcLead = Utils::WizStyleHelper::drawText(p, rcLead, strInfo, 1, Qt::AlignVCenter, textColor.isValid() ? textColor : colorDate, fontThumb);
            nLeadHeight = rcLead.height();
            rcLead = rcd.adjusted(rcLead.width() + rcLead.x() - rcd.x(), 0, 0, 0);
        }
    }

    if (!location.isEmpty()) {
        QRect rcGreyPoint = rcLead.adjusted(0, 8, 0, 0);
        rcGreyPoint.setWidth(4);
        rcGreyPoint.setHeight(4);
        Utils::WizStyleHelper::drawPixmapWithScreenScaleFactor(p, rcGreyPoint, pixGreyPoint);
        rcLead.adjust(4, 0, 0, 0);

        QColor colorLocation = Utils::WizStyleHelper::listViewItemLocation(bSelected, bFocused);
        QString strInfo(location);
        rcLead = Utils::WizStyleHelper::drawText(p, rcLead, strInfo, 1, Qt::AlignVCenter, textColor.isValid() ? textColor : colorLocation,
                                              fontThumb, true, Qt::ElideMiddle);
        nLeadHeight = rcLead.height();
    }

    QRect rcSummary(rcd.adjusted(0, nLeadHeight + 8, 0, 0));
    if (nBadgeType & DocTypeEncrytedInSummary) {
        QIcon badgeIcon(listViewBadge(BadgeEncryptedInSummary));
        QSize sz = badgeIcon.availableSizes().first();
        QRect rcPix(rcSummary.x() + (rcSummary.width() - sz.width()) / 2, rcSummary.y() + (rcSummary.height() - sz.height()) / 2,
                    sz.width(), sz.height());
        if (bSelected && bFocused) {
            badgeIcon.paint(p, rcPix, Qt::AlignCenter, QIcon::Active, QIcon::On);
        } else {
            badgeIcon.paint(p, rcPix, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }

    } else {

        if (!thumbPix.isNull()) {
            QRect rcPix = drawThumbnailPixmap(p, rcd, thumbPix);
            rcSummary.setRight(rcPix.left() - 4);
        }

        if (!abs.isEmpty()) {          //  笔记内容
            QString strText(abs);
            rcSummary.adjust(0, -4, 0, 0);
            p->setClipRect(rcSummary);
            QColor colorSummary = Utils::WizStyleHelper::listViewItemSummary(bSelected, bFocused);
            if (!strText.isEmpty()) {
                Utils::WizStyleHelper::drawText(p, rcSummary, strText, 2, Qt::AlignVCenter, textColor.isValid() ? textColor : colorSummary, fontThumb);
            }
            p->setClipRect(rc);
        }
    }
}



void WizStyleHelper::drawListViewItemSearchResult(QPainter* p, const QRect& rc, const QString& title, const QString& info,
                                  const QString& abs, bool bFocused, bool bSelected, QColor textColor)
{
    QString newTitle = title;
    newTitle = newTitle.replace("<", "&lt;");
    newTitle = newTitle.replace(">", "&gt;");
    newTitle = newTitle.replace("&lt;em&gt;", "<em>");
    newTitle = newTitle.replace("&lt;/em&gt;", "</em>");
    //
    if (bFocused || bSelected) {
        textColor = QColor();
    }

    QColor colorSummary = Utils::WizStyleHelper::listViewItemSummary(bSelected, bFocused);
    QString summaryHtmlColor = "#" + ::WizColorToString(textColor.isValid() ? textColor : colorSummary);
    //
    QColor colorInfo = Utils::WizStyleHelper::listViewItemLocation(bSelected, bFocused);
    QString infoHtmlColor = "#" + ::WizColorToString(textColor.isValid() ? textColor : colorInfo);

    QColor titleColor = Utils::WizStyleHelper::listViewItemTitle(bSelected, bFocused);
    QString titleHtmlColor = "#" + ::WizColorToString(textColor.isValid() ? textColor : titleColor);
    //
    QString infoHtmlColorHtml = "<font color='" + infoHtmlColor + "'>";
    QString infoHtmlColorHtmlEnd = "</font>";
    //
    QString summaryHtmlColorHtml = "<font color='" + summaryHtmlColor + "'>";
    QString summaryHtmlColorHtmlEnd = "</font>";
    //
    QString titleHtmlColorHtml = "<font color='" + titleHtmlColor + "'>";
    QString titleHtmlColorHtmlEnd = "</font>";
    //
    QString title2 = "<div style='font-size:14px;line-height:130%'>" + titleHtmlColorHtml + newTitle + titleHtmlColorHtmlEnd + "</div>";
    //
    QString info2 = infoHtmlColorHtml + info + infoHtmlColorHtmlEnd;
    QString abs2 = summaryHtmlColorHtml + abs + summaryHtmlColorHtmlEnd;
    //
    QString other = "<div style='font-size:12px;line-height:140%'>" + info2  + "<br />" + abs2 + "</div>";

    QString html = title2 + other;
    //
    html = html.replace("<em>", "<font color='red'>");
    html = html.replace("</em>", "</font>");
    //
    QRect rcd = rc.adjusted(2, 0, 0, 0); //
    //
    QTextDocument* doc = new QTextDocument(NULL);
    doc->setUndoRedoEnabled(false);
    doc->setHtml(html);
    doc->setTextWidth(rcd.width());
    doc->setUseDesignMetrics(true);
    ////doc->setDefaultTextOption ( QTextOption (Qt::AlignHCenter )  );
    //// height from doc QTextDocument
    //// http://fop-miniscribus.googlecode.com/svn/trunk/fop_miniscribus.1.0.0/src/floating_box/floatdiagram.cpp
    //////setMaximumHeight(DocumentHighgtActual());

    //
    p->save();
    p->translate(rcd.topLeft());
    QRect rc2(0, 0, rcd.width(), rcd.height());
    doc->drawContents(p, rc2);
    p->restore();
    //
    doc->deleteLater();
}


} // namespace Utils
