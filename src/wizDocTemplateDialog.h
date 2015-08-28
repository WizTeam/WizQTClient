#ifndef WIZDOCTEMPLATEDIALOG_H
#define WIZDOCTEMPLATEDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <QMap>

namespace Ui {
class CWizDocTemplateDialog;
}

class CWizSettings;

class CWizTemplateFileItem : public QTreeWidgetItem
{
public:
    explicit CWizTemplateFileItem(const QString& filePath, QTreeWidgetItem *parent = 0);

    QString filePath() const;

private:
    QString m_filePath;
};

enum TemplateType
{
    BuildInTemplate,
    CustomTemplate
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
    void itemClicked(QTreeWidgetItem*item, int);

private slots:
    void on_btn_downloadNew_clicked();

    void on_btn_ok_clicked();

    void on_btn_cancel_clicked();

    void on_pushButton_import_clicked();

    void on_btn_delete_clicked();

private:
    //
    void initTemplateFileTreeWidget();
    QString languangeCode() const;
    QString previewFileName();
    void initFolderTemplateItems(const QString& strFoler, TemplateType type);
    void initFolderItems(QTreeWidgetItem *parentItem, const QString& strDir,
                         CWizSettings& settings, TemplateType type);

    bool getLocalization(CWizSettings& settings, const QString& strKey, QString& strValue);
    //
    bool importTemplateFile(const QString& strFileName);
    //
    void resetTempalteTree();

    void createSettingsFile(const QString& strFileName);

private:
    Ui::CWizDocTemplateDialog *ui;
    QString m_selectedTemplate;
};

#endif // WIZDOCTEMPLATEDIALOG_H
