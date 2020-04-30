#ifndef WIZPREFERENCEDIALOG_H
#define WIZPREFERENCEDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QFontDialog>

#include "WizDef.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"
//#include "WizProxyDialog.h"

namespace Ui {
    class WizPreferenceWindow;
}

class WizDatabaseManager;

class WizPreferenceWindow: public QDialog
{
    Q_OBJECT

public:
    WizPreferenceWindow(WizExplorerApp& app, QWidget* parent);
    WizUserSettings& userSettings() const { return m_app.userSettings(); }

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
    //
    void on_enableSpellCheck(bool checked);

private slots:
    void on_checkBox_stateChanged(int arg1);
    void on_checkBoxTrayIcon_toggled(bool checked);
#ifndef Q_OS_MAC
    void on_checkBoxDarkMode_clicked(bool checked);
#endif
    void on_comboBox_unit_currentIndexChanged(int index);
    void on_spinBox_top_valueChanged(double arg1);
    void on_spinBox_bottom_valueChanged(double arg1);
    void on_spinBox_left_valueChanged(double arg1);
    void on_spinBox_right_valueChanged(double arg1);
    void on_checkBoxSystemStyle_toggled(bool checked);
    void on_pushButtonBackgroundColor_clicked();
    void on_pushButtonClearBackground_clicked();

    void on_checkBoxManuallySort_toggled(bool checked);

    void on_tabWidget_currentChanged(int index);

    void on_comboLineHeight_currentIndexChanged(int index);
    void on_btnResetLineHeight_clicked();

    void on_comboParaSpacing_currentIndexChanged(int index);
    void on_btnResetParaSpacing_clicked();

private:
    Ui::WizPreferenceWindow *ui;
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;

    QStringList m_locales;
    QStringList m_skins;
    QPointer<QFontDialog> m_fontDialog;
    bool m_biniting;

    void setSyncGroupTimeLine(int nDays);
    void updateEditorBackgroundColor(const QString& strColorName, bool save);
    void updateEditorLineHeight(const QString& strLineHeight, bool save);
    void updateEditorParaSpacing(const QString& spacing, bool save);
};


#endif // WIZPREFERENCEWINDOW_H
