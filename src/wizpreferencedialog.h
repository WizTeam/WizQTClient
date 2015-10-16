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

    void showPrintMarginPage();

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
    void on_comboDownloadAttachments_activated(int index);

    void labelProxy_linkActivated(const QString& link);

    void onButtonFontSelect_clicked();
    void onButtonFontSelect_confirmed();

private slots:
    void on_checkBox_stateChanged(int arg1);
    void on_checkBoxTrayIcon_toggled(bool checked);
    void on_comboBox_unit_currentIndexChanged(int index);
    void on_spinBox_top_valueChanged(double arg1);
    void on_spinBox_bottom_valueChanged(double arg1);
    void on_spinBox_left_valueChanged(double arg1);
    void on_spinBox_right_valueChanged(double arg1);
    void on_checkBoxSystemStyle_toggled(bool checked);
    void on_checkBoxSearchEncryNote_toggled(bool checked);
    void on_lineEditNotePassword_editingFinished();
    void on_pushButtonBackgroundColor_clicked();
    void on_pushButtonClearBackground_clicked();

    void on_checkBoxManuallySort_toggled(bool checked);

    void on_pushButtonChoseMarkdwonTemplate_clicked();



    void on_tabWidget_currentChanged(int index);

private:
    Ui::CWizPreferenceWindow *ui;
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;

    QStringList m_locales;
    QStringList m_skins;
    QPointer<QFontDialog> m_fontDialog;

    void setSyncGroupTimeLine(int nDays);
    void updateEditorBackgroundColor(const QString& strColorName);
};


#endif // WIZPREFERENCEWINDOW_H
