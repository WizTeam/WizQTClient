#ifndef WIZDOCTEMPLATEDIALOG_H
#define WIZDOCTEMPLATEDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <QMap>

namespace Ui {
class CWizDocTemplateDialog;
}

class CWizSettings;
class CWizDatabaseManager;
class CWizDocumentTransitionView;

enum TemplateType
{
    BuildInTemplate,
    CustomTemplate,
    WizServerTemplate
};


struct TemplateData
{
    TemplateType type;
    int id;
    QString strName;
    QString strTitle;
    bool isFree;
    QString strFolder;
    QString strFileName;
    QString strVersion;
};

class CWizTemplateFileItem : public QTreeWidgetItem
{
public:
    explicit CWizTemplateFileItem(const TemplateData& data, QTreeWidgetItem *parent = 0);

    const TemplateData& templateData() const;

private:
    TemplateData m_data;
};


class CWizDocTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizDocTemplateDialog(CWizDatabaseManager& dbMgr,QWidget *parent = 0);
    ~CWizDocTemplateDialog();

signals:
    void documentTemplateSelected(QString strFile);
    void upgradeVipRequest();

public slots:
    void itemClicked(QTreeWidgetItem*item, int);

private slots:
    void on_btn_ok_clicked();

    void on_btn_cancel_clicked();

    void on_pushButton_import_clicked();

    void on_btn_delete_clicked();

    void download_templateFile_finished(QString fileName,bool ok);

    void load_templateDemo_finished(bool Ok);

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

    void parseTemplateData(const QString& json, QList<TemplateData>& templateData);

    void getPurchasedTemplates();

    bool isTemplateUsable(int templateId);

private:
    Ui::CWizDocTemplateDialog *ui;
    CWizDocumentTransitionView* m_transitionView;
    QString m_demoUrl;
    CWizDatabaseManager& m_dbMgr;
};

#endif // WIZDOCTEMPLATEDIALOG_H
