#include "wiztaglistwidget.h"

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>

#include "share/wizuihelper.h"
#include "share/wizkmcore.h"


class CWizTagListWidgetItem: public QListWidgetItem
{
public:
    CWizTagListWidgetItem(const WIZTAGDATA& tag, QListWidget* parent = 0)
        : QListWidgetItem(parent)
        , m_tag(tag)
    {
        setText(CWizDatabase::TagNameToDisplayName(tag.strName));
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setChecked(false);
    }

    void setChecked(bool b)
    {
        m_bChecked = b;
        setCheckState(b ? Qt::Checked : Qt::Unchecked);
    }

    const WIZTAGDATA& tag() const { return m_tag; }
    bool tagState() const { return m_bChecked; }

private:
    WIZTAGDATA m_tag;
    bool m_bChecked;
};


CWizTagListWidget::CWizTagListWidget(CWizDatabaseManager& db, QWidget* parent)
    : CWizPopupWidget(parent)
    , m_dbMgr(db)
{
    setGeometry(0, 0, sizeHint().width(), sizeHint().height());
    setContentsMargins(8, 20, 8, 8);


    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QHBoxLayout* layoutTitle = new QHBoxLayout();
    m_tagsEdit = new QLineEdit(this);
    layoutTitle->addWidget(new QLabel(tr("Tags:"), this));
    layoutTitle->addWidget(m_tagsEdit);
    layoutTitle->setSpacing(8);
    layoutTitle->setMargin(4);

    m_list = new QListWidget(this);
    m_list->setAttribute(Qt::WA_MacShowFocusRect, false);

    QPalette pal;
#ifdef Q_OS_LINUX
    pal.setBrush(QPalette::Base, QBrush("#D7D7D7"));
#elif defined(Q_OS_MAC)
    pal.setBrush(QPalette::Base, QBrush("#F7F7F7"));
#endif
    m_list->setPalette(pal);

    layout->addLayout(layoutTitle);
    layout->addWidget(m_list);

    connect(m_list, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(on_list_itemClicked(QListWidgetItem*)));
    connect(m_tagsEdit, SIGNAL(editingFinished()), SLOT(on_tagsEdit_editingFinished()));
}

void CWizTagListWidget::setDocument(const WIZDOCUMENTDATA& data)
{
    m_list->clear();
    m_document = data;

    CWizDatabase& db = m_dbMgr.db(data.strKbGUID);
    CWizTagDataArray arrayTag;
    db.GetAllTags(arrayTag);
    CString strGUIDs = db.GetDocumentTagGUIDsString(m_document.strGUID);

    foreach (const WIZTAGDATA& tag, arrayTag) {
        CWizTagListWidgetItem* item = new CWizTagListWidgetItem(tag, m_list);
        m_list->addItem(item);

        if (-1 != strGUIDs.indexOf(tag.strGUID)) {
            item->setChecked(true);
        }
    }

    updateTagsText();
}

void CWizTagListWidget::updateTagsText()
{
    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);
    QString strTagsText = db.GetDocumentTagDisplayNameText(m_document.strGUID);
    m_tagsEdit->setText(strTagsText);
}

void CWizTagListWidget::on_list_itemClicked(QListWidgetItem* item)
{
    CWizTagListWidgetItem* it = dynamic_cast<CWizTagListWidgetItem *>(item);

    bool bStateChanged = (it->checkState() && !it->tagState()) ||
            (!it->checkState() && it->tagState());

    if (!bStateChanged)
        return;

    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);
    CWizDocument doc(db, m_document);

    if (item->checkState()) {
        doc.AddTag(it->tag());
        it->setChecked(true);
    } else {
        doc.RemoveTag(it->tag());
        it->setChecked(false);
    }

    updateTagsText();
}

void CWizTagListWidget::on_tagsEdit_editingFinished()
{
    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);
    CString tagsText = m_tagsEdit->text();

    CWizTagDataArray arrayTagNew;
    db.TagsTextToTagArray(tagsText, arrayTagNew);

    CWizTagDataArray arrayTagOld;
    db.GetDocumentTags(m_document.strGUID, arrayTagOld);

    if (::WizKMDataArrayIsEqual(arrayTagOld, arrayTagNew)) {
        return;
    }

    db.SetDocumentTags(m_document, arrayTagNew);
    setDocument(m_document);    //refresh tags
}
