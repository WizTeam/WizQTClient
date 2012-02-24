#include "wiztaglistwidget.h"

#include <QBoxLayout>
#include <QScrollArea>

CWizTagListWidget::CWizTagListWidget(QWidget* parent)
    : CWizPopupWidget(parent)
{
    QLayout* layoutMain = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(layoutMain);
    layoutMain->setMargin(0);
    //
    QScrollArea* scroll = new QScrollArea(this);
    layoutMain->addWidget(scroll);
}
