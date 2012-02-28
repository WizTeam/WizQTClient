#ifndef WIZOPTIONSWIDGET_H
#define WIZOPTIONSWIDGET_H

#include "share/wizpopupwidget.h"
#include "wiznotesettings.h"


class CWizOptionsWidget : public CWizPopupWidget
{
    Q_OBJECT
public:
    CWizOptionsWidget(QWidget* parent);
    //
public slots:
    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);
    void on_checkAutoSync_clicked(bool checked);
    void on_checkDownloadAllNotesData_clicked(bool checked);
Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
};

#endif // WIZOPTIONSWIDGET_H
