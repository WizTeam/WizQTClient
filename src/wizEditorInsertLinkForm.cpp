#include "wizEditorInsertLinkForm.h"
#include "ui_wizEditorInsertLinkForm.h"

#include <QUrl>

CWizEditorInsertLinkForm::CWizEditorInsertLinkForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizEditorInsertLinkForm)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
}

CWizEditorInsertLinkForm::~CWizEditorInsertLinkForm()
{
    delete ui;
}

//QString CWizEditorInsertLinkForm::getContent()
//{
//    return ui->editContent->text();
//}

//void CWizEditorInsertLinkForm::setContent(const QString& strText)
//{
//    ui->editContent->setText(strText);
//}

QString CWizEditorInsertLinkForm::getUrl()
{
    QUrl url(ui->editUrl->text());
    return url.toEncoded();
}

void CWizEditorInsertLinkForm::setUrl(const QString& strText)
{
    ui->editUrl->setText(strText);
}

//void CWizEditorInsertLinkForm::clear()
//{
//    ui->editContent->clear();
//    ui->editUrl->clear();
//}

void CWizEditorInsertLinkForm::on_pushButton_cancel_clicked()
{
    reject();
}

void CWizEditorInsertLinkForm::on_pushButton_ok_clicked()
{
    accept();
}
