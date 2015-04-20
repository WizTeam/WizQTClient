#include "wizSearchWidget.h"

#ifdef USECOCOATOOLBAR
#include "mac/wizSearchWidget_mm.h"
#else
#include "share/wizsettings.h"
#include "wizdef.h"
#include "utils/stylehelper.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>

CWizSearchWidget::CWizSearchWidget(QWidget* parent /* = 0 */)
    : QWidget(parent)
    , m_widthHint(360)
{
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    setContentsMargins(1, 0, 1, 1);

    m_editSearch = new CWizSearchEdit(this);
    m_editSearch->setTextMargins(20, 1, 0, 1);

    m_editSearch->setStyleSheet("QLineEdit{background-color:#eeeeee;border:1px solid #aeaeae; border-radius:10px;}"
                                "QLineEdit::focus{background-color:#ffffff;border:1px solid #6699cb; border-radius:10px;}");


    // avoid focus rect on OSX, this should be a bug of qt style sheet
    m_editSearch->setAttribute(Qt::WA_MacShowFocusRect, 0);

    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_editSearch);
    layout->setStretch(1, 1);


    connect(m_editSearch, SIGNAL(returnPressed()), \
            SLOT(on_search_returnPressed()));
    connect(m_editSearch, SIGNAL(textChanged(QString)), \
            SLOT(on_searchTextChanged(QString)), Qt::QueuedConnection);
}

void CWizSearchWidget::clear()
{
    m_editSearch->clear();
}

void CWizSearchWidget::focus()
{
    m_editSearch->selectAll();
    m_editSearch->setFocus();
}

QSize CWizSearchWidget::sizeHint() const
{
    return QSize(m_widthHint, height());
}

void CWizSearchWidget::setWidthHint(int nWidth)
{
    m_widthHint = nWidth;
}

void CWizSearchWidget::on_search_returnPressed()
{
    Q_EMIT doSearch(m_editSearch->text());
}

void CWizSearchWidget::on_searchTextChanged(QString str)
{
//    if (str.isEmpty())
//    {
//        Q_EMIT doSearch("");
//    }
}

CWizSearchEdit::CWizSearchEdit(QWidget* parent) : QLineEdit(parent)
{
    bool bHighPixel = WizIsHighPixel();
    QString strSearch = bHighPixel ? "mactoolbarsearch@2x" : "mactoolbarsearch";
    QString strSearchIcon = Utils::StyleHelper::skinResourceFileName(strSearch);
    m_searchIcon = QPixmap(strSearchIcon);
    QString strDelete = bHighPixel ? "mactoolbardelete@2x" : "mactoolbardelete";
    QString strDeleteIcon = Utils::StyleHelper::skinResourceFileName(strDelete);
    m_deleteIcon = QPixmap(strDeleteIcon);
}

void CWizSearchEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);

    QPainter pt(this);
    QSize szIcon = m_searchIcon.size();
    WizScaleIconSizeForRetina(szIcon);
    QRect rcIcon(QPoint(4, (height() - szIcon.height()) / 2 + 1),
                 szIcon);
    pt.drawPixmap(rcIcon, m_searchIcon);

    if (!text().isEmpty())
    {
        szIcon = m_deleteIcon.size();
        WizScaleIconSizeForRetina(szIcon);
        rcIcon = QRect(QPoint(width() - szIcon.width() - 4, (height() - szIcon.height()) / 2),
                       szIcon);
        pt.drawPixmap(rcIcon, m_deleteIcon);
    }
}

void CWizSearchEdit::mousePressEvent(QMouseEvent* event)
{
    if (!text().isEmpty())
    {
        QRect rect(QPoint(width() - m_deleteIcon.width() - 4, (height() - m_deleteIcon.height()) / 2), m_deleteIcon.size());
        if (rect.contains(event->pos()))
        {
            setText("");
            //send returnPress signal to reset search status
            emit returnPressed();
        }
    }
    QLineEdit::mousePressEvent(event);
}

#endif
