#include "wizpreferencewindow.h"

#include "wiznotesettings.h"
#include "share/wizmisc.h"

CWizPreferenceWindow::CWizPreferenceWindow(QWidget* parent)
    : QDialog(parent)
{
    m_tab = new QTabWidget(this);
    m_tab->addTab(new CWizPreferenceGeneralTab(this), tr("General"));
    m_tab->addTab(new CWizPreferenceReadingTab(this), tr("Reading"));
    m_tab->addTab(new CWizPreferenceSyncingTab(this), tr("Syncing"));

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tab);
    mainLayout->addWidget(m_buttons);
    setLayout(mainLayout);

    setWindowTitle(tr("Preference"));
    setFixedSize(400, 600);
}

CWizPreferenceGeneralTab::CWizPreferenceGeneralTab(QWidget* parent)
    : QWidget(parent)
{
    QGroupBox* groupBoxLang = new QGroupBox(tr("Languages"), this);
    QComboBox* comboBoxLang = new QComboBox(groupBoxLang);
    comboBoxLang->addItem(tr("Chinese"));
    comboBoxLang->addItem(tr("English"));

    QVBoxLayout* groupBoxLangLayout = new QVBoxLayout(groupBoxLang);
    groupBoxLangLayout->addWidget(comboBoxLang);
    groupBoxLangLayout->addSpacing(30);
    groupBoxLang->setLayout(groupBoxLangLayout);

#ifndef Q_WS_MAC
    QGroupBox *groupBoxSkin = new QGroupBox(tr("Skin"), this);

    QComboBox* comboSkin = new QComboBox(groupBoxSkin);
    m_labelRestartForSkin = new QLabel(groupBoxSkin);

    QVBoxLayout* groupBoxSkinLayout = new QVBoxLayout(groupBoxSkin);
    groupBoxSkinLayout->addWidget(comboSkin);
    groupBoxSkinLayout->addWidget(m_labelRestartForSkin);
    groupBoxSkin->setLayout(groupBoxSkinLayout);

    QString strApplySkinText = tr("Restart to apply the new skin");
    QString strApplySkinLabelText = QString("<a href=\"restart\">%1</a>").arg(strApplySkinText);
    m_labelRestartForSkin->setText(strApplySkinLabelText);
    m_labelRestartForSkin->setVisible(false);

    CString strCurrSkinName = ::WizGetSkinName();
    std::map<CString, CString> skins;
    typedef std::map<CString, CString>::value_type SKIN;
    ::WizGetSkins(skins);
    foreach (const SKIN& skin, skins)
    {
        comboSkin->addItem(skin.second);
        if (strCurrSkinName == skin.first)
        {
            comboSkin->setCurrentIndex(comboSkin->count() - 1);
        }
    }
#endif

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    setLayout(layoutMain);

    layoutMain->addWidget(groupBoxLang);
#ifndef Q_WS_MAC
    layoutMain->addWidget(groupBoxSkin);
#endif
    layoutMain->addStretch();

    // callbacks
#ifndef Q_OS_MAC
    connect(comboSkin, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_comboSkin_currentIndexChanged(QString)));
    connect(m_labelRestartForSkin, SIGNAL(linkActivated(QString)), this, SLOT(on_labelRestartForSkin_linkActivated(QString)));
#endif
}


#ifndef Q_OS_MAC
void CWizPreferenceGeneralTab::on_comboSkin_currentIndexChanged(const QString& text)
{
    WizSetSkinDisplayName(text);
    m_labelRestartForSkin->setVisible(true);
    emit settingsChanged(wizoptionsSkin);
}

void CWizPreferenceGeneralTab::on_labelRestartForSkin_linkActivated(const QString& text)
{
    Q_UNUSED(text);
    close();
    emit restartForSettings();
}
#endif

CWizPreferenceReadingTab::CWizPreferenceReadingTab(QWidget* parent)
    : QWidget(parent)
{
    QGroupBox* groupBoxNoteView = new QGroupBox(tr("Default note view"), this);

    QRadioButton* radioAuto = new QRadioButton(tr("Auto (Keep view state)"), groupBoxNoteView);
    QRadioButton* radioAlwaysReading = new QRadioButton(tr("Always open note in Reading-View"), groupBoxNoteView);
    QRadioButton* radioAlwaysEditing = new QRadioButton(tr("Always open note in Editing-View"), groupBoxNoteView);

    switch (WizGetDefaultNoteView())
    {
    case viewmodeAlwaysEditing:
        radioAlwaysEditing->setChecked(true);
        break;
    case viewmodeAlwaysReading:
        radioAlwaysReading->setChecked(true);
        break;
    default:
        radioAuto->setChecked(true);
        break;
    }

    QVBoxLayout* groupBoxNoteViewLayout = new QVBoxLayout(groupBoxNoteView);
    groupBoxNoteViewLayout->addWidget(radioAuto);
    groupBoxNoteViewLayout->addWidget(radioAlwaysEditing);
    groupBoxNoteViewLayout->addWidget(radioAlwaysReading);
    groupBoxNoteView->setLayout(groupBoxNoteViewLayout);

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    setLayout(layoutMain);

    layoutMain->addWidget(groupBoxNoteView);
    layoutMain->addStretch();


}

CWizPreferenceSyncingTab::CWizPreferenceSyncingTab(QWidget* parent)
    : QWidget(parent)
{
    QGroupBox* groupBoxSync = new QGroupBox(tr("Sync"), this);

    QCheckBox* checkAutoSync = new QCheckBox(tr("Auto sync"), this);
    QCheckBox* checkDownloadAllNotesData = new QCheckBox(tr("Download all notes data"), this);

    checkAutoSync->setChecked(WizIsAutoSync());
    checkDownloadAllNotesData->setChecked(WizIsDownloadAllNotesData());

    QString proxySettings = WizFormatString1("<a href=\"proxy_settings\">%1</a>", tr("Proxy settings"));
    QLabel* labelProxySettings =   new QLabel(proxySettings, this);
    labelProxySettings->setAlignment(Qt::AlignRight);

    QVBoxLayout* groupBoxSyncLayout = new QVBoxLayout(groupBoxSync);
    groupBoxSyncLayout->addWidget(checkAutoSync);
    groupBoxSyncLayout->addWidget(checkDownloadAllNotesData);
    groupBoxSyncLayout->addWidget(labelProxySettings);
    groupBoxSync->setLayout(groupBoxSyncLayout);

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    setLayout(layoutMain);

    layoutMain->addWidget(groupBoxSync);
    layoutMain->addStretch();

    // callbacks
    connect(checkAutoSync, SIGNAL(clicked(bool)), this, SLOT(on_checkAutoSync_clicked(bool)));
    connect(checkDownloadAllNotesData, SIGNAL(clicked(bool)), this, SLOT(on_checkDownloadAllNotesData_clicked(bool)));
    connect(labelProxySettings, SIGNAL(linkActivated(QString)), this, SLOT(on_labelProxy_linkActivated(QString)));
}

void CWizPreferenceSyncingTab::on_checkAutoSync_clicked(bool checked)
{
    ::WizSetAutoSync(checked);
    emit settingsChanged(wizoptionsSync);
}
void CWizPreferenceSyncingTab::on_checkDownloadAllNotesData_clicked(bool checked)
{
    ::WizSetDownloadAllNotesData(checked);
    emit settingsChanged(wizoptionsSync);
}

void CWizPreferenceSyncingTab::on_labelProxy_linkActivated(const QString & link)
{
    Q_UNUSED(link);
    //
    ProxyDialog dlg(parentWidget());
    if (QDialog::Accepted != dlg.exec())
        return;
    //
    emit settingsChanged(wizoptionsSync);
}
