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

CWizTemplateFileItem* convertToTempalteFileItem(QTreeWidgetItem *item)
{
    return dynamic_cast<CWizTemplateFileItem *>(item);
}

CWizDocTemplateDialog::CWizDocTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizDocTemplateDialog)
{
    ui->setupUi(this);
    ui->btn_useLocal->setVisible(false);

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
    shiftStackIndex(StackIndex_downloadNew);
}

void CWizDocTemplateDialog::on_btn_useLocal_clicked()
{
    shiftStackIndex(StackIndex_useLocal);
}

void CWizDocTemplateDialog::shiftStackIndex(CWizDocTemplateDialog::StackIndex index)
{
    switch (index) {
    case StackIndex_downloadNew:
    {
        ui->btn_downloadNew->setVisible(false);
        ui->btn_useLocal->setVisible(true);
        ui->stackedWidget->setCurrentIndex(1);
        //http://api.wiz.cn/?p=wiz&l=2052&v=4.2.167.1&c=gettemplate&a=&random=26381078&cn=MACHINE&plat=x86&skin=blue
        QString strUrl = WizService::ApiEntry::standardCommandUrl("gettemplate");
        ui->webView_download->load(strUrl);
    }
        break;
    case StackIndex_useLocal:
        ui->btn_downloadNew->setVisible(true);
        ui->btn_useLocal->setVisible(false);
        ui->stackedWidget->setCurrentIndex(0);
        break;
    }
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
    QDir dir(Utils::PathResolve::downloadedTemplatePath());
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
