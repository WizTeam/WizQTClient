#include "stylehelper.h"

#include <QFontMetrics>
#include <QSize>
#include <QRect>
#include <QTransform>
#include <QPainter>
#include <QVector>
#include <QTextLayout>
#include <QDebug>
#include <QApplication>

#include <extensionsystem/pluginmanager.h>

#include "pathresolve.h"
#include "../share/wizmisc.h"
#include "../share/wizsettings.h"

#ifdef Q_OS_MAC
#include "mac/wizmachelper.h"
#endif

namespace Utils {

CWizSettings* StyleHelper::m_settings = 0;

void StyleHelper::initPainterByDevice(QPainter* p)
{
#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0); // factor == 2 on retina

    QTransform trans;
    trans.scale(factor, factor);
    p->setWorldTransform(trans);
#endif
}

QPixmap StyleHelper::pixmapFromDevice(const QSize& sz)
{
#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0); // factor == 2 on retina
    QSize sz2 = sz * factor;
#else
    QSize sz2 = sz;
#endif

    return QPixmap(sz2);
}

QSize StyleHelper::applyScreenScaleFactor(const QSize& sz)
{
#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0); // factor == 2 on retina
    QSize sz2 = sz * factor;
#else
    QSize sz2 = sz;
#endif

    return sz2;
}

QString StyleHelper::themeName()
{
    QSettings* st = ExtensionSystem::PluginManager::settings();
    QString strTheme = st->value("Theme/Name").toString();
    if (strTheme.isEmpty()) {
        st->setValue("Theme/Name", "default");
        return "default";
    }

    return strTheme;
}

QString StyleHelper::skinResourceFileName(const QString& strName, bool need2x)
{
    bool use2x = need2x && ::WizIsHighPixel();
    return ::WizGetSkinResourceFileName(themeName(),
                                        (use2x ? strName + "@2x" : strName));
}

QIcon StyleHelper::loadIcon(const QString& strName)
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

QRegion StyleHelper::borderRadiusRegion(const QRect& rect)
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

QRegion StyleHelper::borderRadiusRegionWithTriangle(const QRect& rect, bool triangleAlginLeft,
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

QColor StyleHelper::splitterLineColor()
{
    return QColor("#DBDBDB");
}


QString StyleHelper::wizCommonListViewStyleSheet()
{
    return QString("QListView{ border-width: 1px; \
        background-color:#FFFFFF; \
        padding: 1px; \
        border-style: solid; \
        border-color: #ECECEC; \
        border-radius: 5px; \
        border-bottom-color:#E0E0E0;}");
}

QString StyleHelper::wizCommonStyleSheet()
{
    QString location = Utils::PathResolve::skinResourcesPath(themeName()) + "style.qss";
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

QString StyleHelper::wizCommonScrollBarStyleSheet(int marginTop)
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

QSize StyleHelper::treeViewItemIconSize()
{
    return QSize(14, 14);
}

int StyleHelper::treeViewItemHeight()
{
    return 28;
}

QColor StyleHelper::treeViewBackground()
{
    return QColor(getValue("Category/Background", "#F2F0EE").toString());
}

QColor StyleHelper::treeViewItemBackground(int stat)
{    
    if (stat == Selected) {
        return QColor(getValue("Category/ItemSelectedNoFocus", "#cecece").toString());
    } else if (stat == Active) {
        return QColor(getValue("Category/ItemSelected", "#5990EF").toString());
    }

    Q_ASSERT(0);
    return QColor();
}

QColor StyleHelper::treeViewItemCategoryBackground()
{    
    QColor co(getValue("Category/ItemCategory", "#ffffff").toString());
    co.setAlpha(15);
    return co;
}

QColor StyleHelper::treeViewItemCategoryText()
{
    return QColor(getValue("Category/ItemCategoryText", "#777775").toString());
}
QColor StyleHelper::treeViewItemLinkText()
{    
    return QColor(getValue("Category/ItemLinkText", "#a0aaaf").toString());
}

QColor StyleHelper::treeViewItemBottomLine()
{    
    return QColor(getValue("Category/ItemBottomLine", "#DFDFD7").toString());
}

QColor StyleHelper::treeViewItemMessageBackground()
{    
    return QColor(getValue("Category/ItemMessageBackground", "#3498DB").toString());
}

QColor StyleHelper::treeViewItemMessageText()
{    
    return QColor(getValue("Category/ItemMessageText", "#FFFFFF").toString());
}

QColor StyleHelper::treeViewItemText(bool bSelected, bool bSecondLevel)
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

QColor StyleHelper::treeViewItemTextExtend(bool bSelected)
{    
    if (bSelected) {
        return QColor(getValue("Category/TextExtendSelected", "#e6e6e6").toString());
    } else {
        return QColor(getValue("Category/TextExtend", "#888888").toString());
    }
}

QColor StyleHelper::treeViewSectionItemText()
{
    return QColor(getValue("Category/SectionItemText", "#C1C1C1").toString());
}

void StyleHelper::drawTreeViewItemBackground(QPainter* p, const QRect& rc, bool bFocused)
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

void StyleHelper::drawTreeViewItemIcon(QPainter* p, const QRect& rc, const QIcon& icn, bool bSelected)
{
    if (bSelected) {
        icn.paint(p, rc, Qt::AlignCenter, QIcon::Selected);
    } else {
        icn.paint(p, rc, Qt::AlignCenter, QIcon::Normal);
    }
}

void StyleHelper::drawTreeViewBadge(QPainter* p, const QRect& rc, const QString& str)
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

void StyleHelper::drawPixmapWithScreenScaleFactor(QPainter* p, const QRect& rcOrign, const QPixmap& pix)
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

int StyleHelper::listViewSortControlWidgetHeight()
{    
    return getValue("Documents/SortControlWidgetHeight", 20).toInt();
}

int StyleHelper::messageViewItemHeight()
{
    return 83;
}

int StyleHelper::listViewItemHeight(int nType)
{
//    QFont f;
    switch (nType) {
    case ListTypeOneLine:
        return 38;
//        return fontHead(f) + margin() * 4;
    case ListTypeTwoLine:
        return 68;
//        return fontHead(f) + fontNormal(f) + margin() * 5;
    case ListTypeThumb:
        return 122;
//        return thumbnailHeight() + margin() * 2;
    case ListTypeSection:
        return 20;
    default:
        Q_ASSERT(0);
        return 0;
    }
}


QColor StyleHelper::listViewBackground()
{    
    return QColor(getValue("Documents/Background", "#ffffff").toString());
}

int StyleHelper::listViewItemHorizontalPadding()
{
    return 6;
}

QColor StyleHelper::listViewItemSeperator()
{    
    return QColor(getValue("Documents/Line", "#e7e7e7").toString());
}

QColor StyleHelper::listViewSectionItemText()
{    
    return QColor(getValue("Documents/SectionItemText", "#a7a7a7").toString());
}

QColor StyleHelper::listViewSectionItemBackground()
{    
    return QColor(getValue("Documents/SectionItemBackground", "#f7f7f7").toString());
}

QColor StyleHelper::listViewItemBackground(int stat)
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

QColor StyleHelper::listViewItemType(bool bSelected, bool bFocused)
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

QColor StyleHelper::listViewItemTitle(bool bSelected, bool bFocused)
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

QColor StyleHelper::listViewItemLead(bool bSelected, bool bFocused)
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

QColor StyleHelper::listViewItemLocation(bool bSelected, bool bFocused)
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

QColor StyleHelper::listViewItemSummary(bool bSelected, bool bFocused)
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

QColor StyleHelper::listViewMultiLineFirstLine(bool bSelected)
{    
    if (bSelected) {
        return QColor(getValue("MultiLineList/FirstSelected", "#000000").toString());
    } else {
        return QColor(getValue("MultiLineList/First", "#000000").toString());
    }
}

QColor StyleHelper::listViewMultiLineOtherLine(bool bSelected)
{    
    if (bSelected) {
        return QColor(getValue("MultiLineList/OtherSelected", "#666666").toString());
    } else {
        return QColor(getValue("MultiLineList/Other", "#666666").toString());
    }
}

QIcon StyleHelper::listViewBadge(int type)
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

void StyleHelper::drawListViewItemBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect)
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


void StyleHelper::drawListViewItemBackground(QPainter* p, const QRect& rc, StyleHelper::ListViewBGType bgType)
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
    default:
        break;
    }
}

void StyleHelper::drawListViewItemSeperator(QPainter* p, const QRect& rc, ListViewBGType bgType,
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
        const int nSeperatorLeftMargin = 12;
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

QSize StyleHelper::avatarSize(bool bNoScreenFactor)
{
    int nHeight = avatarHeight(bNoScreenFactor);
    return QSize(nHeight, nHeight);
}

int StyleHelper::avatarHeight(bool bNoScreenFactor)
{
    QFont f;
    int nHeight = 36;//fontHead(f) + fontNormal(f) + margin() * 3 ;
    if (bNoScreenFactor)
        return nHeight;

#ifdef Q_OS_LINUX
    return nHeight;
#else
    float factor = qt_mac_get_scalefactor(0); 
    return nHeight * factor;
#endif
}

QRect StyleHelper::drawAvatar(QPainter* p, const QRect& rc, const QPixmap& pm)
{
    QRect rectAvatar = rc;
    rectAvatar.setSize(avatarSize(true));
    drawPixmapWithScreenScaleFactor(p, rectAvatar, pm);    

    return rectAvatar;
}
int StyleHelper::drawSingleLineText(QPainter* p, const QRect& rc, QString& str, int nFlags, const QColor& color, const QFont& font)
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

void StyleHelper::drawListViewItemSeperator(QPainter* p, const QRect& rc)
{
    QRect rcLine = rc;
    rcLine.adjust(12, 0, 0, 0);
    p->save();
    p->setPen(listViewItemSeperator());
    p->drawLine(rcLine.bottomLeft(), rcLine.bottomRight());
    p->restore();
}

QRect StyleHelper::drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
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

QRect StyleHelper::drawThumbnailPixmap(QPainter* p, const QRect& rc, const QPixmap& pm)
{
    if (pm.isNull()) {
        qDebug() << "[WARNING]pixmap is null when drawing thumbnail";
        return QRect(rc.x(), rc.y(), 0, 0);
    }

    QRect rcd(rc.x() + rc.right() - 66, rc.y() + rc.height() - 60, 50, 50);

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
    path.addRoundedRect(rcd, 4, 4);
    p->setClipPath(path);
    p->drawPixmap(rcd, pm);
    p->restore();
    return rcd;
}

QRect StyleHelper::drawBadgeIcon(QPainter* p, const QRect& rc, int height, int type, bool bFocus, bool bSelect)
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

QRect StyleHelper::drawBadgeIcon(QPainter* p, const QRect& rc, BadgeType nType,  bool bFocus, bool bSelect)
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

int StyleHelper::lineSpacing()
{
    return 4;
}

int StyleHelper::leading()
{
    return 3;
}

int StyleHelper::margin()
{
    return 5;
}

int StyleHelper::thumbnailHeight()
{
    QFont f;
    return fontHead(f) + fontNormal(f) * 3 + margin() * 5;
}

QPolygonF StyleHelper::bubbleFromSize(const QSize& sz, int nAngle, bool bAlignLeft)
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

int StyleHelper::fontHead(QFont& f)
{

#ifdef Q_OS_MAC
//    QSettings* st = ExtensionSystem::PluginManager::settings();
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

int StyleHelper::fontNormal(QFont& f)
{
#ifdef Q_OS_MAC
//    QSettings* st = ExtensionSystem::PluginManager::settings();
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

int StyleHelper::fontThumb(QFont& f)
{
#ifdef Q_OS_MAC
    f.setPixelSize(12);
#endif
    return QFontMetrics(f).height();
}

int StyleHelper::fontExtend(QFont& f)
{
    QSettings* st = ExtensionSystem::PluginManager::settings();
    QString strFont = st->value("Theme/FontFamily").toString();
    if (strFont.isEmpty()) {
        st->setValue("Theme/FontFamily", f.family());
    }

    f.setFamily(strFont);
    f.setPixelSize(9);

    return QFontMetrics(f).height();
}

int StyleHelper::fontCategoryItem(QFont& f)
{
#ifdef Q_OS_MAC
    f.setPixelSize(13);
#endif
    return QFontMetrics(f).height();
}

int StyleHelper::fontSection(QFont& f)
{
#ifdef Q_OS_MAC
    f.setPixelSize(11);
#endif
    return QFontMetrics(f).height();
}

int StyleHelper::editorButtonHeight()
{
    return 28;
}

QMargins StyleHelper::editorBarMargins()
{
    return QMargins(14, 0, 14, 0);
}

int StyleHelper::titleEditorHeight()
{
    return 30;
}

int StyleHelper::editToolBarHeight()
{
    return 30;
}

int StyleHelper::infoBarHeight()
{
    return 34;
}

int StyleHelper::tagBarHeight()
{
    return editToolBarHeight();
}

int StyleHelper::notifyBarHeight()
{
    return 32;
}

QVariant StyleHelper::getValue(const QString& key, const QVariant& defaultValue)
{
    if (!m_settings) {
        m_settings = new CWizSettings(PathResolve::themePath(themeName()) + "skin.ini");
    }

    return m_settings->value(key, defaultValue);
}

QRect StyleHelper::initListViewItemPainter(QPainter* p, const QRect& lrc, ListViewBGType bgType, bool useFullSeperatorLine)
{
    QRect rc = lrc;

    Utils::StyleHelper::drawListViewItemBackground(p, rc, bgType);

    Utils::StyleHelper::drawListViewItemSeperator(p, rc, bgType, useFullSeperatorLine);

    int nMargin = Utils::StyleHelper::margin();
    return rc.adjusted(nMargin, nMargin, -nMargin, -nMargin);
}

void StyleHelper::drawListViewItemThumb(QPainter* p, const QRect& rc, int nBadgeType,
                                        const QString& title, const QStringList& lead, const QString& location,
                                        const QString& abs, bool bFocused, bool bSelected, QPixmap thumbPix)
{
    QRect rcd = rc.adjusted(2, 0, 0, 0); //

    QFont fontTitle = p->font();
    int nFontHeight = Utils::StyleHelper::fontHead(fontTitle);

    if (!title.isEmpty()) {
        bool drawEncrpytIconInTitle = nBadgeType & DocTypeEncrytedInTitle;
        bool drawAttachmentInTitle = nBadgeType & DocTypeContainsAttachment;
        int nSpace4AttachIcon = drawEncrpytIconInTitle * 16 + drawAttachmentInTitle * 16;
        QRect rcTitle = rcd.adjusted(0, 4, 0, 0);
        //
        if (nBadgeType & DocTypeAlwaysOnTop)
        {
            QString strBadgeText(QObject::tr("[ Top ]"));
            QColor colorType = Utils::StyleHelper::listViewItemType(bSelected, bFocused);
            rcTitle = Utils::StyleHelper::drawText(p, rcTitle, strBadgeText, 1, Qt::AlignVCenter | Qt::AlignLeft, colorType, fontTitle);
            rcTitle.setCoords(rcTitle.right(), rcTitle.y(), rcd.right(), rcd.bottom());
        }

        //
        QString strTitle(title);
        strTitle.replace("\n", "");
        strTitle.replace("\r", " ");
        rcTitle.adjust(0, 0, - nSpace4AttachIcon, 0);
        QColor colorTitle = Utils::StyleHelper::listViewItemTitle(bSelected, bFocused);
        QRect rcAttach = Utils::StyleHelper::drawText(p, rcTitle, strTitle, 1, Qt::AlignVCenter, colorTitle, fontTitle);
        int titleHeight = rcAttach.height();

        rcAttach.setCoords(rcAttach.right(), rcAttach.top(), rcd.right(), rcTitle.bottom());
        rcAttach.setHeight(nFontHeight);
        if (drawEncrpytIconInTitle) {
            QRect rcEncrypt = Utils::StyleHelper::drawBadgeIcon(p, rcAttach, BadgeEncryptedInTitle, bFocused, false);
            rcAttach.setCoords(rcEncrypt.right() + 4, rcAttach.top(), rcd.right(), rcTitle.bottom());
        }

        if (drawAttachmentInTitle) {
            rcAttach = Utils::StyleHelper::drawBadgeIcon(p, rcAttach, BadgeAttachment, bFocused, false);
        }

        rcd.adjust(0, titleHeight, 0, 0);
    }

    QFont fontThumb;
    nFontHeight = Utils::StyleHelper::fontThumb(fontThumb);
    QPixmap pixGreyPoint(Utils::StyleHelper::skinResourceFileName("document_grey_point", true));
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
                Utils::StyleHelper::drawPixmapWithScreenScaleFactor(p, rcGreyPoint, pixGreyPoint);
                rcLead.adjust(6, 0, 0, 0);
            }

            QColor colorDate = Utils::StyleHelper::listViewItemLead(bSelected, bFocused);
            rcLead = Utils::StyleHelper::drawText(p, rcLead, strInfo, 1, Qt::AlignVCenter, colorDate, fontThumb);
            nLeadHeight = rcLead.height();
            rcLead = rcd.adjusted(rcLead.width() + rcLead.x() - rcd.x(), 0, 0, 0);
        }
    }

    if (!location.isEmpty()) {
        QRect rcGreyPoint = rcLead.adjusted(0, 8, 0, 0);
        rcGreyPoint.setWidth(4);
        rcGreyPoint.setHeight(4);
        Utils::StyleHelper::drawPixmapWithScreenScaleFactor(p, rcGreyPoint, pixGreyPoint);
        rcLead.adjust(4, 0, 0, 0);

        QColor colorLocation = Utils::StyleHelper::listViewItemLocation(bSelected, bFocused);
        QString strInfo(location);
        rcLead = Utils::StyleHelper::drawText(p, rcLead, strInfo, 1, Qt::AlignVCenter, colorLocation,
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
            QColor colorSummary = Utils::StyleHelper::listViewItemSummary(bSelected, bFocused);
            if (!strText.isEmpty()) {
                Utils::StyleHelper::drawText(p, rcSummary, strText, 2, Qt::AlignVCenter, colorSummary, fontThumb);
            }
            p->setClipRect(rc);
        }
    }
}


} // namespace Utils
