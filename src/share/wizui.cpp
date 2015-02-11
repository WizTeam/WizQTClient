#include "wizui.h"

#include <QPainter>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>

#include "wizsettings.h"
#include "wiznotestyle.h"

BOOL CWizSkin9GridImage::Clear()
{
    if (!m_img.isNull())
    {
        m_img = QImage();
    }
    return TRUE;
}

BOOL CWizSkin9GridImage::SplitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount)
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

BOOL CWizSkin9GridImage::SetImage(const CString& strImageFileName, QPoint ptTopLeft)
{
    Clear();
    //
    if (FAILED(m_img.load(strImageFileName)))
        return FALSE;
    //
    int nImageWidth = m_img.width();
    int nImageHeight = m_img.height();
    //
    return SplitRect(QRect(0, 0, nImageWidth, nImageHeight), ptTopLeft, m_arrayImageGrid, 9);
}

BOOL CWizSkin9GridImage::Valid() const
{
    return m_img.width() > 0 && m_img.height() > 0;
}

void CWizSkin9GridImage::Draw(QPainter* p, QRect rc, int nAlpha) const
{
    QRect arrayDest[9];
    //
    SplitRect(rc, m_arrayImageGrid[0].bottomRight(), arrayDest, 9);
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
void CWizSkin9GridImage::DrawBorder(QPainter* p, QRect rc) const
{
    QRect arrayDest[9];
    //
    SplitRect(rc, m_arrayImageGrid[0].bottomRight(), arrayDest, 9);
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




CWizIconLineEditContainer::CWizIconLineEditContainer(QWidget* parent)
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
    m_edit->setStyleSheet(QString("QLineEdit{ border:none; color:#2F2F2F; "
                          "selection-background-color: #8ECAF1;}"));
    m_leftIcon = new QLabel(this);
    m_rightIcon = new QLabel(this);
    //
    m_layout->setSpacing(12);
    m_layout->setContentsMargins(10, 10, 10,10);
    //
    m_layout->addWidget(m_leftIcon);
    m_layout->addWidget(m_edit);
    m_layout->addWidget(m_rightIcon);

}

CWizIconLineEditContainer::~CWizIconLineEditContainer()
{
    if (m_background)
        delete m_background;
}
void CWizIconLineEditContainer::setBackgroundImage(QString fileName, QPoint pt)
{
    m_background = new CWizSkin9GridImage();
    m_background->SetImage(fileName, pt);
}

void CWizIconLineEditContainer::setLeftIcon(QString fileName)
{
    m_leftIcon->setPixmap(QPixmap(fileName));
}
void CWizIconLineEditContainer::setRightIcon(QString fileName)
{
    m_rightIcon->setPixmap(QPixmap(fileName));
}

void CWizIconLineEditContainer::setPlaceholderText(const QString &strText)
{
    m_edit->setPlaceholderText(strText);
}

void CWizIconLineEditContainer::setAutoClearRightIcon(bool bAutoClean)
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

void CWizIconLineEditContainer::paintEvent(QPaintEvent *event)
{
    if (m_background && m_background->Valid())
    {
        QPainter paint(this);
        m_background->Draw(&paint, rect(), 0);
    }
    else
    {
        QWidget::paintEvent(event);
    }
}


void CWizIconLineEditContainer::mousePressEvent(QMouseEvent *event)
{
    if (m_rightIcon->geometry().contains(event->pos()))
    {
        emit rightIconClicked();
    }
}

void CWizIconLineEditContainer::cleanRightIcon()
{
    m_rightIcon->setPixmap(QPixmap());
}


CWizImageButton::CWizImageButton(QWidget *parent)
    :QPushButton(parent)
{
}

void CWizImageButton::setButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
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

    CWizSettings settings(::WizGetSkinResourcePath(strSkinName) + "skin.ini");
    int clientMarginLeft = settings.GetInt(name, "MarginLeft", def.left());
    int clientMarginTop = settings.GetInt(name, "MarginTop", def.top());
    int clientMarginRight = settings.GetInt(name, "MarginRight", def.right());
    int clientMarginBottom = settings.GetInt(name, "MarginBottom", def.bottom());
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
