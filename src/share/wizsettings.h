#ifndef WIZSETTINGS_H
#define WIZSETTINGS_H

#include "wizqthelper.h"
#include <QSettings>

class CWizSettings : public QSettings
{
public:
    CWizSettings(const CString& strFileName);
public:
    CString GetString(const CString& strSection, const CString& strKey, const CString& strDef = "");
    BOOL SetString(const CString& strSection, const CString& strKey, const CString& str);

    int GetInt(const CString& strSection, const CString& strKey, int nDef = 0);
    BOOL SetInt(const CString& strSection, const CString& strKey, int val);

    QColor GetColor(const CString& strSection, const CString& strKey, QColor defColor);

    CString GetEncryptedString(const CString& strSection, const CString& strKey, const CString& strDef = "");
    BOOL SetEncryptedString(const CString& strSection, const CString& strKey, const CString& str);
    //
    void GetSections(CWizStdStringArray& arrayAction);
    void GetKeys(const CString& strSection, CWizStdStringArray& arrayAction);
};


CString WizGetString(const CString& strSection, const CString& strKey, const CString& strDef = "");
BOOL WizSetString(const CString& strSection, const CString& strKey, const CString& str);

int WizGetInt(const CString& strSection, const CString& strKey, int nDef = 0);
BOOL WizSetInt(const CString& strSection, const CString& strKey, int val);

BOOL WizGetBool(const CString& strSection, const CString& strKey, bool def = false);
BOOL WizSetBool(const CString& strSection, const CString& strKey, bool val);


CString WizGetEncryptedString(const CString& strSection, const CString& strKey, const CString& strDef = "");
BOOL WizSetEncryptedString(const CString& strSection, const CString& strKey, const CString& str);

CString WizGetShortcut(const CString& strName, const CString& strDef = "");

QColor WizGetSkinColor(const CString& strSection, const CString& strName, const QColor& colorDef);
int WizGetSkinInt(const CString& strSection, const CString& strName, int def);


#endif // WIZSETTINGS_H
