#ifndef WIZMISC_H
#define WIZMISC_H

#include <stdint.h>

#include <QIcon>
#include <QBuffer>
#include <QByteArray>
#include <QSettings>
#include <functional>

#include "WizObject.h"
#include "WizMd5.h"

class WizDatabase;
class WizDatabaseManager;
class WizProgressDialog;

#define WIZNOTE_OBSOLETE

QString WizGetEmailPrefix(const QString& strMail);

QString WizGetTimeStamp();
WizOleDateTime WizGetCurrentTime();
bool WizStringToDateTime(const QString& str, WizOleDateTime& t, QString& strError);
WizOleDateTime WizStringToDateTime(const CString& str);
COLORREF WizStringToColor(const CString& str);
QColor WizStringToColor2(const CString& str);

std::string WizBSTR2UTF8(const CString& str);

CString WizFormatString0(const CString& str);
CString WizFormatString1(const CString& strFormat, int n);
CString WizFormatString1(const CString& strFormat, const CString& strParam1);
CString WizFormatString2(const CString& strFormat, const CString& strParam1, const CString& strParam2);
CString WizFormatString3(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3);
CString WizFormatString4(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4);
CString WizFormatString5(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5);
CString WizFormatString6(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6);
CString WizFormatString7(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7);
CString WizFormatString8(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7, const CString& strParam8);
CString WizFormatInt(__int64 n);

WizOleDateTime WizIniReadDateTimeDef(const CString& strFile, const CString& strSection, const CString& strKey, WizOleDateTime defaultData = WizOleDateTime());
void WizIniWriteDateTime(const CString& strFile, const CString& strSection, const CString& strKey, WizOleDateTime dateTime);
CString WizIniReadStringDef(const CString& strFile, const CString& strSection, const CString& strKey);
void WizIniWriteString(const CString& strFile, const CString& strSection, const CString& strKey, const CString& strValue);
int WizIniReadIntDef(const CString& strFile, const CString& strSection, const CString& strKey, int defaultValue = 0);
void WizIniWriteInt(const CString& strFile, const CString& strSection, const CString& strKey, int nValue);

time_t WizTimeGetTimeT(const WizOleDateTime& t);

CString WizIntToStr(int n);

BOOL WizSplitTextToArray(const CString& strText, QChar ch, CWizStdStringArray& arrayResult);
BOOL WizSplitTextToArray(CString strText, const CString& strSplitterText, BOOL bMatchCase, CWizStdStringArray& arrayResult);
void WizStringArrayToText(const CWizStdStringArray& arrayText, CString& strText, const CString& strSplitter);
int WizFindInArray(const CWizStdStringArray& arrayText, const CString& strFind);
int WizFindInArrayNoCase(const CWizStdStringArray& arrayText, const CString& strFind);
void WizStringArrayEraseEmptyLine(CWizStdStringArray& arrayText);
void WizStringArrayRemoveMultiElement(CWizStdStringArray& arrayText);
void WizStringArrayRemoveMultiElementNoCase(CWizStdStringArray& arrayText);


QChar getWizSearchSplitChar();

CString WizStringArrayGetValue(const CWizStdStringArray& arrayText, const CString& valueName);
void WizCommandLineToStringArray(const CString& commandLine, CWizStdStringArray& arrayLine);
CString WizGetCommandLineValue(const CString& strCommandLine, const CString& strKey);

BOOL WizStringSimpleSplit(const CString& str, char ch, CString& strLeft, CString& strRight);

CString WizDateToLocalString(const WizOleDateTime& t);

void WizGetTranslatedLocales(QStringList& locales);
bool WizIsChineseLanguage(const QString& local);
QString WizGetDefaultTranslatedLocal();
QString WizGetTranslatedLocaleDisplayName(int index);

bool WizIsPredefinedLocation(const QString& strLocation);
QString WizGetAppFileName();
QString WizLocation2Display(const QString& strLocation);

QString WizGetFileSizeHumanReadalbe(const QString& strFileName);

void WizPathAddBackslash(QString& strPath);
void WizPathRemoveBackslash(CString& strPath);
CString WizPathAddBackslash2(const CString& strPath);
CString WizPathRemoveBackslash2(const CString& strPath);
void WizEnsurePathExists(const CString& strPath);
void WizEnsureFileExists(const QString& strFileName);


#define EF_INCLUDEHIDDEN			0x01
#define EF_INCLUDESUBDIR			0x02

void WizEnumFiles(const QString& strPath, const QString& strExts, CWizStdStringArray& arrayFiles, UINT uFlags);
void WizEnumFolders(const QString& strPath, CWizStdStringArray& arrayFolders, UINT uFlags);
QString WizFolderNameByPath(const QString& strPath);

BOOL WizCopyFile(const CString& strSrcFileName, const CString& strDestFileName, BOOL bFailIfExists);
bool WizCopyFolder(const QString& strSrcDir, const QString& strDestDir, bool bCoverFileIfExist);
void WizGetNextFileName(CString& strFileName);

QString WizEncryptPassword(const QString& strPassword);
QString WizDecryptPassword(const QString& strEncryptedText);


bool WizLoadUnicodeTextFromFile(const QString& strFileName, QString& strText);
bool WizLoadUtf8TextFromFile(const QString& strFileName, QString& strText);
bool WizLoadTextFromResource(const QString& resourceName, QString& text);

bool WizSaveUnicodeTextToUtf16File(const QString& strFileName, const QString& strText);
bool WizSaveUnicodeTextToUtf8File(const QString& strFileName, const QString& strText);
bool WizSaveUnicodeTextToUtf8File(const QString& strFileName, const QByteArray& strText);
bool WizSaveUnicodeTextToUtf8File(const QString& strFileName, const QString& strText, bool addBom);
bool WizSaveUnicodeTextToData(QByteArray& data, const QString& strText, bool addBom);

CString WizDateTimeToIso8601String(const WizOleDateTime& t);
BOOL WizIso8601StringToDateTime(CString str, WizOleDateTime& t, CString& strError);
CString WizDateTimeToString(const WizOleDateTime& t);
CString WizStringToSQL(const CString& str);
CString WizTimeToSQL(const QDateTime &t);
CString WizTimeToSQL(const WizOleDateTime& t);
CString WizColorToString(COLORREF cr);
CString WizColorToString(const QColor& cr);
CString WizColorToSQL(COLORREF cr);
CString WizColorToSQL(const QColor& cr);

intptr_t WizStrStrI_Pos(const CString& str, const CString& find, int nStart = 0);

CString WizInt64ToStr(__int64 n);

CString WizGenGUIDLowerCaseLetterOnly();

BOOL WizBase64Encode(const QByteArray& arrayData, QString& str);
BOOL WizBase64Decode(const QString& str, QByteArray& arrayData);

CString WizStringToBase64(const CString& strSource);
CString WizStringFromBase64(const CString& strBase64);

// skin related
QString WizGetDefaultSkinName();
void WizGetSkins(QStringList& skins);
QString WizGetSkinResourcePath(const QString& strSkinName);
QString WizGetSkinDisplayName(const QString& strSkinName, const QString& strLocale);
QString WizGetSkinResourceFileName(const QString& strSkinName, const QString& strName);

struct WizIconOptions
{
    QColor selectedColor;
    QColor darkColor;
    QColor darkSelectedColor;
    WizIconOptions()
        : selectedColor(Qt::transparent)
        , darkColor(Qt::transparent)
        , darkSelectedColor(Qt::transparent)
    {
    }
    WizIconOptions(QColor selected, QColor dark, QColor darkSelected)
        : selectedColor(selected)
        , darkColor(dark)
        , darkSelectedColor(darkSelected)
    {
    }
};

QIcon WizLoadSkinIcon(const QString& strSkinName, const QString& strIconName, const QSize& iconSize = QSize(16, 16), const WizIconOptions& options = WizIconOptions());

bool WizCreateThumbnailForAttachment(QImage& img, const QString& attachFileName, const QSize& iconSize);

QString WizGetHtmlBodyContent(const QString& strHtml);
bool WizGetBodyContentFromHtml(QString& strHtml, bool bNeedTextParse);
void WizHtml2Text(const QString& strHtml, QString& strText);
QString WizText2Html(const QString& text);
void WizHTMLAppendTextInHead(const QString& strText, QString& strHTML);

void WizDeleteFolder(const CString& strPath);
BOOL WizDeleteAllFilesInFolder(const CString& strPath);

bool WizImage2Html(const QString& strImageFile, QString& strHtml, QString strDestImagePath);

QString WizGetImageHtmlLabelWithLink(const QString& imageFile, const QString& linkHref);
QString WizGetImageHtmlLabelWithLink(const QString& imageFile, const QSize& imgSize, const QString& linkHref);
QString WizStr2Title(const QString& str);

BOOL WizIsValidFileNameNoPath(const CString& strFileName);
void WizMakeValidFileNameNoPath(CString& strFileName);
void WizMakeValidFileNameNoPathLimitLength(CString& strFileName, int nMaxTitleLength);
void WizMakeValidFileNameNoPathLimitFullNameLength(CString& strFileName, int nMaxFullNameLength);
CString WizMakeValidFileNameNoPathReturn(const CString& strFileName);

bool WizSaveDataToFile(const QString& strFileName, const QByteArray& arrayData);
bool WizLoadDataFromFile(const QString& strFileName, QByteArray& arrayData);

//web dialog
void WizShowWebDialogWithToken(const QString& windowTitle, const QString& url,
                               QWidget* parent = 0, const QSize& sz = QSize(WizSmartScaleUI(800), WizSmartScaleUI(480)), bool dialogResizable = false);
void WizShowWebDialogWithTokenDelayed(const QString& windowTitle, const QString& url,
                               QWidget* parent = 0, const QSize& sz = QSize(WizSmartScaleUI(800), WizSmartScaleUI(480)), bool dialogResizable = false);

void WizShowDocumentHistory(const WIZDOCUMENTDATA& doc, QWidget* parent = 0);
void WizShowAttachmentHistory(const WIZDOCUMENTATTACHMENTDATA& attach, QWidget* parent = 0);

bool WizIsOffline();
bool WizIsHighPixel();

bool WizURLDownloadToFile(const QString& url, const QString& fileName, bool isImage);
bool WizURLDownloadToData(const QString& url, QByteArray& data);

bool WizURLDownloadToData(const QString& url, QByteArray& data, QObject* receiver, const char *member);


///  make sure document exist, if not try to download document, show download dialog by default.
bool WizMakeSureDocumentExistAndBlockWidthDialog(WizDatabase& db, const WIZDOCUMENTDATA& doc);
bool WizMakeSureDocumentExistAndBlockWidthEventloop(WizDatabase& db, const WIZDOCUMENTDATA& doc);

bool WizMakeSureAttachmentExistAndBlockWidthEventloop(WizDatabase& db, const WIZDOCUMENTATTACHMENTDATAEX& attachData);
bool WizMakeSureAttachmentExistAndBlockWidthDialog(WizDatabase& db, const WIZDOCUMENTATTACHMENTDATAEX& attachData);

//
void WizMime2Note(const QByteArray& bMime, WizDatabaseManager& dbMgr, CWizDocumentDataArray& arrayDocument);

//
void WizCopyNoteAsInternalLink(const WIZDOCUMENTDATA& document);
void WizCopyNotesAsInternalLink(const QList<WIZDOCUMENTDATA>& documents);
void WizCopyNoteAsWebClientLink(const WIZDOCUMENTDATA& document);
void WizCopyNotesAsWebClientLink(const QList<WIZDOCUMENTDATA>& documents);
QString WizNoteToWizKMURL(const WIZDOCUMENTDATA& document);
void WizNoteToHtmlLink(const WIZDOCUMENTDATA& document, QString& strHtml, QString& strLink);
void WizNotesToHtmlLink(const QList<WIZDOCUMENTDATA>& documents, QString& strHtml, QString &strLink);

bool WizIsNoteContainsFrameset(const WIZDOCUMENTDATA& doc);
bool WizIsMarkdownNote(const WIZDOCUMENTDATA& doc);

enum WizKMUrlType
{
    WizUrl_Invalid,
    WizUrl_Document,
    WizUrl_Attachment
};

bool IsWizKMURL(const QString& strURL);
bool WizIsKMURLOpenDocument(const QString& strURL);
WizKMUrlType GetWizUrlType(const QString& strURL);
QString GetParamFromWizKMURL(const QString& strURL, const QString& strParamName);


struct WizLocalUser {
    QString strGuid;
    QString strDataFolderName;
    QString strUserId;
    int nUserType;
};

bool WizGetLocalUsers(QList<WizLocalUser>& userList);
QString WizGetLocalUserId(const QList<WizLocalUser>& userList, const QString& strGuid);
QString WizGetLocalFolderName(const QList<WizLocalUser>& userList, const QString& strGuid);


class WizBufferAlloc
{
public:
    WizBufferAlloc(int nInitSize = 0);
    ~WizBufferAlloc();
private:
    BYTE* m_pBuffer;
    int m_nSize;
public:
    BYTE* getBuffer();
    BOOL setNewSize(int nNewSize);
};


class WizWaitCursor
{
public:
    WizWaitCursor();
    ~WizWaitCursor();
};


class WizTempFileGuard
{
    QString m_fileName;
public:
    WizTempFileGuard(const QString& fileName);
    WizTempFileGuard();
    ~WizTempFileGuard();
    QString fileName();
};

class WizIniFileEx
{
public:
    WizIniFileEx();
    ~WizIniFileEx();
    void loadFromFile(const QString& strFile);
    void getSection(const QString& section, CWizStdStringArray& arrayData);
    void getSection(const QString& section, QMap<QString, QString>& dataMap);

private:
    QSettings* m_settings;
};

class QThread;
void WizWaitForThread(QThread* pThread);

#endif // WIZMISC_H
