#include "WizAttachmentListWidget.h"

#include <QBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMenu>
#include <QLabel>
#include <QDebug>
#include <QEventLoop>

#include "share/WizGlobal.h"

#include "WizDef.h"
#include "share/WizDatabaseManager.h"
#include "share/WizFileMonitor.h"
#include "share/WizMisc.h"
#include "share/WizUIHelper.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WizMessageBox.h"
#include "utils/WizPathResolve.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizMisc.h"
#include "WizNoteStyle.h"
#include "WizButton.h"
#include "WizMainWindow.h"
#include "share/WizThreads.h"


const int nAttachmentListViewItemHeight  = 40;


#define WIZACTION_ATTACHMENT_ADD    QObject::tr("Add...")
//#define WIZACTION_ATTACHMENT_DOWNLOAD    QObject::tr("Download")
#define WIZACTION_ATTACHMENT_SAVEAS QObject::tr("Save as...")
#define WIZACTION_ATTACHMENT_OPEN   QObject::tr("Open...")
#define WIZACTION_ATTACHMENT_DELETE QObject::tr("Delete")
#define WIZACTION_ATTACHMENT_HISTORY QObject::tr("History...")

WizAttachmentListView::WizAttachmentListView(QWidget* parent)
    : WizMultiLineListWidget(2, parent)
    , m_dbMgr(*WizDatabaseManager::instance())
{
    QString strTheme = Utils::WizStyleHelper::themeName();
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

    WizMainWindow* mainWindow = WizGlobal::mainWindow();
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

QAction* WizAttachmentListView::findAction(const QString& strName)
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



int WizAttachmentListView::wrapTextLineIndex() const
{
    return 1;
}
bool WizAttachmentListView::imageAlignLeft() const
{
    return true;
}
int WizAttachmentListView::imageWidth() const
{
    return 28;
}
QString WizAttachmentListView::itemText(const QModelIndex& index, int line) const
{
    if (const WizAttachmentListViewItem* item = attachmentItemFromIndex(index))
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

QPixmap WizAttachmentListView::itemImage(const QModelIndex& index) const
{
    if (const WizAttachmentListViewItem* item = attachmentItemFromIndex(index))
    {
        QString path = m_dbMgr.db(item->attachment().strKbGUID).getAttachmentFileName(item->attachment().strGUID);
        int nIconSize = WizIsHighPixel() ? 64 : 32;
        return m_iconProvider.icon(path).pixmap(nIconSize, nIconSize);
    }
    //
    return QPixmap();
}

bool WizAttachmentListView::itemExtraImage(const QModelIndex& index, const QRect& itemBound, QRect& rcImage, QPixmap& extraPix) const
{
    if (const WizAttachmentListViewItem* item = attachmentItemFromIndex(index))
    {
        WizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
        WizMainWindow* mainWindow = WizGlobal::mainWindow();
        //
        const QSize iconSize(WizSmartScaleUI(16), WizSmartScaleUI(16));
        static QIcon download = WizLoadSkinIcon(mainWindow->userSettings().skin(), "document_needDownload", iconSize);
        static QIcon upload = WizLoadSkinIcon(mainWindow->userSettings().skin(), "document_needUpload", iconSize);
        static QPixmap pixmapDownload = download.pixmap(iconSize);
        static QPixmap pixmapUpload = upload.pixmap(iconSize);
        //
        if (!db.isAttachmentDownloaded(item->attachment().strGUID))
        {
            extraPix = pixmapDownload;
        }
        else if (db.isAttachmentModified(item->attachment().strGUID))
        {
            extraPix = pixmapUpload;
        }
        else
            return false;
        //
        int nMargin = -1;
        rcImage.setLeft(itemBound.right() - iconSize.width() - nMargin);
        rcImage.setTop(itemBound.bottom() - iconSize.height() - nMargin);
        rcImage.setSize(iconSize);

        return true;
    }

    return false;
}

void WizAttachmentListView::resetAttachments()
{
    clear();

    CWizDocumentAttachmentDataArray arrayAttachment;
    m_dbMgr.db(m_document.strKbGUID).getDocumentAttachments(m_document.strGUID, arrayAttachment);

//    CWizDocumentAttachmentDataArray::const_iterator it;
    for (auto it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        addItem(newAttachmentItem(*it));

        if (isAttachmentModified(*it))
        {
            updateAttachmentInfo(*it);
        }
    }
}


void WizAttachmentListView::setDocument(const WIZDOCUMENTDATA& document)
{
    disconnect(this, SLOT(resetAttachments()));
    m_document = document;
    connect(&(m_dbMgr.db(m_document.strKbGUID)), SIGNAL(attachmentsUpdated())
            , SLOT(resetAttachments()));
    resetAttachments();
}

const WizAttachmentListViewItem* WizAttachmentListView::attachmentItemFromIndex(const QModelIndex& index) const
{
    return dynamic_cast<const WizAttachmentListViewItem*>(itemFromIndex(index));
}
void WizAttachmentListView::addAttachments()
{
    QStringList files = QFileDialog::getOpenFileNames(
                             this,
                             tr("Add attachments"),
                             QDir::home().absolutePath(),
                             tr("All files(*)"));
    //
    WizDatabase& db = m_dbMgr.db(m_document.strKbGUID);

    bool ok = false;
    foreach (QString fileName, files)
    {
        QFileInfo info(fileName);
        if (info.size() == 0)
        {
            WizMessageBox::warning(nullptr , tr("Info"), tr("Can not add a 0 bit size file as attachment! File name : ' %1 '").arg(fileName));
            continue;
        }

        if (info.isBundle())
        {
            WizMessageBox::warning(nullptr, tr("Info"), tr("Can not add a bundle file as attachment! File name : ' %1 '").arg(fileName));
            continue;
        }

        WIZDOCUMENTATTACHMENTDATA data;
        data.strKbGUID = m_document.strKbGUID; // needed by under layer
        if (db.addAttachment(m_document, fileName, data))
        {
            ok = true;
        }
        addItem(newAttachmentItem(data));
    }
    //
    if (ok)
    {
        WizMainWindow::instance()->quickSyncKb(db.isGroup() ? db.kbGUID() : "");
    }
}

void WizAttachmentListView::openAttachment(WizAttachmentListViewItem* item)
{
    if (!item)
        return;

    WIZDOCUMENTATTACHMENTDATA attachment = item->attachment();

    WizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.isObjectDataDownloaded(attachment.strGUID, "attachment");
    QString strFileName = db.getAttachmentFileName(item->attachment().strGUID);
    bool bExists = WizPathFileExists(strFileName);
    if (!bIsLocal || !bExists) {        
        item->setIsDownloading(true);
        forceRepaint();

        m_downloaderHost->downloadData(item->attachment(), [=](){
            qDebug() << "try to open file in lambda function : " << strFileName;
            QDesktopServices::openUrl(QUrl::fromLocalFile(strFileName));
        });
        return;
    }

    // try to set the attachement read-only.
    QFile file(strFileName);
    if (file.exists() && !db.canEditAttachment(attachment) && (file.permissions() & QFileDevice::WriteUser))
    {
        QFile::Permissions permissions = file.permissions();
        permissions = permissions & ~QFileDevice::WriteOwner & ~QFileDevice::WriteUser
                & ~QFileDevice::WriteGroup & ~QFileDevice::WriteOther;
        file.setPermissions(permissions);
    }

    qDebug() << "try to open file : " << strFileName;
    QDesktopServices::openUrl(QUrl::fromLocalFile(strFileName));
    //
    WizMainWindow* mainWindow = WizMainWindow::instance();
    //
    WizFileMonitor& monitor = WizFileMonitor::instance();
    connect(&monitor, SIGNAL(fileModified(QString,QString,QString,QString,QDateTime)),
            mainWindow, SLOT(onAttachmentModified(QString,QString,QString,QString,QDateTime)), Qt::UniqueConnection);

    monitor.addFile(attachment.strKbGUID, attachment.strGUID, strFileName, attachment.strDataMD5);
}

void WizAttachmentListView::downloadAttachment(WizAttachmentListViewItem* item)
{
    if (!item)
        return;

    const WIZDOCUMENTATTACHMENTDATA& attachment = item->attachment();

    WizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.isObjectDataDownloaded(attachment.strGUID, "attachment");
    QString strFileName = db.getAttachmentFileName(item->attachment().strGUID);
    bool bExists = WizPathFileExists(strFileName);
    if (!bIsLocal || !bExists) {
        startDownload(item);
    }
}

void WizAttachmentListView::contextMenuEvent(QContextMenuEvent * e)
{
    resetPermission();
    m_menu->popup(e->globalPos());
}

void WizAttachmentListView::resetPermission()
{
    QString strUserId = m_dbMgr.db().getUserId();
    int nPerm = m_dbMgr.db(m_document.strKbGUID).permission();
    if (WizDatabase::isInDeletedItems(m_document.strLocation)) {
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
            WizAttachmentListViewItem* attachItem = dynamic_cast<WizAttachmentListViewItem*>(item);
            if (attachItem && (attachItem->isDownloading() || attachItem->isUploading())) {
                bDownloadEnable = false;
                bOpenOrSaveEnable = false;
                bDeleteEnable = false;
            }
            else{
                const WIZDOCUMENTATTACHMENTDATA& attachment = attachItem->attachment();
                WizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
                bool bIsLocal = db.isObjectDataDownloaded(attachment.strGUID, "attachment");
                QString strFileName = db.getAttachmentFileName(attachment.strGUID);
                bool bExists = WizPathFileExists(strFileName);
                if (!bIsLocal || !bExists)
                {
                    bDownloadEnable = bDownloadEnable | true;
                }
                else if (!bExists)
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

void WizAttachmentListView::startDownload(WizAttachmentListViewItem* item)
{
    m_downloaderHost->downloadData(item->attachment());
    item->setIsDownloading(true);

    forceRepaint();
}

WizAttachmentListViewItem* WizAttachmentListView::newAttachmentItem(const WIZDOCUMENTATTACHMENTDATA& att)
{
    WizAttachmentListViewItem* newItem = new WizAttachmentListViewItem(att, this);
    newItem->setSizeHint(QSize(width(), nAttachmentListViewItemHeight));
    connect(newItem, SIGNAL(updateRequet()), SLOT(forceRepaint()));
    connect(m_downloaderHost, SIGNAL(downloadDone(WIZOBJECTDATA,bool)), newItem,
            SLOT(on_downloadFinished(WIZOBJECTDATA,bool)));
    connect(m_downloaderHost, SIGNAL(downloadProgress(QString,int,int)), newItem,
            SLOT(on_downloadProgress(QString,int,int)));

    return newItem;
}

void WizAttachmentListView::waitForDownload()
{
    QEventLoop loop;
    connect(m_downloaderHost, SIGNAL(downloadDone(WIZOBJECTDATA,bool)), &loop,
            SLOT(quit()));
    loop.exec();
}

bool WizAttachmentListView::isAttachmentModified(const WIZDOCUMENTATTACHMENTDATAEX& attachment)
{
    return false;
    //
    //下面的方式是错误的，不应该按照这种方式检测，而应该检测文件的md5。
    //
    QString fileNmae = m_dbMgr.db(attachment.strKbGUID).getAttachmentFileName(attachment.strGUID);
    QFileInfo info(fileNmae);
    if (info.exists())
    {
        return info.lastModified() > attachment.tDataModified;
    }
    return false;
}

void WizAttachmentListView::updateAttachmentInfo(const WIZDOCUMENTATTACHMENTDATAEX& attachment)
{
    QString fileNmae = m_dbMgr.db(attachment.strKbGUID).getAttachmentFileName(attachment.strGUID);
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

void WizAttachmentListView::on_action_addAttachment()
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

void WizAttachmentListView::on_action_saveAttachmentAs()
{
    QList<QListWidgetItem*> items = selectedItems();
    if (items.isEmpty())
        return;
    //
    QString lastPath = WizMainWindow::instance()->userSettings().lastAttachmentPath();
    //
    if (items.size() == 1)
    {
        if (WizAttachmentListViewItem* item = dynamic_cast<WizAttachmentListViewItem*>(items[0]))
        {
            WizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
            bool bIsLocal = db.isObjectDataDownloaded(item->attachment().strGUID, "attachment");
            bool bExists = WizPathFileExists(db.getAttachmentFileName(item->attachment().strGUID));
            if (!bIsLocal || !bExists) {
                item->setIsDownloading(true);
                forceRepaint();

                QString kbGUID = item->attachment().strKbGUID;
                QString guid = item->attachment().strGUID;
                QString attachName = item->attachment().strName;
                m_downloaderHost->downloadData(item->attachment(), [=](){
                    WizExecuteOnThread(WIZ_THREAD_MAIN, [=](){
                        //
                        QString newName = attachName;
                        if (!lastPath.isEmpty()) {
                            newName = lastPath + newName;
                        }
                        //
                        QString strFileName = QFileDialog::getSaveFileName(this, QString(), newName);
                        if (strFileName.isEmpty())
                            return;
                        //
                        WizMainWindow::instance()->userSettings().setLastAttachmentPath(Utils::WizMisc::extractFilePath(strFileName));

                        if (!::WizCopyFile(m_dbMgr.db(kbGUID).getAttachmentFileName(guid), strFileName, FALSE))
                        {
                            QMessageBox::critical(nullptr, qApp->applicationName(), tr("Can not save attachment to %1").arg(strFileName));
                            return;
                        }
                    });
                });
            }
            else
            {
                QString newName = item->attachment().strName;
                if (!lastPath.isEmpty()) {
                    newName = lastPath + newName;
                }
                //
                QString strFileName = QFileDialog::getSaveFileName(this, QString(), newName);
                if (strFileName.isEmpty())
                    return;

                WizMainWindow::instance()->userSettings().setLastAttachmentPath(Utils::WizMisc::extractFilePath(strFileName));
                //
                if (!::WizCopyFile(db.getAttachmentFileName(item->attachment().strGUID), strFileName, FALSE))
                {
                    QMessageBox::critical(this, qApp->applicationName(), tr("Can not save attachment to %1").arg(strFileName));
                    return;
                }
            }
        }
    }
    else
    {
        CString strDir = QFileDialog::getExistingDirectory(this, tr("Save attachments to"), lastPath);
        ::WizPathAddBackslash(strDir);
        //
        WizMainWindow::instance()->userSettings().setLastAttachmentPath(strDir);

        foreach (QListWidgetItem* it, items)
        {
            if (WizAttachmentListViewItem* item = dynamic_cast<WizAttachmentListViewItem*>(it))
            {
                WizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
                bool bIsLocal = db.isObjectDataDownloaded(item->attachment().strGUID, "attachment");
                bool bExists = WizPathFileExists(db.getAttachmentFileName(item->attachment().strGUID));
                if (!bIsLocal || !bExists)
                {
                    item->setIsDownloading(true);
                    forceRepaint();

                    // download
                    QString kbGUID = item->attachment().strKbGUID;
                    QString guid = item->attachment().strGUID;
                    QString attachName = item->attachment().strName;
                    m_downloaderHost->downloadData(item->attachment(), [=](){
                        WizExecuteOnThread(WIZ_THREAD_MAIN, [=](){
                            QString strFileName = QFileDialog::getSaveFileName(this, QString(), attachName);
                            if (strFileName.isEmpty())
                                return;

                            if (!::WizCopyFile(m_dbMgr.db(kbGUID).getAttachmentFileName(guid), strFileName, FALSE))
                            {
                                QMessageBox::critical(nullptr, qApp->applicationName(), tr("Can not save attachment to %1").arg(strFileName));
                                return;
                            }
                        });
                    });
                }
                else
                {
                    CString strFileName = strDir + item->attachment().strName;
                    WizGetNextFileName(strFileName);
                    //
                    if (!::WizCopyFile(db.getAttachmentFileName(item->attachment().strGUID), strFileName, FALSE))
                    {
                        QMessageBox::critical(this, qApp->applicationName(), tr("Can not save attachment to %1").arg(strFileName));
                        continue;
                    }
                }
            }
        }
    }

    emit closeRequest();
}

void WizAttachmentListView::on_action_openAttachment()
{
    foreach (QListWidgetItem* it, selectedItems())
    {
        if (WizAttachmentListViewItem* item = dynamic_cast<WizAttachmentListViewItem*>(it))
        {
            openAttachment(item);
        }
    }

    emit closeRequest();
}

void WizAttachmentListView::on_action_deleteAttachment()
{
    QList<QListWidgetItem*> itemList = selectedItems();

    QString strKbGUID;
    for (int i = 0; i < itemList.count(); i++)
    {
        QListWidgetItem* it = itemList.at(i);
        if (!it->isSelected())
            continue;
        if (WizAttachmentListViewItem* item = dynamic_cast<WizAttachmentListViewItem*>(it))
        {
            strKbGUID = item->attachment().strKbGUID;
            WizDatabase& db = m_dbMgr.db(strKbGUID);
            db.deleteAttachment(item->attachment(), true, true, false);
        }
    }

    resetAttachments();
    if (!strKbGUID.isEmpty())
    {
        WizDatabase& db = m_dbMgr.db(strKbGUID);
        WizMainWindow::instance()->quickSyncKb(db.isGroup() ? db.kbGUID() : "");
    }
}

void WizAttachmentListView::on_action_attachmentHistory()
{
    QList<QListWidgetItem*> itemList = selectedItems();
    if (itemList.size() != 1)
        return;

    QListWidgetItem* it = itemList.first();
    if (WizAttachmentListViewItem* item = dynamic_cast<WizAttachmentListViewItem*>(it))
    {
        ::WizShowAttachmentHistory(item->attachment());
//        strKbGUID = item->attachment().strKbGUID;
//        CWizDatabase& db = m_dbMgr.db(strKbGUID);
//        db.DeleteAttachment(item->attachment(), true, true, false);
    }

}

void WizAttachmentListView::on_list_itemDoubleClicked(QListWidgetItem* it)
{
    if (WizAttachmentListViewItem* item = dynamic_cast<WizAttachmentListViewItem*>(it))
    {
        openAttachment(item);
        emit closeRequest();
    }
}

void WizAttachmentListView::forceRepaint()
{
    viewport()->repaint();
}



/* ----------------------- CWizAttachmentListWidget ----------------------- */
WizAttachmentListWidget::WizAttachmentListWidget(QWidget* parent)
    : WizPopupWidget(parent)
    , m_list(new WizAttachmentListView(this))
{
    QString strTheme = Utils::WizStyleHelper::themeName();
    setContentsMargins(0, 13, 0, 0);

    setFixedWidth(sizeHint().width());

    QIcon iconAddAttachment = ::WizLoadSkinIcon(strTheme, "document_add_attachment");
    QAction* actionAddAttach = new QAction(iconAddAttachment, tr("Add attachments"), this);
    connect(actionAddAttach, SIGNAL(triggered()), SLOT(on_addAttachment_clicked()));
    m_btnAddAttachment = new WizButton(this);
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

    m_list->setStyleSheet(Utils::WizStyleHelper::wizCommonScrollBarStyleSheet());
    //
    if (isDarkMode()) {
        setStyleSheet("background-color: #333333");
        m_list->setStyleSheet("background-color: #272727");
    }
}

bool WizAttachmentListWidget::setDocument(const WIZDOCUMENTDATA& doc)
{    
    WizDatabase& db = WizDatabaseManager::instance()->db(doc.strKbGUID);
    WIZDOCUMENTDATA document;
    if (!db.documentFromGuid(doc.strGUID, document))
        return false;

    m_list->setDocument(document);

    // reset permission
    if (db.canEditDocument(document) && !WizDatabase::isInDeletedItems(document.strLocation)) {
        m_btnAddAttachment->setEnabled(true);
    } else {
        m_btnAddAttachment->setEnabled(false);
    }

    return true;
}

QSize WizAttachmentListWidget::sizeHint() const
{
    return QSize(271, 310);
}

void WizAttachmentListWidget::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

void WizAttachmentListWidget::on_addAttachment_clicked()
{
    m_list->addAttachments();
}

void WizAttachmentListWidget::on_attachList_closeRequest()
{
    close();
}



WizAttachmentListViewItem::WizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att,
                                                       QListWidget* view)
    : QListWidgetItem(view, 0)
    , m_attachment(att)
    , m_loadState(Unkonwn)
    , m_loadProgress(0)
{
    setToolTip(att.strName);
}

QString WizAttachmentListViewItem::detailText(const WizAttachmentListView* view) const
{
    if (Unkonwn == m_loadState || Downloaded == m_loadState)
    {
        QString strKbGUID = m_attachment.strKbGUID;
        CString strFileName = view->m_dbMgr.db(strKbGUID).getAttachmentFileName(m_attachment.strGUID);
        qint64 size = Utils::WizMisc::getFileSize(strFileName);
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

bool WizAttachmentListViewItem::isDownloading() const
{
    return Downloading == m_loadState;
}

void WizAttachmentListViewItem::setIsDownloading(bool isDownloading)
{
    m_loadState = isDownloading ? Downloading : Downloaded;
}

bool WizAttachmentListViewItem::isUploading() const
{
    return Uploading == m_loadState;
}

void WizAttachmentListViewItem::setIsUploading(bool isUploading)
{
    m_loadState = isUploading ? Uploading : Downloaded;
}

int WizAttachmentListViewItem::loadProgress() const
{
    return m_loadProgress;
}

void WizAttachmentListViewItem::on_downloadFinished(const WIZOBJECTDATA& data, bool bSucceed)
{
    if (data.strObjectGUID == m_attachment.strGUID)
    {
        m_loadState = Downloaded;
        emit updateRequet();
    }
}

void WizAttachmentListViewItem::on_downloadProgress(QString objectGUID, int totalSize, int loadedSize)
{
    if (objectGUID == m_attachment.strGUID)
    {
        m_loadProgress = ((float)loadedSize / (totalSize +1)) * 100;
        m_loadState == Downloading ? 0 : m_loadState = Downloading;
        emit updateRequet();
    }
}
