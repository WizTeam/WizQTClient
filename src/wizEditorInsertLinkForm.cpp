#include "wizEditorInsertLinkForm.h"
#include "ui_wizEditorInsertLinkForm.h"

CWizEditorInsertLinkForm::CWizEditorInsertLinkForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizEditorInsertLinkForm)
{
    ui->setupUi(this);
    setFixedSize(size());
    setWindowModality(Qt::ApplicationModal);
}

CWizEditorInsertLinkForm::~CWizEditorInsertLinkForm()
{
    delete ui;
}

QString CWizEditorInsertLinkForm::getContent()
{
    return ui->editContent->text();
}

QString CWizEditorInsertLinkForm::getUrl()
{
    return ui->editUrl->text();
}

void CWizEditorInsertLinkForm::clear()
{
    ui->editContent->clear();
    ui->editUrl->clear();
}
