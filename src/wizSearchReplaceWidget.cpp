#include "wizSearchReplaceWidget.h"
#include "ui_wizSearchReplaceWidget.h"

CWizSearchReplaceWidget::CWizSearchReplaceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CWizSearchReplaceWidget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::WindowStaysOnTopHint);
}

CWizSearchReplaceWidget::~CWizSearchReplaceWidget()
{
    emit findNext("", false);
    delete ui;
}

void CWizSearchReplaceWidget::on_btn_pre_clicked()
{
    emit findPre(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void CWizSearchReplaceWidget::on_btn_next_clicked()
{
    emit findNext(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void CWizSearchReplaceWidget::on_btn_replace_clicked()
{
    emit replaceAndFindNext(ui->lineEdit_source->text(), ui->lineEdit_repalce->text(),
                            ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void CWizSearchReplaceWidget::on_btn_replaceAll_clicked()
{
    emit replaceAll(ui->lineEdit_source->text(), ui->lineEdit_repalce->text(),
                    ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void CWizSearchReplaceWidget::on_lineEdit_source_returnPressed()
{
    emit findNext(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}
