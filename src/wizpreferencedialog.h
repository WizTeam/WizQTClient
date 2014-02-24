#ifndef WIZPREFERENCEDIALOG_H
#define WIZPREFERENCEDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QFontDialog>

#include "wizdef.h"
#include "share/wizsettings.h"
#include "share/wizmisc.h"
//#include "wizproxydialog.h"

namespace Ui {
    class CWizPreferenceWindow;
}

class CWizDatabaseManager;

class CWizPreferenceWindow: public QDialog
{
    Q_OBJECT

public:
    CWizPreferenceWindow(CWizExplorerApp& app, QWidget* parent);
    CWizUserSettings& userSettings() const { return m_app.userSettings(); }

private:
    Ui::CWizPreferenceWindow *ui;
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;

    QStringList m_locales;
    QStringList m_skins;
    QPointer<QFontDialog> m_fontDialog;

    void setSyncGroupTimeLine(int nDays);

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
    void restartForSettings();

public Q_SLOTS:
    void on_comboLang_currentIndexChanged(int index);

    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);

    void on_comboSyncInterval_activated(int index);
    void on_comboSyncMethod_activated(int index);
    void on_comboSyncGroupMethod_activated(int index);

    void labelProxy_linkActivated(const QString& link);

    void onButtonFontSelect_clicked();
    void onButtonFontSelect_confirmed();

};


#endif // WIZPREFERENCEWINDOW_H
