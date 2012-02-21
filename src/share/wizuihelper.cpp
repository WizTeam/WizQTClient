#include "wizuihelper.h"

#ifndef Q_OS_MAC

#include "share/wizmisc.h"
#include <QLabel>


QWidget* createSearchWidget(CWizSearchBox* searchBox)
{
    QWidget* widget = new QWidget(searchBox);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    widget->setSizePolicy(sizePolicy);
    //
    QIcon icon = ::WizLoadSkinIcon("search");
    //
    QLabel* iconLabel = new QLabel(widget);
    iconLabel->setPixmap(icon.pixmap(16, 16));
    //
    QLineEdit* editSearch = new QLineEdit(widget);
    //
    iconLabel->setStyleSheet("border-width:0;border-style:outset");
    editSearch->setStyleSheet("border-width:0;border-style:outset");
    //
    QHBoxLayout* layout = new QHBoxLayout(widget);
    widget->setLayout(layout);
    //
    layout->setSpacing(0);
    layout->setMargin(1);
    layout->addWidget(iconLabel);
    layout->addWidget(editSearch);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    //
    widget->setContentsMargins(1, 1, 1, 1);
    widget->setMinimumHeight(std::max<int>(16, editSearch->sizeHint().height()));
    widget->setMinimumWidth(250);
    //
    widget->setStyleSheet("QWidget{background-color:#ffffff;border-color:#bbbbbb;border-width:1;border-style:solid}");
    //
    CWizSearchBox::connect(editSearch, SIGNAL(editingFinished()), searchBox, SLOT(on_search_editingFinished()));
    CWizSearchBox::connect(editSearch, SIGNAL(textEdited(QString)), searchBox, SLOT(on_search_edited(QString)));
    //
    return widget;
}

CWizSearchBox::CWizSearchBox(QWidget* parent /*= 0*/)
    : QWidget(parent)
    , m_search(createSearchWidget(this))
{
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    //
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(layout);
    layout->setMargin(0);
    //
    layout->addWidget(m_search);
    layout->setStretch(0, 1);
}

#endif
void CWizSearchBox::on_search_editingFinished()
{
    emit doSearch(m_keywords);
}
void CWizSearchBox::on_search_edited(const QString& str)
{
    m_keywords = str;
}




QSize CWizSearchBox::sizeHint() const
{
#ifdef Q_OS_MAC
    return m_search->sizeHint() + QSize(6, 2);
#else
    return m_search->sizeHint() + QSize(8, 4);
#endif
}



CWizSpacer::CWizSpacer(QWidget *parent)
:QWidget(parent)
{
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
}




/////////////////////////////////////////////////////////////////////////////



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
    arrayRect[1] = QRect(QPoint(x2, y1), QPoint(x3, y2));
    arrayRect[2] = QRect(QPoint(x3, y1), QPoint(x4, y2));

    arrayRect[3] = QRect(QPoint(x1, y2), QPoint(x2, y3));
    arrayRect[4] = QRect(QPoint(x2, y2), QPoint(x3, y3));
    arrayRect[5] = QRect(QPoint(x3, y2), QPoint(x4, y3));

    arrayRect[6] = QRect(QPoint(x1, y3), QPoint(x2, y4));
    arrayRect[7] = QRect(QPoint(x2, y3), QPoint(x3, y4));
    arrayRect[8] = QRect(QPoint(x3, y3), QPoint(x4, y4));
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
        if (rcDest.width() > 255)
        {
            int i = 0;
            i = 1;
        }
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


