#include "wizFolderSelector.h"
#include "ui_wizFolderSelector.h"

CWizFolderSelector::CWizFolderSelector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CWizFolderSelector)
{
    ui->setupUi(this);

    initFolders();
}

CWizFolderSelector::~CWizFolderSelector()
{
    delete ui;
}

void CWizFolderSelector::showEvent(QShowEvent* event)
{
    initFolders();
    QDialog::showEvent(event);
}

void CWizFolderSelector::initFolders()
{

}
