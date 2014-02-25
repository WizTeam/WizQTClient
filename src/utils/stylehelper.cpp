#include "stylehelper.h"

#include <QFontMetrics>
#include <QSize>
#include <QRect>
#include <QTransform>
#include <QPainter>
#include <QSettings>
#include <QVector>
#include <QTextLayout>
#include <QDebug>
#include <QApplication>

#include <extensionsystem/pluginmanager.h>

#include "pathresolve.h"
#include "../share/wizmisc.h"

#ifdef Q_OS_MAC
#include "mac/wizmachelper.h"
#endif

namespace Utils {

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

QIcon StyleHelper::loadIcon(const QString& strName)
{
    QString strIconNormal = ::WizGetSkinResourceFileName(themeName(), strName);
    QString strIconActive1 = ::WizGetSkinResourceFileName(themeName(), strName+ "_on");
    QString strIconActive2 = ::WizGetSkinResourceFileName(themeName(), strName+ "_selected");

    if (!QFile::exists(strIconNormal)) {
        qDebug() << "load icon failed, filePath:" << strIconNormal;
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

QColor StyleHelper::treeViewBackground()
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    return QColor(st.value("Category/Background", "#808080").toString());
}

QColor StyleHelper::treeViewItemBackground(int stat)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (stat == Selected) {
        return QColor(st.value("Category/ItemSelectedNoFocus", "#26292b").toString());
    } else if (stat == Active) {
        return QColor(st.value("Category/ItemSelected", "#0073c5").toString());
    }

    Q_ASSERT(0);
    return QColor();
}

QColor StyleHelper::treeViewItemCategoryBackground()
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    QColor co(st.value("Category/ItemCategory", "#ffffff").toString());
    co.setAlpha(15);
    return co;
}

QColor StyleHelper::treeViewItemCategoryText()
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    return QColor(st.value("Category/ItemCategoryText", "#697882").toString());
}
QColor StyleHelper::treeViewItemLinkText()
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    return QColor(st.value("Category/ItemLinkText", "#a0aaaf").toString());
}

QColor StyleHelper::treeViewItemText(bool bSelected)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (bSelected) {
        return QColor(st.value("Category/TextSelected", "#ffffff").toString());
    } else {
        return QColor(st.value("Category/Text", "#a0aaaf").toString());
    }
}

QColor StyleHelper::treeViewItemTextExtend(bool bSelected)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (bSelected) {
        return QColor(st.value("Category/TextExtendSelected", "#e6e6e6").toString());
    } else {
        return QColor(st.value("Category/TextExtend", "#969696").toString());
    }
}

void StyleHelper::drawTreeViewItemBackground(QPainter* p, const QRect& rc, bool bFocused)
{
    QRect rcd(rc);
    QColor bg1 = treeViewItemBackground(Active);
    QColor bg2 = treeViewItemBackground(Selected);

    p->save();
    if (bFocused) {
        p->fillRect(rcd, bg1);
    } else {
        p->fillRect(rcd, bg2);
        rcd.setWidth(5); // FIXME
        p->fillRect(rcd, bg1);
    }
    p->restore();
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

int StyleHelper::listViewItemHeight(int nType)
{
    QFont f;
    switch (nType) {
    case ListTypeOneLine:
        return fontHead(f) + margin() * 4;
    case ListTypeTwoLine:
        return fontHead(f) + fontNormal(f) + margin() * 5;
    case ListTypeThumb:
        return thumbnailHeight() + margin() * 2;
    default:
        Q_ASSERT(0);
        return 0;
    }
}


QColor StyleHelper::listViewBackground()
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    return QColor(st.value("Documents/Background", "#ffffff").toString());
}

QColor StyleHelper::listViewItemSeperator()
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    return QColor(st.value("Documents/Line", "#d9dcdd").toString());
}

QColor StyleHelper::listViewItemBackground(int stat)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (stat == Normal) {
        return QColor(st.value("Documents/ItemLoseFocusBackground", "#e1e1e1").toString());
    } else if (stat == Active) {
        return QColor(st.value("Documents/ItemFocusBackground", "#0c8eec").toString());
    }

    Q_ASSERT(0);
    return QColor();
}

QColor StyleHelper::listViewItemTitle(bool bSelected, bool bFocused)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (bSelected) {
        if (bFocused) {
            return QColor(st.value("Documents/TitleFocus", "#ffffff").toString());
        } else {
            return QColor(st.value("Documents/TitleLoseFocus", "#6a6a6a").toString());
        }
    } else {
        return QColor(st.value("Documents/Title", "#464646").toString());
    }
}

QColor StyleHelper::listViewItemLead(bool bSelected, bool bFocused)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (bSelected) {
        if (bFocused) {
            return QColor(st.value("Documents/DateFocus", "#ffffff").toString());
        } else {
            return QColor(st.value("Documents/DateLoseFocus", "#6a6a6a").toString());
        }
    } else {
        return QColor(st.value("Documents/Date", "#0000ff").toString());
    }
}

QColor StyleHelper::listViewItemSummary(bool bSelected, bool bFocused)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (bSelected) {
        if (bFocused) {
            return QColor(st.value("Documents/SummaryFocus", "#ffffff").toString());
        } else {
            return QColor(st.value("Documents/SummaryLoseFocus", "#6a6a6a").toString());
        }
    } else {
        return QColor(st.value("Documents/Summary", "#8c8c8c").toString());
    }
}

QIcon StyleHelper::listViewBadge(int type)
{
    if (type == BadgeNormal) {
        return loadIcon("document_badge");
    } else if (type == BadgeEncryted){
        return loadIcon("document_badge_encrypted");
    }

    Q_ASSERT(0);
    return QIcon();
}

void StyleHelper::drawListViewItemBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect)
{
    if (bSelect) {
        if (bFocus) {
            p->fillRect(rc, listViewItemBackground(Active));
        } else {
            p->fillRect(rc, listViewItemBackground(Normal));
        }
    }
}

void StyleHelper::drawListViewItemSeperator(QPainter* p, const QRect& rc)
{
    QRect rcLine = rc;
    rcLine.adjust(5, 0, -5, 0);
    p->save();
    p->setPen(listViewItemSeperator());
    p->drawLine(rcLine.bottomLeft(), rcLine.bottomRight());
    p->restore();
}

QSize StyleHelper::avatarSize()
{
    return QSize(avatarHeight(), avatarHeight());
}

int StyleHelper::avatarHeight()
{
    QFont f;
    return fontHead(f) + fontNormal(f) + margin() * 3;
}

QRect StyleHelper::drawAvatar(QPainter* p, const QRect& rc, const QPixmap& pm)
{
    QRect rectAvatar = rc;
    rectAvatar.setSize(avatarSize());

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->drawPixmap(rectAvatar, pm);
    p->restore();

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
QRect StyleHelper::drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
                          int nFlags, const QColor& color, const QFont& font, bool bElided)
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
            lineText = p->fontMetrics().elidedText(str, Qt::ElideRight, rcRet.width());
            nWidth = qMax<int>(p->fontMetrics().width(lineText), nWidth);
        } else {
            lineText = str.left(line.textLength());
            nWidth = qMax<int>(line.width(), nWidth);
        }

        str.remove(0, line.textLength());
        p->drawText(rcRet, nFlags, lineText);

        nHeight += nHeightLine;
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

    QRect rcd = rc.adjusted(rc.width() - rc.height(), margin(), -margin(), -margin());

    int nWidth = 0, nHeight = 0;
    if (pm.width() > rcd.width() || pm.height() > rcd.height()) {
        double fRate = qMin<double>(double(rcd.width()) / pm.width(), double(rcd.height()) / pm.height());
        nWidth = int(pm.width() * fRate);
        nHeight = int(pm.height() * fRate);
    } else {
        nWidth = pm.width();
        nHeight = pm.height();
    }

    int adjustX = (rcd.width() - nWidth) / 2;
    int adjustY = (rcd.height() - nHeight) / 2;
    rcd.adjust(adjustX, adjustY, -adjustX, -adjustY);
    p->drawPixmap(rcd, pm);

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

int StyleHelper::lineSpacing()
{
    return 5;
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

QPolygon StyleHelper::bubbleFromSize(const QSize& sz, int nAngle, bool bAlignLeft)
{
    Q_ASSERT(sz.width() > 31);
    Q_ASSERT(sz.height() > 11);

    QVector<QPoint> ps;
    if (bAlignLeft) {
        ps.push_back(QPoint(0, nAngle));
        ps.push_back(QPoint(11, nAngle));
        ps.push_back(QPoint(11 + nAngle, 0));
        ps.push_back(QPoint(11 + nAngle * 2, nAngle));
        ps.push_back(QPoint(sz.width(), nAngle));
        ps.push_back(QPoint(sz.width(), sz.height()));
        ps.push_back(QPoint(0, sz.height()));
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

    return QPolygon(ps);
}

int StyleHelper::fontHead(QFont& f)
{
    QSettings* st = ExtensionSystem::PluginManager::settings();
    QString strFont = st->value("Theme/FontFamily").toString();
    if (strFont.isEmpty()) {
        st->setValue("Theme/FontFamily", f.family());
    }

    f.setFamily(strFont);
    //f.setPixelSize(13);
    f.setBold(true);

    return QFontMetrics(f).height();
}

int StyleHelper::fontNormal(QFont& f)
{
    QSettings* st = ExtensionSystem::PluginManager::settings();
    QString strFont = st->value("Theme/FontFamily").toString();
    if (strFont.isEmpty()) {
        st->setValue("Theme/FontFamily", f.family());
    }

    f.setFamily(strFont);
    //f.setPixelSize(12);

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

QRect StyleHelper::initListViewItemPainter(QPainter* p, QPixmap* pm, const QRect& lrc, bool bFocused, bool bSelected)
{
    QRect rc(0, 0, lrc.width(), lrc.height());

    pm->fill(Utils::StyleHelper::listViewBackground());

    p->begin(pm);
    Utils::StyleHelper::initPainterByDevice(p);
    Utils::StyleHelper::drawListViewItemSeperator(p, rc);
    Utils::StyleHelper::drawListViewItemBackground(p, rc, bFocused, bSelected);

    int nMargin = Utils::StyleHelper::margin();
    return rc.adjusted(nMargin, nMargin, -nMargin, -nMargin);
}

void StyleHelper::drawListViewItemThumb(QPainter* p, const QRect& rc, int nBadgeType,
                                        const QString& title, const QString& lead, const QString& abs,
                                        bool bFocused, bool bSelected)
{
    QRect rcd(rc);

    QFont fontTitle;
    int nFontHeight = Utils::StyleHelper::fontHead(fontTitle);

    if (!title.isEmpty()) {
        QRect rcTitle = Utils::StyleHelper::drawBadgeIcon(p, rcd, nFontHeight, nBadgeType, bFocused, bSelected);

        rcTitle.setCoords(rcTitle.right(), rcTitle.y(), rcd.right(), rcd.bottom());
        QString strTitle(title);
        QColor colorTitle = Utils::StyleHelper::listViewItemTitle(bSelected, bFocused);
        rcTitle = Utils::StyleHelper::drawText(p, rcTitle, strTitle, 1, Qt::AlignVCenter, colorTitle, fontTitle);
        rcd.adjust(0, rcTitle.height() + margin(), 0, 0);
    }

    QFont fontThumb;
    nFontHeight = Utils::StyleHelper::fontNormal(fontThumb);

    QRect rcLead;
    if (!lead.isEmpty()) {
        QString strInfo(lead);
        QColor colorDate = Utils::StyleHelper::listViewItemLead(bSelected, bFocused);
        rcLead = Utils::StyleHelper::drawText(p, rcd, strInfo, 1, Qt::AlignVCenter, colorDate, fontThumb);
    }

    if (!abs.isEmpty()) {
        QString strText(abs);
        QRect rcLine1(rcd.adjusted(rcLead.width(), 0, 0, 0));
        QColor colorSummary = Utils::StyleHelper::listViewItemSummary(bSelected, bFocused);
        rcLine1 = Utils::StyleHelper::drawText(p, rcLine1, strText, 1, Qt::AlignVCenter, colorSummary, fontThumb, false);

        if (!strText.isEmpty()) {
            QRect rcLine2(rcd.adjusted(0, rcLine1.height(), 0, 0));
            rcLine2 = Utils::StyleHelper::drawText(p, rcLine2, strText, 2, Qt::AlignVCenter, colorSummary, fontThumb);
        }
    }
}


} // namespace Utils
