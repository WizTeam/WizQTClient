#include "WizTemplatePurchaseDialog.h"
#include "ui_WizTemplatePurchaseDialog.h"

#ifdef Q_OS_MAC

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "share/jsoncpp/json/json.h"
#include "share/WizDatabase.h"
#include "share/WizDatabaseManager.h"
#include "share/WizMessageBox.h"
#include "sync/WizApiEntry.h"

#define WIZ_PRODUCT_TEMPLATE   "cn.wiz.wiznotemac.templates"

#define APPSTORE_IAP "APPSTORE_IAP"
#define APPSTORE_UNFINISHED_TEMPLATE_TRANSATION  "UNFINISHED_TEMPLATE_TRANSATION"

const int nWaitingTime = 2 * 60 * 1000;

WizTemplatePurchaseDialog::WizTemplatePurchaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizTemplatePurchaseDialog),
    m_net(new QNetworkAccessManager(this)),
    m_tmplId(0),
    m_iapHelper(nullptr)
{
    ui->setupUi(this);
    ui->btn_quit->setVisible(false);
    m_waitingTimer.setSingleShot(true);
}

WizTemplatePurchaseDialog::~WizTemplatePurchaseDialog()
{
    delete ui;
}

void WizTemplatePurchaseDialog::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    if (productList.size() != 1)
        return;

    changeToTemplatePage();
    ui->btn_purchase->setEnabled(true);
    ui->label_tmplPrice->setText(productList.first().localizedPrice);
}

void WizTemplatePurchaseDialog::onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID)
{
    QMetaObject::invokeMethod(this, "processIAPPurchaseResult", Qt::QueuedConnection, Q_ARG(bool, ok),
                              Q_ARG(const QByteArray&, receipt), Q_ARG(const QString&, strTransationID));
}

void WizTemplatePurchaseDialog::showStatusMeesage(const QString& text)
{
    changeToStatusPage();
    ui->label_status->setText(text);
}

void WizTemplatePurchaseDialog::changeToStatusPage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void WizTemplatePurchaseDialog::changeToTemplatePage()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void WizTemplatePurchaseDialog::checkRecipt(const QByteArray& receipt, const QString& strTransationID, int templateId)
{
    m_transationID = strTransationID;

    QString strPlat = "macosx";
    QString asServerUrl = WizCommonApiEntry::asServerUrl();
    QString checkUrl = asServerUrl + "/a/pay2/ios";
    //    QString checkUrl = "https://sandbox.itunes.apple.com/verifyReceipt";
    //    QString checkUrl = "https://buy.itunes.apple.com/verifyReceipt";
    WizDatabase& db = WizDatabaseManager::instance()->db();
    QString userID = db.getUserId();
    QString userGUID = db.getUserGuid();
    QString receiptBase64 = receipt.toBase64();
    receiptBase64 = QString(QUrl::toPercentEncoding(receiptBase64));
    QString strExtInfo = QString("client_type=%1&user_id=%2&user_guid=%3&transaction_id=%4&template_id=%5&receipt=%6")
            .arg(strPlat).arg(userID).arg(userGUID).arg(strTransationID).arg(QString::number(templateId)).arg(receiptBase64);

    qDebug() << "transation id = " << strTransationID;
//    qDebug() << "check receipt : " << checkUrl << strExtInfo;

    m_net->disconnect(this);
    connect(m_net, SIGNAL(finished(QNetworkReply*)), SLOT(checkReciptFinished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(checkUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    m_net->post(request, strExtInfo.toUtf8());

    // 开始倒计时，倒计时结束则认为处理失败
    m_waitingTimer.start(nWaitingTime);
}

void WizTemplatePurchaseDialog::on_purchase_failed(const QString& error)
{
    changeToStatusPage();
    showStatusMeesage(error);
    ui->btn_quit->setVisible(true);
}

void WizTemplatePurchaseDialog::on_purchase_successed()
{
    changeToStatusPage();
    showStatusMeesage("Purchase success");
    ui->btn_quit->setVisible(true);

    emit purchaseSuccess();
}

void WizTemplatePurchaseDialog::showTemplateInfo(int tmplId, const QString& tmplName, const QString& thumbUrl)
{
    changeToTemplatePage();

    m_tmplId = tmplId;
    ui->label_tmplThumb->setText(tr("Loading thumb image..."));
    ui->label_tmplName->setText(tmplName);
    ui->label_tmplPrice->setText(tr("Loading..."));
    //       
    m_net->disconnect(this);
    connect(m_net, SIGNAL(finished(QNetworkReply*)), SLOT(imageDownloadFinished(QNetworkReply*)));
    m_net->get(QNetworkRequest(thumbUrl));

    qDebug() << "download thumb from : " << thumbUrl;

    if (!m_iapHelper)
    {
        m_iapHelper = new WizIAPHelper(this);
    }
    QList<QString> m_productList;
    m_productList.append(WIZ_PRODUCT_TEMPLATE);
    m_iapHelper->requestProducts(m_productList);
    ui->btn_purchase->setEnabled(false);
}

void WizTemplatePurchaseDialog::imageDownloadFinished(QNetworkReply* reply)
{
    QByteArray ba = reply->readAll();
    if (ba.isEmpty())
    {
        ui->label_tmplThumb->setText(tr("Load thumb image failed."));
        return;
    }

    QPixmap pix;
    pix.loadFromData(ba);
    pix = pix.scaled(ui->label_tmplThumb->minimumSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->label_tmplThumb->setPixmap(pix);
}

void WizTemplatePurchaseDialog::on_btn_purchase_clicked()
{
    //
    m_iapHelper->purchaseProduct(WIZ_PRODUCT_TEMPLATE);
    changeToStatusPage();
    showStatusMeesage(tr("Connecting to AppStore..."));
    //
    m_waitingTimer.start(nWaitingTime);
}

void WizTemplatePurchaseDialog::on_btn_cancel_clicked()
{
    reject();
}

void WizTemplatePurchaseDialog::on_btn_quit_clicked()
{
    reject();
}

void WizTemplatePurchaseDialog::checkReciptFinished(QNetworkReply* reply)
{
    //
    m_waitingTimer.stop();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        on_purchase_failed(tr("Network error : %1").arg(reply->errorString()));
        return;
    }

    QString strResult = reply->readAll();
    reply->deleteLater();

    parseCheckResult(strResult, m_transationID);
}

void WizTemplatePurchaseDialog::processIAPPurchaseResult(bool ok, const QByteArray &receipt, const QString &strTransationID)
{
    m_waitingTimer.stop();

    if (!ok)
    {
        changeToStatusPage();
        showStatusMeesage(tr("Purchase failed, please try again later."));
        ui->btn_quit->setVisible(true);
    }
    else
    {
        changeToStatusPage();
        showStatusMeesage(tr("Waiting for Wiz server..."));

        //
        saveUnfinishedTransation(strTransationID, m_tmplId);
        checkRecipt(receipt, strTransationID, m_tmplId);
    }
}

void WizTemplatePurchaseDialog::onWaitingTimerOut()
{
    changeToStatusPage();
    showStatusMeesage(tr("Can not connect to Server, please try again later."));
    ui->btn_quit->setVisible(true);
}

void WizTemplatePurchaseDialog::parseCheckResult(const QString& strResult, const QString& strTransationID)
{
    if (strResult.isEmpty())
        return;

    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(strResult.toUtf8().constData(), d))
        return;

    if (d.isMember("error_code"))
    {
        QString strError = QString::fromStdString(d["error"].asString());
        qDebug() << strError;
        on_purchase_failed(strError);
        return;
    }

    if (d.isMember("return_code")) {
        int nCode = d["return_code"].asInt();
        if (nCode == 200)
        {
            qDebug() <<"IAP purchase successed!";
            on_purchase_successed();
            removeTransationFromUnfinishedList(strTransationID);
            //
            processUnfinishedTransation();
            return;
        }
        else
        {
            QString message = QString::fromStdString(d["return_message"].asString());
            qDebug() << "check on server failed , code :  " << nCode << "  message : " << message;
            on_purchase_failed(message);
            return;
        }
    }
}

QStringList WizTemplatePurchaseDialog::getUnfinishedTransations()
{
    QString transation = WizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHED_TEMPLATE_TRANSATION);
    return transation.split(';', QString::SkipEmptyParts);
}

void WizTemplatePurchaseDialog::processUnfinishedTransation()
{
    QStringList idList = getUnfinishedTransations();
    qDebug() << "process unfinished transation : " << idList;
    if (idList.size() == 0 || idList.first().isEmpty())
        return;

    QByteArray receipt;
    if (!m_iapHelper)
    {
        m_iapHelper = new WizIAPHelper(this);
    }
    m_iapHelper->loadLocalReceipt(receipt);
//    qDebug() << "process unfinished transation receipt : " << receipt;
    if (receipt.isEmpty())
    {
        WizMessageBox::warning(this, tr("Info"), tr("Can not load receipt!"));
        qDebug() << "local receipt load failed";
        return;
    }

    changeToStatusPage();
    showStatusMeesage(tr("Waiting for Wiz server..."));

    QStringList transationData = idList.first().split(',', QString::SkipEmptyParts);
    checkRecipt(receipt, transationData.first(), transationData.last().toInt());
}

void WizTemplatePurchaseDialog::saveUnfinishedTransation(const QString& strTransationID, int templateId)
{
    QString transationData = strTransationID + "," + QString::number(templateId);
    QString transation = WizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHED_TEMPLATE_TRANSATION);
    QStringList list = transation.split(';', QString::SkipEmptyParts);
    if (!list.contains(transationData))
    {
        list.append(transationData);
    }
    transation = list.join(';');
    WizDatabaseManager::instance()->db().setMeta(APPSTORE_IAP, APPSTORE_UNFINISHED_TEMPLATE_TRANSATION, transation);
}

void WizTemplatePurchaseDialog::removeTransationFromUnfinishedList(const QString& strTransationID)
{
    QString transation = WizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHED_TEMPLATE_TRANSATION);
    QStringList list = transation.split(';', QString::SkipEmptyParts);
    for (QString str : list)
    {
        QStringList transationData = str.split(',', QString::SkipEmptyParts);
        if (transationData.size() != 2 || transationData.first() != strTransationID)
            continue;

        list.removeOne(str);
        transation = list.join(';');
        WizDatabaseManager::instance()->db().setMeta(APPSTORE_IAP, APPSTORE_UNFINISHED_TEMPLATE_TRANSATION, transation);
    }
}

#endif
