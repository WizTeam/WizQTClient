#include "wiznoteinfoform.h"
#include "ui_wiznoteinfoform.h"

#include <QFile>

#include "share/wizobject.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "utils/logger.h"
#include "wizmainwindow.h"

CWizNoteInfoForm::CWizNoteInfoForm(QWidget *parent)
    : CWizPopupWidget(parent)
    , ui(new Ui::CWizNoteInfoForm)
{
    ui->setupUi(this);
    setContentsMargins(0, 20, 0, 0);

    ui->editTitle->setReadOnly(true);
    ui->editCreateTime->setReadOnly(true);
    ui->editUpdateTime->setReadOnly(true);
    ui->editURL->setReadOnly(true);
    ui->editAuthor->setReadOnly(true);
    ui->checkEncrypted->setEnabled(false);

    QString openDocument = WizFormatString1("<a href=\"locate\" style=\"color:#3CA2E0;\">%1</a>", tr("Locate"));
    ui->labelOpenDocument->setText(openDocument);
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
    Q_ASSERT(!data.strKbGUID.isEmpty());
    m_docKbGuid = data.strKbGUID;
    m_docGuid = data.strGUID;

    CWizDatabase& db = CWizDatabaseManager::instance()->db(data.strKbGUID);
    QString doc = db.GetDocumentFileName(data.strGUID);
    QString sz = ::WizGetFileSizeHumanReadalbe(doc);

    ui->editTitle->setText(data.strTitle);

    // private document
    if (data.strKbGUID == CWizDatabaseManager::instance()->db().kbGUID()) {
        ui->labelNotebook->setText(data.strLocation);

        QString tags = db.GetDocumentTagsText(data.strGUID);
        ui->labelTags->setText(tags);

        ui->editAuthor->setText(data.strAuthor);

    // group document
    } else {
        CWizTagDataArray arrayTag;
        if (!db.GetDocumentTags(data.strGUID, arrayTag)) {
            ui->labelNotebook->clear();
        } else {
            if (arrayTag.size() > 1) {
                TOLOG1("Group document should only have one tag: %1", data.strTitle);
            }

            QString tagText;
            if (arrayTag.size()) {
                tagText = db.getTagTreeText(arrayTag[0].strGUID);
            }

            ui->labelNotebook->setText("/" + db.name() + tagText + "/");
        }

        ui->labelTags->clear();
        ui->editAuthor->setText(data.strAuthor);
    }

    // common fields
    ui->editCreateTime->setText(data.tCreated.toString());
    ui->editUpdateTime->setText(data.tModified.toString());
    ui->editURL->setText(data.strURL);
    ui->labelOpenURL->setText(WizFormatString2("<a href=\"%1\">%2</a>", data.strURL, tr("Open")));
    ui->labelSize->setText(sz);
    ui->checkEncrypted->setChecked(data.nProtected ? true : false);
}

void CWizNoteInfoForm::on_labelOpenDocument_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    Core::Internal::MainWindow *mainWindow = Core::Internal::MainWindow::instance();
    if (mainWindow)
    {
        mainWindow->locateDocument(m_docKbGuid, m_docGuid);
        hide();
    }
}
