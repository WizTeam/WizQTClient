#ifndef WIZPREFERENCEDIALOG_H
#define WIZPREFERENCEDIALOG_H

#include <QDialog>

#include "wizdef.h"
#include "share/wizsettings.h"
#include "share/wizmisc.h"
#include "wizproxydialog.h"

namespace Ui {
    class CWizPreferenceWindow;
}

class CWizPreferenceWindow: public QDialog
{
    Q_OBJECT

public:
    CWizPreferenceWindow(CWizExplorerApp& app, QWidget* parent);
    CWizUserSettings& userSettings() const { return m_app.userSettings(); }

private:
    Ui::CWizPreferenceWindow *ui;
    CWizExplorerApp& m_app;
    bool m_bRestart;

    QStringList m_locales;
    int m_iSelectedLocale;

    QStringList m_skins;
    QString m_strSelectedSkin;

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
    void restartForSettings();

private slots:
    virtual void accept();

    void on_comboLang_currentIndexChanged(int index);
    //void on_comboSkin_currentIndexChanged(int index);

    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);

    void on_comboSyncInterval_activated(int index);
    void on_comboSyncMethod_activated(int index);

    //void on_checkAutoSync_clicked(bool checked);
    //void on_checkDownloadAllNotesData_clicked(bool checked);
    void labelProxy_linkActivated(const QString& link);
};


#endif // WIZPREFERENCEWINDOW_H
