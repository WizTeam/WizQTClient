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

void CWizSearchBox::on_search_editingFinished()
{
    emit doSearch(m_keywords);
}
void CWizSearchBox::on_search_edited(const QString& str)
{
    m_keywords = str;
}

#endif


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

