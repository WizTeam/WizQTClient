#include "wizattachmentlistwidget.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>

#include <coreplugin/icore.h>

#include "share/wizDatabaseManager.h"
#include "share/wizFileMonitor.h"
#include "wiznotestyle.h"
#include "share/wizmisc.h"
#include "share/wizuihelper.h"
#include "wizdef.h"
#include "wizButton.h"
#include "share/wizObjectDataDownloader.h"

#include "wizmainwindow.h"
#include "utils/pathresolve.h"

using namespace Core::Internal;


#define WIZACTION_ATTACHMENT_ADD    QObject::tr("Add...")
#define WIZACTION_ATTACHMENT_SAVEAS QObject::tr("Save as...")
#define WIZACTION_ATTACHMENT_OPEN   QObject::tr("Open...")
#define WIZACTION_ATTACHMENT_DELETE QObject::tr("Delete")

CWizAttachmentListView::CWizAttachmentListView(QWidget* parent)
    : CWizMultiLineListWidget(2, parent)
    , m_dbMgr(*CWizDatabaseManager::instance())
{
    // FIXME
    QString strTheme = "default";
    setFrameStyle(QFrame::NoFrame);
    setStyle(WizGetStyle(strTheme));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    QPalette pal;
#ifdef Q_OS_LINUX
    pal.setBrush(QPalette::Base, QBrush("#D7D7D7"));
#elif defined(Q_OS_MAC)
    pal.setBrush(QPalette::Base, QBrush("#F7F7F7"));
#endif
    setPalette(pal);

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
    m_menu->addAction(WIZACTION_ATTACHMENT_SAVEAS, this, SLOT(on_action_saveAttachmentAs()));
    m_menu->addAction(WIZACTION_ATTACHMENT_OPEN, this, SLOT(on_action_openAttachment()));
    m_menu->addSeparator();
    m_menu->addAction(WIZACTION_ATTACHMENT_DELETE, this, SLOT(on_action_deleteAttachment()));
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
    return 32;
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
        QString name = item->attachment().strName;
        return m_iconProvider.icon(name).pixmap(32, 32);
    }
    //
    return QPixmap();
}

bool CWizAttachmentListView::itemExtraImage(const QModelIndex& index, const QRect& itemBound, QRect& rcImage, QPixmap& extraPix) const
{
    if (const CWizAttachmentListViewItem* item = attachmentItemFromIndex(index))
    {
        QString strIcoPath;
        CWizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
        MainWindow* mainWindow = qobject_cast<MainWindow *>(Core::ICore::mainWindow());
        if (!db.IsAttachmentDownloaded(item->attachment().strGUID))
        {
            strIcoPath = ::WizGetSkinResourcePath(mainWindow->userSettings().skin()) + "downloading.bmp";
        }
        else if (db.IsAttachmentModified(item->attachment().strGUID))
        {
            strIcoPath = ::WizGetSkinResourcePath(mainWindow->userSettings().skin()) + "uploading.bmp";
        }
        else
            return false;

        QPixmap fullPix(strIcoPath);
        extraPix = fullPix.copy(0, 0, fullPix.height(), fullPix.height());
        extraPix.setMask(extraPix.createMaskFromColor(Qt::black, Qt::MaskInColor));
        int nMargin = -1;
        rcImage.setLeft(itemBound.right() - extraPix.width() - nMargin);
        rcImage.setTop(itemBound.bottom() - extraPix.height() - nMargin);
        rcImage.setSize(extraPix.size());

        return true;
    }

    return false;
}

void CWizAttachmentListView::resetAttachments()
{
    clear();

    CWizDocumentAttachmentDataArray arrayAttachment;
    m_dbMgr.db(m_document.strKbGUID).GetDocumentAttachments(m_document.strGUID, arrayAttachment);

    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        addItem(newAttachmentItem(*it));
    }
}


void CWizAttachmentListView::setDocument(const WIZDOCUMENTDATA& document)
{
    m_document = document;
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
                             tr("All files(*.*)"));
    //
    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);

    bool ok = false;
    foreach (QString fileName, files)
    {
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

    const WIZDOCUMENTATTACHMENTDATA& attachment = item->attachment();

    CWizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.IsObjectDataDownloaded(attachment.strGUID, "attachment");
    QString strFileName = db.GetAttachmentFileName(item->attachment().strGUID);
    bool bExists = PathFileExists(strFileName);
    if (!bIsLocal || !bExists) {
        //m_downloadDialog->downloadData(attachment);
        startDownLoad(item);
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(strFileName));

    CWizFileMonitor& monitor = CWizFileMonitor::instance();
    connect(&monitor, SIGNAL(fileModified(QString,QString,QString,QString,QDateTime)),
            &m_dbMgr.db(), SLOT(onAttachmentModified(QString,QString,QString,QString,QDateTime)), Qt::UniqueConnection);

    /*需要使用文件的修改日期,从服务器上下载下的文件修改日期必定大于数据库中日期.*/
    QFileInfo info(strFileName);
    monitor.addFile(attachment.strKbGUID, attachment.strGUID, strFileName,
                    attachment.strDataMD5, info.lastModified());
}

void CWizAttachmentListView::contextMenuEvent(QContextMenuEvent * e)
{
    resetPermission();
    m_menu->popup(e->globalPos());
}

void CWizAttachmentListView::resetPermission()
{
    QString strUserId = m_dbMgr.db().getUserId();
    int nPerm = m_dbMgr.db(m_document.strKbGUID).permission();

    if (nPerm <= WIZ_USERGROUP_EDITOR
            || (nPerm == WIZ_USERGROUP_AUTHOR && m_document.strOwner == strUserId)) {
        findAction(WIZACTION_ATTACHMENT_ADD)->setEnabled(true);
        findAction(WIZACTION_ATTACHMENT_DELETE)->setEnabled(true);
    } else {
        findAction(WIZACTION_ATTACHMENT_ADD)->setEnabled(false);
        findAction(WIZACTION_ATTACHMENT_DELETE)->setEnabled(false);
    }

    // if user select noting
    if (!selectedItems().size()) {
        findAction(WIZACTION_ATTACHMENT_OPEN)->setEnabled(false);
        findAction(WIZACTION_ATTACHMENT_SAVEAS)->setEnabled(false);
        findAction(WIZACTION_ATTACHMENT_DELETE)->setEnabled(false);
    } else {
        foreach (QListWidgetItem* item, selectedItems()) {
            CWizAttachmentListViewItem* attachItem = dynamic_cast<CWizAttachmentListViewItem*>(item);
            if (attachItem && (attachItem->isDownloading() || attachItem->isUploading())) {
                findAction(WIZACTION_ATTACHMENT_OPEN)->setEnabled(false);
                findAction(WIZACTION_ATTACHMENT_SAVEAS)->setEnabled(false);
                findAction(WIZACTION_ATTACHMENT_DELETE)->setEnabled(false);

                return;
            }
        }
        findAction(WIZACTION_ATTACHMENT_OPEN)->setEnabled(true);
        findAction(WIZACTION_ATTACHMENT_SAVEAS)->setEnabled(true);
    }
}

void CWizAttachmentListView::startDownLoad(CWizAttachmentListViewItem* item)
{
    m_downloaderHost->download(item->attachment());
    item->setIsDownloading(true);

    forceRepaint();
}

CWizAttachmentListViewItem* CWizAttachmentListView::newAttachmentItem(const WIZDOCUMENTATTACHMENTDATA& att)
{
    CWizAttachmentListViewItem* newItem = new CWizAttachmentListViewItem(att);
    connect(newItem, SIGNAL(updateRequet()), SLOT(forceRepaint()));
    connect(m_downloaderHost, SIGNAL(downloadDone(WIZOBJECTDATA,bool)), newItem,
            SLOT(on_downloadFinished(WIZOBJECTDATA,bool)));
    connect(m_downloaderHost, SIGNAL(downloadProgress(QString,int,int)), newItem,
            SLOT(on_downloadProgress(QString,int,int)));

    return newItem;
}

void CWizAttachmentListView::on_action_addAttachment()
{
    addAttachments();
}

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
            QString strFileName = QFileDialog::getSaveFileName(this, QString(), item->attachment().strName);
            if (strFileName.isEmpty())
                return;

            CWizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
            bool bIsLocal = db.IsObjectDataDownloaded(item->attachment().strGUID, "attachment");
            bool bExists = PathFileExists(db.GetAttachmentFileName(item->attachment().strGUID));
            if (!bIsLocal || !bExists) {
                //m_downloadDialog->downloadData(item->attachment());
                startDownLoad(item);
                return;
            }

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
                    startDownLoad(item);
                    continue;
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
}

void CWizAttachmentListView::on_action_deleteAttachment()
{
    foreach (QListWidgetItem* it, selectedItems())
    {
        if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
        {
            CWizDatabase& db = m_dbMgr.db(item->attachment().strKbGUID);
            db.DeleteAttachment(item->attachment(), true);
        }
    }

    resetAttachments();
}

void CWizAttachmentListView::on_list_itemDoubleClicked(QListWidgetItem* it)
{
    if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
    {
        openAttachment(item);
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
    // FIXME
    QString strTheme = "default";
    setContentsMargins(0, 20, 0, 0);

    QIcon iconAddAttachment = ::WizLoadSkinIcon(strTheme, "document_add_attachment");
    QAction* actionAddAttach = new QAction(iconAddAttachment, tr("Add attachments"), this);
    connect(actionAddAttach, SIGNAL(triggered()), SLOT(on_addAttachment_clicked()));
    m_btnAddAttachment = new CWizButton(this);
    m_btnAddAttachment->setAction(actionAddAttach);

    QHBoxLayout* layoutHeader = new QHBoxLayout();
    layoutHeader->setContentsMargins(20, 0, 20, 0);
    layoutHeader->addWidget(new QLabel(tr("Attachments"), this));
    layoutHeader->addStretch();
    layoutHeader->addWidget(m_btnAddAttachment);

    QVBoxLayout* layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->setSpacing(5);
    setLayout(layoutMain);

    layoutMain->addLayout(layoutHeader);
    layoutMain->addWidget(m_list);
}

void CWizAttachmentListWidget::setDocument(const WIZDOCUMENTDATA& doc)
{
    if (m_currentDocument != doc.strGUID)
    {
        m_list->setDocument(doc);
        m_currentDocument = doc.strGUID;

        // reset permission
        if (CWizDatabaseManager::instance()->db(doc.strKbGUID).CanEditDocument(doc)) {
            m_btnAddAttachment->setEnabled(true);
        } else {
            m_btnAddAttachment->setEnabled(false);
        }
    }
}

void CWizAttachmentListWidget::on_addAttachment_clicked()
{
    m_list->addAttachments();
}



CWizAttachmentListViewItem::CWizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att) : m_attachment(att)
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
        qint64 size = ::WizGetFileSize(strFileName);
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
        return QString(tr("Uploading (%1 %) ...").arg(m_loadProgress));
    }
    return QString(tr("Unknow State"));
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
