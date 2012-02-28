#include "wiztaglistwidget.h"
#include "share/wizuihelper.h"

#include "share/wizkmcore.h"

#include <QBoxLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QLineEdit>


CWizTagListWidget::CWizTagListWidget(CWizDatabase& db, QWidget* parent)
    : CWizPopupWidget(parent)
    , m_tagsEdit(NULL)
    , m_scroll(NULL)
    , m_db(db)
{
    QLayout* layoutMain = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(layoutMain);
    layoutMain->setMargin(0);
    //
    QLayout* layoutTitle = new QHBoxLayout(this);
    m_tagsEdit = new QLineEdit(this);
    layoutTitle->addWidget(new QLabel(tr("Tags:"), this));
    layoutTitle->addWidget(m_tagsEdit);
    layoutTitle->setSpacing(8);
    layoutTitle->setMargin(4);
    //
    m_scroll = new QScrollArea(this);
    m_scroll->setFrameStyle(QFrame::NoFrame);
    //
    layoutMain->addItem(layoutTitle);
    layoutMain->addWidget(m_scroll);
    //
    m_scroll->setMaximumHeight(400);
    //
    connect(m_tagsEdit, SIGNAL(editingFinished()), this, SLOT(on_tagsEdit_editingFinished()));
}
//
void CWizTagListWidget::setDocument(const WIZDOCUMENTDATA& data)
{
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
    m_scroll->setWidget(newWidget);
    m_scroll->setWidgetResizable(true);

    m_document = data;
    //
    CWizTagDataArray arrayTag;
    //
    m_db.GetAllTags(arrayTag);
    //
    CString strGUIDs = m_db.GetDocumentTagGUIDsString(m_document.strGUID);
    //
    foreach (const WIZTAGDATA& tag, arrayTag)
    {
        CWizTagCheckBox* checkBox = new CWizTagCheckBox(tag, newWidget);
        layout->addWidget(checkBox);
        if (-1 != strGUIDs.indexOf(tag.strGUID))
        {
            checkBox->setChecked(true);
        }
        //
        connect(checkBox, SIGNAL(tagChecked(CWizTagCheckBox*,int)), this, SLOT(on_tagCheckBox_checked(CWizTagCheckBox*,int)));
    }
    //
    layout->addWidget(new CWizVerSpacer(newWidget));
    //
    updateTagsText();
}

void CWizTagListWidget::updateTagsText()
{
    CString strTagsText = m_db.GetDocumentTagDisplayNameText(m_document.strGUID);
    m_tagsEdit->setText(strTagsText);
}

void CWizTagListWidget::on_tagCheckBox_checked(CWizTagCheckBox* sender, int state)
{
    CWizDocument doc(m_db, m_document);
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
    CString tagsText = m_tagsEdit->text();
    //
    CWizTagDataArray arrayTagNew;
    m_db.TagsTextToTagArray(tagsText, arrayTagNew);
    //
    CWizTagDataArray arrayTagOld;
    m_db.GetDocumentTags(m_document.strGUID, arrayTagOld);
    //
    if (::WizKMDataArrayIsEqual(arrayTagOld, arrayTagNew))
    {
        return;
    }
    //
    m_db.SetDocumentTags(m_document, arrayTagNew);
    //
    setDocument(m_document);    //refresh tags
}


///////////////////////////////////////////////////////////////////////////////
CWizTagCheckBox::CWizTagCheckBox(const WIZTAGDATA& tag, QWidget* parent)
    : QCheckBox(CWizDatabase::TagNameToDisplayName(tag.strName), parent)
    , m_tag(tag)
{

    connect(this, SIGNAL(stateChanged(int)), this, SLOT(on_checkbox_checked(int)));
}

void CWizTagCheckBox::on_checkbox_checked(int state)
{
    emit tagChecked(this, state);
}
