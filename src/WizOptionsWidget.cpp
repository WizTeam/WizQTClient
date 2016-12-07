#include "WizOptionsWidget.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

#include "wiznotesettings.h"
#include "share/WizMisc.h"

#include "proxydialog.h"


WizOptionsWidget::WizOptionsWidget(QWidget* parent)
    : WizPopupWidget(parent)
#ifndef Q_OS_MAC
    , m_labelRestartForSkin(NULL)
#endif
{
    QBoxLayout* layoutMain = new QBoxLayout(QBoxLayout::TopToBottom, this);
    //
    setLayout(layoutMain);
    //
    ////////////////////////////////////////////
    //note view
    QGroupBox *groupBoxNoteView = new QGroupBox(tr("Default note view"), this);
    //
    QRadioButton *radioAuto = new QRadioButton(tr("Auto (Keep view state)"), groupBoxNoteView);
    QRadioButton *radioAlwaysReading = new QRadioButton(tr("Always open note in Reading-View"), groupBoxNoteView);
    QRadioButton *radioAlwaysEditing = new QRadioButton(tr("Always open note in Editing-View"), groupBoxNoteView);

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
    groupBoxNoteViewLayout->addStretch(1);
    groupBoxNoteView->setLayout(groupBoxNoteViewLayout);
    //
    ////////////////////////////////////////
    //sync
    QGroupBox *groupBoxSync = new QGroupBox(tr("Sync"), this);
    //
    QCheckBox* checkAutoSync = new QCheckBox(tr("Auto sync"), this);
    QCheckBox* checkDownloadAllNotesData = new QCheckBox(tr("Download all notes data"), this);
    checkAutoSync->setChecked(WizIsAutoSync());
    checkDownloadAllNotesData->setChecked(WizIsDownloadAllNotesData());
    QString proxySettings = WizFormatString1("<a href=\"proxy_settings\">%1</a>", tr("Proxy settings"));
    QLabel* labelProxySettings =   new QLabel(proxySettings, this);
    labelProxySettings->setAlignment(Qt::AlignRight);
    QVBoxLayout* groupBoxSyncLayout = new QVBoxLayout(groupBoxSync);
    groupBoxSync->setLayout(groupBoxSyncLayout);
    groupBoxSyncLayout->addWidget(checkAutoSync);
    groupBoxSyncLayout->addWidget(checkDownloadAllNotesData);
    groupBoxSyncLayout->addWidget(labelProxySettings);
    //
#ifndef Q_OS_MAC
    ////////////////////////////////////////
    //sync
    QGroupBox *groupBoxSkin = new QGroupBox(tr("Skin"), this);
    //
    QComboBox* comboSkin = new QComboBox(groupBoxSkin);
    m_labelRestartForSkin = new QLabel(groupBoxSkin);
    QVBoxLayout* groupBoxSkinLayout = new QVBoxLayout(groupBoxSkin);
    groupBoxSkin->setLayout(groupBoxSkinLayout);
    groupBoxSkinLayout->addWidget(comboSkin);
    groupBoxSkinLayout->addWidget(m_labelRestartForSkin);
    //
    QString strApplySkinText = tr("Restart to apply the new skin");
    CString strApplySkinLabelText = QString("<a href=\"restart\">%1</a>").arg(strApplySkinText);
    m_labelRestartForSkin->setText(strApplySkinLabelText);
    m_labelRestartForSkin->setVisible(false);
    //
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
    //
#endif

    ////////////////////////////////////////
    //main
    layoutMain->addWidget(groupBoxNoteView);
    layoutMain->addWidget(groupBoxSync);
#ifndef Q_OS_MAC
    layoutMain->addWidget(groupBoxSkin);
#endif
    layoutMain->addStretch(1);
    //
    //events
    connect(radioAuto, SIGNAL(clicked(bool)), this, SLOT(on_radioAuto_clicked(bool)));
    connect(radioAlwaysReading, SIGNAL(clicked(bool)), this, SLOT(on_radioAlwaysReading_clicked(bool)));
    connect(radioAlwaysEditing, SIGNAL(clicked(bool)), this, SLOT(on_radioAlwaysEditing_clicked(bool)));
    //
    connect(checkAutoSync, SIGNAL(clicked(bool)), this, SLOT(on_checkAutoSync_clicked(bool)));
    connect(checkDownloadAllNotesData, SIGNAL(clicked(bool)), this, SLOT(on_checkDownloadAllNotesData_clicked(bool)));
    connect(labelProxySettings, SIGNAL(linkActivated(QString)), this, SLOT(on_labelProxy_linkActivated(QString)));

    //
#ifndef Q_OS_MAC
    connect(comboSkin, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_comboSkin_currentIndexChanged(QString)));
    connect(m_labelRestartForSkin, SIGNAL(linkActivated(QString)), this, SLOT(on_labelRestartForSkin_linkActivated(QString)));
#endif

}

void WizOptionsWidget::on_radioAuto_clicked(bool chcked)
{
    if (chcked)
    {
        WizSetDefaultNoteView(viewmodeKeep);
        emit settingsChanged(wizoptionsNoteView);
    }
}


void WizOptionsWidget::on_radioAlwaysReading_clicked(bool chcked)
{
    if (chcked)
    {
        WizSetDefaultNoteView(viewmodeAlwaysReading);
        emit settingsChanged(wizoptionsNoteView);
    }
}


void WizOptionsWidget::on_radioAlwaysEditing_clicked(bool chcked)
{
    if (chcked)
    {
        WizSetDefaultNoteView(viewmodeAlwaysEditing);
        emit settingsChanged(wizoptionsNoteView);
    }
}
void WizOptionsWidget::on_checkAutoSync_clicked(bool checked)
{
    ::WizSetAutoSync(checked);
    emit settingsChanged(wizoptionsSync);
}
void WizOptionsWidget::on_checkDownloadAllNotesData_clicked(bool checked)
{
    ::WizSetDownloadAllNotesData(checked);
    emit settingsChanged(wizoptionsSync);
}


void WizOptionsWidget::on_labelProxy_linkActivated(const QString & link)
{
    Q_UNUSED(link);
    //
    WizProxyDialog dlg(parentWidget());
    if (QDialog::Accepted != dlg.exec())
        return;
    //
    emit settingsChanged(wizoptionsSync);
}


#ifndef Q_OS_MAC

void WizOptionsWidget::on_comboSkin_currentIndexChanged(const QString& text)
{
    WizSetSkinDisplayName(text);
    m_labelRestartForSkin->setVisible(true);
    emit settingsChanged(wizoptionsSkin);
}
void WizOptionsWidget::on_labelRestartForSkin_linkActivated(const QString& text)
{
    Q_UNUSED(text);
    close();
    emit restartForSettings();
}


#endif
