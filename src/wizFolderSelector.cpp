#include "wizFolderSelector.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>

#include "wizdef.h"
#include "share/wizDatabase.h"
#include "share/wizMessageBox.h"
#include "wizFolderView.h"

CWizFolderSelector::CWizFolderSelector(const QString& strTitle, CWizExplorerApp& app,
                                       unsigned int nPermission, QWidget *parent)
    : QDialog(parent)
    , m_app(app)
    , m_nMinPermission(nPermission)
    , m_bAcceptRoot(true)
    , m_bKeepTime(true)
    , m_bKeepTags(true)
{
    setWindowTitle(strTitle);
    setFixedSize(400, 420);

    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);

    m_folderView = new CWizFolderView(app, this);
    layout->addWidget(m_folderView);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    connect(buttonBox, SIGNAL(accepted()), SLOT(on_accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    layout->addWidget(buttonBox);
}

void CWizFolderSelector::setCopyStyle(bool showKeepTagsOption)
{
    QVBoxLayout* lay = qobject_cast<QVBoxLayout*>(layout());

    QCheckBox* checkKeepTime = new QCheckBox(tr("Keep create/update time"), this);
    checkKeepTime->setCheckState(m_bKeepTime ? Qt::Checked : Qt::Unchecked);
    connect(checkKeepTime, SIGNAL(stateChanged(int)), SLOT(on_checkKeepTime_stateChanged(int)));

    lay->insertWidget(1, checkKeepTime);

    if (showKeepTagsOption)
    {
        QCheckBox* checkKeepTags = new QCheckBox(tr("Keep tags"), this);
        checkKeepTags->setCheckState(m_bKeepTags ? Qt::Checked : Qt::Unchecked);
        connect(checkKeepTags, SIGNAL(stateChanged(int)), SLOT(on_checkKeepTags_stateChanged(int)));

        lay->insertWidget(2, checkKeepTags);
    }
}

bool CWizFolderSelector::isKeepTime() const
{
    return m_bKeepTime;
}

bool CWizFolderSelector::isKeepTag() const
{
    return m_bKeepTags;
}

bool CWizFolderSelector::isSelectGroupFolder()
{
    QTreeWidgetItem* item = m_folderView->currentItem();

    if (item->type() > QTreeWidgetItem::UserType)
    {
        CWizCategoryViewItemBase* baseItem = dynamic_cast<CWizCategoryViewItemBase*>(item);
        if (!baseItem)
            return false;

        int nPermission = m_app.databaseManager().db(baseItem->kbGUID()).permission();
        if (nPermission > m_nMinPermission)
        {
            if (nPermission >= WIZ_USERGROUP_READER)
            {
                CWizMessageBox::warning(this, tr("Info"), tr("You have no permission to create note in this group!"));
            }
            else if (nPermission >= WIZ_USERGROUP_EDITOR)
            {
                CWizMessageBox::warning(this, tr("Info"), tr("You have no permission to create folder in this group!"));
            }
            return false;
        }
    }

    qDebug() << "folder selector , item type : " << item->type() << " accept root : " << m_bAcceptRoot;

    if (!m_bAcceptRoot)
        return item->type() == Category_GroupNoTagItem || item->type() == Category_GroupItem;

    return item->type() == Category_GroupRootItem || item->type() == Category_GroupNoTagItem
            || item->type() == Category_GroupItem;
}

QString CWizFolderSelector::selectedFolder()
{
    QTreeWidgetItem* item = m_folderView->currentItem();
    if (item->type() == Category_AllFoldersItem)
    {
        return "/";
    }
    else if (item->type() == Category_FolderItem)
    {
        CWizCategoryViewFolderItem* p = dynamic_cast<CWizCategoryViewFolderItem*>(item);
        Q_ASSERT(p);

        return p->location();
    }

    return NULL;
}

bool CWizFolderSelector::isSelectPersonalFolder()
{
    QTreeWidgetItem* item = m_folderView->currentItem();

    qDebug() << "folder selector , item type : " << item->type() << " accept root : " << m_bAcceptRoot;
    if (!m_bAcceptRoot)
        return item->type() == Category_FolderItem;

    return item->type() == Category_AllFoldersItem || item->type() == Category_FolderItem;
}

WIZTAGDATA CWizFolderSelector::selectedGroupFolder()
{
    QTreeWidgetItem* item = m_folderView->currentItem();
    if (item->type() == Category_GroupItem)
    {
        CWizCategoryViewGroupItem* p = dynamic_cast<CWizCategoryViewGroupItem*>(item);
        if (p)
        {
            return p->tag();
        }
    }
    else if (item->type() == Category_GroupNoTagItem || item->type() == Category_GroupRootItem)
    {
        CWizCategoryViewItemBase* p = dynamic_cast<CWizCategoryViewItemBase*>(item);
        if (p)
        {
            WIZTAGDATA tag;
            tag.strKbGUID = p->kbGUID();
            return tag;
        }
    }

    WIZTAGDATA tag;
    return tag;
}

void CWizFolderSelector::on_accept()
{
    // accept only if user select folder item.
    if (isSelectPersonalFolder() || isSelectGroupFolder())
    {
        accept();
    }
}

void CWizFolderSelector::on_checkKeepTime_stateChanged(int state)
{
    if (Qt::Checked == state) {
        m_bKeepTime = true;
    } else {
        m_bKeepTime = false;
    }
}

void CWizFolderSelector::on_checkKeepTags_stateChanged(int state)
{
    if (Qt::Checked == state) {
        m_bKeepTags = true;
    } else {
        m_bKeepTags = false;
    }
}
