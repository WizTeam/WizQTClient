﻿#include "WizPreferenceDialog.h"
#include "ui_WizPreferenceDialog.h"

#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QTimer>

#include "share/WizGlobal.h"
#include "utils/WizPathResolve.h"
#include "share/WizMessageBox.h"
#include "share/WizDatabaseManager.h"
#include "share/WizThreads.h"
#include "share/WizRequest.h"

#include "WizMainWindow.h"
#include "WizProxyDialog.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"

#include "widgets/WizExecutingActionDialog.h"


WizPreferenceWindow::WizPreferenceWindow(WizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WizPreferenceWindow)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
{
    ui->setupUi(this);
    setWindowIcon(QIcon());
    setWindowTitle(tr("Preference"));

    connect(ui->btnClose, SIGNAL(clicked()), SLOT(accept()));

    // general tab
    ::WizGetTranslatedLocales(m_locales);
    ui->comboLang->blockSignals(true);
    for (int i = 0; i < m_locales.count(); i++) {
        ui->comboLang->addItem(::WizGetTranslatedLocaleDisplayName(i));
    }

    for (int i = 0; i < ui->comboLang->count(); i++) {
        if (m_locales[i] == userSettings().locale()) {
            ui->comboLang->setCurrentIndex(i);
        }
    }
    ui->comboLang->blockSignals(false);

    ui->checkBox->blockSignals(true);
    Qt::CheckState checkState = userSettings().autoCheckUpdate() ? Qt::Checked : Qt::Unchecked;
    ui->checkBox->setCheckState(checkState);
    ui->checkBox->blockSignals(false);

    ui->checkBoxTrayIcon->blockSignals(true);
    checkState = userSettings().showSystemTrayIcon() ? Qt::Checked : Qt::Unchecked;
    ui->checkBoxTrayIcon->setCheckState(checkState);
    ui->checkBoxTrayIcon->blockSignals(false);

#ifdef BUILD4APPSTORE
    // hide language choice and upgrade for appstore
    ui->comboLang->setEnabled(false);
    ui->checkBox->setVisible(false);
#endif

#ifndef Q_OS_LINUX
  ui->checkBoxSystemStyle->setVisible(false);
#endif
  checkState = userSettings().useSystemBasedStyle() ? Qt::Checked : Qt::Unchecked;
  ui->checkBoxSystemStyle->blockSignals(true);
  ui->checkBoxSystemStyle->setCheckState(checkState);
  ui->checkBoxSystemStyle->blockSignals(false);

    // reading tab
    switch (userSettings().noteViewMode())
    {
        case viewmodeAlwaysEditing:
            ui->radioAlwaysEditing->setChecked(true);
            break;
        case viewmodeAlwaysReading:
            ui->radioAlwaysReading->setChecked(true);
            break;
        default:
            ui->radioAuto->setChecked(true);
            break;
    }
    //
    ui->spellCheck->setChecked(userSettings().isEnableSpellCheck());
    connect(ui->spellCheck, SIGNAL(toggled(bool)), this, SLOT(on_enableSpellCheck(bool)));

    // syncing tab
    int nInterval = userSettings().syncInterval();
    switch (nInterval) {
        case 5:
            ui->comboSyncInterval->setCurrentIndex(0);
            break;
        case 15:
            ui->comboSyncInterval->setCurrentIndex(1);
            break;
        case 30:
            ui->comboSyncInterval->setCurrentIndex(2);
            break;
        case 60:
            ui->comboSyncInterval->setCurrentIndex(3);
            break;
        case -1:
            ui->comboSyncInterval->setCurrentIndex(4);
            break;
        default:
            ui->comboSyncInterval->setCurrentIndex(1);
    }

    switch (m_dbMgr.db().getObjectSyncTimeline()) {
        case -1:
            ui->comboSyncMethod->setCurrentIndex(0);
            break;
        case 1:
            ui->comboSyncMethod->setCurrentIndex(1);
            break;
        case 7:
            ui->comboSyncMethod->setCurrentIndex(2);
            break;
        case 30:
            ui->comboSyncMethod->setCurrentIndex(3);
            break;
        case 99999:
            ui->comboSyncMethod->setCurrentIndex(4);
            break;
        default:
            ui->comboSyncMethod->setCurrentIndex(4);
    }

    int nDays = 1;
    if (m_dbMgr.count()) {
        nDays = m_dbMgr.at(0).getObjectSyncTimeline();
    }

    switch (nDays) {
        case -1:
            ui->comboSyncGroupMethod->setCurrentIndex(0);
            break;
        case 1:
            ui->comboSyncGroupMethod->setCurrentIndex(1);
            break;
        case 7:
            ui->comboSyncGroupMethod->setCurrentIndex(2);
            break;
        case 30:
            ui->comboSyncGroupMethod->setCurrentIndex(3);
            break;
        case 99999:
            ui->comboSyncGroupMethod->setCurrentIndex(4);
            break;
        default:
            ui->comboSyncGroupMethod->setCurrentIndex(1);
    }

    bool downloadAttachments = m_dbMgr.db().getDownloadAttachmentsAtSync();
    ui->comboDownloadAttachments->setCurrentIndex(downloadAttachments ? 1 : 0);

    connect(ui->comboSyncInterval, SIGNAL(activated(int)), SLOT(on_comboSyncInterval_activated(int)));
    connect(ui->comboSyncMethod, SIGNAL(activated(int)), SLOT(on_comboSyncMethod_activated(int)));
    connect(ui->comboSyncGroupMethod, SIGNAL(activated(int)), SLOT(on_comboSyncGroupMethod_activated(int)));
    connect(ui->comboDownloadAttachments, SIGNAL(activated(int)), SLOT(on_comboDownloadAttachments_activated(int)));

    QString proxySettings = WizFormatString1("<a href=\"proxy_settings\" style=\"color:#3CA2E0;\">%1</a>", tr("Proxy settings"));
    ui->labelProxySettings->setText(proxySettings);
    connect(ui->labelProxySettings, SIGNAL(linkActivated(const QString&)),
            SLOT(labelProxy_linkActivated(const QString&)));

    // format tab
    QString strFont = QString("%1  %2").
            arg(m_app.userSettings().defaultFontFamily())
            .arg(m_app.userSettings().defaultFontSize());
    ui->editFont->setText(strFont);

    connect(ui->buttonFontSelect, SIGNAL(clicked()), SLOT(onButtonFontSelect_clicked()));

    //
    ui->comboBox_unit->setCurrentIndex(m_app.userSettings().printMarginUnit());
    ui->spinBox_bottom->setValue(m_app.userSettings().printMarginValue(wizPositionBottom));
    ui->spinBox_left->setValue(m_app.userSettings().printMarginValue(wizPositionLeft));
    ui->spinBox_right->setValue(m_app.userSettings().printMarginValue(wizPositionRight));
    ui->spinBox_top->setValue(m_app.userSettings().printMarginValue(wizPositionTop));

    QString strColor = m_app.userSettings().editorBackgroundColor();
    updateEditorBackgroundColor(strColor);

    bool manuallySortFolders = m_app.userSettings().isManualSortingEnabled();
    ui->checkBoxManuallySort->setChecked(manuallySortFolders);
    //
    connect(ui->useNewSync, SIGNAL(clicked(bool)), SLOT(useNewSyncClicked(bool)));
    ui->useNewSync->setChecked(WizToken::userInfo().syncType == 1);
    if (ui->useNewSync->isChecked())
    {
        ui->useNewSync->setEnabled(false);
    }
}

void WizPreferenceWindow::showPrintMarginPage()
{
    ui->tabWidget->setCurrentWidget(ui->tabPrint);
}

void WizPreferenceWindow::on_radioAuto_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeKeep);
    Q_EMIT settingsChanged(wizoptionsNoteView);
}

void WizPreferenceWindow::on_radioAlwaysReading_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeAlwaysReading);
    Q_EMIT settingsChanged(wizoptionsNoteView);
}

void WizPreferenceWindow::on_radioAlwaysEditing_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeAlwaysEditing);
    Q_EMIT settingsChanged(wizoptionsNoteView);
}

void WizPreferenceWindow::on_comboSyncInterval_activated(int index)
{
    switch (index) {
        case 0:
            userSettings().setSyncInterval(5);
            break;
        case 1:
            userSettings().setSyncInterval(15);
            break;
        case 2:
            userSettings().setSyncInterval(30);
            break;
        case 3:
            userSettings().setSyncInterval(60);
            break;
        case 4:
            userSettings().setSyncInterval(-1);
            break;
        default:
            Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::on_comboSyncMethod_activated(int index)
{
    switch (index) {
        case 0:
            m_dbMgr.db().setObjectSyncTimeLine(-1);
            break;
        case 1:
            m_dbMgr.db().setObjectSyncTimeLine(1);
            break;
        case 2:
            m_dbMgr.db().setObjectSyncTimeLine(7);
            break;
        case 3:
            m_dbMgr.db().setObjectSyncTimeLine(30);
            break;
        case 4:
            m_dbMgr.db().setObjectSyncTimeLine(99999);
            break;
        default:
            Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::on_comboSyncGroupMethod_activated(int index)
{
    switch (index) {
    case 0:
        setSyncGroupTimeLine(-1);
        break;
    case 1:
        setSyncGroupTimeLine(1);
        break;
    case 2:
        setSyncGroupTimeLine(7);
        break;
    case 3:
        setSyncGroupTimeLine(30);
        break;
    case 4:
        setSyncGroupTimeLine(99999);
        break;
    default:
        Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::setSyncGroupTimeLine(int nDays)
{
    for (int i = 0; i < m_dbMgr.count(); i++) {
        m_dbMgr.at(i).setObjectSyncTimeLine(nDays);
    }
}

void WizPreferenceWindow::labelProxy_linkActivated(const QString& link)
{
    Q_UNUSED(link);

    WizProxyDialog dlg(this);
    if (QDialog::Accepted != dlg.exec()) {
        Q_EMIT settingsChanged(wizoptionsSync);
    }
}

void WizPreferenceWindow::onButtonFontSelect_clicked()
{
    if (!m_fontDialog) {
        m_fontDialog = new QFontDialog(this);

        // FIXME: Qt bugs here https://bugreports.qt-project.org/browse/QTBUG-27415
        // upgrade Qt library to 5.0 should fix this issue
        m_fontDialog->setOptions(QFontDialog::DontUseNativeDialog);
    }

    QString strFont = m_app.userSettings().defaultFontFamily();
    int nSize = m_app.userSettings().defaultFontSize();
    QFont font(strFont, nSize);
    m_fontDialog->setCurrentFont(font);

    connect(m_fontDialog,SIGNAL(accepted()),this,SLOT(onButtonFontSelect_confirmed()));
    m_fontDialog->exec();
}

void WizPreferenceWindow::onButtonFontSelect_confirmed()
{
    QFont font = m_fontDialog->currentFont();
    QString str = font.family() + " " + QString::number(font.pointSize());
    ui->editFont->setText(str);

    m_app.userSettings().setDefaultFontFamily(font.family());
    m_app.userSettings().setDefaultFontSize(font.pointSize());

    Q_EMIT settingsChanged(wizoptionsFont);
}

void WizPreferenceWindow::on_comboLang_currentIndexChanged(int index)
{
    QString strLocaleName = m_locales[index];
    if (strLocaleName.compare(userSettings().locale())) {
        userSettings().setLocale(strLocaleName);

        WizMessageBox::information(this, tr("Info"), tr("Language will be changed after restart WizNote."));
    }
}

void WizPreferenceWindow::on_checkBox_stateChanged(int arg1)
{
    bool autoUpdate = (arg1 == Qt::Checked);
    m_app.userSettings().setAutoCheckUpdate(autoUpdate);

    if (autoUpdate) {
        WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
        mainWindow->checkWizUpdate();
    }
}

void WizPreferenceWindow::on_checkBoxTrayIcon_toggled(bool checked)
{
    m_app.userSettings().setShowSystemTrayIcon(checked);
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->setSystemTrayIconVisible(checked);
}

void WizPreferenceWindow::on_comboBox_unit_currentIndexChanged(int index)
{
    m_app.userSettings().setPrintMarginUnit(index);
}

void WizPreferenceWindow::on_spinBox_top_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionTop, arg1);
}

void WizPreferenceWindow::on_spinBox_bottom_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionBottom, arg1);
}

void WizPreferenceWindow::on_spinBox_left_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionLeft, arg1);
}

void WizPreferenceWindow::on_spinBox_right_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionRight, arg1);
}


void WizPreferenceWindow::on_checkBoxSystemStyle_toggled(bool checked)
{
    m_app.userSettings().setUseSystemBasedStyle(checked);

    WizMessageBox::information(m_app.mainWindow(), tr("Info"), tr("Application style will be changed after restart WizNote."));
}


void WizPreferenceWindow::on_pushButtonBackgroundColor_clicked()
{
    QColorDialog dlg;
    dlg.setCurrentColor(m_app.userSettings().editorBackgroundColor());
    if (dlg.exec() == QDialog::Accepted)
    {
        QString strColor = dlg.currentColor().name();
        updateEditorBackgroundColor(strColor);
    }
}

void WizPreferenceWindow::on_pushButtonClearBackground_clicked()
{
    updateEditorBackgroundColor("");
}

void WizPreferenceWindow::updateEditorBackgroundColor(const QString& strColorName)
{
    m_app.userSettings().setEditorBackgroundColor(strColorName);
    ui->pushButtonBackgroundColor->setStyleSheet(QString("QPushButton "
                                                             "{ border: 1px; background: %1; height:20px;} ").arg(strColorName));
    ui->pushButtonBackgroundColor->setText(strColorName.isEmpty() ? tr("Click to select color") : QString());
    ui->pushButtonClearBackground->setVisible(!strColorName.isEmpty());

    Q_EMIT settingsChanged(wizoptionsFont);
}

void WizPreferenceWindow::on_checkBoxManuallySort_toggled(bool checked)
{
    m_app.userSettings().setManualSortingEnable(checked);
    emit settingsChanged(wizoptionsFolders);
}

void WizPreferenceWindow::on_comboDownloadAttachments_activated(int index)
{
    switch (index) {
    case 0:
        m_dbMgr.db().setDownloadAttachmentsAtSync(false);
        break;
    case 1:
        m_dbMgr.db().setDownloadAttachmentsAtSync(true);
        break;
    default:
        Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::on_tabWidget_currentChanged(int index)
{
//    if (index == 1)
//    {
//        setFixedHeight(350);
//        resize(width(), 350);
//    }
//    else
//    {
//        setFixedHeight(290);
//        resize(width(), 290);
//    }
}

void WizPreferenceWindow::useNewSyncClicked(bool checked)
{
    if (checked)
    {
        QString message = tr("Before checking, you need to know:\n\nDon't support returning old sync mode;\nWhen used, other client must be updated to the latest version, or can't log in.\n\nDo you want to use?");
        if (QMessageBox::Yes != WizMessageBox::question(this, message))
        {
            ui->useNewSync->setChecked(false);
            return;
        }

        //
        WizExecutingActionDialog::executeAction(tr("Change user settings..."), WIZ_THREAD_NETWORK, [=] {
            //
            bool ret = false;
            QString errorMessage = tr("Network error");
            //
            QString asUrl = WizCommonApiEntry::newAsServerUrl();
            if (!asUrl.isEmpty())
            {
                QString url = asUrl + "/as/user/settings/sync_type/1?token=" + WizToken::token();
                //
                WIZSTANDARDRESULT result = WizRequest::execStandardJsonRequest(url, "PUT");
                if (result)  {
                    ret = true;
                } else {
                    errorMessage = result.returnMessage;
                }
            }
            //
            ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
                //
                QTimer::singleShot(300, [=]{
                    if (ret) {
                        ui->useNewSync->setEnabled(false);
                        WizMessageBox::information(this, tr("Succeeded to change user settings, please restart WizNote."));
                    } else {
                        WizMessageBox::warning(this, tr("Failed to change user settings:\n\n %1").arg(errorMessage));
                        ui->useNewSync->setChecked(false);
                    }
                });
                //

            });
            //

        });
    }

}

void WizPreferenceWindow::on_enableSpellCheck(bool checked)
{
    userSettings().setEnableSpellCheck(checked);
    Q_EMIT settingsChanged(wizoptionsSpellCheck);
}
