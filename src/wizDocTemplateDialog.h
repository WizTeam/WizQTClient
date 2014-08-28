#ifndef WIZDOCTEMPLATEDIALOG_H
#define WIZDOCTEMPLATEDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class CWizDocTemplateDialog;
}

class CWizTemplateFileItem : public QTreeWidgetItem
{
public:
    explicit CWizTemplateFileItem(const QString& filePath, QTreeWidgetItem *parent = 0);

    QString filePath() const;

private:
    QString m_filePath;
};


class CWizDocTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizDocTemplateDialog(QWidget *parent = 0);
    ~CWizDocTemplateDialog();

signals:
    void documentTemplateSelected(QString strFile);

public slots:
    void itemDoubleClicked(QTreeWidgetItem*item, int);

private slots:
    void on_btn_downloadNew_clicked();

    void on_btn_useLocal_clicked();

    void on_btn_ok_clicked();

    void on_btn_cancle_clicked();

private:
    enum StackIndex {
        StackIndex_downloadNew,
        StackIndex_useLocal
    };

    void shiftStackIndex(StackIndex index);
    //
    void initTemplateFileTreeWidget();
    void initBuiltinTemplateItems();
    void initDownloadedTemplateItems();
    QString previewFileName();

private:
    Ui::CWizDocTemplateDialog *ui;
    QString m_selectedTemplate;
};

#endif // WIZDOCTEMPLATEDIALOG_H
