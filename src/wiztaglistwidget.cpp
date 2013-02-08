#include "wiztaglistwidget.h"
#include "share/wizuihelper.h"

#include "share/wizkmcore.h"

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

#include <QBoxLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QLineEdit>


CWizTagListWidget::CWizTagListWidget(CWizDatabaseManager& db, QWidget* parent)
    : CWizPopupWidget(parent)
    , m_tagsEdit(NULL)
    , m_scroll(NULL)
    , m_dbMgr(db)
{
    QWidget* root = new QWidget(this);
    root->setGeometry(0, 0, sizeHint().width(), sizeHint().height());
    root->setContentsMargins(8, 20, 8, 8);
    QVBoxLayout* layoutMain = new QVBoxLayout(root);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    root->setLayout(layoutMain);

    QHBoxLayout* layoutTitle = new QHBoxLayout(root);
    m_tagsEdit = new QLineEdit(root);
    layoutTitle->addWidget(new QLabel(tr("Tags:"), root));
    layoutTitle->addWidget(m_tagsEdit);
    layoutTitle->setSpacing(8);
    layoutTitle->setMargin(4);
    layoutMain->addLayout(layoutTitle);

    m_scroll = new QScrollArea(root);
    m_scroll->setFrameStyle(QFrame::NoFrame);
    layoutMain->addWidget(m_scroll);
    layoutMain->addWidget(new CWizVerSpacer(root));

    m_scroll->setMaximumHeight(400);

    connect(m_tagsEdit, SIGNAL(editingFinished()), SLOT(on_tagsEdit_editingFinished()));
}

void CWizTagListWidget::setDocument(const WIZDOCUMENTDATA& data)
{
    CWizDatabase& db = m_dbMgr.db(data.strKbGUID);
    if (QWidget* oldWidget = m_scroll->takeWidget())
    {
        //oldWidget->children();

        delete oldWidget;
    }
    //
    QWidget* newWidget = new QWidget(m_scroll);
    //
    QLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, newWidget);
    newWidget->setLayout(layout);
    //
    m_document = data;
    //
    CWizTagDataArray arrayTag;
    //
    db.GetAllTags(arrayTag);
    //
    CString strGUIDs = db.GetDocumentTagGUIDsString(m_document.strGUID);
    //
    foreach (const WIZTAGDATA& tag, arrayTag)
    {
        CWizTagCheckBox* checkBox = new CWizTagCheckBox(tag, newWidget);
        layout->addWidget(checkBox);
        if (-1 != strGUIDs.indexOf(tag.strGUID))
        {
            checkBox->setChecked(true);
        }

        connect(checkBox, SIGNAL(tagChecked(CWizTagCheckBox*, int)), \
                SLOT(on_tagCheckBox_checked(CWizTagCheckBox*, int)));
    }
    //
    updateTagsText();
    //
    newWidget->adjustSize();
    //
    m_scroll->setWidget(newWidget);
    m_scroll->setWidgetResizable(false);
}

void CWizTagListWidget::updateTagsText()
{
    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);
    CString strTagsText = db.GetDocumentTagDisplayNameText(m_document.strGUID);
    m_tagsEdit->setText(strTagsText);
}

void CWizTagListWidget::on_tagCheckBox_checked(CWizTagCheckBox* sender, int state)
{
    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);
    CWizDocument doc(db, m_document);
    const WIZTAGDATA& tag = sender->tag();
    if (state == Qt::Checked)
    {
        doc.AddTag(tag);
    }
    else
    {
        doc.RemoveTag(tag);
    }
    //
    updateTagsText();
}
void CWizTagListWidget::on_tagsEdit_editingFinished()
{
    CWizDatabase& db = m_dbMgr.db(m_document.strKbGUID);
    CString tagsText = m_tagsEdit->text();
    //
    CWizTagDataArray arrayTagNew;
    db.TagsTextToTagArray(tagsText, arrayTagNew);
    //
    CWizTagDataArray arrayTagOld;
    db.GetDocumentTags(m_document.strGUID, arrayTagOld);
    //
    if (::WizKMDataArrayIsEqual(arrayTagOld, arrayTagNew))
    {
        return;
    }
    //
    db.SetDocumentTags(m_document, arrayTagNew);
    //
    setDocument(m_document);    //refresh tags
}


///////////////////////////////////////////////////////////////////////////////
CWizTagCheckBox::CWizTagCheckBox(const WIZTAGDATA& tag, QWidget* parent)
    : QCheckBox(CWizDatabase::TagNameToDisplayName(tag.strName), parent)
    , m_tag(tag)
{

    connect(this, SIGNAL(stateChanged(int)), SLOT(on_checkbox_checked(int)));
}

void CWizTagCheckBox::on_checkbox_checked(int state)
{
    emit tagChecked(this, state);
}
