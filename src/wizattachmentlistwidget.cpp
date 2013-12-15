#include "wizattachmentlistwidget.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QLabel>

#include <coreplugin/icore.h>

#include "share/wizDatabaseManager.h"

#include "wiznotestyle.h"
#include "share/wizmisc.h"
#include "share/wizuihelper.h"
#include "wizdef.h"
#include "wizButton.h"
#include "share/wizObjectDataDownloader.h"

#include "wizmainwindow.h"
#include "utils/pathresolve.h"

using namespace Core::Internal;


class CWizAttachmentListViewItem : public QListWidgetItem
{
public:
    CWizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att) : m_attachment(att) {}
    const WIZDOCUMENTATTACHMENTDATA& attachment() const { return m_attachment; }

    QString detailText(const CWizAttachmentListView* view) const
    {
        QString strKbGUID = m_attachment.strKbGUID;
        CString strFileName = view->m_dbMgr.db(strKbGUID).GetAttachmentFileName(m_attachment.strGUID);
        qint64 size = ::WizGetFileSize(strFileName);
        CString strSize = 0 == size ? CString(QObject::tr("Un-downloaded")) : WizFormatInt(size);
        CString strType = view->m_iconProvider.type(m_attachment.strName);
        return strSize + "  " + strType;
    }

private:
    WIZDOCUMENTATTACHMENTDATA m_attachment;
};

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
    //connect(m_downloaderHost, SIGNAL(downloadDone(const WIZOBJECTDATA&, bool)),
    //        SLOT(on_download_finished(const WIZOBJECTDATA&, bool)));

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


void CWizAttachmentListView::resetAttachments()
{
    clear();

    CWizDocumentAttachmentDataArray arrayAttachment;
    m_dbMgr.db(m_document.strKbGUID).GetDocumentAttachments(m_document.strGUID, arrayAttachment);

    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        addItem(new CWizAttachmentListViewItem(*it));
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
                             tr("/home"),
                             tr("All files(*.*)"));

    foreach (QString fileName, files)
    {
        WIZDOCUMENTATTACHMENTDATA data;
        data.strKbGUID = m_document.strKbGUID; // needed by under layer
        m_dbMgr.db(m_document.strKbGUID).AddAttachment(m_document, fileName, data);
        addItem(new CWizAttachmentListViewItem(data));
    }
}

void CWizAttachmentListView::openAttachment(CWizAttachmentListViewItem* item)
{
    if (!item)
        return;

    CString strTempPath = Utils::PathResolve::tempPath();
    const WIZDOCUMENTATTACHMENTDATA& attachment = item->attachment();

    CWizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.IsObjectDataDownloaded(attachment.strGUID, "attachment");
    bool bExists = PathFileExists(db.GetAttachmentFileName(attachment.strGUID));
    if (!bIsLocal || !bExists) {
        //m_downloadDialog->downloadData(attachment);
        m_downloaderHost->download(attachment);
        return;
    }

    CString strTempFileName = strTempPath + item->attachment().strName;
    ::WizGetNextFileName(strTempFileName);

    if (!::WizCopyFile(db.GetAttachmentFileName(item->attachment().strGUID), strTempFileName, FALSE))
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Can not save attachment to %1").arg(strTempFileName));
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(strTempFileName));
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
        findAction(WIZACTION_ATTACHMENT_OPEN)->setEnabled(true);
        findAction(WIZACTION_ATTACHMENT_SAVEAS)->setEnabled(true);
    }
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
                m_downloaderHost->download(item->attachment());
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
                    m_downloaderHost->download(item->attachment());
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
    m_list->setDocument(doc);;

    // reset permission
    if (CWizDatabaseManager::instance()->db(doc.strKbGUID).CanEditDocument(doc)) {
        m_btnAddAttachment->setEnabled(true);
    } else {
        m_btnAddAttachment->setEnabled(false);
    }
}

void CWizAttachmentListWidget::on_addAttachment_clicked()
{
    m_list->addAttachments();
}
