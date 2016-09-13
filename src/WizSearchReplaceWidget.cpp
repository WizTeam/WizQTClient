#include "WizSearchReplaceWidget.h"
#include "ui_WizSearchReplaceWidget.h"

WizSearchReplaceWidget::WizSearchReplaceWidget(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::WizSearchReplaceWidget)
{
    ui->setupUi(this);

    //setWindowFlags(Qt::WindowStaysOnTopHint);  //could cause window fullscreen on mac
}

WizSearchReplaceWidget::~WizSearchReplaceWidget()
{
    emit findNext("", false);
    delete ui;
}

void WizSearchReplaceWidget::showInEditor(const QRect& rcEditor)
{
    QPoint pos;
    pos.setX(rcEditor.x() + (rcEditor.width() - width()) / 2);
    pos.setY(rcEditor.y() + (rcEditor.height() - height()) / 2);
    setGeometry(QRect(pos, size()));

    setParent(parentWidget());
    show();
    setWindowState(windowState() & ~Qt::WindowFullScreen | Qt::WindowActive);
    setFixedSize(size());
    activateWindow();
    raise();
}

void WizSearchReplaceWidget::closeEvent(QCloseEvent* event)
{
    clearAllText();
    QWidget::closeEvent(event);
}

void WizSearchReplaceWidget::on_btn_pre_clicked()
{
    emit findPre(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_btn_next_clicked()
{
    emit findNext(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_btn_replace_clicked()
{
    emit replaceAndFindNext(ui->lineEdit_source->text(), ui->lineEdit_repalce->text(),
                            ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_btn_replaceAll_clicked()
{
    emit replaceAll(ui->lineEdit_source->text(), ui->lineEdit_repalce->text(),
                    ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_lineEdit_source_returnPressed()
{
    emit findNext(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::clearAllText()
{
    emit findNext("", false);
    ui->lineEdit_repalce->clear();
    ui->lineEdit_source->clear();
}
