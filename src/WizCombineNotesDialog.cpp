#include "WizCombineNotesDialog.h"
#include "ui_WizCombineNotesDialog.h"
#include <QPushButton>
#include <QPointer>
#include <QTimer>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "share/WizMisc.h"
#include "share/WizHtml2Zip.h"
#include "share/WizSettings.h"
#include "utils/WizPathResolve.h"
#include "utils/WizLogger.h"
#include "utils/WizMisc.h"
#include "share/WizMessageBox.h"
#include "core/WizNoteManager.h"

#include "WizMainWindow.h"
#include "WizDocumentListView.h"


WizCombineNotesDialog::WizCombineNotesDialog(WizDatabaseManager& dbMgr, const CWizDocumentDataArray& documents, QWidget *parent /* = 0 */)
    : QDialog(parent)
    , ui(new Ui::WizCombineNotesDialog)
    , m_documents(documents)
    , m_dbMgr(dbMgr)
{
    ui->setupUi(this);
    setFixedSize(size());

    //
    for (auto document : documents) {
        QListWidgetItem* item = new QListWidgetItem(document.strTitle);
        QVariant data;
        data.setValue(new WIZDOCUMENTDATA(document));
        item->setData(Qt::UserRole, data);
        item->setText(document.strTitle);
        //
        ui->listNotes->addItem(item);
    }
    //
    ui->btnMoveUp->setEnabled(false);
    ui->btnMoveDown->setEnabled(false);
    //
    if (WizMainWindow* mainWindow = WizMainWindow::instance())
    {
        WizUserSettings& settings = mainWindow->userSettings();
        QString splitter = settings.get("CombineNote", "splitter");
        if (splitter == "<!--none-->")
        {
            splitter = "";
        }
        else if (splitter.isEmpty())
        {
            splitter = "<hr />";
        }
        //
        ui->editSplitter->setText(splitter);
        ui->btnAddTitle->setChecked(settings.get("CombineNote", "addTitle") == "1");
    }
    //
    ui->editNewTitle->setText(documents[0].strTitle);
    //
    //
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);

    connect(ui->btnPreview, SIGNAL(clicked(bool)), SLOT(preview()));
    connect(ui->listNotes, SIGNAL(currentRowChanged(int)), SLOT(currentRowChanged(int)));
    //
    connect(ui->btnMoveUp, SIGNAL(clicked(bool)), SLOT(moveUp()));
    connect(ui->btnMoveDown, SIGNAL(clicked(bool)), SLOT(moveDown()));
    //
    //
    connect(m_timer, SIGNAL(timeout()), SLOT(checkButtonStatusTimeout()));
    m_timer->start();
}

WizCombineNotesDialog::~WizCombineNotesDialog()
{
    delete ui;
}


void WizCombineNotesDialog::checkButtonStatusTimeout()
{
    currentRowChanged(ui->listNotes->currentRow());
}

void WizCombineNotesDialog::currentRowChanged(int currentRow)
{
    if (currentRow > 0) {
        ui->btnMoveUp->setEnabled(true);
    } else {
        ui->btnMoveUp->setEnabled(false);
    }
    //
    if (currentRow < ui->listNotes->count() - 1)
    {
        ui->btnMoveDown->setEnabled(true);
    } else {
        ui->btnMoveDown->setEnabled(false);
    }
}

bool WizKMCombineDocumentsToHtmlFile(WizDatabaseManager& dbMgr, const CWizDocumentDataArray& arrayDocument, QString splitter, bool addTitle, QString& strResultFileName, CWizDocumentAttachmentDataArray& arrayAttachment, QString& strError);

CWizDocumentDataArray WizCombineNotesDialog::getResultDocuments()
{
    CWizDocumentDataArray ret;
    for (int i = 0; i < ui->listNotes->count(); i++)
    {
        QListWidgetItem* item = ui->listNotes->item(i);
        QVariant data = item->data(Qt::UserRole);
        WIZDOCUMENTDATA* doc = data.value<WIZDOCUMENTDATA *>();
        ret.push_back(*doc);
    }
    return ret;
}

void WizCombineNotesDialog::preview()
{
    QString strResultFileName;
    CWizDocumentAttachmentDataArray arrayAttachment;
    QString strError;
    bool ret = WizKMCombineDocumentsToHtmlFile(m_dbMgr, getResultDocuments(), ui->editSplitter->text(), ui->btnAddTitle->isChecked(), strResultFileName, arrayAttachment, strError);
    if (!ret)
    {
        WizMessageBox::critical(this, tr("Failed to combine note: %1").arg(strError));
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(strResultFileName));
}

void WizCombineNotesDialog::moveUp()
{
    int currIndex = ui->listNotes->currentRow();
    QListWidgetItem *current = ui->listNotes->takeItem(currIndex);

    ui->listNotes->insertItem(currIndex - 1, current);
    //
    ui->listNotes->setCurrentRow(currIndex - 1);
}

void WizCombineNotesDialog::moveDown()
{
    int currIndex = ui->listNotes->currentRow();
    QListWidgetItem *current = ui->listNotes->takeItem(currIndex);

    ui->listNotes->insertItem(currIndex + 1, current);
    //
    ui->listNotes->setCurrentRow(currIndex + 1);
}

void WizCombineNotesDialog::accept()
{
    if (WizMainWindow* mainWindow = WizMainWindow::instance())
    {
        WizUserSettings& settings = mainWindow->userSettings();
        QString splitter = ui->editSplitter->text();
        if (splitter.isEmpty())
        {
            splitter = "<!--none-->";
        }
        settings.set("CombineNote", "splitter", splitter);
        //
        settings.set("CombineNote", "addTitle", ui->btnAddTitle->isChecked() ? "1" : "0");
    }
    //
    QString strResultFileName;
    CWizDocumentAttachmentDataArray arrayAttachment;
    QString strError;
    bool ret = WizKMCombineDocumentsToHtmlFile(m_dbMgr, getResultDocuments(), ui->editSplitter->text(), ui->btnAddTitle->isChecked(), strResultFileName, arrayAttachment, strError);
    if (!ret)
    {
        WizMessageBox::critical(this, tr("Failed to combine note: %1").arg(strError));
        return;
    }
    //
    QString folder = Utils::WizMisc::extractFilePath(strResultFileName);
    QString strZiwFileName = folder + WizGenGUIDLowerCaseLetterOnly() + ".ziw";
    bool bZip = ::WizFolder2Zip(folder, strZiwFileName);
    if (!bZip)
    {
        WizMessageBox::critical(this, tr("Info"), tr("Failled to compress notes data"));
        return;
    }
    //
    WIZDOCUMENTDATA doc = m_documents[0];
    doc.strGUID.clear();
    doc.strTitle = ui->editNewTitle->text();
    //
    WizNoteManager noteManager(m_dbMgr);
    if (!noteManager.createNoteByTemplate(doc, WIZTAGDATA(), strZiwFileName))
    {
        WizMessageBox::critical(this, tr("Info"), tr("Failed to create new data"));
        return;
    }
    //
    for (auto att: arrayAttachment)
    {
        WizDatabase& db = m_dbMgr.db(att.strKbGUID);
        //
        WIZDOCUMENTATTACHMENTDATA attRet;
        if (!db.addAttachment(doc, db.getAttachmentFileName(att.strGUID), attRet))
        {
            WizMessageBox::critical(this, tr("Info"), tr("Failed to add attachment"));
            return;
        }
        //
        attRet.strName = att.strName;
        db.updateAttachment(attRet);
    }

    //
    if (WizMainWindow* mainWindow = WizMainWindow::instance())
    {
        if (WizDocumentListView* list = mainWindow->documentList())
        {
            int index = list->documentIndexFromGUID(doc.strGUID);
            if (index != -1)
            {
                WizDocumentListViewDocumentItem* pItem = list->documentItemAt(index);
                pItem->document().tCreated = pItem->document().tCreated.addSecs(-5);
            } else {
                doc.tCreated = doc.tCreated.addSecs(-5);    //避免自动进入编辑模式
            }
            //
            list->addAndSelectDocument(doc);
        }
    }
    //
    QDialog::accept();
}


ptrdiff_t WizStrRStrI_Pos(const CString& strText, const CString& strFind, ptrdiff_t nEnd = -1)
{
    return strText.lastIndexOf(strFind, nEnd, Qt::CaseInsensitive);
}


QString WizGetHtmlContentHiddenTagBegin()
{
    static const QString lpszBegin = _T("<!--WizHtmlContentBegin-->");
    return lpszBegin;
}

QString WizGetHtmlContentHiddenTagEnd()
{
    static const QString lpszEnd = _T("<!--WizHtmlContentEnd-->");
    return lpszEnd;
}



BOOL WizHtmlGetContentByHiddenTag(const CString& strHtmlText, CString& strContent)
{
    static const QString lpszBegin = WizGetHtmlContentHiddenTagBegin();
    static const QString lpszEnd = WizGetHtmlContentHiddenTagEnd();
    static const int nBeginLen = (int)lpszBegin.length();
    static const int nEndLen = (int)lpszEnd.length();
    //
    int nBegin = strHtmlText.find(lpszBegin);
    if (-1 == nBegin)
        return FALSE;
    //

    int nEnd = (int)WizStrRStrI_Pos(strHtmlText, lpszEnd);
    if (-1 == nEnd)
        return FALSE;
    //
    if (nBegin > nEnd)
        return FALSE;
    //
    strContent = strHtmlText.mid(nBegin + nBeginLen, nEnd - nBegin - nBeginLen);
    //
    return TRUE;
}

BOOL WizHtmlContentAppendHiddenTag(CString& strContent)
{
    strContent = CString(WizGetHtmlContentHiddenTagBegin()) + strContent + WizGetHtmlContentHiddenTagEnd();
    //
    return TRUE;
}

BOOL WizHtmlGetContentByHiddenTagWithHiddenTag(const CString& strHtmlText, CString& strContent)
{
    if (!WizHtmlGetContentByHiddenTag(strHtmlText, strContent))
        return FALSE;
    //
    return WizHtmlContentAppendHiddenTag(strContent);
}



BOOL WizHTMLGetBodyTextWithoutBodyTag(QString lpszText, CString& strRet)
{
    int pBegin = WizStrStrI_Pos(lpszText, _T("<body"));
    if (pBegin != -1)
    {
        pBegin = WizStrStrI_Pos(lpszText, _T(">"), pBegin);
        if (pBegin)
            pBegin++;
    }
    else
    {
        pBegin = WizStrStrI_Pos(lpszText, _T("<html"));
        //
        if (pBegin != -1)
        {
            pBegin = WizStrStrI_Pos(lpszText, _T(">"), pBegin);
            if (pBegin)
                pBegin++;
        }
        else
        {
            pBegin = 0;
        }
    }
    //could not find char '>', _tcschr failed
    if (pBegin == -1)
        pBegin = 0;
    //
    int pEnd = WizStrRStrI_Pos(lpszText, _T("</body"));
    if (pEnd == -1)
        pEnd = WizStrRStrI_Pos(lpszText, _T("</html"));
    if (pEnd == -1)
        pEnd = lpszText.length();
    //
    strRet = lpszText.mid(pBegin, pEnd - pBegin);
    //
    return TRUE;
}

BOOL WizCombineHtmlText(CString& strTextTo, QString lpszTextFrom)
{
    if (strTextTo.isEmpty())
    {
        strTextTo = lpszTextFrom;
        return TRUE;
    }
    //
    /*
    ////强制获得正文////
    */
    int nInsertPos = (int)WizStrRStrI_Pos(strTextTo, WizGetHtmlContentHiddenTagEnd());
    if (-1 == nInsertPos)
        nInsertPos = (int)WizStrRStrI_Pos(strTextTo, _T("</body"));
    if (-1 == nInsertPos)
        nInsertPos = (int)WizStrRStrI_Pos(strTextTo, _T("</html"));
    if (-1 == nInsertPos)
        nInsertPos = (int)strTextTo.length();
    //
    CString strFrom;
    if (!WizHtmlGetContentByHiddenTag(lpszTextFrom, strFrom))
    {
        if (!WizHTMLGetBodyTextWithoutBodyTag(lpszTextFrom, strFrom))
            return FALSE;
    }
    //
    strTextTo.insert(nInsertPos, strFrom);
    //
    return TRUE;
}



BOOL WizHTMLIsCommentedOutCode(UINT nPos, CString strHTML)
{
    if (-1 == nPos)
        return FALSE;
    //
    CString strSubText = strHTML.right(strHTML.length() - nPos);
    int nEndPos = strSubText.find("-->");
    if (-1 == nEndPos)
        return FALSE;
    //
    CString strSubText2 = strSubText.left(nEndPos);
    if (-1 == strSubText2.find("<!--"))
        return TRUE;
    //
    return FALSE;
}

void WizHTMLInsertTextBeforeBody(QString lpszText, CString& strHTML)
{
    BOOL nInserted = FALSE;
    //
    ptrdiff_t nPos = WizStrStrI_Pos(strHTML, _T("<body"));
    //
    while (nPos >= 0 && nPos < strHTML.length())
    {
        if (!WizHTMLIsCommentedOutCode(nPos, strHTML))
            break;
        //
        nPos = WizStrStrI_Pos(strHTML, _T("<body"), nPos + 1);
    }
    //
    if (-1 != nPos)
    {
        nPos = WizStrStrI_Pos(strHTML, _T(">"), nPos);
        if (nPos != -1)
        {
            nPos++;
            //
            strHTML.insert(int(nPos), lpszText);
            //
            nInserted = TRUE;
        }
    }
    //
    if (!nInserted)
    {
        strHTML = CString(lpszText) + strHTML;
    }
}

bool WizKMCombineDocumentsToHtmlFile(WizDatabaseManager& dbMgr, const CWizDocumentDataArray& arrayDocument, QString splitter, bool addTitle, QString& strResultFileName, CWizDocumentAttachmentDataArray& arrayAttachment, QString& strError)
{
    QString strTempPath = Utils::WizPathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + "/";
    ::WizEnsurePathExists(strTempPath);
    //
    CWizStdStringArray htmls;
    //
    size_t nCount = arrayDocument.size();
    for (size_t i = 0; i < nCount; i++)
    {
        WIZDOCUMENTDATA doc = arrayDocument[i];
        //
        WizDatabase& db = dbMgr.db(doc.strKbGUID);
        if (!db.isDocumentDownloaded(doc.strGUID))
        {
            strError = QObject::tr("Note has not been downloaded");
            return false;
        }
        //
        if (!db.documentToHtmlFile(doc, strTempPath))
        {
            strError = QObject::tr("Failed to get note data");
            return false;
        }
        //
        QString htmlFileName = strTempPath + "index.html";
        if (!QFileInfo::exists(htmlFileName))
        {
            strError = QObject::tr("Failed to get note data");
            return false;
        }
        //
        QString html;
        if (!WizLoadUnicodeTextFromFile(htmlFileName, html))
        {
            strError = QObject::tr("Failed to read note data");
            return false;
        }
        //
        htmls.push_back(html);
        //
        CWizDocumentAttachmentDataArray atts;
        db.getDocumentAttachments(doc.strGUID, atts);
        for (WIZDOCUMENTATTACHMENTDATA att : atts)
        {
            if (!db.isAttachmentDownloaded(att.strGUID))
            {
                strError = QObject::tr("Note attachment %1 has not been downloaded").arg(att.strName);
                return false;
            }
            //
            arrayAttachment.push_back(att);
        }
    }
    //
    CString strAllHTML;
    //
    for (size_t i = 0; i < nCount; i++)
    {
        CString html = htmls[i];
        WIZDOCUMENTDATA doc = arrayDocument[i];
        if (-1 != WizStrStrI_Pos(html, CString(_T("<frameset"))))
        {
            strError = QObject::tr("Cannot combine html because html contains frame set");
            return false;
        }
        //
        CString strTitle = doc.strTitle;
        WizOleDateTime t = doc.tCreated;
        CString strDate = t.toLocalLongDate();
        //
        if (addTitle)
        {
            CString strInfoHtml = WizFormatString2(_T("<h2>%1 (%2)</h2>"), strTitle, strDate);
            ::WizHTMLInsertTextBeforeBody(strInfoHtml, html);
        }
        //
        if (!WizCombineHtmlText(strAllHTML, html))
        {
            strError = QObject::tr("Cannot combine html");
            return false;
        }
        //
        if (i < nCount - 1)
        {
            if (!splitter.isEmpty())
            {
                WizCombineHtmlText(strAllHTML, splitter);
            }
        }
    }
    //
    strResultFileName = strTempPath + "index.html";
    //
    bool ret = ::WizSaveUnicodeTextToUtf8File(strResultFileName, strAllHTML);
    if (!ret)
    {
        strError = QObject::tr("Cannot save note html");
        return false;
    }
    return true;
}


