#include "WizCommonUI.h"

#include <QClipboard>
#include <QApplication>

#include "WizMisc.h"
#include "utils/WizPathResolve.h"


WizCommonUI::WizCommonUI(QObject* parent)
    : QObject(parent)
{
}

QString WizCommonUI::loadTextFromFile(const QString& strFileName)
{
    QString strText;
    ::WizLoadUnicodeTextFromFile(strFileName, strText);
    return strText;
}

QString WizCommonUI::clipboardToImage(int hwnd, const QString& strOptions)
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
    if (strTempPath.isEmpty())
    {
        strTempPath = Utils::WizPathResolve::tempPath();
    }
    else
    {
        ::WizPathAddBackslash(strTempPath);
        ::WizEnsurePathExists(strTempPath);
    }
    //
    CString strFileName = strTempPath + WizIntToStr(WizGetTickCount()) + ".png";
    if (!image.save(strFileName))
        return CString();
    //
    return strFileName;
}
