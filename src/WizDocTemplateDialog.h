#ifndef WIZDOCTEMPLATEDIALOG_H
#define WIZDOCTEMPLATEDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <QMap>
#include <QVariant>

namespace Ui {
class WizDocTemplateDialog;
}

class WizSettings;
class WizDatabaseManager;
class WizDocumentTransitionView;
class WizTemplatePurchaseDialog;

enum TemplateType
{
    BuildInTemplate,
    CustomTemplate,
    WizServerTemplate
};


struct TemplateData
{
    //
    TemplateData();
    //
    TemplateType type;
    int id;
    QString strName;
    QString strTitle;
    bool isFree;
    QString strFolder;
    QString strFileName;
    QString strVersion;
    QString buildInName;

    //
    QString strThumbUrl;
    QString strDemoUrl;

    QVariant toQVariant() const;
    void fromQVariant(const QVariant& var);
};

class WizTemplateFileItem : public QTreeWidgetItem
{
public:
    explicit WizTemplateFileItem(const TemplateData& data, QTreeWidgetItem *parent = 0);

    const TemplateData& templateData() const;

private:
    TemplateData m_data;
};


class WizDocTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizDocTemplateDialog(WizDatabaseManager& dbMgr,QWidget *parent = 0);
    ~WizDocTemplateDialog();

signals:
    void documentTemplateSelected(const TemplateData& tmplData);
    void upgradeVipRequest();

public slots:
    void itemClicked(QTreeWidgetItem*item, int);

private slots:
    void on_btn_ok_clicked();

    void on_btn_cancel_clicked();

    void download_templateFile_finished(QString fileName,bool ok);

    void load_templateDemo_finished(bool Ok);

    void purchaseFinished();

    void checkUnfinishedTransation();

private:
    //
    void initTemplateFileTreeWidget();
    QString previewFileName();

    void getPurchasedTemplates();    

    void createPurchaseDialog();

private:
    Ui::WizDocTemplateDialog *ui;
    WizDocumentTransitionView* m_transitionView;
    WizDatabaseManager& m_dbMgr;
    WizTemplatePurchaseDialog* m_purchaseDialog;
};

void getTemplatesFromJsonData(const QByteArray& ba, QMap<int, TemplateData>& tmplMap);

bool getTemplateListFroNewNoteMenu(QList<TemplateData>& tmplList);

bool isTemplateUsable(const TemplateData& tmplData, WizDatabaseManager& dbMgr);

enum WizTemplateUpgradeResult {
    UpgradeResult_None,
    UpgradeResult_UpgradeVip,
    UpgradeResult_PurchaseTemplate
};

WizTemplateUpgradeResult showTemplateUnusableDialog(QWidget* parent);

#endif // WIZDOCTEMPLATEDIALOG_H
