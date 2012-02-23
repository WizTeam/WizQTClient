#include "wizattachmentlistwidget.h"
#include "wiznotestyle.h"
#include "share/wizmisc.h"
#include "share/wizimagepushbutton.h"
#include "share/wizuihelper.h"
#include <QBoxLayout>
#include <QFileDialog>
#include <QDesktopServices>

class CWizAttachmentListViewItem : public QListWidgetItem
{
    WIZDOCUMENTATTACHMENTDATA m_attachment;
public:
    CWizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att)
        : m_attachment(att)
    {

    }
public:
    QString detailText(const CWizAttachmentListView* view) const
    {
        CString strFileName = view->m_db.GetAttachmentFileName(m_attachment.strGUID);
        long long size = ::WizGetFileSize(strFileName);
        //
        CString strSize = WizFormatInt(size);
        //
        CString strType = view->m_iconProvider.type(m_attachment.strName);
        //
        return strSize + "  " + strType;
    }
    const WIZDOCUMENTATTACHMENTDATA& attachment() const
    {
        return m_attachment;
    }
};

CWizAttachmentListView::CWizAttachmentListView(CWizDatabase& db, QWidget* parent)
    : CWizMultiLineListWidget(2, parent)
    , m_db(db)
    , m_menu(NULL)
{
    setFrameStyle(QFrame::NoFrame);
    //
    setStyle(WizGetStyle());
    //
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    //
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_list_itemDoubleClicked(QListWidgetItem*)));
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
    //
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.GetDocumentAttachments(m_document.strGUID, arrayAttachment);
    //
    for (CWizDocumentAttachmentDataArray::const_iterator it = arrayAttachment.begin();
    it != arrayAttachment.end();
    it++)
    {
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
    //
    foreach (QString fileName, files)
    {
        WIZDOCUMENTATTACHMENTDATA data;
        m_db.AddAttachment(m_document, fileName, data);
        addItem(new CWizAttachmentListViewItem(data));
    }
}
void CWizAttachmentListView::openAttachment(CWizAttachmentListViewItem* item)
{
    if (!item)
        return;
    //
    CString strTempPath = ::WizGlobal()->GetTempPath();
    //
    CString strTempFileName = strTempPath + item->attachment().strName;
    ::WizGetNextFileName(strTempFileName);
    //
    if (!::WizCopyFile(m_db.GetAttachmentFileName(item->attachment().strGUID), strTempFileName, FALSE))
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Can not save attachment to %1").arg(strTempFileName));
        return;
    }
    //
    QDesktopServices::openUrl(QUrl::fromLocalFile(strTempFileName));
}

void CWizAttachmentListView::contextMenuEvent(QContextMenuEvent * e)
{
    if (!m_menu)
    {
        m_menu = new QMenu(this);
        m_menu->addAction(tr("Add..."), this, SLOT(on_action_addAttachment()));
        m_menu->addSeparator();
        m_menu->addAction(tr("Save as..."), this, SLOT(on_action_saveAttachmentAs()));
        m_menu->addAction(tr("Open..."), this, SLOT(on_action_openAttachment()));
        m_menu->addSeparator();
        m_menu->addAction(tr("Delete"), this, SLOT(on_action_deleteAttachment()));
    }
    //
    m_menu->popup(mapToGlobal(e->pos()));
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
            //
            if (!::WizCopyFile(m_db.GetAttachmentFileName(item->attachment().strGUID), strFileName, FALSE))
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
        //
        foreach (QListWidgetItem* it, items)
        {
            if (CWizAttachmentListViewItem* item = dynamic_cast<CWizAttachmentListViewItem*>(it))
            {
                CString strFileName = strDir + item->attachment().strName;
                WizGetNextFileName(strFileName);
                //
                if (!::WizCopyFile(m_db.GetAttachmentFileName(item->attachment().strGUID), strFileName, FALSE))
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
            m_db.DeleteAttachment(item->attachment(), TRUE);
        }
    }
    //
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

CWizAttachmentListWidget::CWizAttachmentListWidget(CWizDatabase& db, QWidget* parent)
    : CWizPopupWidget(parent)
    , m_list(new CWizAttachmentListView(db, this))
{
    QLayout* layoutMain = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(layoutMain);
    layoutMain->setMargin(0);
    //
    QLayout* layoutHeader = new QHBoxLayout(this);
    layoutHeader->setMargin(0);
    QIcon iconAddAttachment = ::WizLoadSkinIcon("add_attachment_button");
    CWizImagePushButton* addAttachmentButton = new CWizImagePushButton(iconAddAttachment, "", this);
    addAttachmentButton->setStyle(WizGetStyle());
    addAttachmentButton->setToolTip(tr("Add attachments"));
    layoutHeader->addWidget(new QLabel(tr("Attachments"), this));
    layoutHeader->addWidget(new CWizSpacer(this));
    layoutHeader->addWidget(addAttachmentButton);
    connect(addAttachmentButton, SIGNAL(clicked()), this, SLOT(on_addAttachment_clicked()));
    //
    QWidget* line = new QWidget(this);
    line->setMaximumHeight(1);
    line->setMinimumHeight(1);
    line->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#d9dcdd");
    //
    layoutMain->addItem(layoutHeader);
    layoutMain->addWidget(line);
    layoutMain->addWidget(m_list);
}

void CWizAttachmentListWidget::setDocument(const WIZDOCUMENTDATA& document)
{
    m_list->setDocument(document);;
}

void CWizAttachmentListWidget::on_addAttachment_clicked()
{
    m_list->addAttachments();
}
