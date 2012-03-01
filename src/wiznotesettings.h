#ifndef WIZNOTESETTINGS_H
#define WIZNOTESETTINGS_H

enum WizDocumentViewMode
{
    viewmodeAlwaysEditing,
    viewmodeAlwaysReading,
    viewmodeKeep
};

WizDocumentViewMode WizGetDefaultNoteView();
void WizSetDefaultNoteView(WizDocumentViewMode view);


enum WizOptionsType
{
    wizoptionsNoteView,
    wizoptionsSync,
    wizoptionsSkin
};

bool WizIsAutoSync();
void WizSetAutoSync(bool b);

bool WizIsDownloadAllNotesData();
void WizSetDownloadAllNotesData(bool b);

class QString;
bool WizGetNotice(QString& strNotice);
void WizSetNotice(const QString& strNotice);

#endif // WIZNOTESETTINGS_H
