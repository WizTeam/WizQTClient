#include "wizdownloadobjectdatadialog.h"
#include "ui_wizdownloadobjectdatadialog.h"

#include <QMessageBox>

WizDownloadObjectDataDialog::WizDownloadObjectDataDialog(CWizDatabase& db, const CString& strAccountsApiURL, const WIZOBJECTDATA& data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizDownloadObjectDataDialog)
    , m_data(data)
    , m_downloader(db, strAccountsApiURL, *this, data)
    , m_bUserCanceled(false)
{
    ui->setupUi(this);
    //
    ui->labelDownloading->setText(ui->labelDownloading->text().arg(m_data.strDisplayName));
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    //
    m_downloader.startDownload();
    //
    connect(&m_downloader, SIGNAL(done(bool)), this, SLOT(on_downloader_downloadObjectDataDone(bool)));
    //
    setFixedSize(size());
}

WizDownloadObjectDataDialog::~WizDownloadObjectDataDialog()
{
    m_downloader.abort();
    //
    delete ui;
}


void WizDownloadObjectDataDialog::addErrorLog(const CString& strMsg)
{
    m_slError.append(strMsg);
}

void WizDownloadObjectDataDialog::changeObjectDataProgress(int pos)
{
    ui->progressBar->setValue(pos);
}

void WizDownloadObjectDataDialog::reject()
{
    m_bUserCanceled = true;
    m_downloader.abort();
    //
    QDialog::reject();
}

void WizDownloadObjectDataDialog::on_downloader_downloadObjectDataDone(bool succeeded)
{
    if (succeeded)
    {
        accept();
    }
    else
    {
        QDialog::reject();
    }
}

bool WizDownloadObjectDataDialog::downloadObjectData(CWizDatabase& db, const CString& strAccountsApiURL, const WIZOBJECTDATA& data, QWidget* parent)
{
    WizDownloadObjectDataDialog dlg(db, strAccountsApiURL, data, parent);
    bool ret = QDialog::Accepted == dlg.exec();
    if (!ret
        && !dlg.m_bUserCanceled)
    {
        CString strMsg = QString(tr("Failed to download data!\n\n%1")).arg(dlg.m_slError.join("\n"));
        QMessageBox::critical(parent, parent->windowTitle(), strMsg);
    }
    return ret;
}
