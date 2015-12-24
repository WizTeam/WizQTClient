#include "wizMarkdownTemplateDialog.h"
#include "ui_wizMarkdownTemplateDialog.h"
#include "utils/stylehelper.h"
#include "utils/pathresolve.h"
#include "share/wizmisc.h"
#include "extensionsystem/pluginmanager.h"
#include "wiznotestyle.h"
#include <QSettings>
#include <QFileDialog>
#include <QFontMetrics>
#include <QWebView>
#include <QTimer>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QDebug>

CWizMarkdownTemplateDialog::CWizMarkdownTemplateDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CWizMarkdownTemplateDialog)
    , m_menu(nullptr)
{
    ui->setupUi(this);
    ui->listWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listWidget->setTextElideMode(Qt::ElideMiddle);
    CWizListItemStyle<CWizTemplateItem>* listStyle = new CWizListItemStyle<CWizTemplateItem>();
    ui->listWidget->setStyle(listStyle);
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    QTimer::singleShot(100, this, SLOT(initListWidget()));

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(on_customContextMenuRequested(QPoint)));
}

CWizMarkdownTemplateDialog::~CWizMarkdownTemplateDialog()
{
    delete ui;
}

bool CWizMarkdownTemplateDialog::initListWidget()
{
    QMap<QString, bool> itemMap;
    QString strSelectedFile;
    if (getListDataFromSettings(itemMap, strSelectedFile))
    {
        QMap<QString, bool>::iterator it;
        for (it = itemMap.begin(); it != itemMap.end(); it++)
        {
            QString strFileName = it.key();
            QFileInfo info(strFileName);
            QString strTitle = info.baseName();
            bool isCustom = it.value();
            CWizTemplateItem* item = new CWizTemplateItem(ui->listWidget);
            item->setTitle(strTitle);
            item->setIsCustom(isCustom);
            item->setFileName(strFileName);
            item->setSizeHint(QSize(ui->listWidget->width(), 40));
        }
    }
    else
    {
        QStringList itemList;
        QString strPath = Utils::PathResolve::resourcesPath() + "files/markdown/markdown/";
        if (!getListDataFromLocalFiles(strPath, itemList))
        {
            QMessageBox::critical(0, tr("Info"), tr("Can not find template files."));
            return false;
        }
        strSelectedFile = Utils::PathResolve::resourcesPath() + "files/markdown/markdown/github2.css";;
        foreach (QString strFileName, itemList) {
            QString strTitle = strFileName;
            CWizTemplateItem* item = new CWizTemplateItem(ui->listWidget);
            item->setTitle(strTitle.remove(".css"));
            item->setIsCustom(false);
            item->setFileName(strPath + strFileName);
            item->setSizeHint(QSize(ui->listWidget->width(), 40));
        }
    }
    selectItemByLocation(strSelectedFile);
    loadMarkdownHtml(strSelectedFile);

    ui->listWidget->sortItems();

    return true;
}

bool CWizMarkdownTemplateDialog::getListDataFromSettings(QMap<QString, bool>& itemList,
                                                         QString& selectedFile)
{
    QSettings* settings = ExtensionSystem::PluginManager::settings();
    settings->beginGroup("MarkdownTemplate");
    QStringList items = settings->childKeys();
    foreach (const QString& itemLocation, items) {
        if (itemLocation == "SelectedItem")
        {
            QByteArray ba = QByteArray::fromBase64(settings->value("SelectedItem").toByteArray());
            selectedFile = QString::fromUtf8(ba);
        }
        else
        {
            QByteArray ba = QByteArray::fromBase64(itemLocation.toUtf8());
            bool isCustom = settings->value(itemLocation).toBool();
            QString strLocation = QString::fromUtf8(ba);
            itemList.insert(strLocation, isCustom);
        }
    }
    settings->endGroup();

    return !itemList.isEmpty();
}

bool CWizMarkdownTemplateDialog::saveListDataToSettings()
{
    QSettings* settings = ExtensionSystem::PluginManager::settings();
    settings->beginGroup("MarkdownTemplate");
    settings->remove("");
    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        CWizTemplateItem* item = dynamic_cast<CWizTemplateItem*>(ui->listWidget->item(i));
        Q_ASSERT(item);
        QString strLocation = QString::fromUtf8(item->fileName().toUtf8().toBase64());
        settings->setValue(strLocation, item->isCustom());
    }
    if (CWizTemplateItem* selectedItem = dynamic_cast<CWizTemplateItem*>(ui->listWidget->currentItem()))
    {
        QByteArray ba = selectedItem->fileName().toUtf8();
        settings->setValue("SelectedItem", ba.toBase64());
    }
    settings->endGroup();
    settings->sync();

    return true;
}

bool CWizMarkdownTemplateDialog::getListDataFromLocalFiles(const QString& strPath,
                                                           QStringList& itemList)
{
    QDir dir(strPath);
    QStringList filter;
    filter << "*.css" << "*.CSS";
    itemList = dir.entryList(filter, QDir::Files);

    return true;
}

void CWizMarkdownTemplateDialog::loadMarkdownHtml(const QString& strCssFile)
{
    QString strHtml;
    if (strCssFile.isEmpty())
    {
        strHtml = QString(tr("<p>Please select a css file to preview.</p>"));
    }
    else
    {
        if (QFile::exists(strCssFile))
        {
            QString strHtmlFile = Utils::PathResolve::resourcesPath() + "files/markdown/markdown.html";
            QFile f(strHtmlFile);
            if (!f.open(QIODevice::ReadOnly)) {
                qDebug() << "[Markdown]Failed to get html text.";
                return;
            }

            QTextStream ts(&f);
            strHtml = ts.readAll();
            f.close();

            Q_ASSERT(strHtml.indexOf("${CSS_FILE_PATH}") != -1);
            strHtml.replace("${CSS_FILE_PATH}", strCssFile);
        }
        else
        {
            strHtml = QString(tr("<p>CSS file %1 can not be founded, please check wether file exists.</p>")).arg(strCssFile);
        }
    }
    ui->webView->setHtml(strHtml);
}

void CWizMarkdownTemplateDialog::selectItemByLocation(const QString& strFileName)
{
    if (strFileName.isEmpty())
        return;

    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        CWizTemplateItem* item = dynamic_cast<CWizTemplateItem*>(ui->listWidget->item(i));
        Q_ASSERT(item);
        if (item->fileName() == strFileName)
        {
            ui->listWidget->setItemSelected(item, true);
            return;
        }
    }
}

CWizTemplateItem::CWizTemplateItem(QListWidget* view, int type)
    : QListWidgetItem(view, type)
{
}

void CWizTemplateItem::draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    p->save();

    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();
    drawItemBackground(p, vopt->rect, bSelected, bFocused);

    // draw title
    QRect rcTitle = vopt->rect;
    rcTitle.setHeight(rcTitle.height() / 2);
    rcTitle.adjust(5, 0, -5, 0);
    QPen pen(Qt::black);
    pen.setWidth(0);
    p->setPen(pen);
    QFont fontTitle;
    fontTitle.setBold(true);
    p->setFont(fontTitle);
    p->drawText(rcTitle, Qt::AlignLeft | Qt::AlignVCenter, m_title);

    // draw loaction
    QRect rcLocation = vopt->rect;
    rcLocation.setY(rcLocation.y() + rcLocation.height() / 2);
    rcLocation.setWidth(listWidget()->width());
    rcLocation.adjust(5, 0, -5, 0);
    pen.setColor(Qt::lightGray);
    p->setPen(pen);
    QFont font;
    QString text;
    if (m_isCustom)
    {
        text = QObject::tr("Custom template");
    }
    else
    {
        text = QObject::tr("Embedded template");
    }
    p->setFont(font);
    p->drawText(rcLocation, Qt::AlignLeft | Qt::AlignVCenter, text);

    p->restore();
}

QRect CWizTemplateItem::drawItemBackground(QPainter* p, const QRect& rect, bool selected, bool focused) const
{
    if (selected && focused)
    {
        return Utils::StyleHelper::initListViewItemPainter(p, rect,Utils::StyleHelper::ListBGTypeActive);
    }
    else if (selected && !focused)
    {
        return Utils::StyleHelper::initListViewItemPainter(p, rect,  Utils::StyleHelper::ListBGTypeHalfActive);
    }

    return Utils::StyleHelper::initListViewItemPainter(p, rect, Utils::StyleHelper::ListBGTypeNone);
}

QString CWizTemplateItem::title() const
{
    return m_title;
}

void CWizTemplateItem::setTitle(const QString& title)
{
    m_title = title;
}
bool CWizTemplateItem::isCustom() const
{
    return m_isCustom;
}

void CWizTemplateItem::setIsCustom(bool location)
{
    m_isCustom = location;
}
QString CWizTemplateItem::fileName() const
{
    return m_fileName;
}

void CWizTemplateItem::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

bool CWizTemplateItem::operator<(const QListWidgetItem& other) const
{
    if (const CWizTemplateItem* pItem = dynamic_cast<const CWizTemplateItem*>(&other))
    {
        if (pItem->isCustom() != isCustom())
            return !isCustom();

        return fileName().localeAwareCompare(pItem->fileName()) < 0;
    }

    return QListWidgetItem::operator <(other);
}

void CWizMarkdownTemplateDialog::on_listWidget_itemSelectionChanged()
{
    if (CWizTemplateItem* item = dynamic_cast<CWizTemplateItem*>(ui->listWidget->currentItem()))
    {
        QString strCss = item->fileName();
        loadMarkdownHtml(strCss);

        ui->pushButton_delete->setEnabled(item->isCustom());
    }
}


void CWizMarkdownTemplateDialog::on_pushButton_Add_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
                            this,
                            tr("Select one or more files to open"),
                            QDir::homePath(),
                            tr("CSS files (*.css)"));

    WizEnsurePathExists(Utils::PathResolve::customMarkdownTemplatesPath());

    foreach (QString file, files)
    {
        //copy file to template loaction
        QFileInfo info(file);
        QString newFile = Utils::PathResolve::customMarkdownTemplatesPath() + info.fileName();
        WizDeleteFile(newFile);
        if (!QFile::copy(file, newFile))
        {
            qDebug() << "copy markdown template failed : " << file << " to : " << newFile;
            return;
        }

        CWizTemplateItem* item = new CWizTemplateItem(ui->listWidget);
        item->setTitle(info.baseName());
        item->setIsCustom(true);
        item->setFileName(newFile);
        item->setSizeHint(QSize(ui->listWidget->width(), 40));
        ui->listWidget->addItem(item);
    }

    ui->listWidget->sortItems();
}

void CWizMarkdownTemplateDialog::on_pushButton_OK_clicked()
{
    saveListDataToSettings();
    accept();
}

void CWizMarkdownTemplateDialog::on_pushButton_Cancel_clicked()
{
    reject();
}

void CWizMarkdownTemplateDialog::on_pushButton_delete_clicked()
{
    if (CWizTemplateItem* item = dynamic_cast<CWizTemplateItem*>(ui->listWidget->currentItem()))
    {
        QString strCss = item->fileName();
        WizDeleteFile(strCss);
        QListWidgetItem* itemDelete = ui->listWidget->takeItem(ui->listWidget->row(item));
        delete itemDelete;
    }
}

void CWizMarkdownTemplateDialog::on_customContextMenuRequested(const QPoint& pos)
{
    if (!m_menu)
    {
        m_menu = new QMenu(ui->listWidget);
        m_menu->addAction(tr("Save as..."), this, SLOT(on_actionSaveAs_clicked()));
    }
    m_menu->popup(ui->listWidget->mapToGlobal(pos));
}

void CWizMarkdownTemplateDialog::on_actionSaveAs_clicked()
{
    if (CWizTemplateItem* item = dynamic_cast<CWizTemplateItem*>(ui->listWidget->currentItem()))
    {
        QString strCss = item->fileName();
        QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                        QDir::homePath() + "/untitled.css",
                                                           tr("CSS file (*.css)"));
        QFile::copy(strCss, filePath);
    }
}
