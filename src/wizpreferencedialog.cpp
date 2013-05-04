#include "wizpreferencedialog.h"
#include "ui_wizpreferencedialog.h"

#include <QFontDialog>
#include <QFontDatabase>
#include <QDebug>

CWizPreferenceWindow::CWizPreferenceWindow(CWizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CWizPreferenceWindow)
    , m_app(app)
    , m_bRestart(false)
{
    ui->setupUi(this);
    setWindowIcon(QIcon());

    connect(ui->buttonOK, SIGNAL(clicked()), SLOT(accept()));

    // general tab
    ::WizGetTranslatedLocales(m_locales);
    for (int i = 0; i < m_locales.count(); i++) {
        ui->comboLang->addItem(::WizGetTranslatedLocaleDisplayName(i));
    }

    for (int i = 0; i < ui->comboLang->count(); i++) {
        if (!m_locales[i].compare(userSettings().locale())) {
            ui->comboLang->setCurrentIndex(i);
        }
    }

    // just hide skin setup and upgrade notfiy on mac for convenience.
//#ifdef Q_WS_MAC
    //ui->groupBoxSkin->hide();
    //ui->checkBox->hide();
//#endif // Q_WS_MAC

//    ::WizGetSkins(m_skins);
//    for (int i = 0; i < m_skins.count(); i++) {
//        ui->comboSkin->addItem(::WizGetSkinDisplayName(m_skins[i], userSettings().locale()));
//    }

//    QString strCurSkinName = userSettings().skin();
//    for (int i = 0; i < ui->comboSkin->count(); i++) {
//        if (!strCurSkinName.compare(m_skins[i])) {
//            ui->comboSkin->setCurrentIndex(i);
//        }
//    }

    connect(ui->comboLang, SIGNAL(currentIndexChanged(int)), SLOT(on_comboLang_currentIndexChanged(int)));
    //connect(ui->comboSkin, SIGNAL(currentIndexChanged(int)), SLOT(on_comboSkin_currentIndexChanged(int)));

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

    connect(ui->radioAuto, SIGNAL(clicked(bool)), SLOT(on_radioAuto_clicked(bool)));
    connect(ui->radioAlwaysReading, SIGNAL(clicked(bool)), SLOT(on_radioAlwaysReading_clicked(bool)));
    connect(ui->radioAlwaysEditing, SIGNAL(clicked(bool)), SLOT(on_radioAlwaysEditing_clicked(bool)));

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

    int nMethod = userSettings().syncMethod();
    switch (nMethod) {
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
            ui->comboSyncMethod->setCurrentIndex(2);
    }

    connect(ui->comboSyncInterval, SIGNAL(activated(int)), SLOT(on_comboSyncInterval_activated(int)));
    connect(ui->comboSyncMethod, SIGNAL(activated(int)), SLOT(on_comboSyncMethod_activated(int)));

    QString proxySettings = WizFormatString1("<a href=\"proxy_settings\">%1</a>", tr("Proxy settings"));
    ui->labelProxySettings->setText(proxySettings);
    connect(ui->labelProxySettings, SIGNAL(linkActivated(const QString&)), SLOT(labelProxy_linkActivated(const QString&)));

    // format tab
    QString strFont = QString("%1  %2").
            arg(m_app.userSettings().defaultFontFamily())
            .arg(m_app.userSettings().defaultFontSize());
    ui->editFont->setText(strFont);

    connect(ui->buttonFontSelect, SIGNAL(clicked()), SLOT(onButtonFontSelect_clicked()));
}

void CWizPreferenceWindow::on_comboLang_currentIndexChanged(int index)
{
    m_iSelectedLocale = index;
}

//void CWizPreferenceWindow::on_comboSkin_currentIndexChanged(int index)
//{
//    m_strSelectedSkin = m_skins[index];
//}

void CWizPreferenceWindow::on_radioAuto_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeKeep);
    emit settingsChanged(wizoptionsNoteView);
}

void CWizPreferenceWindow::on_radioAlwaysReading_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeAlwaysReading);
    emit settingsChanged(wizoptionsNoteView);
}

void CWizPreferenceWindow::on_radioAlwaysEditing_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeAlwaysEditing);
    emit settingsChanged(wizoptionsNoteView);
}

void CWizPreferenceWindow::on_comboSyncInterval_activated(int index)
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

    emit settingsChanged(wizoptionsSync);
}

void CWizPreferenceWindow::on_comboSyncMethod_activated(int index)
{
    switch (index) {
        case 0:
            userSettings().setSyncMethod(-1);
            break;
        case 1:
            userSettings().setSyncMethod(1);
            break;
        case 2:
            userSettings().setSyncMethod(7);
            break;
        case 3:
            userSettings().setSyncMethod(30);
            break;
        case 4:
            userSettings().setSyncMethod(99999);
            break;
        default:
            Q_ASSERT(0);
    }

    emit settingsChanged(wizoptionsSync);
}

void CWizPreferenceWindow::labelProxy_linkActivated(const QString& link)
{
    Q_UNUSED(link);

    ProxyDialog dlg(this);
    if (QDialog::Accepted != dlg.exec()) {
        emit settingsChanged(wizoptionsSync);
    }
}

void CWizPreferenceWindow::onButtonFontSelect_clicked()
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

    m_fontDialog->open(this, SLOT(onButtonFontSelect_confirmed()));
}

void CWizPreferenceWindow::onButtonFontSelect_confirmed()
{
    QFont font = m_fontDialog->currentFont();
    QString str = font.family() + " " + QString::number(font.pointSize());
    ui->editFont->setText(str);

    m_app.userSettings().setDefaultFontFamily(font.family());
    m_app.userSettings().setDefaultFontSize(font.pointSize());
}

void CWizPreferenceWindow::accept()
{
//    if (m_strSelectedSkin.compare(userSettings().skin())) {
//        userSettings().setSkin(m_strSelectedSkin);
//        m_bRestart = true;
//    }

    QString strLocaleName = m_locales[m_iSelectedLocale];
    if (strLocaleName.compare(userSettings().locale())) {
        userSettings().setLocale(strLocaleName);
        m_bRestart = true;
    }

    if (m_bRestart) {
        emit restartForSettings();
    }

    QDialog::accept();
}
