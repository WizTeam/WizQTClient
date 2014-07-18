#include "wizSearchReplaceWidget.h"
#include "ui_wizSearchReplaceWidget.h"

CWizSearchReplaceWidget::CWizSearchReplaceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CWizSearchReplaceWidget)
{
    ui->setupUi(this);
}

CWizSearchReplaceWidget::~CWizSearchReplaceWidget()
{
    delete ui;
}
