#include "wizFolderSelector.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>

#include "wizFolderView.h"

CWizFolderSelector::CWizFolderSelector(const QString& strTitle, CWizExplorerApp& app, QWidget *parent)
    : QDialog(parent)
    , m_app(app)
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
    connect(buttonBox, SIGNAL(accepted()), SLOT(on_accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    layout->addWidget(buttonBox);
}

void CWizFolderSelector::setCopyStyle()
{
    QVBoxLayout* lay = qobject_cast<QVBoxLayout*>(layout());

    QCheckBox* checkKeepTime = new QCheckBox(tr("Keep create/update time"), this);
    checkKeepTime->setCheckState(m_bKeepTime ? Qt::Checked : Qt::Unchecked);
    connect(checkKeepTime, SIGNAL(stateChanged(int)), SLOT(on_checkKeepTime_stateChanged(int)));

    QCheckBox* checkKeepTags = new QCheckBox(tr("Keep tags"), this);
    checkKeepTags->setCheckState(m_bKeepTags ? Qt::Checked : Qt::Unchecked);
    connect(checkKeepTags, SIGNAL(stateChanged(int)), SLOT(on_checkKeepTags_stateChanged(int)));

    lay->insertWidget(1, checkKeepTime);
    lay->insertWidget(2, checkKeepTags);
}

QString CWizFolderSelector::selectedFolder()
{
    CWizCategoryViewAllFoldersItem* pRoot = dynamic_cast<CWizCategoryViewAllFoldersItem*>(m_folderView->currentItem());
    if (pRoot) {
        return "/";
    }

    CWizCategoryViewFolderItem* p = dynamic_cast<CWizCategoryViewFolderItem*>(m_folderView->currentItem());
    if (p) {
        return p->location();
    }

    return NULL;
}

void CWizFolderSelector::on_accept()
{
    // accept only if user select folder item.
    if (!m_bAcceptRoot) {
        CWizCategoryViewFolderItem* p = dynamic_cast<CWizCategoryViewFolderItem*>(m_folderView->currentItem());
        if (!p)
            return;
    }

    accept();
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
