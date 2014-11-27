#include "wizSearchReplaceWidget.h"
#include "ui_wizSearchReplaceWidget.h"

CWizSearchReplaceWidget::CWizSearchReplaceWidget(QDialog* parent) :
    QDialog(parent),
    ui(new Ui::CWizSearchReplaceWidget)
{
    ui->setupUi(this);

    //setWindowFlags(Qt::WindowStaysOnTopHint);  //could cause window fullscreen on mac
}

CWizSearchReplaceWidget::~CWizSearchReplaceWidget()
{
    emit findNext("", false);
    delete ui;
}

void CWizSearchReplaceWidget::showInEditor(const QRect& rcEditor)
{
    QPoint pos;
    pos.setX(rcEditor.x() + (rcEditor.width() - width()) / 2);
    pos.setY(rcEditor.y() + (rcEditor.height() - height()) / 2);
    setGeometry(QRect(pos, size()));

    show();
    setWindowState(windowState() & ~Qt::WindowFullScreen | Qt::WindowActive);
    setFixedSize(size());
    activateWindow();
    raise();
}

void CWizSearchReplaceWidget::closeEvent(QCloseEvent* event)
{
    clearAllText();
    QWidget::closeEvent(event);
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

void CWizSearchReplaceWidget::clearAllText()
{
    ui->lineEdit_repalce->clear();
    ui->lineEdit_source->clear();
}
