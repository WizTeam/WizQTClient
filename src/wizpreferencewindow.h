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

class CWizPreferenceWindow: public QDialog
{
     Q_OBJECT

public:
     CWizPreferenceWindow(QWidget* parent);

private:
     QTabWidget* m_tab;
     QDialogButtonBox* m_buttons;
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
