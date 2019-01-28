#include "WizNoteInfoForm.h"
#include "ui_WizNoteInfoForm.h"

#include <QFile>

#include "share/WizObject.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "utils/WizLogger.h"
#include "WizMainWindow.h"
#include "share/WizAnalyzer.h"
#include "share/WizMessageBox.h"

QString formatLabelLink(const QString& linkHref, const QString& text)
{
    return WizFormatString2("<a href=\"%1\" style=\"color:#448aff;"
                    "text-decoration:none;\">%2</a>", linkHref, text);
}

WizNoteInfoForm::WizNoteInfoForm(QWidget *parent)
    : WizPopupWidget(parent)
    , ui(new Ui::WizNoteInfoForm)
    , m_size(QSize(370, 370))
{
    //
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
    //
    if (isDarkMode()) {

        if (isDarkMode()) {
    #ifdef Q_OS_MAC
            setStyleSheet("background-color:#272727; border-radius:4px;");
    #else
            setStyleSheet("background-color:#444444; border-radius:4px;");
    #endif
            ui->editURL->setStyleSheet("background-color:#333333");
            ui->editAuthor->setStyleSheet("background-color:#333333");
        }



    } else {
        setStyleSheet("background-color:#FFFFFF; border-radius:4px;");
    }

    if (isDarkMode()) {
        WizApplyDarkModeStyles(this);
    }
}

WizNoteInfoForm::~WizNoteInfoForm()
{
    delete ui;
}

QSize WizNoteInfoForm::sizeHint() const
{
    return m_size;
}

void WizNoteInfoForm::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

void WizNoteInfoForm::setDocument(const WIZDOCUMENTDATA& data)
{
    Q_ASSERT(!data.strKbGUID.isEmpty());
    m_docKbGuid = data.strKbGUID;
    m_docGuid = data.strGUID;

    WizDatabase& db = WizDatabaseManager::instance()->db(data.strKbGUID);
    m_size.setHeight(db.isGroup() ? 320 : 370);
    setGroupLabelVisible(db.isGroup());
    QString doc = db.getDocumentFileName(data.strGUID);
    QString sz = ::WizGetFileSizeHumanReadalbe(doc);
    m_sizeText = sz;

    QFont font;
    QFontMetrics fm(font);
    const int nMaxTextWidth = 280;
    QString strLocation = db.getDocumentLocation(data);
    // private document
    if (data.strKbGUID == WizDatabaseManager::instance()->db().kbGUID()) {
        QString tags = db.getDocumentTagsText(data.strGUID);
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

    QString userAlias = db.getDocumentOwnerAlias(data);
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

    bool canEdit = (db.canEditDocument(data) && !WizDatabase::isInDeletedItems(data.strLocation));
    ui->editAuthor->setReadOnly(!canEdit);
    ui->editURL->setReadOnly(!canEdit);
    ui->checkEncrypted->setEnabled(canEdit && !db.isGroup());
}

void WizNoteInfoForm::setWordCount(int nWords, int nChars, int nCharsWithSpace, int nNonAsianWords, int nAsianChars)
{
    QString textFormat = tr("Words: %1\nCharacters (no spaces): %2\nCharacters (with spaces): %3\nNon-Asianwords: %4\nAsian characters: %5");
    //
    QString text = textFormat.arg(nWords).arg(nChars).arg(nCharsWithSpace).arg(nNonAsianWords).arg(nAsianChars);
    //
    ui->labelSize->setText(m_sizeText + "\n" + text);
}


void WizNoteInfoForm::setGroupLabelVisible(bool isGroupNote)
{
    ui->labelEncrypted->setVisible(!isGroupNote);
    ui->checkEncrypted->setVisible(!isGroupNote);
    ui->labelTags->setVisible(!isGroupNote);
    ui->labelTagsLabel->setVisible(!isGroupNote);

    ui->labelOwner->setVisible(isGroupNote);
    ui->labelOwnerLabel->setVisible(isGroupNote);
}

void WizNoteInfoForm::on_labelOpenDocument_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    WizMainWindow *mainWindow = WizMainWindow::instance();
    if (mainWindow)
    {
        mainWindow->locateDocument(m_docKbGuid, m_docGuid);
        hide();
    }
}

void WizNoteInfoForm::on_editURL_editingFinished()
{
    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        if (doc.strURL != ui->editURL->text())
        {
            doc.strURL= ui->editURL->text();
            db.modifyDocumentInfo(doc);
        }
    }
}

void WizNoteInfoForm::on_editAuthor_editingFinished()
{
    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        if (doc.strAuthor != ui->editAuthor->text())
        {
            doc.strAuthor = ui->editAuthor->text();
            db.modifyDocumentInfo(doc);
        }
    }
}

void WizNoteInfoForm::on_checkEncrypted_clicked(bool checked)
{
    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        if (checked)
        {
            db.encryptDocument(doc);
        }
        else
        {
            if (doc.nProtected)
            {
                if (!db.cancelDocumentEncryption(doc))
                    return;
            }
        }
    }
}

void WizNoteInfoForm::on_labelHistory_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        WizShowDocumentHistory(doc, nullptr);
        WizGetAnalyzer().logAction("showVersionHistory");
    }
}

void WizNoteInfoForm::on_labelOpenURL_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    QUrl url(link);
    if (url.isValid())
    {
        QDesktopServices::openUrl(url.toString());
    }
    else
    {
        WizMessageBox::information(nullptr, tr("Info"), tr("Url invalid, can not open!"));
    }
}
