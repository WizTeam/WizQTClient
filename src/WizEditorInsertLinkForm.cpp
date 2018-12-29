#include "WizEditorInsertLinkForm.h"
#include "ui_WizEditorInsertLinkForm.h"
#include "share/WizUIBase.h"
#include <QUrl>

WizEditorInsertLinkForm::WizEditorInsertLinkForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizEditorInsertLinkForm)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    //
    if (isDarkMode()) {
        ui->editUrl->setStyleSheet(QString("background-color:%1").arg(WizColorLineEditorBackground.name()));
        WizApplyDarkModeStyles(this);
    }
    //
}

WizEditorInsertLinkForm::~WizEditorInsertLinkForm()
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

QString WizEditorInsertLinkForm::getUrl()
{
    QUrl url(ui->editUrl->text());
    return url.toEncoded();
}

void WizEditorInsertLinkForm::setUrl(const QString& strText)
{
    ui->editUrl->setText(strText);
}

//void CWizEditorInsertLinkForm::clear()
//{
//    ui->editContent->clear();
//    ui->editUrl->clear();
//}

void WizEditorInsertLinkForm::on_pushButton_cancel_clicked()
{
    reject();
}

void WizEditorInsertLinkForm::on_pushButton_ok_clicked()
{
    accept();
}
