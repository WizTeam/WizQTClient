#include "wiznotesettings.h"

#include "share/wizsettings.h"

WizDocumentViewMode WizGetDefaultNoteView()
{
    CWizSettings settings(WizGetSettingsFileName());
    return (WizDocumentViewMode)(settings.GetInt("Common", "NoteView", viewmodeKeep));
}

void WizSetDefaultNoteView(WizDocumentViewMode view)
{
    CWizSettings settings(WizGetSettingsFileName());
    settings.SetInt("Common", "NoteView", view);
}

bool WizIsAutoSync()
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.GetBool("Common", "AutoSync", true);
}

void WizSetAutoSync(bool b)
{
    CWizSettings settings(WizGetSettingsFileName());
    settings.SetBool("Common", "AutoSync", b);
}


bool WizIsDownloadAllNotesData()
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.GetBool("Common", "DownloadAllNotesData", false);
}

void WizSetDownloadAllNotesData(bool b)
{
    CWizSettings settings(WizGetSettingsFileName());
    settings.SetBool("Common", "DownloadAllNotesData", b);
}

bool WizGetNotice(QString& strNotice)
{
    CWizSettings settings(WizGetSettingsFileName());
    strNotice = settings.GetString("Common", "Notice", strNotice);
    return !strNotice.isEmpty();
}

void WizSetNotice(const QString& strNotice)
{
    CWizSettings settings(WizGetSettingsFileName());
    settings.SetString("Common", "Notice", strNotice);
}
