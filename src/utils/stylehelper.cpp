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

#include "pathresolve.h"

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
    QSettings st(PathResolve::userSettingsFilePath());
    QString strTheme = st.value("theme/name").toString();
    if (strTheme.isEmpty()) {
        st.setValue("theme/name", "default");
        return "default";
    }

    return strTheme;
}

QColor StyleHelper::listViewSeperator()
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    return QColor(st.value("Documents/Line", "#d9dcdd").toString());
}

QColor StyleHelper::listViewBackground(int stat)
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

QColor StyleHelper::listViewTitle(int stat)
{
    QSettings st(PathResolve::themePath(themeName()) + "skin.ini");
    if (stat == Normal) {
        return QColor(st.value("Documents/TitleLoseFocus", "#6A6A6A").toString());
    } else if (stat == Active) {
        return QColor(st.value("Documents/Title", "#464646").toString());
    }

    Q_ASSERT(0);
    return QColor();
}

void StyleHelper::drawListViewBackground(QPainter* p, const QRect& rc, bool bFocus, bool bSelect)
{
    if (bSelect) {
        if (bFocus) {
            p->fillRect(rc, listViewBackground(Active));
        } else {
            p->fillRect(rc, listViewBackground(Normal));
        }
    }
}

void StyleHelper::drawListViewSeperator(QPainter* p, const QRect& rc)
{
    QRect rcLine = rc;
    rcLine.adjust(5, 0, -5, 0);
    p->save();
    p->setPen(listViewSeperator());
    p->drawLine(rcLine.bottomLeft(), rcLine.bottomRight());
    p->restore();
}

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

QRect StyleHelper::drawText(QPainter* p, const QRect& rc, QString& str, int nLines,
                          int nFlags, const QColor& color, const QFont& font)
{
    Q_ASSERT(rc.width() > 0);

    if (str.isEmpty()) {
        qDebug() << "[WARNING]: the text should not be empty when drawing!";
        return QRect();
    }

    QFontMetrics fm(font);
    if (rc.height() < (fm.height() + fm.leading()) * nLines) {
        qDebug() << "[WARNING]: space is not enough for drawing!";
    }

    p->save();
    p->setPen(color);
    p->setFont(font);

    int nWidth = 0;
    int nHeight = 0;
    int nHeightLine = p->fontMetrics().height() + leading();

    QRect rcRet(rc.x(), rc.y(), rc.width(), nHeightLine);
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
        if (nLines == 1) { // the last line
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

        nLines--;
    }
    textLayout.endLayout();

    rcRet.setRect(rc.x(), rc.y(), nWidth, nHeight);
    p->restore();

    return rcRet;
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
    int nExtra = QFontMetrics(f).height() * 2 + margin() * 2;
    return margin() * 2 + avatarHeight() + nExtra;
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

QFont StyleHelper::fontHead()
{
    QFont f;

    QSettings st(PathResolve::userSettingsFilePath());
    QString strFont = st.value("theme/fontFamily").toString();
    if (strFont.isEmpty()) {
        st.setValue("theme/fontFamily", f.family());
    }

    f.setFamily(strFont);
    f.setPixelSize(12);

    return f;
}

QFont StyleHelper::fontNormal()
{
    QFont f;

    QSettings st(PathResolve::userSettingsFilePath());
    QString strFont = st.value("theme/fontFamily").toString();
    if (strFont.isEmpty()) {
        st.setValue("theme/fontFamily", f.family());
    }

    f.setFamily(strFont);
    f.setPixelSize(12);

    return f;
}

} // namespace Utils
