#include "WizDocTemplateDialog.h"
#include "ui_WizDocTemplateDialog.h"

#include <QFile>
#include <QTextStream>
#include <QDir>
#include <fstream>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "share/jsoncpp/json/json.h"
#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include "share/WizMisc.h"
#include "share/WizZip.h"
#include "share/WizSettings.h"
#include "share/WizThreads.h"
#include "share/WizEventLoop.h"
#include "share/WizMessageBox.h"
#include "share/WizObjectDataDownloader.h"
#include "widgets/WizTemplatePurchaseDialog.h"
#include "sync/WizApiEntry.h"
#include "sync/WizToken.h"
#include "core/WizAccountManager.h"
#include "core/WizNoteManager.h"
#include "WizDocumentTransitionView.h"
#include "WizMainWindow.h"

/*
 * 服务器的模板列表会在本地缓存一份，允许用户离线状况下使用
 * 每次打开该窗口时都会从服务器获取一次列表
*/


WizTemplateFileItem* convertToTempalteFileItem(QTreeWidgetItem *item)
{
    return dynamic_cast<WizTemplateFileItem *>(item);
}

WizDocTemplateDialog::WizDocTemplateDialog(WizDatabaseManager& dbMgr, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizDocTemplateDialog)
    , m_dbMgr(dbMgr)
    , m_purchaseDialog(nullptr)
{
    ui->setupUi(this);

    initTemplateFileTreeWidget();

    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(itemClicked(QTreeWidgetItem*,int)));

    ui->treeWidget->setAttribute(Qt::WA_MacShowFocusRect,false);

    ui->treeWidget->setMaximumWidth(200);
    m_transitionView = new WizDocumentTransitionView(this);
    ui->horizontalLayout_2->addWidget(m_transitionView);
    if (isDarkMode()) {
        m_transitionView->setStyleSheet(".QWidget{background-color:#666666;} QToolButton {border:0px; padding:0px; border-radius:0px;background-color:#F5F5F5;}");
    } else {
        m_transitionView->setStyleSheet(".QWidget{background-color:#FFFFFF;} QToolButton {border:0px; padding:0px; border-radius:0px;background-color:#F5F5F5;}");
    }
    m_transitionView->hide();
    m_transitionView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(ui->webView_preview, SIGNAL(loadFinished(bool)), SLOT(load_templateDemo_finished(bool)));

    WizNoteManager noteManager(m_dbMgr);
    noteManager.downloadTemplatePurchaseRecord();
}

WizDocTemplateDialog::~WizDocTemplateDialog()
{
    delete ui;
}

bool getGenericTemplateData(QList<TemplateData>& tmplList)
{
    // 内置的空白模板
    TemplateData tmplEmpty;
    tmplEmpty.type = CustomTemplate;
    tmplEmpty.strFileName = Utils::WizPathResolve::resourcesPath() + "templates/generic/newnote.ziw";
    tmplEmpty.strName = QObject::tr("Empty Note");
    tmplEmpty.strTitle = QObject::tr("Untitled");
    tmplEmpty.isFree = true;
    tmplList.append(tmplEmpty);
    //
    // 内置的markdown模板
    TemplateData tmplMarkdown;
    tmplMarkdown.type = CustomTemplate;
    tmplMarkdown.strFileName = Utils::WizPathResolve::resourcesPath() + "templates/generic/markdown.md.ziw";
    tmplMarkdown.strName = QObject::tr("Markdown Note");
    tmplMarkdown.strTitle = QObject::tr("Markdown Note.md");
    tmplMarkdown.isFree = true;
    tmplList.append(tmplMarkdown);
    //
    return true;
}

void WizDocTemplateDialog::initTemplateFileTreeWidget()
{    
    //
    QTreeWidgetItem *genericItem = new QTreeWidgetItem(ui->treeWidget);
    genericItem->setData(0, Qt::UserRole, CustomTemplate);
    genericItem->setText(0, tr("Generic templates"));
    //
    QList<TemplateData> tmplList;
    getGenericTemplateData(tmplList);
    for (TemplateData tmpl : tmplList)
    {
        WizTemplateFileItem *item = new WizTemplateFileItem(tmpl, genericItem);
        item->setText(0, tmpl.strName);
        genericItem->addChild(item);
    }

    //
    //init template list download from server
    QString jsonFile = Utils::WizPathResolve::wizTemplateJsonFilePath();
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
            QTreeWidgetItem *onlineItem = new QTreeWidgetItem(ui->treeWidget);
            onlineItem->setData(0, Qt::UserRole, WizServerTemplate);
            onlineItem->setText(0, tr("Recommended templates"));
            for (TemplateData tmpl : templateList)
            {
                tmpl.type = WizServerTemplate;
                WizTemplateFileItem *item = new WizTemplateFileItem(tmpl, onlineItem);
                item->setText(0, tmpl.strName);
                onlineItem->addChild(item);
            }
        }
    }
    //

    ui->treeWidget->expandAll();

#ifdef BUILD4APPSTORE
    QTimer::singleShot(0, this, SLOT(checkUnfinishedTransation()));
#endif
    //
    if (isDarkMode()) {
        ui->treeWidget->setStyleSheet("background-color:#666666");
    }
}


QString WizDocTemplateDialog::previewFileName()
{
    WizMainWindow *window = WizMainWindow::instance();
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

void WizDocTemplateDialog::getPurchasedTemplates()
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
       QNetworkAccessManager manager;
       QString url = WizCommonApiEntry::asServerUrl() + "/a/templates/record?token=" + WizToken::token();
       QNetworkReply* reply = manager.get(QNetworkRequest(url));
       WizAutoTimeOutEventLoop loop(reply);
       loop.exec();

       if (loop.error() != QNetworkReply::NoError)
           return;

       QByteArray ba = loop.result();
       QString file = Utils::WizPathResolve::wizTemplatePurchaseRecordFile();
       std::ofstream recordFile(file.toUtf8().constData(), std::ios::out | std::ios::trunc);
       recordFile << ba.constData();
    });
}

bool isTemplateUsable(const TemplateData& tmplData, WizDatabaseManager& dbMgr)
{
    if (WizServerTemplate != tmplData.type || tmplData.isFree)
        return true;

    return true;
}

void WizDocTemplateDialog::createPurchaseDialog()
{
#ifdef Q_OS_MAC
    if (m_purchaseDialog)
        return;

    m_purchaseDialog = new WizTemplatePurchaseDialog(this);
    connect(m_purchaseDialog, SIGNAL(purchaseSuccess()), SLOT(purchaseFinished()));
#endif
}

void WizDocTemplateDialog::on_btn_ok_clicked()
{
    WizTemplateFileItem * pItem = convertToTempalteFileItem(ui->treeWidget->currentItem());

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

#ifdef Q_OS_MAC
                    if (!m_purchaseDialog)
                    {
                        createPurchaseDialog();
                    }

                    m_purchaseDialog->setModal(true);
                    m_purchaseDialog->showTemplateInfo(pItem->templateData().id, pItem->templateData().strName, pItem->templateData().strThumbUrl);
                    m_purchaseDialog->open();
                    return;
#endif
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

void WizDocTemplateDialog::itemClicked(QTreeWidgetItem *item, int)
{
    m_transitionView->hide();
    ui->webView_preview->show();

    WizTemplateFileItem * pItem = convertToTempalteFileItem(item);
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
                QString strUrl = WizCommonApiEntry::asServerUrl() + "/a/templates/download/" + QString::number(pItem->templateData().id);
                qDebug() << "download template data form url : " << strUrl;

                WizFileDownloader* downloader = new WizFileDownloader(strUrl, info.fileName(), info.absolutePath() + "/", false);
                connect(downloader, SIGNAL(downloadDone(QString,bool)), SLOT(download_templateFile_finished(QString,bool)));
                downloader->startDownload();
            }
        }
        else if (QFile::exists(strZiwFile))
        {
            QString strTempFolder = Utils::WizPathResolve::tempPath() +Utils::WizMisc::extractFileTitle(strZiwFile) + "/";
            if (WizUnzipFile::extractZip(strZiwFile, strTempFolder))
            {
                QString previewFile = strTempFolder + previewFileName();
                if (!QFile::exists(previewFile))
                {
                    if (QFile::exists(strTempFolder + "preview.html"))
                    {
                        previewFile = "file://" + strTempFolder + "preview.html";
                    }
                    else
                    {
                        previewFile = "file://" + strTempFolder + "index.html";
                    }
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

WizTemplateFileItem::WizTemplateFileItem(const TemplateData& data, QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent)
    , m_data(data)
{
    setText(0, Utils::WizMisc::extractFileTitle(data.strFileName));
}

const TemplateData& WizTemplateFileItem::templateData() const
{
    return m_data;
}

void WizDocTemplateDialog::on_btn_cancel_clicked()
{
    reject();
}



void WizDocTemplateDialog::download_templateFile_finished(QString fileName, bool ok)
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

void WizDocTemplateDialog::load_templateDemo_finished(bool Ok)
{
    if (!Ok)
    {
        ui->webView_preview->hide();
        m_transitionView->showAsMode("", WizDocumentTransitionView::ErrorOccured);
    }
}

void WizDocTemplateDialog::purchaseFinished()
{
    WizNoteManager noteManager(m_dbMgr);
    noteManager.downloadTemplatePurchaseRecord();
}

void WizDocTemplateDialog::checkUnfinishedTransation()
{
#ifdef BUILD4APPSTORE
    QStringList idList = WizTemplatePurchaseDialog::getUnfinishedTransations();
    if (idList.size() == 0 || idList.first().isEmpty())
        return;

    int result = WizMessageBox::information(this, tr("Info"), tr("You have unfinished transation, continue to process it?"),
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
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(ba.constData(), d))
        return;
    if (!d.isMember("templates"))
        return;

    QString demoUrl;
    if (d.isMember("preview_link"))
    {
        //  http://sandbox.wiz.cn/libs/templates/demo/{file_name}/index.html
        demoUrl = QString::fromUtf8(d["preview_link"].asString().c_str());
    }

    QString thumbUrl;
    if (d.isMember("thumb_link"))
    {
        thumbUrl = QString::fromUtf8(d["thumb_link"].asString().c_str());
    }

    const Json::Value templates = d["templates"];
    for(Json::ArrayIndex i = 0; i < templates.size(); i++)
    {
        const Json::Value& templateObj = templates[i];

        TemplateData data;
        data.strThumbUrl = thumbUrl;
        data.strDemoUrl = demoUrl;
        data.type = WizServerTemplate;

        if (templateObj.isMember("fileName"))
        {
            data.strFileName = QString::fromStdString(templateObj["fileName"].asString());
            data.strThumbUrl.replace("{file_name}", data.strFileName);
            data.strDemoUrl.replace("{file_name}",data.strFileName);
            data.strFileName = Utils::WizPathResolve::customNoteTemplatesPath() + data.strFileName + ".ziw";
        }
        if (templateObj.isMember("folder"))
        {
            data.strFolder = QString::fromStdString(templateObj["folder"].asString());
        }
        if (templateObj.isMember("id"))
        {
            data.id = templateObj["id"].asInt();
        }
        if (templateObj.isMember("name"))
        {
            data.strName = QString::fromStdString(templateObj["name"].asString());
        }
        if (templateObj.isMember("title"))
        {
            data.strTitle = QString::fromStdString(templateObj["title"].asString());
        }
        if (templateObj.isMember("version"))
        {
            data.strVersion = QString::fromStdString(templateObj["version"].asString());
        }
        if (templateObj.isMember("isFree"))
        {
            data.isFree = templateObj["isFree"].asBool();
        }        

        tmplMap.insert(data.id, data);
    }
}

TemplateData::TemplateData()
    : type(BuildInTemplate)
    , id(0)
    , isFree(true)
{

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
    varMap.insert("buildInName", buildInName);

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
    buildInName = varMap.value("buildInName").toString();
}

//获取模板列表，用于主窗口的新建笔记按钮快速创建笔记
bool getTemplateListFroNewNoteMenu(QList<TemplateData>& tmplList)
{
    // 内置的空白模板
    TemplateData tmplEmpty;
    tmplEmpty.type = CustomTemplate;
    tmplEmpty.strFileName = Utils::WizPathResolve::resourcesPath() + "templates/generic/newnote.ziw";
    tmplEmpty.strName = QObject::tr("Empty Note");
    tmplEmpty.strTitle = QObject::tr("Untitled");
    tmplEmpty.isFree = true;
    tmplList.append(tmplEmpty);
    //
    // 内置的markdown模板
    TemplateData tmplMarkdown;
    tmplMarkdown.type = CustomTemplate;
    tmplMarkdown.strFileName = Utils::WizPathResolve::resourcesPath() + "templates/generic/markdown.md.ziw";
    tmplMarkdown.strName = QObject::tr("Markdown Note");
    tmplMarkdown.strTitle = QObject::tr("Markdown Note.md");
    tmplMarkdown.isFree = true;
    tmplList.append(tmplMarkdown);
    //
    // 内置的handwriting模板
    TemplateData tmplHandwriting;
    tmplHandwriting.type = BuildInTemplate;
    tmplHandwriting.strFileName = Utils::WizPathResolve::resourcesPath() + "templates/generic/handwriting.ziw";;
    tmplHandwriting.strName = QObject::tr("Handwriting Note");
    tmplHandwriting.strTitle = QObject::tr("Handwriting Note");
    tmplHandwriting.buildInName = "svgpainter";
    tmplHandwriting.isFree = true;
    tmplList.append(tmplHandwriting);

    // 内置的outline模板
    TemplateData tmplOutlone;
    tmplOutlone.type = BuildInTemplate;
    tmplOutlone.strFileName = Utils::WizPathResolve::resourcesPath() + "templates/generic/newnote.ziw";;
    tmplOutlone.strName = QObject::tr("Outline Note");
    tmplOutlone.strTitle = QObject::tr("Outline Note");
    tmplOutlone.buildInName = "outline";
    tmplOutlone.isFree = true;
    tmplList.append(tmplOutlone);

    // sep
    TemplateData tmplSep;
    tmplSep.type = CustomTemplate;
    tmplSep.strFileName = "-";
    tmplSep.strName = "-";
    tmplSep.strTitle = "-";
    tmplSep.isFree = true;
    tmplList.append(tmplSep);

    // 通过服务器下载的笔记模板
    QString jsonFile = Utils::WizPathResolve::wizTemplateJsonFilePath();
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
    WizMessageBox msg(parent);
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle(QObject::tr("Info"));
    msg.setText(QObject::tr("You can use this template after upgrading to VIP or buy it."));
    QPushButton* cancelButton = msg.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
#ifdef BUILD4APPSTORE
    QPushButton* buyButton = msg.addButton(QObject::tr("Purchase"), QMessageBox::AcceptRole);
#endif
    QPushButton* vipButton = msg.addButton(QObject::tr("Upgrade to VIP"), QMessageBox::YesRole);
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
