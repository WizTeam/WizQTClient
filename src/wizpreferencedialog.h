#ifndef WIZPREFERENCEDIALOG_H
#define WIZPREFERENCEDIALOG_H

#include "ui_wizpreferencedialog.h"

#include <QDialog>

#include "wizdef.h"
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
    void on_comboSkin_currentIndexChanged(int index);

    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);

    void on_checkAutoSync_clicked(bool checked);
    void on_checkDownloadAllNotesData_clicked(bool checked);
    void labelProxy_linkActivated(const QString& link);

    void on_buttonCheckUpgrade_clicked();
};


#endif // WIZPREFERENCEWINDOW_H
