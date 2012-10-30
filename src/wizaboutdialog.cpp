#include "wizaboutdialog.h"
#include "ui_wizaboutdialog.h"

#include "share/wizmisc.h"
#include <QFileInfo>

AboutDialog::AboutDialog(CWizExplorerApp& app, QWidget *parent)
    : QDialog(parent)
    , m_app(app)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setFixedSize(size());

    QPixmap pixmap(::WizGetSkinResourceFileName(m_app.userSettings().skin(), "about_logo"));
    ui->labelIcon->setPixmap(pixmap);

#if defined Q_OS_MAC
    QString strProduct(tr("WizNote for Mac"));
#elif defined Q_OS_LINUX
    QString strProduct(tr("WizNote for Linux"));
#else
    QString strProduct(tr("WizNote for Windows"));
#endif

    QFileInfo fi(::WizGetAppFileName());
    QDateTime t = fi.lastModified();
    QString strBuildNumber("build %1.%2.%3");
    strBuildNumber = strBuildNumber.arg(t.date().year()).arg(t.date().month()).arg(t.date().day());

    QString strInfo("<span style=\"font-weight:bold;font-size:16pt\">%1</span><br /><span>%2 %3</span>");
    strInfo = strInfo.arg(strProduct, IWizGlobal::instance()->version(), strBuildNumber);

    ui->labelAbout->setTextFormat(Qt::AutoText);
    ui->labelAbout->setText(strInfo);
    ui->labelLink->setOpenExternalLinks(true);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
