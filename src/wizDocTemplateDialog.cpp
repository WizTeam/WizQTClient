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
    QString strUrl = WizService::CommonApiEntry::standardCommandUrl("gettemplate", true);
    QDesktopServices::openUrl(strUrl);
}



void CWizDocTemplateDialog::initTemplateFileTreeWidget()
{
    QString strFoler = Utils::PathResolve::builtinTemplatePath();
    initFolderTemplateItems(strFoler);
    strFoler = Utils::PathResolve::downloadedTemplatesPath();
    initFolderTemplateItems(strFoler);

    ui->treeWidget->expandAll();
}

void CWizDocTemplateDialog::initFolderTemplateItems(const QString& strFoler)
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
        initFolderItems(topLevelItem, strDir, settings);
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
                                            const QString& strDir, CWizSettings& settings)
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
            QString strDestPath = Utils::PathResolve::downloadedTemplatesPath();
            QDir dir(strTempFolder);
            QStringList folderList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
            if (folderList.count() != 1)
            {
                return false;
            }
            strDestPath = strDestPath + folderList.first();
            WizEnsurePathExists(strDestPath);
            dir = QDir(strTempFolder + "/" + folderList.first());
            QString strfileName = dir.entryList(QDir::Files | QDir::NoDotAndDotDot).first();
            if (!QFile::copy(dir.path() + "/" + strfileName, strDestPath + "/" + strfileName))
            {
                return false;
            }

            //
            QString strSettingFile = Utils::PathResolve::downloadedTemplatesPath() + "template.ini";
            if (!QFile::exists(strSettingFile))
            {
                createSettingsFile(strSettingFile);
            }
            CWizSettings settings(strSettingFile);
            QString newSettingsFile = strTempFolder + "/" + "template.ini";
            CWizSettings newSettings(newSettingsFile);
            QString strTitle = Utils::Misc::extractFileTitle(strfileName);
            QStringList suffixList;
            suffixList << "" <<  "_2052" << "_1028";
            foreach (QString strSuffix, suffixList)
            {
                QString strVaule = newSettings.GetString("common", strTitle + strSuffix);
                if (!strVaule.isEmpty())
                {
                    settings.SetString("Strings", strTitle + strSuffix, strVaule);
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
    CWizTemplateFileItem * pItem = convertToTempalteFileItem(item);
    if (pItem)
    {
        QString strZiwFile = pItem->filePath();
        QString strTempFolder = Utils::PathResolve::tempPath() +Utils::Misc::extractFileTitle(strZiwFile) + "/";
        if (CWizUnzipFile::extractZip(strZiwFile, strTempFolder))
        {
            QString indexFile = "file://" + strTempFolder + previewFileName();
            m_selectedTemplate = strZiwFile;
            ui->webView_preview->load(QUrl(indexFile));
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
