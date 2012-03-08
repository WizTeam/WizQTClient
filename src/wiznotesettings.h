#ifndef WIZNOTESETTINGS_H
#define WIZNOTESETTINGS_H

#include <QString>

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

bool WizGetNotice(QString& strNotice);
void WizSetNotice(const QString& strNotice);


#endif // WIZNOTESETTINGS_H
