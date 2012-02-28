#include "wizoptionswidget.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include "wiznotesettings.h"



CWizOptionsWidget::CWizOptionsWidget(QWidget* parent)
    : CWizPopupWidget(parent)
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
    QVBoxLayout* groupBoxSyncLayout = new QVBoxLayout(groupBoxSync);
    groupBoxSync->setLayout(groupBoxSyncLayout);
    groupBoxSyncLayout->addWidget(checkAutoSync);
    groupBoxSyncLayout->addWidget(checkDownloadAllNotesData);
    //
    ////////////////////////////////////////
    //main
    layoutMain->addWidget(groupBoxNoteView);
    layoutMain->addWidget(groupBoxSync);
    layoutMain->addStretch(1);
    //
    //events
    connect(radioAuto, SIGNAL(clicked(bool)), this, SLOT(on_radioAuto_clicked(bool)));
    connect(radioAlwaysReading, SIGNAL(clicked(bool)), this, SLOT(on_radioAlwaysReading_clicked(bool)));
    connect(radioAlwaysEditing, SIGNAL(clicked(bool)), this, SLOT(on_radioAlwaysEditing_clicked(bool)));
    //
    connect(checkAutoSync, SIGNAL(clicked(bool)), this, SLOT(on_checkAutoSync_clicked(bool)));
    connect(checkDownloadAllNotesData, SIGNAL(clicked(bool)), this, SLOT(on_checkDownloadAllNotesData_clicked(bool)));

}

void CWizOptionsWidget::on_radioAuto_clicked(bool chcked)
{
    if (chcked)
    {
        WizSetDefaultNoteView(viewmodeKeep);
        emit settingsChanged(wizoptionsNoteView);
    }
}


void CWizOptionsWidget::on_radioAlwaysReading_clicked(bool chcked)
{
    if (chcked)
    {
        WizSetDefaultNoteView(viewmodeAlwaysReading);
        emit settingsChanged(wizoptionsNoteView);
    }
}


void CWizOptionsWidget::on_radioAlwaysEditing_clicked(bool chcked)
{
    if (chcked)
    {
        WizSetDefaultNoteView(viewmodeAlwaysEditing);
        emit settingsChanged(wizoptionsNoteView);
    }
}
void CWizOptionsWidget::on_checkAutoSync_clicked(bool checked)
{
    ::WizSetAutoSync(checked);
    emit settingsChanged(wizoptionsSync);
}
void CWizOptionsWidget::on_checkDownloadAllNotesData_clicked(bool checked)
{
    ::WizSetDownloadAllNotesData(checked);
    emit settingsChanged(wizoptionsSync);
}

