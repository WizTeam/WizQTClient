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
    iconLabel->setStyleSheet("QLabel{border-width:0;border-style:outset}");
    editSearch->setStyleSheet("QLineEdit{border-width:0;border-style:outset}");
    //
    QHBoxLayout* layout = new QHBoxLayout(widget);
    widget->setLayout(layout);
    //
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(iconLabel);
    layout->addWidget(editSearch);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    //
    widget->setContentsMargins(1, 1, 1, 1);
    widget->setMinimumHeight(std::max<int>(16, editSearch->sizeHint().height()));
    widget->setMinimumWidth(250);
    //
    widget->setObjectName("WizSearchBoxClient");
    widget->setStyleSheet("QWidget#WizSearchBoxClient{background-color:#ffffff;border-color:#bbbbbb;border-width:1;border-style:solid}");
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


CWizVerSpacer::CWizVerSpacer(QWidget* parent)
    :QWidget(parent)
{
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setSizePolicy(sizePolicy);
}



#ifndef Q_OS_MAC

class CWizSplitterHandle : public QSplitterHandle
{
    CWizSplitter* m_splitter;
public:
    CWizSplitterHandle(Qt::Orientation orientation, CWizSplitter *parent);
    void paintEvent(QPaintEvent *);
};



CWizSplitterHandle::CWizSplitterHandle(Qt::Orientation orientation, CWizSplitter *parent)
    : QSplitterHandle(orientation, parent)
    , m_splitter(parent)
{
}

// Paint the horizontal handle as a gradient, paint
// the vertical handle as a line.
void CWizSplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    //
    painter.fillRect(QRect(0, 0, width(), height()), m_splitter->splitterColor());
}

QSplitterHandle *CWizSplitter::createHandle()
{
    return new CWizSplitterHandle(orientation(), this);
}

void CWizSplitter::setSplitterColor(const QColor& color)
{
    m_splitterColor = color;
}

#endif
