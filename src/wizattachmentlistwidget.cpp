#include "wizattachmentlistwidget.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QDesktopServices>

#include "wiznotestyle.h"
#include "share/wizmisc.h"
#include "share/wizuihelper.h"
#include "share/wizdownloadobjectdata.h"
#include "wizdef.h"

#include "wizmainwindow.h"


class CWizAttachmentListViewItem : public QListWidgetItem
{
    WIZDOCUMENTATTACHMENTDATA m_attachment;
public:
    CWizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att)
        : m_attachment(att)
    {

    }

    QString detailText(const CWizAttachmentListView* view) const
    {
        QString strKbGUID = m_attachment.strKbGUID;
        CString strFileName = view->m_dbMgr.db(strKbGUID).GetAttachmentFileName(m_attachment.strGUID);
        qint64 size = ::WizGetFileSize(strFileName);
        CString strSize = 0 == size ? CString(QObject::tr("Un-downloaded")) : WizFormatInt(size);
        CString strType = view->m_iconProvider.type(m_attachment.strName);
        return strSize + "  " + strType;
    }

    const WIZDOCUMENTATTACHMENTDATA& attachment() const
    {
        return m_attachment;
    }
};

#define WIZACTION_ATTACHMENT_ADD    QObject::tr("Add...")
#define WIZACTION_ATTACHMENT_SAVEAS QObject::tr("Save as...")
#define WIZACTION_ATTACHMENT_OPEN   QObject::tr("Open...")
#define WIZACTION_ATTACHMENT_DELETE QObject::tr("Delete")

CWizAttachmentListView::CWizAttachmentListView(CWizExplorerApp& app, QWidget* parent)
    : CWizMultiLineListWidget(2, parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
{
    setFrameStyle(QFrame::NoFrame);
    setStyle(WizGetStyle(m_app.userSettings().skin()));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    m_downloadDialog = mainWindow->objectDownloadDialog();

    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(on_list_itemDoubleClicked(QListWidgetItem*)));

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

    CString strTempPath = ::WizGlobal()->GetTempPath();
    const WIZDOCUMENTATTACHMENTDATA& attachment = item->attachment();

    CWizDatabase& db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.IsObjectDataDownloaded(attachment.strGUID, "attachment");
    bool bExists = PathFileExists(db.GetAttachmentFileName(attachment.strGUID));
    if (!bIsLocal || !bExists) {
        m_downloadDialog->downloadData(attachment);
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
    m_menu->popup(mapToGlobal(e->pos()));
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
                m_downloadDialog->downloadData(item->attachment());
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
                    m_downloadDialog->downloadData(item->attachment());
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWizAttachmentListWidget::CWizAttachmentListWidget(CWizExplorerApp& app, QWidget* parent)
    : CWizPopupWidget(parent)
    , m_app(app)
    , m_list(new CWizAttachmentListView(app, this))
{
    QWidget* root = new QWidget(this);
    root->setGeometry(0, 0, sizeHint().width(), sizeHint().height());
    root->setContentsMargins(8, 20, 8, 8);

    QVBoxLayout* layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    root->setLayout(layoutMain);

    QWidget* header = new QWidget(root);
    QHBoxLayout* layoutHeader = new QHBoxLayout();
    layoutHeader->setContentsMargins(0, 0, 0, 0);
    header->setLayout(layoutHeader);

    QIcon iconAddAttachment = ::WizLoadSkinIcon(m_app.userSettings().skin(), palette().window().color(), "add_attachment_button");
    m_btnAddAttachment = new CWizImagePushButton(iconAddAttachment, "", root);
    m_btnAddAttachment->setStyle(WizGetStyle(m_app.userSettings().skin()));
    m_btnAddAttachment->setToolTip(tr("Add attachments"));
    layoutHeader->addWidget(new CWizFixedSpacer(QSize(4, 2), root));
    layoutHeader->addWidget(new QLabel(tr("Attachments"), root));
    layoutHeader->addWidget(new CWizSpacer(root));
    layoutHeader->addWidget(m_btnAddAttachment);
    connect(m_btnAddAttachment, SIGNAL(clicked()), SLOT(on_addAttachment_clicked()));

    QWidget* line = new QWidget(root);
    line->setMaximumHeight(1);
    line->setMinimumHeight(1);
    line->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#d9dcdd");

    layoutMain->addWidget(header);
    layoutMain->addWidget(line);
    layoutMain->addWidget(m_list);
}

void CWizAttachmentListWidget::setDocument(const WIZDOCUMENTDATA& document)
{
    m_list->setDocument(document);;
    resetPermission();
}

void CWizAttachmentListWidget::resetPermission()
{
    QString strUserId = m_app.databaseManager().db().getUserId();
    int nPerm = m_app.databaseManager().db(m_list->document().strKbGUID).permission();

    if (nPerm <= WIZ_USERGROUP_EDITOR
            || (nPerm == WIZ_USERGROUP_AUTHOR && m_list->document().strOwner == strUserId)) {
        m_btnAddAttachment->setEnabled(true);
    } else {
        m_btnAddAttachment->setEnabled(false);
    }
}

void CWizAttachmentListWidget::on_addAttachment_clicked()
{
    m_list->addAttachments();
}
