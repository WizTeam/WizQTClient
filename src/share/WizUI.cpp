#include "WizUI.h"

#include <QPainter>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>

#include "WizSettings.h"
#include "WizNoteStyle.h"
#include "WizUIBase.h"

BOOL WizSkin9GridImage::clear()
{
    if (!m_img.isNull())
    {
        m_img = QImage();
    }
    return TRUE;
}

BOOL WizSkin9GridImage::splitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount)
{
    ATLASSERT(nArrayCount == 9);
    //
    QRect* arrayRect = parrayRect;
    //
    int nWidth = rcSrc.width();
    int nHeight = rcSrc.height();
    //
    if (ptTopLeft.x() <= 0)
        return FALSE;
    if (ptTopLeft.y() <= 0)
        return FALSE;
    if (ptTopLeft.x() >= nWidth / 2)
        return FALSE;
    if (ptTopLeft.y() >= nHeight / 2)
        return FALSE;
    //
    int x1 = rcSrc.left() + 0;
    int x2 = rcSrc.left() + ptTopLeft.x();
    int x3 = rcSrc.left() + nWidth - ptTopLeft.x();
    int x4 = rcSrc.left() + nWidth;
    //
    int y1 = rcSrc.top() + 0;
    int y2 = rcSrc.top() + ptTopLeft.y();
    int y3 = rcSrc.top() + nHeight - ptTopLeft.y();
    int y4 = rcSrc.top() + nHeight;
    //
    arrayRect[0] = QRect(QPoint(x1, y1), QPoint(x2, y2));
    arrayRect[1] = QRect(QPoint(x2 + 1, y1), QPoint(x3, y2));
    arrayRect[2] = QRect(QPoint(x3 + 1, y1), QPoint(x4, y2));

    arrayRect[3] = QRect(QPoint(x1, y2 + 1), QPoint(x2, y3));
    arrayRect[4] = QRect(QPoint(x2 + 1, y2 + 1), QPoint(x3, y3));
    arrayRect[5] = QRect(QPoint(x3 + 1, y2 + 1), QPoint(x4, y3));

    arrayRect[6] = QRect(QPoint(x1, y3 + 1), QPoint(x2, y4));
    arrayRect[7] = QRect(QPoint(x2 + 1, y3 + 1), QPoint(x3, y4));
    arrayRect[8] = QRect(QPoint(x3 + 1, y3 + 1), QPoint(x4, y4));
    //
    return TRUE;
}

BOOL WizSkin9GridImage::setImage(const CString& strImageFileName, QPoint ptTopLeft, QColor darkColor)
{
    clear();
    //
    if (!m_img.load(strImageFileName))
        return FALSE;
    //
    if (isDarkMode() && darkColor != Qt::transparent) {
        m_img = qimageWithTintColor(m_img, darkColor);
    }

    //
    int nImageWidth = m_img.width();
    int nImageHeight = m_img.height();
    //
    return splitRect(QRect(0, 0, nImageWidth, nImageHeight), ptTopLeft, m_arrayImageGrid, 9);
}

BOOL WizSkin9GridImage::setImage(const QImage& image, QPoint ptTopLeft, QColor darkColor)
{
    clear();
    //
    if (isDarkMode() && darkColor != Qt::transparent) {
        m_img = qimageWithTintColor(image, darkColor);
    } else {
        m_img = image;
    }
    //
    int nImageWidth = m_img.width();
    int nImageHeight = m_img.height();
    //
    return splitRect(QRect(0, 0, nImageWidth, nImageHeight), ptTopLeft, m_arrayImageGrid, 9);
}

BOOL WizSkin9GridImage::valid() const
{
    return m_img.width() > 0 && m_img.height() > 0;
}

void WizSkin9GridImage::draw(QPainter* p, QRect rc, int nAlpha) const
{
    QRect arrayDest[9];
    //
    splitRect(rc, m_arrayImageGrid[0].bottomRight(), arrayDest, 9);
    //
    for (int i = 0; i < 9; i++)
    {
        const QRect& rcSrc = m_arrayImageGrid[i];
        const QRect& rcDest = arrayDest[i];
        //
        if (nAlpha > 0 && nAlpha <= 255)
        {
            p->drawImage(rcDest, m_img, rcSrc);
        }
        else
        {
            p->drawImage(rcDest, m_img, rcSrc);
        }
    }
}
void WizSkin9GridImage::drawBorder(QPainter* p, QRect rc) const
{
    QRect arrayDest[9];
    //
    splitRect(rc, m_arrayImageGrid[0].bottomRight(), arrayDest, 9);
    //
    for (int i = 0; i < 9; i++)
    {
        if (i == 4)
            continue;
        //
        const QRect& rcSrc = m_arrayImageGrid[i];
        const QRect& rcDest = arrayDest[i];
        //
        p->drawImage(rcDest, m_img, rcSrc);
    }
}

WizIconLineEditContainer::WizIconLineEditContainer(QWidget* parent)
    : QWidget(parent)
    , m_background(NULL)
    , m_layout(NULL)
    , m_edit(NULL)
    , m_leftIcon(NULL)
    , m_rightIcon(NULL)
{
    m_layout = new QHBoxLayout(this);
    m_edit = new QLineEdit(this);
    m_edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    if (isDarkMode()) {
        m_edit->setStyleSheet(QString("QLineEdit{ border:none; color:#999999; "
                              "selection-background-color: #cccccc;}"));
    } else {
        m_edit->setStyleSheet(QString("QLineEdit{ border:none; color:#2F2F2F; "
                              "selection-background-color: #8ECAF1;}"));
    }
    m_leftIcon = new QLabel(this);
    m_rightIcon = new QLabel(this);
    //
    m_layout->setSpacing(12);
    m_layout->setContentsMargins(10, 10, 10,10);
    //
    m_layout->addWidget(m_leftIcon);
    m_layout->addWidget(m_edit);
    m_layout->addWidget(m_rightIcon);
    //
#ifndef Q_OS_MAC
    if (isDarkMode()) {
        QString style = QString("background-color:%1").arg(WizColorLineEditorBackground.name());
        m_leftIcon->setStyleSheet(style);
        m_rightIcon->setStyleSheet(style);
        m_edit->setStyleSheet("color:#ffffff");
    }
#endif
}

WizIconLineEditContainer::~WizIconLineEditContainer()
{
    if (m_background)
        delete m_background;
}
void WizIconLineEditContainer::setBackgroundImage(QString fileName, QPoint pt, QColor darkColor)
{
    m_background = new WizSkin9GridImage();
    m_background->setImage(fileName, pt, darkColor);
}

void WizIconLineEditContainer::setLeftIcon(QString fileName)
{
    m_leftIcon->setPixmap(QPixmap(fileName));
}
void WizIconLineEditContainer::setRightIcon(QString fileName)
{
    m_rightIcon->setPixmap(QPixmap(fileName));
}

void WizIconLineEditContainer::setPlaceholderText(const QString &strText)
{
    m_edit->setPlaceholderText(strText);
}

void WizIconLineEditContainer::setAutoClearRightIcon(bool bAutoClean)
{
    if (bAutoClean)
    {
        connect(m_edit, SIGNAL(textEdited(QString)), SLOT(cleanRightIcon()));
    }
    else
    {
        disconnect(m_edit, SIGNAL(textEdited(QString)), this, SLOT(cleanRightIcon()));
    }
}

void WizIconLineEditContainer::paintEvent(QPaintEvent *event)
{
    if (m_background && m_background->valid())
    {
        QPainter paint(this);
        m_background->draw(&paint, rect(), 0);
    }
    else
    {
        QWidget::paintEvent(event);
    }
}


void WizIconLineEditContainer::mousePressEvent(QMouseEvent *event)
{
    if (m_rightIcon->geometry().contains(event->pos()))
    {
        emit rightIconClicked();
    }
}

void WizIconLineEditContainer::cleanRightIcon()
{
    m_rightIcon->setPixmap(QPixmap());
}


WizStyleButton::WizStyleButton(QWidget *parent)
    :QPushButton(parent)
{
}

void WizStyleButton::setButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                                     const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                                     const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor)
{
    QStyle* style = WizGetImageButtonStyle(normalBackgroundFileName, hotBackgroundFileName,
                                           downBackgroundFileName, disabledBackgroundFileName, normalTextColor,
                                           activeTextColor, disableTextColor);
    setStyle(style);
}




void WizInitWidgetMargins(const QString& strSkinName, QWidget* widget, const QString& name)
{
    QMargins def = widget->contentsMargins();

    WizSettings settings(::WizGetSkinResourcePath(strSkinName) + "skin.ini");
    int clientMarginLeft = settings.getInt(name, "MarginLeft", def.left());
    int clientMarginTop = settings.getInt(name, "MarginTop", def.top());
    int clientMarginRight = settings.getInt(name, "MarginRight", def.right());
    int clientMarginBottom = settings.getInt(name, "MarginBottom", def.bottom());
    widget->setContentsMargins(clientMarginLeft, clientMarginTop, clientMarginRight, clientMarginBottom);
}


QWidget* WizInitWidgetMarginsEx(const QString& strSkinName, QWidget* widget, const QString& name)
{
    QWidget* wrap = new QWidget(widget->parentWidget());
    QLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, wrap);

    wrap->setLayout(layout);
    layout->setMargin(0);

    WizInitWidgetMargins(strSkinName, wrap, name);
    layout->addWidget(widget);

    return wrap;
}

