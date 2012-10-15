#include "wizupdaterdialog.h"
#include "ui_wizupdaterdialog.h"

#include "share/wizmisc.h"

CWizUpdaterDialog::CWizUpdaterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizUpdaterDialog)
{
    ui->setupUi(this);
    setFixedSize(size());
    setWindowFlags(Qt::CustomizeWindowHint);

    QPixmap pixmap(::WizGetResourcesPath() + "skins/wiznote64.png");
    ui->labelIcon->setPixmap(pixmap);
}

void CWizUpdaterDialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_bMovable = true;
        m_lastPos = event->pos();
    }
}

void CWizUpdaterDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons().testFlag(Qt::LeftButton) && m_bMovable) {
        move(pos() + (event->pos() - m_lastPos));
    }
}

void CWizUpdaterDialog::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_bMovable = false;
    }
}

CWizUpdaterDialog::~CWizUpdaterDialog()
{
    delete ui;
}
