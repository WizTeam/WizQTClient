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
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
