#include "wizcommonui.h"
#include "wizmisc.h"

#include <QClipboard>
#include <QApplication>


CWizCommonUI::CWizCommonUI(QObject* parent)
    : QObject(parent)
{
}

QString CWizCommonUI::LoadTextFromFile(const QString& strFileName)
{
    CString strText;
    ::WizLoadUnicodeTextFromFile(strFileName, strText);
    return strText;
}

QString CWizCommonUI::ClipboardToImage(int hwnd, const QString& strOptions)
{
    Q_UNUSED(hwnd);
    //
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard)
        return CString();
    //
    //
    QImage image = clipboard->image();
    if (image.isNull())
        return CString();
    //
    CString strTempPath = ::WizGetCommandLineValue(strOptions, "TempPath");
    if (strTempPath.IsEmpty())
    {
        strTempPath = WizGlobal()->GetTempPath();
    }
    else
    {
        ::WizPathAddBackslash(strTempPath);
        ::WizEnsurePathExists(strTempPath);
    }
    //
    CString strFileName = strTempPath + WizIntToStr(GetTickCount()) + ".png";
    if (!image.save(strFileName))
        return CString();
    //
    return strFileName;
}
