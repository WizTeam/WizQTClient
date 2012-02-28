#include "wiznotesettings.h"

#include "share/wizsettings.h"

WizDocumentViewMode WizGetDefaultNoteView()
{
    return (WizDocumentViewMode)WizGetInt("Common", "NoteView", viewmodeKeep);
}
void WizSetDefaultNoteView(WizDocumentViewMode view)
{
    WizSetInt("Common", "NoteView", view);
}

bool WizIsAutoSync()
{
    return WizGetBool("Common", "AutoSync", true);
}

void WizSetAutoSync(bool b)
{
    WizSetBool("Common", "AutoSync", b);
}


bool WizIsDownloadAllNotesData()
{
    return WizGetBool("Common", "DownloadAllNotesData", false);
}

void WizSetDownloadAllNotesData(bool b)
{
    WizSetBool("Common", "DownloadAllNotesData", b);
}

bool WizGetNotice(QString& strNotice)
{
    strNotice = WizGetString("Common", "Notice", strNotice);
    return !strNotice.isEmpty();
}

void WizSetNotice(const QString& strNotice)
{
    WizSetString("Common", "Notice", strNotice);
}
