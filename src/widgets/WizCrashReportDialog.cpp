#include "WizCrashReportDialog.h"
#include "ui_WizCrashReportDialog.h"

#ifdef Q_OS_MAC
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QPlainTextEdit>
#include "share/jsoncpp/json/json.h"
#include "WizDef.h"
#include "share/WizEventLoop.h"
#include "share/WizMisc.h"
#include "share/WizThreads.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"

WizCrashReportDialog::WizCrashReportDialog(const QString& text, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizCrashReportDialog)
    , m_reports(text)
    , m_textEdit(new QPlainTextEdit(this))
{
    ui->setupUi(this);

    m_textEdit->setVisible(false);
    m_textEdit->setReadOnly(true);
    layout()->addWidget(m_textEdit);
}

WizCrashReportDialog::~WizCrashReportDialog()
{
    delete ui;
}

void WizCrashReportDialog::on_btn_yes_clicked()
{
    accept();

    QString text = m_reports;
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [text](){
        QString reportText = text;
        const int maxSize = 1024 * 128 - 100;
        if (reportText.size() > maxSize)
        {
            reportText.truncate(maxSize);
            reportText.append("\n Wow! too may words, truncated!");
        }

        //
        QString url = WizOfficialApiEntry::crashReportUrl();
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
            "application/x-www-form-urlencoded");

        QUrlQuery postData;
        postData.addQueryItem("platform", "mac");
        postData.addQueryItem("token", "");
        postData.addQueryItem("subject", "Version: " + QString(WIZ_CLIENT_VERSION) + "  UUID : " + WizGenGUIDLowerCaseLetterOnly());
        postData.addQueryItem("error", reportText);

        QNetworkAccessManager net;
        QNetworkReply* reply = net.post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

        WizAutoTimeOutEventLoop loop(reply);
        loop.exec();

        if (loop.error() != QNetworkReply::NoError || loop.result().isEmpty())
        {
            qDebug() << "[Crash report]Upload failed!";
            return;
        }

        Json::Value d;
        Json::Reader reader;
        if (!reader.parse(loop.result().constData(), d))
            return;

        if (!d.isMember("return_code"))
        {
            qDebug() << "[Crash report]Can not get return code ";
            return;
        }

        int returnCode = d["return_code"].asInt();
        if (returnCode != 200)
        {
            qDebug() << "[Crash report]Return code was not 200, error :  " << returnCode << loop.result();
            return;
        }
        else
        {
            qDebug() << "[Crash report]Upload OK";
        }

    });
}

void WizCrashReportDialog::on_btn_no_clicked()
{
    reject();
}

void WizCrashReportDialog::on_btn_details_clicked()
{
    m_textEdit->setVisible(!m_textEdit->isVisible());
    if (m_textEdit->toPlainText().isEmpty())
    {
        m_textEdit->insertPlainText(m_reports);
    }

    if (m_textEdit->isVisible())
    {
        setMaximumHeight(QWIDGETSIZE_MAX);
        resize(width(), 450);
    }
    else
    {
        setMaximumHeight(100);
        resize(width(), 100);
    }

}

#endif
