#include "wiznoteinfoform.h"
#include "ui_wiznoteinfoform.h"

#include <QFile>

CWizNoteInfoForm::CWizNoteInfoForm(CWizDatabase& db, QWidget *parent)
    : CWizPopupWidget(parent)
    , m_db(db)
    , ui(new Ui::CWizNoteInfoForm)
{
    ui->setupUi(this);
    setContentsMargins(8, 20, 8, 8);
    setAutoFillBackground(true);

    ui->editTitle->setReadOnly(true);
    ui->editCreateTime->setReadOnly(true);
    ui->editUpdateTime->setReadOnly(true);
    ui->editURL->setReadOnly(true);
    ui->editAuthor->setReadOnly(true);
    ui->checkEncrypted->setEnabled(false);
}

CWizNoteInfoForm::~CWizNoteInfoForm()
{
    delete ui;
}

QSize CWizNoteInfoForm::sizeHint() const
{
    return QSize(420, 350);
}

void CWizNoteInfoForm::setDocument(const WIZDOCUMENTDATA& data)
{
    QString doc = m_db.GetDocumentFileName(data.strGUID);
    QString sz = ::WizGetFileSizeHumanReadalbe(doc);

    CWizTagDataArray arrayTag;
    m_db.GetDocumentTags(data.strGUID, arrayTag);

    QString tags;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        tags += it->strName;
        tags += "; ";
    }
    tags = tags.remove(tags.size() - 2, 2);

    ui->editTitle->setText(data.strTitle);
    ui->labelNotebook->setText(data.strLocation);
    ui->labelTags->setText(tags);
    ui->editCreateTime->setText(data.tCreated.toString());
    ui->editUpdateTime->setText(data.tModified.toString());
    ui->editURL->setText(data.strURL);
    ui->labelOpenURL->setText(WizFormatString2("<a href=\"%1\">%2</a>", data.strURL, tr("Open")));
    ui->labelSize->setText(sz);
    ui->editAuthor->setText(data.strAuthor);
    ui->checkEncrypted->setChecked(data.nProtected ? true : false);
}
