#ifndef WIZPREFERENCEWINDOW_H
#define WIZPREFERENCEWINDOW_H

#include "ui_preferencedialog.h"

#include <QDialog>

#include "wiznotesettings.h"
#include "proxydialog.h"

namespace Ui {
    class CWizPreferenceWindow;
}

class CWizPreferenceWindow: public QDialog
{
     Q_OBJECT

public:
     CWizPreferenceWindow(QWidget* parent);

private:
    Ui::CWizPreferenceWindow *ui;
    bool m_bRestart;

#ifndef Q_OS_MAC
    QLabel* m_labelRestartForSkin;
#endif

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
    void restartForSettings();

private slots:
    virtual void accept();

    void on_comboSkin_currentIndexChanged(const QString& text);
    //void on_labelRestartForSkin_linkActivated(const QString& text);

    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);

    void on_checkAutoSync_clicked(bool checked);
    void on_checkDownloadAllNotesData_clicked(bool checked);
    void on_labelProxy_linkActivated(const QString & link);
};


#endif // WIZPREFERENCEWINDOW_H
