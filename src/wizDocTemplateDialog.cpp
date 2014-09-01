#include "wizDocTemplateDialog.h"
#include "ui_wizDocTemplateDialog.h"
#include "utils/pathresolve.h"
#include "share/wizmisc.h"
#include "share/wizzip.h"
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

    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
}

CWizDocTemplateDialog::~CWizDocTemplateDialog()
{
    delete ui;
}

void CWizDocTemplateDialog::on_btn_downloadNew_clicked()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("gettemplate");
    QDesktopServices::openUrl(strUrl);
}



void CWizDocTemplateDialog::initTemplateFileTreeWidget()
{
    initBuiltinTemplateItems();
    initDownloadedTemplateItems();

    ui->treeWidget->expandAll();
}

void CWizDocTemplateDialog::initBuiltinTemplateItems()
{
    QString strBuiltinPath = Utils::PathResolve::builtinTemplatePath();
    QFile file(strBuiltinPath + "template.ini");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QTreeWidgetItem *topLevelItem = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.left(1) == "+")
        {
            topLevelItem = new QTreeWidgetItem(ui->treeWidget);
            topLevelItem->setText(0, line.right(line.length() - 1));
            ui->treeWidget->addTopLevelItem(topLevelItem);
        }
        else if (line.left(1) == "-")
        {
            QString strFileName = strBuiltinPath + line.right(line.length() -1);
            CWizTemplateFileItem *item = new CWizTemplateFileItem(strFileName, topLevelItem);
            topLevelItem->addChild(item);
        }
    }
}

void CWizDocTemplateDialog::initDownloadedTemplateItems()
{
    QDir dir(Utils::PathResolve::downloadedTemplatesPath());
    QStringList tList = dir.entryList();

    if (tList.count() > 0)
    {
        QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui->treeWidget);
        topLevelItem->setText(0, tr("Downloaded"));
        ui->treeWidget->addTopLevelItem(topLevelItem);

        foreach (QString strName, tList)
        {
            QString strFileName = dir.path() + strName;
            CWizTemplateFileItem *item = new CWizTemplateFileItem(strFileName, topLevelItem);
            topLevelItem->addChild(item);
        }
    }
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

bool CWizDocTemplateDialog::importTemplateFile(const QString& strFileName)
{
    if (QFile::exists(strFileName))
    {
        QString strTempFolder = Utils::PathResolve::tempPath() + WizExtractFileTitle(strFileName);
        if (CWizUnzipFile::extractZip(strFileName, strTempFolder))
        {
//            QString strDestPath = Utils::PathResolve::downloadedTemplatePath();
//            QFile::copy()
        }
    }
    return false;
}

void CWizDocTemplateDialog::on_btn_ok_clicked()
{
    if (!m_selectedTemplate.isEmpty())
    {
        emit documentTemplateSelected(m_selectedTemplate);
    }
    accept();
}

void CWizDocTemplateDialog::itemDoubleClicked(QTreeWidgetItem *item, int)
{
    CWizTemplateFileItem * pItem = convertToTempalteFileItem(item);
    if (pItem)
    {
        QString strZiwFile = pItem->filePath();
        QString strTempFolder = Utils::PathResolve::tempPath() + WizExtractFileTitle(strZiwFile) + "/";
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
    setText(0, WizExtractFileTitle(filePath));
}

QString CWizTemplateFileItem::filePath() const
{
    return m_filePath;
}

void CWizDocTemplateDialog::on_btn_cancle_clicked()
{
    reject();
}

void CWizDocTemplateDialog::on_pushButton_clicked()
{
    QStringList fileList = QFileDialog::getOpenFileNames(0, tr("Select one or more template files"), QDir::homePath(), "Wiz Template (*.wiztemplate)");
    foreach (QString strFile, fileList) {
        importTemplateFile(strFile);
    }
}
