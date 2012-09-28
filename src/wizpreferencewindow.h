#ifndef WIZPREFERENCEWINDOW_H
#define WIZPREFERENCEWINDOW_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>

#include "wiznotesettings.h"
#include "proxydialog.h"

class CWizPreferenceGeneralTab;
class CWizPreferenceReadingTab;
class CWizPreferenceSyncingTab;

class CWizPreferenceWindow: public QDialog
{
     Q_OBJECT

public:
     CWizPreferenceWindow(QWidget* parent);
     CWizPreferenceGeneralTab* generalTab() { return m_generalTab; }
     CWizPreferenceReadingTab* readingTab() { return m_readingTab; }
     CWizPreferenceSyncingTab* syncingTab() { return m_syncingTab; }

private:
     QTabWidget* m_tab;
     QDialogButtonBox* m_buttons;

    CWizPreferenceGeneralTab* m_generalTab;
    CWizPreferenceReadingTab* m_readingTab;
    CWizPreferenceSyncingTab* m_syncingTab;

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);

private slots:
    void on_settingsChanged_emit(WizOptionsType);
};

class CWizPreferenceGeneralTab: public QWidget
{
    Q_OBJECT

public:
    CWizPreferenceGeneralTab(QWidget* parent);

private:
#ifndef Q_OS_MAC
    QLabel* m_labelRestartForSkin;
#endif

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
#ifndef Q_OS_MAC
    void restartForSettings();
#endif

public slots:
#ifndef Q_OS_MAC
    void on_comboSkin_currentIndexChanged(const QString& text);
    void on_labelRestartForSkin_linkActivated(const QString& text);
#endif
};

class CWizPreferenceReadingTab: public QWidget
{
    Q_OBJECT

public:
    CWizPreferenceReadingTab(QWidget* parent);

public slots:
    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
};

class CWizPreferenceSyncingTab: public QWidget
{
    Q_OBJECT

public:
    CWizPreferenceSyncingTab(QWidget* parent);

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);

public slots:
    void on_checkAutoSync_clicked(bool checked);
    void on_checkDownloadAllNotesData_clicked(bool checked);
    void on_labelProxy_linkActivated(const QString & link);
};

#endif // WIZPREFERENCEWINDOW_H
