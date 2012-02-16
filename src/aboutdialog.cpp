#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "share/wizmisc.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    //
    setFixedSize(size());
    //
    QIcon icon(WizGetSkinResourceFileName("wiznote128"));
    //
    ui->labelIcon->setPixmap(icon.pixmap(128, 128));
    CString strVersion("0.01");
#if defined Q_OS_MAC
    ui->labelAbout->setText(tr("WizNote for Mac %1").arg(strVersion));
#elif defined Q_OS_LINUX
    ui->labelAbout->setText(tr("WizNote for Linux %1").arg(strVersion));
#else
    ui->labelAbout->setText(tr("WizNote for Windows %1").arg(strVersion));
#endif
    ui->labelLink->setOpenExternalLinks(true);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
