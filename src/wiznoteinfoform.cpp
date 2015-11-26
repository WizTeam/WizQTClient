#include "wiznoteinfoform.h"
#include "ui_wiznoteinfoform.h"

#include <QFile>

#include "share/wizobject.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "utils/logger.h"
#include "wizmainwindow.h"
#include "wizLineInputDialog.h"
#include "share/wizAnalyzer.h"
#include "share/wizMessageBox.h"

QString formatLabelLink(const QString& linkHref, const QString& text)
{
    return WizFormatString2("<a href=\"%1\" style=\"color:#5990EF;"
                    "text-decoration:none;\">%2</a>", linkHref, text);
}

CWizNoteInfoForm::CWizNoteInfoForm(QWidget *parent)
    : CWizPopupWidget(parent)
    , ui(new Ui::CWizNoteInfoForm)
    , m_size(QSize(370, 370))
{
    ui->setupUi(this);
    setContentsMargins(0, 8, 0, 0);

    ui->editCreateTime->setReadOnly(true);
    ui->editUpdateTime->setReadOnly(true);
    ui->editAccessTime->setReadOnly(true);
//    ui->editURL->setReadOnly(true);
//    ui->editAuthor->setReadOnly(true);
//    ui->checkEncrypted->setEnabled(false);

    QString openDocument = formatLabelLink("locate", tr("Locate"));
    ui->labelOpenDocument->setText(openDocument);
    QString versionHistory = formatLabelLink("history", tr("Click to view version history"));
    ui->labelHistory->setText(versionHistory);
}

CWizNoteInfoForm::~CWizNoteInfoForm()
{
    delete ui;
}

QSize CWizNoteInfoForm::sizeHint() const
{
    return m_size;
}

void CWizNoteInfoForm::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

void CWizNoteInfoForm::setDocument(const WIZDOCUMENTDATA& data)
{
    Q_ASSERT(!data.strKbGUID.isEmpty());
    m_docKbGuid = data.strKbGUID;
    m_docGuid = data.strGUID;

    CWizDatabase& db = CWizDatabaseManager::instance()->db(data.strKbGUID);
    m_size.setHeight(db.IsGroup() ? 320 : 370);
    setGroupLabelVisible(db.IsGroup());
    QString doc = db.GetDocumentFileName(data.strGUID);
    QString sz = ::WizGetFileSizeHumanReadalbe(doc);

    QFont font;
    QFontMetrics fm(font);
    const int nMaxTextWidth = 280;
    QString strLocation = db.GetDocumentLocation(data);
    // private document
    if (data.strKbGUID == CWizDatabaseManager::instance()->db().kbGUID()) {
        QString tags = db.GetDocumentTagsText(data.strGUID);
        tags = fm.elidedText(tags, Qt::ElideMiddle, nMaxTextWidth);
        ui->labelTags->setText(tags);

        ui->editAuthor->setText(data.strAuthor);

    // group document
    } else {        
        ui->labelTags->clear();
        ui->editAuthor->setText(data.strAuthor);
    }

    strLocation = fm.elidedText(strLocation, Qt::ElideMiddle, nMaxTextWidth);
    ui->labelNotebook->setText(strLocation);

    QString userAlias = db.GetDocumentOwnerAlias(data);
    ui->labelOwner->setText(userAlias);

    // common fields
    ui->editCreateTime->setText(data.tCreated.toString());
    ui->editUpdateTime->setText(data.tDataModified.toString());
    ui->editAccessTime->setText(data.tAccessed.toString());
    ui->editURL->setText(data.strURL);
    QString text = data.strURL.isEmpty() ? "" : formatLabelLink(data.strURL, tr("Open"));
    ui->labelOpenURL->setText(text);
    ui->labelSize->setText(sz);
    ui->checkEncrypted->setChecked(data.nProtected ? true : false);

    bool canEdit = (db.CanEditDocument(data) && !CWizDatabase::IsInDeletedItems(data.strLocation));
    ui->editAuthor->setReadOnly(!canEdit);
    ui->editURL->setReadOnly(!canEdit);
    ui->checkEncrypted->setEnabled(canEdit && !db.IsGroup());
}

void CWizNoteInfoForm::setGroupLabelVisible(bool isGroupNote)
{
    ui->labelEncrypted->setVisible(!isGroupNote);
    ui->checkEncrypted->setVisible(!isGroupNote);
    ui->labelTags->setVisible(!isGroupNote);
    ui->labelTagsLabel->setVisible(!isGroupNote);

    ui->labelOwner->setVisible(isGroupNote);
    ui->labelOwnerLabel->setVisible(isGroupNote);
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

void CWizNoteInfoForm::on_editURL_editingFinished()
{
    WIZDOCUMENTDATA doc;
    CWizDatabase& db = CWizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.DocumentFromGUID(m_docGuid, doc))
    {
        if (doc.strURL != ui->editURL->text())
        {
            doc.strURL= ui->editURL->text();
            db.ModifyDocumentInfo(doc);
        }
    }
}

void CWizNoteInfoForm::on_editAuthor_editingFinished()
{
    WIZDOCUMENTDATA doc;
    CWizDatabase& db = CWizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.DocumentFromGUID(m_docGuid, doc))
    {
        if (doc.strAuthor != ui->editAuthor->text())
        {
            doc.strAuthor = ui->editAuthor->text();
            db.ModifyDocumentInfo(doc);
        }
    }
}

void CWizNoteInfoForm::on_checkEncrypted_clicked(bool checked)
{
    WIZDOCUMENTDATA doc;
    CWizDatabase& db = CWizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.DocumentFromGUID(m_docGuid, doc))
    {
        if (checked)
        {
            db.EncryptDocument(doc);
        }
        else
        {
            QString strUserCipher;
            CWizLineInputDialog dlg(tr("Password"), tr("Please input document password to cancel encrypt."),
                                    "", 0, QLineEdit::Password);
            if (dlg.exec() == QDialog::Rejected)
                return;

            strUserCipher = dlg.input();
            if (strUserCipher.isEmpty())
                return;

            if (doc.nProtected)
            {
                if (!db.CancelDocumentEncryption(doc, strUserCipher))
                    return;
            }
        }
    }
}

void CWizNoteInfoForm::on_labelHistory_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    WIZDOCUMENTDATA doc;
    CWizDatabase& db = CWizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.DocumentFromGUID(m_docGuid, doc))
    {
        WizShowDocumentHistory(doc, nullptr);
        WizGetAnalyzer().LogAction("showVersionHistory");
    }
}

void CWizNoteInfoForm::on_labelOpenURL_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    QUrl url(link);
    if (url.isValid())
    {
        QDesktopServices::openUrl(url.toString());
    }
    else
    {
        CWizMessageBox::information(nullptr, tr("Info"), tr("Url invalid, can not open!"));
    }
}
