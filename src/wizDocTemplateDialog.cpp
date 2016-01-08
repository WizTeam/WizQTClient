#include "wizDocTemplateDialog.h"
#include "ui_wizDocTemplateDialog.h"

#include <QFile>
#include <QTextStream>
#include <QDir>
#include <fstream>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "rapidjson/document.h"
#include "utils/pathresolve.h"
#include "utils/misc.h"
#include "share/wizmisc.h"
#include "share/wizzip.h"
#include "share/wizsettings.h"
#include "share/wizthreads.h"
#include "share/wizEventLoop.h"
#include "share/wizMessageBox.h"
#include "share/wizObjectDataDownloader.h"
#include "widgets/wizTemplatePurchaseDialog.h"
#include "sync/apientry.h"
#include "sync/token.h"
#include "core/wizAccountManager.h"
#include "core/wizNoteManager.h"
#include "wizDocumentTransitionView.h"
#include "wizmainwindow.h"

/*
 * 服务器的模板列表会在本地缓存一份，允许用户离线状况下使用
 * 每次打开该窗口时都会从服务器获取一次列表
*/


CWizTemplateFileItem* convertToTempalteFileItem(QTreeWidgetItem *item)
{
    return dynamic_cast<CWizTemplateFileItem *>(item);
}

CWizDocTemplateDialog::CWizDocTemplateDialog(CWizDatabaseManager& dbMgr, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CWizDocTemplateDialog)
    , m_dbMgr(dbMgr)
    , m_purchaseDialog(nullptr)
{
    ui->setupUi(this);

    initTemplateFileTreeWidget();

    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(itemClicked(QTreeWidgetItem*,int)));

    ui->treeWidget->setAttribute(Qt::WA_MacShowFocusRect,false);

    ui->treeWidget->setMaximumWidth(200);
    m_transitionView = new CWizDocumentTransitionView(this);
    ui->horizontalLayout_2->addWidget(m_transitionView);
    m_transitionView->setStyleSheet(".QWidget{background-color:#FFFFFF;} QToolButton {border:0px; padding:0px; border-radius:0px;background-color:#F5F5F5;}");
    m_transitionView->hide();
    m_transitionView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(ui->webView_preview, SIGNAL(loadFinished(bool)), SLOT(load_templateDemo_finished(bool)));

    CWizNoteManager noteManager(m_dbMgr);
    noteManager.downloadTemplatePurchaseRecord();
}

CWizDocTemplateDialog::~CWizDocTemplateDialog()
{
    delete ui;
}

void CWizDocTemplateDialog::initTemplateFileTreeWidget()
{    
    //init template list download from server
    QString jsonFile = Utils::PathResolve::wizTemplateJsonFilePath();
    if (QFile::exists(jsonFile))
    {
        QFile file(jsonFile);
        if (!file.open(QFile::ReadOnly))
            return;

        QTextStream stream(&file);
        QString jsonData = stream.readAll();
        if (jsonData.isEmpty())
            return;
        //
        QMap<int, TemplateData> tmplMap;
        getTemplatesFromJsonData(jsonData.toUtf8(), tmplMap);
        QList<TemplateData> templateList = tmplMap.values();
        if (!templateList.isEmpty())
        {
            QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui->treeWidget);
            topLevelItem->setData(0, Qt::UserRole, WizServerTemplate);
            topLevelItem->setText(0, tr("Recommended templates"));            
            for (TemplateData tmpl : templateList)
            {
                tmpl.type = WizServerTemplate;
                CWizTemplateFileItem *item = new CWizTemplateFileItem(tmpl, topLevelItem);
                item->setText(0, tmpl.strName);
                topLevelItem->addChild(item);
            }
        }
    }


    QString folerPath = Utils::PathResolve::builtinTemplatePath();
    initFolderTemplateItems(folerPath, BuildInTemplate);
    folerPath = Utils::PathResolve::customNoteTemplatesPath();
    initFolderTemplateItems(folerPath, CustomTemplate);

    ui->treeWidget->expandAll();

#ifdef BUILD4APPSTORE
    QTimer::singleShot(0, this, SLOT(checkUnfinishedTransation()));
#endif
}

void CWizDocTemplateDialog::initFolderTemplateItems(const QString& strFoler, TemplateType type)
{
    CWizSettings settings(strFoler + "template.ini");

    QDir dir(strFoler);
    QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    foreach (QString strDir, dirList)
    {
        QString folderName = strDir;
        strDir = strFoler + strDir + "/";
        folderName += languangeCode();
        QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui->treeWidget);
        topLevelItem->setData(0, Qt::UserRole, type);
        QString strLocalTitle;
        if (getLocalization(settings, folderName, strLocalTitle))
        {
            folderName = strLocalTitle;
        }
        else
        {
            folderName.remove(languangeCode());
        }
        topLevelItem->setText(0, folderName);
        ui->treeWidget->addTopLevelItem(topLevelItem);
        initFolderItems(topLevelItem, strDir, settings, type);
    }
}

QString CWizDocTemplateDialog::languangeCode() const
{
    Core::Internal::MainWindow *window = Core::Internal::MainWindow::instance();
    if (window)
    {
        //FIXME: hardcode
        QString userLocal = window->userSettings().locale();
        if (userLocal == WizGetDefaultTranslatedLocal())
        {
            return "";
        }
        else if (userLocal == "zh_CN")
        {
            return "_2052";
        }
        else if (userLocal == "zh_TW")
        {
            return "_1028";
        }
    }

    return "";
}

QString CWizDocTemplateDialog::previewFileName()
{
    Core::Internal::MainWindow *window = Core::Internal::MainWindow::instance();
    if (window)
    {
        //FIXME: hardcode
        QString userLocal = window->userSettings().locale();
        if (userLocal == WizGetDefaultTranslatedLocal())
        {
            return "preview.html";
        }
        else if (userLocal == "zh_CN")
        {
            return "preview_zh-cn.html";
        }
        else if (userLocal == "zh_TW")
        {
            return "preview_zh-tw.html";
        }
    }

    return "index.html";
}

void CWizDocTemplateDialog::initFolderItems(QTreeWidgetItem* parentItem,
                                            const QString& strDir, CWizSettings& settings, TemplateType type)
{
    QDir dir(strDir);
    QStringList files = dir.entryList();
    foreach (QString ziwFile, files)
    {
        if (ziwFile.right(3) == "ziw")
        {
            QString strTitle = ziwFile;
            strTitle = Utils::Misc::extractFileTitle(strTitle);
            strTitle += languangeCode();
            QString strLocalTitle;
            if (getLocalization(settings, strTitle, strLocalTitle))
            {
                strTitle = strLocalTitle;
            }
            else
            {
                strTitle = Utils::Misc::extractFileTitle(ziwFile);
            }

            ziwFile = strDir + ziwFile;
            TemplateData data;
            data.strFileName = ziwFile;
            data.type = type;
            CWizTemplateFileItem *item = new CWizTemplateFileItem(data, parentItem);
            item->setData(0, Qt::UserRole, type);
            item->setText(0, strTitle);
            parentItem->addChild(item);
        }
    }
}

bool CWizDocTemplateDialog::getLocalization(CWizSettings& settings, const QString& strKey, QString& strValue)
{
    strValue = settings.GetString("Strings", strKey);
    if (strValue.isEmpty())
        return false;

    return true;
}

bool CWizDocTemplateDialog::importTemplateFile(const QString& strFileName)
{
    if (QFile::exists(strFileName))
    {
        QString strTempFolder = Utils::PathResolve::tempPath() + Utils::Misc::extractFileTitle(strFileName);
        if (CWizUnzipFile::extractZip(strFileName, strTempFolder))
        {
            QDir dir(strTempFolder);
            QString strfileName = "template.ziw";
            QString strZiwFile = dir.path() + "/" + strfileName;
            if (QFile::exists(strZiwFile))
            {
                QString strNewFile = dir.dirName();
                QString strDestPath = Utils::PathResolve::customNoteTemplatesPath() + tr("custom") + "/";
                WizEnsurePathExists(strDestPath);
                if (!QFile::copy(strZiwFile, strDestPath + strNewFile + ".ziw"))
                {
                    return false;
                }
            }
            else
            {
                QStringList folderList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
                if (folderList.count() == 1)
                {
                    QString strDestPath = Utils::PathResolve::customNoteTemplatesPath() + folderList.first();
                    WizEnsurePathExists(strDestPath);
                    dir = QDir(strTempFolder + "/" + folderList.first());
                    QString strfileName = dir.entryList(QDir::Files | QDir::NoDotAndDotDot).first();
                    if (!QFile::copy(dir.path() + "/" + strfileName, strDestPath + "/" + strfileName))
                    {
                        return false;
                    }
                }
            }
            resetTempalteTree();
        }
    }
    return false;
}

void CWizDocTemplateDialog::resetTempalteTree()
{
    ui->treeWidget->clear();
    initTemplateFileTreeWidget();
}

void CWizDocTemplateDialog::createSettingsFile(const QString& strFileName)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "[Strings]" << "\n";
    file.close();
}

void CWizDocTemplateDialog::getPurchasedTemplates()
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
       QNetworkAccessManager manager;
       QString url = WizService::CommonApiEntry::asServerUrl() + "/a/templates/record?token=" + WizService::Token::token();
       QNetworkReply* reply = manager.get(QNetworkRequest(url));
       CWizAutoTimeOutEventLoop loop(reply);
       loop.exec();

       if (loop.error() != QNetworkReply::NoError)
           return;

       QByteArray ba = loop.result();
       QString file = Utils::PathResolve::wizTemplatePurchaseRecordFile();
       std::ofstream recordFile(file.toUtf8().constData(), std::ios::out | std::ios::trunc);
       recordFile << ba.constData();
    });
}

bool isTemplateUsable(const TemplateData& tmplData, CWizDatabaseManager& dbMgr)
{
    if (WizServerTemplate != tmplData.type || tmplData.isFree)
        return true;

    CWizAccountManager account(dbMgr);
    if (account.isVip())
        return true;

    QString record = Utils::PathResolve::wizTemplatePurchaseRecordFile();
    if(!QFile::exists(record))
        return false;

    QFile file(record);
    if (!file.open(QFile::ReadOnly))
        return false;

    QTextStream stream(&file);
    QString jsonData = stream.readAll();
    if (jsonData.isEmpty())
        return false;

    rapidjson::Document d;
    d.Parse(jsonData.toUtf8().constData());

    if (d.HasParseError() || !d.HasMember("result"))
        return false;

    const rapidjson::Value& templates = d.FindMember("result")->value;
    for(rapidjson::SizeType i = 0; i < templates.Size(); i++)
    {
        const rapidjson::Value& templateObj = templates[i];

        if (!templateObj.HasMember("templateId"))
            continue;

        if (templateObj.FindMember("templateId")->value.GetInt() == tmplData.id)
            return true;
    }

    return false;
}

void CWizDocTemplateDialog::createPurchaseDialog()
{
    if (m_purchaseDialog)
        return;

    m_purchaseDialog = new CWizTemplatePurchaseDialog(this);
    connect(m_purchaseDialog, SIGNAL(purchaseSuccess()), SLOT(purchaseFinished()));
}

void CWizDocTemplateDialog::on_btn_ok_clicked()
{
    CWizTemplateFileItem * pItem = convertToTempalteFileItem(ui->treeWidget->currentItem());

    if (pItem)
    {
        if (WizServerTemplate == pItem->templateData().type)
        {
            if (!isTemplateUsable(pItem->templateData(), m_dbMgr))
            {

                WizTemplateUpgradeResult result = showTemplateUnusableDialog(this);
                switch (result) {
                case UpgradeResult_None:
                    return;
                    break;
                case UpgradeResult_UpgradeVip:
                    accept();
                    emit upgradeVipRequest();
                    return;
                    break;
                case UpgradeResult_PurchaseTemplate:
                {
                    if (!m_purchaseDialog)
                    {
                        createPurchaseDialog();
                    }

                    m_purchaseDialog->setModal(true);
                    m_purchaseDialog->showTemplateInfo(pItem->templateData().id, pItem->templateData().strName, pItem->templateData().strThumbUrl);
                    m_purchaseDialog->open();
                    return;
                }
                    break;
                default:
                    Q_ASSERT(0);
                    break;
                }
            }
        }

        emit documentTemplateSelected(pItem->templateData());
        accept();
    }
}

void CWizDocTemplateDialog::itemClicked(QTreeWidgetItem *item, int)
{
    m_transitionView->hide();
    ui->webView_preview->show();

    bool bDelAble = item->data(0, Qt::UserRole).toInt() == CustomTemplate;
    ui->btn_delete->setEnabled(bDelAble);
    //
    CWizTemplateFileItem * pItem = convertToTempalteFileItem(item);
    ui->btn_ok->setEnabled(pItem != nullptr);
    if (pItem)
    {
        QString strZiwFile = pItem->templateData().strFileName;
        if (WizServerTemplate == pItem->templateData().type)
        {

            // load demo page from server
            if (!pItem->templateData().strDemoUrl.isEmpty())
            {
                qDebug() << "load template demo from url : " << pItem->templateData().strDemoUrl;
                ui->webView_preview->load(QUrl(pItem->templateData().strDemoUrl));
            }

            QFileInfo info(strZiwFile);
            // download template file
            if (!info.exists())
            {
                QString strUrl = WizService::CommonApiEntry::asServerUrl() + "/a/templates/download/" + QString::number(pItem->templateData().id);
                qDebug() << "download template data form url : " << strUrl;

                CWizFileDownloader* downloader = new CWizFileDownloader(strUrl, info.fileName(), info.absolutePath() + "/", false);
                connect(downloader, SIGNAL(downloadDone(QString,bool)), SLOT(download_templateFile_finished(QString,bool)));
                downloader->startDownload();
            }
        }
        else if (QFile::exists(strZiwFile))
        {
            QString strTempFolder = Utils::PathResolve::tempPath() +Utils::Misc::extractFileTitle(strZiwFile) + "/";
            if (CWizUnzipFile::extractZip(strZiwFile, strTempFolder))
            {
                QString previewFile = strTempFolder + previewFileName();
                if (!QFile::exists(previewFile))
                {
                    previewFile = "file://" + strTempFolder + "index.html";
                }
                else
                {
                    previewFile = "file://" + previewFile;
                }
                ui->webView_preview->load(QUrl(previewFile));
            }
            else
            {
                QMessageBox::information(0, tr("Info"), tr("extract ziw file failed!"));
            }

        }
    }
}


CWizTemplateFileItem::CWizTemplateFileItem(const TemplateData& data, QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent)
    , m_data(data)
{
    setText(0, Utils::Misc::extractFileTitle(data.strFileName));
}

const TemplateData& CWizTemplateFileItem::templateData() const
{
    return m_data;
}

void CWizDocTemplateDialog::on_btn_cancel_clicked()
{
    reject();
}

void CWizDocTemplateDialog::on_pushButton_import_clicked()
{
    QStringList fileList = QFileDialog::getOpenFileNames(0, tr("Select one or more template files"), QDir::homePath(), "Wiz Template (*.wiztemplate)");
    foreach (QString strFile, fileList) {
        importTemplateFile(strFile);
    }
}

void CWizDocTemplateDialog::on_btn_delete_clicked()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item)
        return;

    CWizTemplateFileItem * pItem = convertToTempalteFileItem(item);
    if (pItem)
    {
        QString strZiwFile = pItem->templateData().strFileName;
        WizDeleteFile(strZiwFile);
        pItem->parent()->removeChild(item);
        delete item;
    }
    else
    {
        QString path = Utils::PathResolve::customNoteTemplatesPath() + item->text(0);
        WizDeleteFolder(path);
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        delete item;
    }
}

void CWizDocTemplateDialog::download_templateFile_finished(QString fileName, bool ok)
{
    qDebug() << "template file downloaded ; " << fileName;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* topItem = ui->treeWidget->topLevelItem(i);
        if (topItem->data(0, Qt::UserRole).toInt() == WizServerTemplate)
        {
//            for (int j = 0; j < topItem->childCount(); ++j)
//            {
//                if (CWizTemplateFileItem* tmplItem = dynamic_cast<CWizTemplateFileItem*>(topItem->child(j)))
//                {
//                    if (tmplItem->templateData().strFileName != fileName)
//                        continue;

//                    if (ui->treeWidget->currentItem() == tmplItem && tmplItem->templateData().type !)
//                    {
//                        itemClicked(tmplItem, 0);
//                    }
//                }
//            }
        }
    }
}

void CWizDocTemplateDialog::load_templateDemo_finished(bool Ok)
{
    if (!Ok)
    {
        ui->webView_preview->hide();
        m_transitionView->showAsMode("", CWizDocumentTransitionView::ErrorOccured);
    }
}

void CWizDocTemplateDialog::purchaseFinished()
{
    CWizNoteManager noteManager(m_dbMgr);
    noteManager.downloadTemplatePurchaseRecord();
}

void CWizDocTemplateDialog::checkUnfinishedTransation()
{
#ifdef BUILD4APPSTORE
    QStringList idList = CWizTemplatePurchaseDialog::getUnfinishedTransations();
    if (idList.size() == 0 || idList.first().isEmpty())
        return;

    int result = CWizMessageBox::information(this, tr("Info"), tr("You have unfinished transation, continue to process it?"),
                             QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
    if (QMessageBox::Ok == result)
    {
        if (!m_purchaseDialog)
        {
            createPurchaseDialog();
        }

        m_purchaseDialog->setModal(true);
        m_purchaseDialog->open();
        m_purchaseDialog->processUnfinishedTransation();
    }
#endif
}


void getTemplatesFromJsonData(const QByteArray& ba, QMap<int, TemplateData>& tmplMap)
{
    rapidjson::Document d;
    d.Parse(ba.constData());
    if (d.HasParseError() || !d.HasMember("templates"))
        return;

    QString demoUrl;
    if (d.HasMember("preview_link"))
    {
        //  http://sandbox.wiz.cn/libs/templates/demo/{file_name}/index.html
        demoUrl = d.FindMember("preview_link")->value.GetString();
    }

    QString thumbUrl;
    if (d.HasMember("thumb_link"))
    {
        thumbUrl = d.FindMember("thumb_link")->value.GetString();
    }

    const rapidjson::Value& templates = d.FindMember("templates")->value;
    for(rapidjson::SizeType i = 0; i < templates.Size(); i++)
    {
        const rapidjson::Value& templateObj = templates[i];

        TemplateData data;
        data.strThumbUrl = thumbUrl;
        data.strDemoUrl = demoUrl;
        data.type = WizServerTemplate;

        if (templateObj.HasMember("fileName"))
        {
            data.strFileName = templateObj.FindMember("fileName")->value.GetString();
            data.strThumbUrl.replace("{file_name}", data.strFileName);
            data.strDemoUrl.replace("{file_name}",data.strFileName);
            data.strFileName = Utils::PathResolve::customNoteTemplatesPath() + data.strFileName + ".ziw";
        }
        if (templateObj.HasMember("folder"))
        {
            data.strFolder = templateObj.FindMember("folder")->value.GetString();
        }
        if (templateObj.HasMember("id"))
        {
            data.id = templateObj.FindMember("id")->value.GetInt();
        }
        if (templateObj.HasMember("name"))
        {
            data.strName = templateObj.FindMember("name")->value.GetString();
        }
        if (templateObj.HasMember("title"))
        {
            data.strTitle = templateObj.FindMember("title")->value.GetString();
        }
        if (templateObj.HasMember("version"))
        {
            data.strVersion = templateObj.FindMember("version")->value.GetString();
        }
        if (templateObj.HasMember("isFree"))
        {
            data.isFree = templateObj.FindMember("isFree")->value.GetBool();
        }        

        tmplMap.insert(data.id, data);
    }
}


QVariant TemplateData::toQVariant() const
{
    QMap<QString, QVariant> varMap;
    varMap.insert("type", (int)type);
    varMap.insert("fileName", strFileName);
    varMap.insert("folder", strFolder);
    varMap.insert("id", id);
    varMap.insert("name", strName);
    varMap.insert("title", strTitle);
    varMap.insert("version", strVersion);
    varMap.insert("isFree", isFree);
    varMap.insert("thumb", strThumbUrl);
    varMap.insert("demo", strDemoUrl);

    QVariant var(varMap);
    return var;
}

void TemplateData::fromQVariant(const QVariant& var)
{
    Q_ASSERT(var.type() == QVariant::Map);

    QMap<QString, QVariant> varMap = var.toMap();
    type = (TemplateType)varMap.value("type").toInt();
    strFileName = varMap.value("fileName").toString();
    strFolder = varMap.value("folder").toString();
    id = varMap.value("id").toInt();
    strName = varMap.value("name").toString();
    strTitle = varMap.value("title").toString();
    strVersion = varMap.value("version").toString();
    isFree = varMap.value("isFree").toBool();
    strThumbUrl = varMap.value("thumb").toString();
    strDemoUrl = varMap.value("demo").toString();
}

//获取模板列表，用于主窗口的新建笔记按钮快速创建笔记
bool getTemplateListFroNewNoteMenu(QList<TemplateData>& tmplList)
{
    // 内置的markdown模板
    TemplateData tmpl;
    tmpl.type = CustomTemplate;
    tmpl.strFileName = "Markdown.md.ziw";
    tmpl.strName = "Markdown";
    tmpl.strTitle = "Markdown.md";
    tmpl.isFree = true;

    tmplList.append(tmpl);

    // 通过服务器下载的笔记模板
    QString jsonFile = Utils::PathResolve::wizTemplateJsonFilePath();
    if (QFile::exists(jsonFile))
    {
        QFile file(jsonFile);
        if (!file.open(QFile::ReadOnly))
            return false;

        QTextStream stream(&file);
        QString jsonData = stream.readAll();
        if (jsonData.isEmpty())
            return false;
        //
        QMap<int, TemplateData> tmplMap;
        getTemplatesFromJsonData(jsonData.toUtf8(), tmplMap);
        tmplList.append(tmplMap.values());
    }

    return true;
}


WizTemplateUpgradeResult showTemplateUnusableDialog(QWidget* parent)
{
    CMessageBox msg(parent);
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle(QObject::tr("Info"));
    msg.setText(QObject::tr("You can use this template after upgrading to VIP or buy it."));
    QPushButton* cancelButton = msg.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton* vipButton = msg.addButton(QObject::tr("Upgrade to VIP"), QMessageBox::AcceptRole);
#ifdef BUILD4APPSTORE
    QPushButton* buyButton = msg.addButton(QObject::tr("Purchase"), QMessageBox::AcceptRole);
#endif
    msg.setDefaultButton(vipButton);
    msg.exec();

    if (msg.clickedButton() == nullptr || msg.clickedButton() == cancelButton)
        return UpgradeResult_None;

    if (msg.clickedButton() == vipButton)
    {
        return UpgradeResult_UpgradeVip;
    }

#ifdef BUILD4APPSTORE
    if (msg.clickedButton() == buyButton)
    {
        return UpgradeResult_PurchaseTemplate;
    }
#endif

    return UpgradeResult_None;
}
