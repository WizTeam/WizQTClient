#include "wizDocTemplateDialog.h"
#include "ui_wizDocTemplateDialog.h"
#include "utils/pathresolve.h"
#include "utils/misc.h"
#include "share/wizmisc.h"
#include "share/wizzip.h"
#include "share/wizsettings.h"
#include "wizmainwindow.h"
#include "sync/apientry.h"

#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>


CWizTemplateFileItem* convertToTempalteFileItem(QTreeWidgetItem *item)
{
    return dynamic_cast<CWizTemplateFileItem *>(item);
}

CWizDocTemplateDialog::CWizDocTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizDocTemplateDialog)
{
    ui->setupUi(this);

    initTemplateFileTreeWidget();

    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(itemClicked(QTreeWidgetItem*,int)));
}

CWizDocTemplateDialog::~CWizDocTemplateDialog()
{
    delete ui;
}

void CWizDocTemplateDialog::on_btn_downloadNew_clicked()
{
    QString strUrl = WizService::WizApiEntry::standardCommandUrl("gettemplate");
    QDesktopServices::openUrl(strUrl);
}

void CWizDocTemplateDialog::initTemplateFileTreeWidget()
{
    //REMOVEME:  2015年10月10日 修复缓存路径中的数据会被自动删除的问题
    //将数据移动到固定目录，防止用户添加的数据被删除
    QString strOldPath = Utils::PathResolve::cachePath() + "templates/";
    QDir dir(strOldPath);
    if (dir.exists())
    {
        QStringList dirs = dir.entryList(QDir::AllDirs);
        foreach (QString dirItem , dirs)
        {
            WizCopyFolder(strOldPath + dirItem, Utils::PathResolve::downloadedTemplatesPath() + dirItem, true);
        }
        WizDeleteFolder(strOldPath);
    }


    QString strFoler = Utils::PathResolve::builtinTemplatePath();
    initFolderTemplateItems(strFoler, BuildInTemplate);
    strFoler = Utils::PathResolve::downloadedTemplatesPath();
    initFolderTemplateItems(strFoler, CustomTemplate);

    ui->treeWidget->expandAll();
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
            CWizTemplateFileItem *item = new CWizTemplateFileItem(ziwFile, parentItem);
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
                QString strDestPath = Utils::PathResolve::downloadedTemplatesPath() + tr("custom") + "/";
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
                    QString strDestPath = Utils::PathResolve::downloadedTemplatesPath() + folderList.first();
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

void CWizDocTemplateDialog::on_btn_ok_clicked()
{
    if (!m_selectedTemplate.isEmpty())
    {
        emit documentTemplateSelected(m_selectedTemplate);
    }
    accept();
}

void CWizDocTemplateDialog::itemClicked(QTreeWidgetItem *item, int)
{
    bool bDelAble = item->data(0, Qt::UserRole).toInt() == CustomTemplate;
    ui->btn_delete->setEnabled(bDelAble);
    //
    CWizTemplateFileItem * pItem = convertToTempalteFileItem(item);
    ui->btn_ok->setEnabled(pItem != nullptr);
    if (pItem)
    {
        QString strZiwFile = pItem->filePath();
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
            m_selectedTemplate = strZiwFile;
            ui->webView_preview->load(QUrl(previewFile));
        }
        else
        {
            QMessageBox::information(0, tr("Info"), tr("extract ziw file failed!"));
        }
    }
}


CWizTemplateFileItem::CWizTemplateFileItem(const QString& filePath, QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent)
    , m_filePath(filePath)
{
    setText(0, Utils::Misc::extractFileTitle(filePath));
}

QString CWizTemplateFileItem::filePath() const
{
    return m_filePath;
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
        QString strZiwFile = pItem->filePath();
        WizDeleteFile(strZiwFile);
        pItem->parent()->removeChild(item);
        delete item;
    }
    else
    {
        QString path = Utils::PathResolve::downloadedTemplatesPath() + item->text(0);
        WizDeleteFolder(path);
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        delete item;
    }
}
