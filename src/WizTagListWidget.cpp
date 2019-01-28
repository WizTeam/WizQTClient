#include "WizTagListWidget.h"

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QLabel>

#include "share/WizDatabaseManager.h"
#include "share/WizUIHelper.h"
#include "share/WizKMCore.h"
#include "share/WizObject.h"
#include "share/WizDatabase.h"
#include "widgets/WizScrollBar.h"


class CWizTagListWidgetItem: public QListWidgetItem
{
public:
    CWizTagListWidgetItem(const WIZTAGDATA& tag, QListWidget* parent = 0)
        : QListWidgetItem(parent)
        , m_tag(tag)
    {
        setText(WizDatabase::tagNameToDisplayName(tag.strName));
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setCheckState(Qt::Unchecked);
    }

    const WIZTAGDATA& tag() const { return m_tag; }

private:
    WIZTAGDATA m_tag;
};


WizTagListWidget::WizTagListWidget(QWidget* parent)
    : WizPopupWidget(parent)
    , m_dbMgr(*WizDatabaseManager::instance())
    , m_bUpdating(false)
{
    setContentsMargins(0, 20, 0, 0);

//    m_tagsEdit = new QLineEdit(this);
//    m_tagsEdit->setPlaceholderText(tr("Use semicolon to seperate tags..."));
//    connect(m_tagsEdit, SIGNAL(returnPressed()), SLOT(on_tagsEdit_returnPressed()));

    m_list = new WizListWidgetWithCustomScorllBar(this);
    m_list->setAttribute(Qt::WA_MacShowFocusRect, false);
    connect(m_list, SIGNAL(itemChanged(QListWidgetItem*)),
            SLOT(on_list_itemChanged(QListWidgetItem*)));

    if (isDarkMode()) {
#ifdef Q_OS_MAC
        m_list->setStyleSheet("QListView{padding:0px 0px 0px 0px; background-color:#272727}" \
                              "QListView::item{margin:2px 0px 0px 0px; padding-left:-6px; spacing:2px; color:#a6a6a6}");
#else
        m_list->setStyleSheet("QListView{padding:0px 0px 0px 0px; background-color:#373737}" \
                              "QListView::item{margin:2px 0px 0px 0px; padding-left:-6px; spacing:2px; color:#a6a6a6}");
#endif
    } else {
        m_list->setStyleSheet("QListView{padding:0px 0px 0px 0px;}" \
                              "QListView::item{margin:2px 0px 0px 0px; padding-left:-6px; spacing:2px;}");
    }
    //

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QHBoxLayout* layoutTitle = new QHBoxLayout();

    layoutTitle->addWidget(new QLabel(tr("Tags:"), this));
//    layoutTitle->addWidget(m_tagsEdit);
    layoutTitle->setSpacing(0);
    layoutTitle->setContentsMargins(8, 2, 0, 2);

    layout->addLayout(layoutTitle);
    layout->addWidget(m_list);

    //setGeometry(0, 0, sizeHint().width(), sizeHint().height());
}

void WizTagListWidget::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    WizPopupWidget::showEvent(event);
    //
//    m_tagsEdit->clear();
//    m_tagsEdit->clearFocus();
    m_list->setFocus();
    //
}

void WizTagListWidget::reloadTags()
{
    m_list->clear();

    CWizTagDataArray arrayTag;
    m_dbMgr.db().getAllTags(arrayTag);

    foreach (const WIZTAGDATA& tag, arrayTag) {
        CWizTagListWidgetItem* item = new CWizTagListWidgetItem(tag, m_list);
        m_list->addItem(item);
    }
}

void WizTagListWidget::setDocument(const WIZDOCUMENTDATAEX& doc)
{
    QString dbKbGuid = m_dbMgr.db().kbGUID();
    Q_ASSERT(doc.strKbGUID == dbKbGuid);

    m_bUpdating = true;

    reloadTags();

    m_arrayDocuments.clear();
    m_arrayDocuments.push_back(doc);
    QString strGUIDs = m_dbMgr.db().getDocumentTagGuidsString(doc.strGUID);

    for (int i = 0; i < m_list->count(); i++) {
        CWizTagListWidgetItem* pItem = dynamic_cast<CWizTagListWidgetItem*>(m_list->item(i));

        if (-1 != strGUIDs.indexOf(pItem->tag().strGUID)) {
            pItem->setCheckState(Qt::Checked);
            m_list->takeItem(i);
            m_list->insertItem(0, pItem);
        }
    }

    m_bUpdating = false;
}

void WizTagListWidget::setDocuments(const CWizDocumentDataArray& arrayDocument)
{
    m_bUpdating = true;

    reloadTags();
    m_arrayDocuments.clear();

    if (arrayDocument.size() == 0)
        return;

    QString strAllGUIDs;
    for (CWizDocumentDataArray::const_iterator it = arrayDocument.begin();
         it != arrayDocument.end(); it++) {
        const WIZDOCUMENTDATAEX&  doc = *it;
        Q_ASSERT(doc.strKbGUID == m_dbMgr.db().kbGUID());

        m_arrayDocuments.push_back(doc);

        strAllGUIDs += (m_dbMgr.db().getDocumentTagGuidsString(doc.strGUID) + ";");
    }

    QStringList listGUIDs = strAllGUIDs.split(";");

    for (int i = 0; i < m_list->count(); i++) {
        CWizTagListWidgetItem* pItem = dynamic_cast<CWizTagListWidgetItem*>(m_list->item(i));

        int n  = listGUIDs.count(pItem->tag().strGUID);
        if (n  && n < arrayDocument.size()) {
            m_list->takeItem(i);
            m_list->insertItem(0, pItem);
            pItem->setCheckState(Qt::PartiallyChecked);
        } else if (n == arrayDocument.size()) {
            m_list->takeItem(i);
            m_list->insertItem(0, pItem);
            pItem->setCheckState(Qt::Checked);
        }
    }

    m_bUpdating = false;
}

void WizTagListWidget::on_list_itemChanged(QListWidgetItem* pItem)
{
    if (m_bUpdating)
        return;

    CWizTagListWidgetItem* pItemTag = dynamic_cast<CWizTagListWidgetItem *>(pItem);
    if (!pItemTag)
        return;

    for (CWizDocumentDataArray::const_iterator it = m_arrayDocuments.begin();
         it != m_arrayDocuments.end(); it++) {
        WizDocument doc(m_dbMgr.db(), *it);

        m_list->takeItem(m_list->row(pItemTag));
        if (pItemTag->checkState() == Qt::Checked) {
            m_list->insertItem(0, pItemTag);
            doc.addTag(pItemTag->tag());
        } else if (pItemTag->checkState() == Qt::Unchecked) {
            m_list->insertItem(m_list->count(), pItemTag);
            doc.removeTag(pItemTag->tag());
        } else {
            Q_ASSERT(0);
        }
    }
}

void WizTagListWidget::on_tagsEdit_returnPressed()
{
    QString tagsText ;//= m_tagsEdit->text();

    CWizTagDataArray arrayTagNew;
    m_dbMgr.db().tagsTextToTagArray(tagsText, arrayTagNew);

    for (CWizDocumentDataArray::iterator it = m_arrayDocuments.begin();
         it != m_arrayDocuments.end(); it++) {
        WIZDOCUMENTDATAEX& doc = *it;

        for (CWizTagDataArray::const_iterator it = arrayTagNew.begin();
             it != arrayTagNew.end(); it++) {
            const WIZTAGDATA& tag = *it;
            m_dbMgr.db().insertDocumentTag(doc, tag.strGUID);
        }
    }

    CWizDocumentDataArray arrayDocument(m_arrayDocuments);
    setDocuments(arrayDocument);    //refresh tags

    //
//    m_tagsEdit->clear();
}
