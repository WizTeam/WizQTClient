#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "share/wizmisc.h"
#include <QFileInfo>

AboutDialog::AboutDialog(CWizExplorerApp& app, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
    , m_app(app)
{
    ui->setupUi(this);
    //
    setFixedSize(size());
    //
    QPixmap pixmap(::WizGetSkinResourceFileName(m_app.userSettings().skin(), "about_logo"));
    ui->labelIcon->setPixmap(pixmap);
    //
#if defined Q_OS_MAC
    CString strProduct(tr("WizNote for Mac"));
#elif defined Q_OS_LINUX
    CString strProduct(tr("WizNote for Linux"));
#else
    CString strProduct(tr("WizNote for Windows"));
#endif
    //
    CString strVersionNumber("1.00");
    //
    QFileInfo fi(::WizGetAppFileName());
    QDateTime t = fi.lastModified();
    CString strBuildNumber;
    strBuildNumber.Format("%04d%02d%02d", t.date().year(), t.date().month(), t.date().day());
    //
    bool beta = true;
    CString strVersion(beta ? tr("beta (build %1)") : tr("release (build %1)"));
    strVersion = strVersion.arg(strBuildNumber);
    //
    CString strInfo = WizFormatString3("<span style=\"font-weight:bold;font-size:16pt\">%1</span><br /><span>%2 %3</span>",
                                       strProduct,
                                       strVersionNumber,
                                       strVersion);
    //
    ui->labelAbout->setTextFormat(Qt::AutoText);
    ui->labelAbout->setText(strInfo);
    //
    ui->labelLink->setOpenExternalLinks(true);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
