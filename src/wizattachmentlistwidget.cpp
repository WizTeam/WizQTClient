#include "wizattachmentlistwidget.h"

#include <QBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMenu>
#include <QLabel>
#include <QDebug>
#include <QEventLoop>

#include <coreplugin/icore.h>

#include "wizdef.h"
#include "share/wizDatabaseManager.h"
#include "share/wizFileMonitor.h"
#include "share/wizmisc.h"
#include "share/wizuihelper.h"
#include "share/wizObjectDataDownloader.h"
#include "share/wizMessageBox.h"
#include "utils/pathresolve.h"
#include "utils/stylehelper.h"
#include "utils/misc.h"
#include "wiznotestyle.h"
#include "wizButton.h"
#include "wizmainwindow.h"

using namespace Core::Internal;

const int nAttachmentListViewItemHeight  = 40;


#define WIZACTION_ATTACHMENT_ADD    QObject::tr("Add...")
//#define WIZACTION_ATTACHMENT_DOWNLOAD    QObject::tr("Download")
#define WIZACTION_ATTACHMENT_SAVEAS QObject::tr("Save as...")
#define WIZACTION_ATTACHMENT_OPEN   QObject::tr("Open...")
#define WIZACTION_ATTACHMENT_DELETE QObject::tr("Delete")
#define WIZACTION_ATTACHMENT_HISTORY QObject::tr("History...")

bool CWizAttachmentListView::m_bHasItemWaitingForDownload = false;

CWizAttachmentListView::CWizAttachmentListView(QWidget* parent)
    : CWizMultiLineListWidget(2, parent)
    , m_dbMgr(*CWizDatabaseManager::instance())
{
    QString strTheme = Utils::StyleHelper::themeName();
    setFrameStyle(QFrame::NoFrame);
    setStyle(WizGetStyle(strTheme));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    //QVBoxLayout* layout = new QVBoxLayout();
    //setStyleSheet("background-color: #F7F7F7");

    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            SLOT(on_list_itemDoubleClicked(QListWidgetItem*)));

    MainWindow* mainWindow = qobject_cast<MainWindow *>(Core::ICore::mainWindow());
    m_downloaderHost = mainWindow->downloaderHost();

    // setup context menu
    m_menu = new QMenu(this);
    m_menu->addAction(WIZACTION_ATTACHMENT_ADD, this, SLOT(on_action_addAttachment()));
    m_menu->addSeparator();
    m_menu->addAction(WIZACTION_ATTACHMENT_OPEN, this, SLOT(on_action_openAttachment()));
    m_menu->addAction(WIZACTION_ATTACHMENT_SAVEAS, this, SLOT(on_action_saveAttachmentAs()));
//    m_menu->addAction(WIZACTION_ATTACHMENT_DOWNLOAD, this, SLOT(on_action_downloadAttachment()));
    m_menu->addSeparator();
    m_menu->addAction(WIZACTION_ATTACHMENT_DELETE, this, SLOT(on_action_deleteAttachment()));
    m_menu->addSeparator();
    m_menu->addAction(WIZACTION_ATTACHMENT_HISTORY, this, SLOT(on_action_attachmentHistory()));
}

QAction* CWizAttachmentListView::findAction(const QString& strName)
{
    QList<QAction *> actionList;
    actionList.append(m_menu->actions());

    QList<QAction *>::const_iterator it;
    for (it = actionList.begin(); it != actionList.end(); it++) {
        QAction* action = *it;
        if (action->text() == strName) {
            return action;
        }
    }

    Q_ASSERT(0);
    return NULL;
}



int CWizAttachmentListView::wrapTextLineIndex() const
{
    return 1;
}
bool CWizAttachmentListView::imageAlignLeft() const
{
    return true;
}
int CWizAttachmentListView::imageWidth() const
{
    return 28;
}
QString CWizAttachmentListView::itemText(const QModelIndex& index, int line) const
{
    if (const CWizAttachmentListViewItem* item = attachmentItemFromIndex(index))
    {
        if (0 == line)
        {
           return item->attachment().strName;
        }
        else
        {
            return item->detailText(this);
        }
    }
    //
    return QString("Error");
}

QPixmap CWizAttachmentListView::itemImage(const QModelIndex& index) const
{
    if (const CWizAttachmentListViewItem* item = attachmentItemFromIndex(index))
    {
        QString path = m_dbMgr.db(item->attachment().strKbGUID).GetAttachmentFileName(item->attachment().strGUID);
        int nIconSize = WizIsHighPixel() ? 64 : 32;
        return m_iconProvider.icon(path).pixmap(nIconSize, nIconSize);
    }
    //
    return QPixmap();
}

bool CWizAttachmentListView::itemExtraImage(const QModelIndex& index, const QRect& itemBound, QRect& rcImage, QPixmap& extraPix) const
{
    if (const CWizAttachmentListViewItem* item = attachmentItemFromIndex(index))
    {
        QString strIconPath;
        CWizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
        MainWindow* mainWindow = qobject_cast<MainWindow *>(Core::ICore::mainWindow());
        bool isRetina = WizIsHighPixel();
        strIconPath = ::WizGetSkinResourcePath(mainWindow->userSettings().skin());
        if (!db.IsAttachmentDownloaded(item->attachment().strGUID))
        {
            strIconPath += isRetina ? "document_needDownload@2x.png" : "document_needDownload.png";
        }
        else if (db.IsAttachmentModified(item->attachment().strGUID))
        {
            strIconPath += isRetina ? "document_needUpload@2x.png" : "document_needUpload.png";
        }
        else
            return false;

        extraPix = QPixmap(strIconPath);
        QSize szImage = extraPix.size();
        WizScaleIconSizeForRetina(szImage);
        int nMargin = -1;
        rcImage.setLeft(itemBound.right() - szImage.width() - nMargin);
        rcImage.setTop(itemBound.bottom() - szImage.height() - nMargin);
        rcImage.setSize(szImage);

        return true;
    }

    return false;
}

void CWizAttachmentListView::resetAttachments()
{
    clear();

    CWizDocumentAttachmentDataArray arrayAttachment;
    m_dbMgr.db(m_document.strKbGUID).GetDocumentAttachments(m_document.strGUID, arrayAttachment);

//    CWizDocumentAttachmentDataArray::const_iterator it;
    for (auto it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        addItem(newAttachmentItem(*it));

        if (isAttachmentModified(*it))
        {
            updateAttachmentInfo(*it);
        }
    }
}


void CWizAttachmentListView::setDocument(const WIZDOCUMENTDATA& document)
{
    disconnect(this, SLOT(resetAttachments()));
    m_document = document;
    connect(&(m_dbMgr.db(m_document.strKbGUID)), SIGNAL(attachmentsUpdated())
            , SLOT(resetAttachments()));
    resetAttachments();
}

const CWizAttachmentListViewItem* CWizAttachmentListView::attachmentItemFromIndex(const QModelIndex& index) const
{
    return dynamic_cast<const CWizAttachmentListViewItem*>(itemFromIndex(index));
}
void CWizAttachmentListView::addAttachments()
{
    QStringList files = QFileDialog::getOpenFileNames(
                             this,
                             tr("Add attachments"),
                             QDir::home().absolutePath(),
                             tr("All files(*)"));
    //
    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);

    bool ok = false;
    foreach (QString fileName, files)
    {
        QFileInfo info(fileName);
        if (info.size() == 0)
        {
            CWizMessageBox::warning(nullptr , tr("Info"), tr("Can not add a 0 bit size file as attachment! File name : ' %1 '").arg(fileName));
            continue;
        }

        if (info.isBundle())
        {
            CWizMessageBox::warning(nullptr, tr("Info"), tr("Can not add a bundle file as attachment! File name : ' %1 '").arg(fileName));
            continue;
        }

        WIZDOCUMENTATTACHMENTDATA data;
        data.strKbGUID = m_document.strKbGUID; // needed by under layer
        if (db.AddAttachment(m_document, fileName, data))
        {
            ok = true;
        }
        addItem(newAttachmentItem(data));
    }
    //
    if (ok)
    {
        MainWindow::quickSyncKb(db.IsGroup() ? db.kbGUID() : "");
    }
}

void CWizAttachmentListView::openAttachment(CWizAttachmentListViewItem* item)
{
    if (!item)
        return;

    WIZDOCUMENTATTACHMENTDATA attachment = item->attachment();

    CWizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.IsObjectDataDownloaded(attachment.strGUID, "attachment");
    QString strFileName = db.GetAttachmentFileName(item->attachment().strGUID);
    bool bExists = PathFileExists(strFileName);
    if (!bIsLocal || !bExists) {
        startDownload(item);
        qDebug() << "start download attach : " << item->attachment().strName;
        m_bHasItemWaitingForDownload = true;
        waitForDownload();
        m_bHasItemWaitingForDownload = false;
        qDebug() << "attachment download finished : " << item->attachment().strName;
    }

#if QT_VERSION > 0x050000
    // try to set the attachement read-only.
    QFile file(strFileName);
    if (file.exists() && !db.CanEditAttachment(attachment) && (file.permissions() & QFileDevice::WriteUser))
    {
        QFile::Permissions permissions = file.permissions();
        permissions = permissions & ~QFileDevice::WriteOwner & ~QFileDevice::WriteUser
                & ~QFileDevice::WriteGroup & ~QFileDevice::WriteOther;
        file.setPermissions(permissions);
    }
#endif

    qDebug() << "try to open file : " << strFileName;
    QDesktopServices::openUrl(QUrl::fromLocalFile(strFileName));

    CWizFileMonitor& monitor = CWizFileMonitor::instance();
    connect(&monitor, SIGNAL(fileModified(QString,QString,QString,QString,QDateTime)),
            &m_dbMgr.db(), SLOT(onAttachmentModified(QString,QString,QString,QString,QDateTime)), Qt::UniqueConnection);

    /*需要使用文件的修改日期来判断文件是否被改动,从服务器上下载下的文件修改日期必定大于数据库中日期.*/
    QFileInfo info(strFileName);
    monitor.addFile(attachment.strKbGUID, attachment.strGUID, strFileName,
                    attachment.strDataMD5, info.lastModified());
}

void CWizAttachmentListView::downloadAttachment(CWizAttachmentListViewItem* item)
{
    if (!item)
        return;

    const WIZDOCUMENTATTACHMENTDATA& attachment = item->attachment();

    CWizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.IsObjectDataDownloaded(attachment.strGUID, "attachment");
    QString strFileName = db.GetAttachmentFileName(item->attachment().strGUID);
    bool bExists = PathFileExists(strFileName);
    if (!bIsLocal || !bExists) {
        startDownload(item);
    }
}

void CWizAttachmentListView::contextMenuEvent(QContextMenuEvent * e)
{
    resetPermission();
    m_menu->popup(e->globalPos());
}

void CWizAttachmentListView::resetPermission()
{
    QString strUserId = m_dbMgr.db().GetUserId();
    int nPerm = m_dbMgr.db(m_document.strKbGUID).permission();
    if (CWizDatabase::IsInDeletedItems(m_document.strLocation)) {
        nPerm = WIZ_USERGROUP_READER;
    }

    if (nPerm <= WIZ_USERGROUP_EDITOR
            || (nPerm == WIZ_USERGROUP_AUTHOR && m_document.strOwner == strUserId)) {
        findAction(WIZACTION_ATTACHMENT_ADD)->setEnabled(true);
        findAction(WIZACTION_ATTACHMENT_DELETE)->setEnabled(true);
    } else {
        findAction(WIZACTION_ATTACHMENT_ADD)->setEnabled(false);
        findAction(WIZACTION_ATTACHMENT_DELETE)->setEnabled(false);
    }

    bool bDownloadEnable = true;
    bool bOpenOrSaveEnable = true;
    bool bDeleteEnable = true;
    // if user select noting
    if (!selectedItems().size()) {
        bDownloadEnable = false;
        bOpenOrSaveEnable = false;
        bDeleteEnable = false;
    } else {
        foreach (QListWidgetItem* item, selectedItems()) {
            CWizAttachmentListViewItem* attachItem = dynamic_cast<CWizAttachmentListViewItem*>(item);
            if (attachItem && (attachItem->isDownloading() || attachItem->isUploading())) {
                bDownloadEnable = false;
                bOpenOrSaveEnable = false;
                bDeleteEnable = false;
            }
            else{
                const WIZDOCUMENTATTACHMENTDATA& attachment = attachItem->attachment();
                CWizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
                bool bIsLocal = db.IsObjectDataDownloaded(attachment.strGUID, "attachment");
                QString strFileName = db.GetAttachmentFileName(attachment.strGUID);
                bool bExists = PathFileExists(strFileName);
                if (!bIsLocal || !bExists)
                {
                    bDownloadEnable = bDownloadEnable | true;
                }
                else if (!bExists && m_bHasItemWaitingForDownload)
                {
                    bOpenOrSaveEnable = false;
                }
            }
        }

        findAction(WIZACTION_ATTACHMENT_OPEN)->setEnabled(bOpenOrSaveEnable);
        findAction(WIZACTION_ATTACHMENT_SAVEAS)->setEnabled(bOpenOrSaveEnable);
//        findAction(WIZACTION_ATTACHMENT_DOWNLOAD)->setEnabled(bDownloadEnable);
        findAction(WIZACTION_ATTACHMENT_DELETE)->setEnabled(bDeleteEnable);
    }

    findAction(WIZACTION_ATTACHMENT_HISTORY)->setEnabled(selectedItems().size() == 1);
}

void CWizAttachmentListView::startDownload(CWizAttachmentListViewItem* item)
{
    m_downloaderHost->downloadData(item->attachment());
    item->setIsDownloading(true);

    forceRepaint();
}

CWizAttachmentListViewItem* CWizAttachmentListView::newAttachmentItem(const WIZDOCUMENTATTACHMENTDATA& att)
{
    CWizAttachmentListViewItem* newItem = new CWizAttachmentListViewItem(att, this);
    newItem->setSizeHint(QSize(width(), nAttachmentListViewItemHeight));
    connect(newItem, SIGNAL(updateRequet()), SLOT(forceRepaint()));
    connect(m_downloaderHost, SIGNAL(downloadDone(WIZOBJECTDATA,bool)), newItem,
            SLOT(on_downloadFinished(WIZOBJECTDATA,bool)));
    connect(m_downloaderHost, SIGNAL(downloadProgress(QString,int,int)), newItem,
            SLOT(on_downloadProgress(QString,int,int)));

    return newItem;
}

void CWizAttachmentListView::waitForDownload()
{
    QEventLoop loop;
    connect(m_downloaderHost, SIGNAL(downloadDone(WIZOBJECTDATA,bool)), &loop,
            SLOT(quit()));
    loop.exec();
}

bool CWizAttachmentListView::isAttachmentModified(const WIZDOCUMENTATTACHMENTDATAEX& attachment)
{
    QString fileNmae = m_dbMgr.db(attachment.strKbGUID).GetAttachmentFileName(attachment.strGUID);
    QFileInfo info(fileNmae);
    if (info.exists())
    {
        return info.lastModified() > attachment.tDataModified;
    }
    return false;
}

void CWizAttachmentListView::updateAttachmentInfo(const WIZDOCUMENTATTACHMENTDATAEX& attachment)
{
    QString fileNmae = m_dbMgr.db(attachment.strKbGUID).GetAttachmentFileName(attachment.strGUID);
    QFileInfo info(fileNmae);
    if (info.exists())
    {

        QString strMD5 = WizMd5FileString(fileNmae);
        if (strMD5 == attachment.strDataMD5)
        {
            qDebug() << "file modified, but md5 keep same";
            return;
        }
        //

        WIZDOCUMENTATTACHMENTDATAEX newData = attachment;
        newData.tDataModified = info.lastModified();
        m_dbMgr.db(attachment.strKbGUID).onAttachmentModified(attachment.strKbGUID, attachment.strGUID,
                                                              fileNmae, strMD5, info.lastModified());
    }
}

void CWizAttachmentListView::on_action_addAttachment()
{
    addAttachments();
}

//void CWizAttachmentListView::on_action_downloadAttachment()
//{
//    foreach (QListWidgetItem* it, selectedItems())
//    {
//        if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
//        {
//            downloadAttachment(item);
//        }
//    }
//}

void CWizAttachmentListView::on_action_saveAttachmentAs()
{
    QList<QListWidgetItem*> items = selectedItems();
    if (items.isEmpty())
        return;
    //
    if (items.size() == 1)
    {
        if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(items[0]))
        {
            CWizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
            bool bIsLocal = db.IsObjectDataDownloaded(item->attachment().strGUID, "attachment");
            bool bExists = PathFileExists(db.GetAttachmentFileName(item->attachment().strGUID));
            if (!bIsLocal || !bExists) {
                //m_downloadDialog->downloadData(item->attachment());
                startDownload(item);
                m_bHasItemWaitingForDownload = true;
                waitForDownload();
                m_bHasItemWaitingForDownload = false;
            }

            QString strFileName = QFileDialog::getSaveFileName(this, QString(), item->attachment().strName);
            if (strFileName.isEmpty())
                return;

            if (!::WizCopyFile(db.GetAttachmentFileName(item->attachment().strGUID), strFileName, FALSE))
            {
                QMessageBox::critical(this, qApp->applicationName(), tr("Can not save attachment to %1").arg(strFileName));
                return;
            }
        }
    }
    else
    {
        CString strDir = QFileDialog::getExistingDirectory(this, tr("Save attachments to"));
        ::WizPathAddBackslash(strDir);

        foreach (QListWidgetItem* it, items)
        {
            if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
            {
                CWizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
                bool bIsLocal = db.IsObjectDataDownloaded(item->attachment().strGUID, "attachment");
                bool bExists = PathFileExists(db.GetAttachmentFileName(item->attachment().strGUID));
                if (!bIsLocal || !bExists) {
                    //m_downloadDialog->downloadData(item->attachment());
                    startDownload(item);
                    waitForDownload();
                }

                CString strFileName = strDir + item->attachment().strName;
                WizGetNextFileName(strFileName);
                //
                if (!::WizCopyFile(db.GetAttachmentFileName(item->attachment().strGUID), strFileName, FALSE))
                {
                    QMessageBox::critical(this, qApp->applicationName(), tr("Can not save attachment to %1").arg(strFileName));
                    continue;
                }
            }
        }
    }

    emit closeRequest();
}

void CWizAttachmentListView::on_action_openAttachment()
{
    foreach (QListWidgetItem* it, selectedItems())
    {
        if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
        {
            openAttachment(item);
        }
    }

    emit closeRequest();
}

void CWizAttachmentListView::on_action_deleteAttachment()
{
    QList<QListWidgetItem*> itemList = selectedItems();

    QString strKbGUID;
    for (int i = 0; i < itemList.count(); i++)
    {
        QListWidgetItem* it = itemList.at(i);
        if (!it->isSelected())
            continue;
        if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
        {
            strKbGUID = item->attachment().strKbGUID;
            CWizDatabase& db = m_dbMgr.db(strKbGUID);
            db.DeleteAttachment(item->attachment(), true, true, false);
        }
    }

    resetAttachments();
    if (!strKbGUID.isEmpty())
    {
        CWizDatabase& db = m_dbMgr.db(strKbGUID);
        MainWindow::quickSyncKb(db.IsGroup() ? db.kbGUID() : "");
    }
}

void CWizAttachmentListView::on_action_attachmentHistory()
{
    QList<QListWidgetItem*> itemList = selectedItems();
    if (itemList.size() != 1)
        return;

    QListWidgetItem* it = itemList.first();
    if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
    {
        ::WizShowAttachmentHistory(item->attachment());
//        strKbGUID = item->attachment().strKbGUID;
//        CWizDatabase& db = m_dbMgr.db(strKbGUID);
//        db.DeleteAttachment(item->attachment(), true, true, false);
    }

}

void CWizAttachmentListView::on_list_itemDoubleClicked(QListWidgetItem* it)
{
    if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
    {
        openAttachment(item);
        emit closeRequest();
    }
}

void CWizAttachmentListView::forceRepaint()
{
#if QT_VERSION < 0x050000
    update();
#else
    viewport()->repaint();
#endif
}



/* ----------------------- CWizAttachmentListWidget ----------------------- */
CWizAttachmentListWidget::CWizAttachmentListWidget(QWidget* parent)
    : CWizPopupWidget(parent)
    , m_list(new CWizAttachmentListView(this))
{
    QString strTheme = Utils::StyleHelper::themeName();
    setContentsMargins(0, 13, 0, 0);

    setFixedWidth(sizeHint().width());

    QIcon iconAddAttachment = ::WizLoadSkinIcon(strTheme, "document_add_attachment");
    QAction* actionAddAttach = new QAction(iconAddAttachment, tr("Add attachments"), this);
    connect(actionAddAttach, SIGNAL(triggered()), SLOT(on_addAttachment_clicked()));
    m_btnAddAttachment = new CWizButton(this);
    m_btnAddAttachment->setFixedSize(14, 14);
    m_btnAddAttachment->setAction(actionAddAttach);

    QHBoxLayout* layoutHeader = new QHBoxLayout();
    layoutHeader->setContentsMargins(12, 0, 12, 0);
    layoutHeader->addWidget(new QLabel(tr("Attachments"), this));
    layoutHeader->addStretch();
    layoutHeader->addWidget(m_btnAddAttachment);

    QVBoxLayout* layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->setSpacing(5);
    setLayout(layoutMain);

    layoutMain->addLayout(layoutHeader);
    layoutMain->addWidget(m_list);
    connect(m_list, SIGNAL(closeRequest()), SLOT(on_attachList_closeRequest()));

    QPalette pal = palette();
#ifdef Q_OS_LINUX
    pal.setBrush(QPalette::Window, QBrush("#D7D7D7"));
#elif defined(Q_OS_MAC)
    pal.setBrush(QPalette::Window, QBrush("#F7F7F7"));
#endif
    setPalette(pal);

    setStyleSheet(Utils::StyleHelper::wizCommonScrollBarStyleSheet());
}

bool CWizAttachmentListWidget::setDocument(const WIZDOCUMENTDATA& doc)
{    
    CWizDatabase& db = CWizDatabaseManager::instance()->db(doc.strKbGUID);
    WIZDOCUMENTDATA document;
    if (!db.DocumentFromGUID(doc.strGUID, document))
        return false;

    m_list->setDocument(document);

    // reset permission
    if (db.CanEditDocument(document) && !CWizDatabase::IsInDeletedItems(document.strLocation)) {
        m_btnAddAttachment->setEnabled(true);
    } else {
        m_btnAddAttachment->setEnabled(false);
    }

    return true;
}

QSize CWizAttachmentListWidget::sizeHint() const
{
    return QSize(271, 310);
}

void CWizAttachmentListWidget::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

void CWizAttachmentListWidget::on_addAttachment_clicked()
{
    m_list->addAttachments();
}

void CWizAttachmentListWidget::on_attachList_closeRequest()
{
    close();
}



CWizAttachmentListViewItem::CWizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att,
                                                       QListWidget* view)
    : m_attachment(att)
    , QListWidgetItem(view, 0)
    , m_loadState(Unkonwn)
    , m_loadProgress(0)
{
}

QString CWizAttachmentListViewItem::detailText(const CWizAttachmentListView* view) const
{
    if (Unkonwn == m_loadState || Downloaded == m_loadState)
    {
        QString strKbGUID = m_attachment.strKbGUID;
        CString strFileName = view->m_dbMgr.db(strKbGUID).GetAttachmentFileName(m_attachment.strGUID);
        qint64 size = Utils::Misc::getFileSize(strFileName);
        CString strSize = 0 == size ? CString(QObject::tr("Un-downloaded")) : WizFormatInt(size);
        CString strType = view->m_iconProvider.type(m_attachment.strName);
        return strSize + "  " + strType;
    }
    else if (Downloading == m_loadState)
    {
        return QString(QObject::tr("Downloading (%1 %) ...").arg(m_loadProgress));
    }
    else if (Uploading == m_loadState)
    {
        return QString(QObject::tr("Uploading (%1 %) ...").arg(m_loadProgress));
    }
    return QString(QObject::tr("Unknow State"));
}

bool CWizAttachmentListViewItem::isDownloading() const
{
    return Downloading == m_loadState;
}

void CWizAttachmentListViewItem::setIsDownloading(bool isDownloading)
{
    m_loadState = isDownloading ? Downloading : Downloaded;
}

bool CWizAttachmentListViewItem::isUploading() const
{
    return Uploading == m_loadState;
}

void CWizAttachmentListViewItem::setIsUploading(bool isUploading)
{
    m_loadState = isUploading ? Uploading : Downloaded;
}

int CWizAttachmentListViewItem::loadProgress() const
{
    return m_loadProgress;
}

void CWizAttachmentListViewItem::on_downloadFinished(const WIZOBJECTDATA& data, bool bSucceed)
{
    if (data.strObjectGUID == m_attachment.strGUID)
    {
        m_loadState = Downloaded;
        emit updateRequet();
    }
}

void CWizAttachmentListViewItem::on_downloadProgress(QString objectGUID, int totalSize, int loadedSize)
{
    if (objectGUID == m_attachment.strGUID)
    {
        m_loadProgress = ((float)loadedSize / (totalSize +1)) * 100;
        m_loadState == Downloading ? 0 : m_loadState = Downloading;
        emit updateRequet();
    }
}
