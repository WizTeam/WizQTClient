#include "WizIAPDialog.h"
#include "ui_WizIAPDialog.h"
#include <QMessageBox>
#include <QTextBrowser>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QUrl>
#include <QDebug>

#if defined Q_OS_MAC
#include "share/jsoncpp/json/json.h"
#include "utils/WizStyleHelper.h"
#include "sync/WizToken.h"
#include "share/WizMisc.h"
#include "share/WizMessageBox.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "sync/WizApiEntry.h"
#include "share/WizGlobal.h"
#include "mac/WizIAPHelper.h"
#include "WizMainWindow.h"

#define WIZ_PRODUCT_MONTH "cn.wiz.wiznote.mac.pro.monthly"
#define WIZ_PRODUCT_YEAR "cn.wiz.wiznote.mac.pro.yearly"

#define APPSTORE_IAP "APPSTORE_IAP"
#define APPSTORE_UNFINISHEDTRANSATION  "UNFINISHED_TRANSATION"

WizIAPDialog::WizIAPDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::WizIAPDialog)
  , m_waitingMsgBox(new QMessageBox(this))
  , m_iAPhelper(0)
  , m_net(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(1);
    hideInfoLabel();
    setPurchaseAvailable(false);

    initStyles();
    //
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(WizGlobal::mainWindow());
    if (mainWindow) {
        //
        WizWebEngineView::initWebEngineView(ui->webView, {{"WizExplorerApp", mainWindow->object()}});
    }


    connect(&m_timer, SIGNAL(timeout()), SLOT(onWaitingTimeOut()));
    connect(m_net, SIGNAL(finished(QNetworkReply*)), SLOT(checkReceiptFinished(QNetworkReply*)));

    // call check receipt function in main thread
    connect(this, SIGNAL(checkReceiptRequest(QByteArray,QString)), this,
            SLOT(onCheckReceiptRequest(QByteArray,QString)), Qt::QueuedConnection);
}

WizIAPDialog::~WizIAPDialog()
{
    delete ui;
}

void WizIAPDialog::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    QTimer::singleShot(0, this, SLOT(stopWaitTimer()));
    qDebug() << "load product list finished: " << productList.size();
    if (productList.isEmpty())
        return;

    setPurchaseAvailable(true);
    foreach (CWizIAPProduct product, productList)
    {
        if (product.id == WIZ_PRODUCT_MONTH)
        {
            ui->btn_month->setText(QString(tr("%1 / month")).arg(product.localizedPrice));
        }
        else if (product.id == WIZ_PRODUCT_YEAR)
        {
            ui->btn_year->setText(QString(tr("%1 / year")).arg(product.localizedPrice));
        }
    }
    QTimer::singleShot(0, this, SLOT(checkUnfinishedTransation()));
}

void WizIAPDialog::onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID)
{
    m_waitingMsgBox->done(ok);    

    if (ok)
    {
        saveUnfinishedTransation(strTransationID);   
        //
        emit checkReceiptRequest(receipt, strTransationID);
    }
    else
    {
        hideInfoLabel();
        ui->label_failed->setVisible(true);
    }
}

void WizIAPDialog::loadUserInfo()
{
    setWindowTitle(tr("Account settings"));
    ui->stackedWidget->setCurrentIndex(0);
    QString extInfo = WizCommonApiEntry::appstoreParam(false);
    QString strToken = WizToken::token();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("user_info", strToken, extInfo);
    qDebug() << "load user info : " << strUrl;
    ui->webView->load(QUrl(strUrl));
}

void WizIAPDialog::loadIAPPage()
{
    setWindowTitle(tr("Upgrade VIP"));
    ui->stackedWidget->setCurrentIndex(1);
    hideInfoLabel();
    if (!ui->btn_month->isEnabled())
    {
        QTimer::singleShot(10, this, SLOT(loadProducts()));
    }
}

int WizIAPDialog::exec()
{
    if (ui->stackedWidget->currentIndex() == 1)
    {
        hideInfoLabel();
        if (ui->btn_month->isEnabled())
        {
            checkUnfinishedTransation();
        }
    }
    return QDialog::exec();
}

void WizIAPDialog::initStyles()
{
    QString strThemeName = Utils::WizStyleHelper::themeName();

    ui->btn_goBack->setText("");
    QString strBack= ::WizGetSkinResourceFileName(strThemeName, "iap_back");
    QString strBackOn = ::WizGetSkinResourceFileName(strThemeName, "iap_back_on");
    ui->btn_goBack->setStyleSheet(QString("QToolButton {border:none; background-image: url(%1);"
                                          "background-position: center; background-repeat: no-repeat} QToolButton:pressed {background-image: url(%2); }")
                                  .arg(strBack).arg(strBackOn));
    ui->label_headTitle->setStyleSheet(QString("QLabel {border: none;background: none;"
                                               "font:16px #5e5e5e; padding-right: 70px;}"));

    ui->btn_month->setStyleSheet(QString("QPushButton { border: 1px; border-radius: 4px; background-color: #48a4e0; "
                                          "color: #ffffff;  font:14px; } QPushButton:pressed {background-color: #227dbf; }"));
    ui->btn_month->setCursor(Qt::PointingHandCursor);

    ui->btn_year->setStyleSheet(QString("QPushButton { border: 1px; border-radius: 4px; background-color: #48a4e0; "
                                          "color: #ffffff;  font:14px; } QPushButton:pressed {background-color: #227dbf; }"));
    ui->btn_year->setCursor(Qt::PointingHandCursor);

    QString strIcon1 = ::WizGetSkinResourceFileName(strThemeName, "iap_huifubanben");
    ui->label_text1->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-left: 100px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon1));
    QString strIcon2 = ::WizGetSkinResourceFileName(strThemeName, "iap_note");
    ui->label_text2->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-right: 80px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon2));
    QString strIcon3 = ::WizGetSkinResourceFileName(strThemeName, "iap_liuliang");
    ui->label_text3->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-left: 100px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon3));
    QString strIcon4 = ::WizGetSkinResourceFileName(strThemeName, "iap_user");
    ui->label_text4->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-right: 80px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon4));
}

void WizIAPDialog::createIAPHelper()
{
    if (m_iAPhelper == 0)
        m_iAPhelper = new WizIAPHelper(this);
}

void WizIAPDialog::setPurchaseAvailable(bool b)
{
    ui->label_downloading->setVisible(!b);
    ui->btn_month->setEnabled(b);
    ui->btn_year->setEnabled(b);
}

void WizIAPDialog::hideInfoLabel()
{
    ui->label_downloading->setVisible(false);
    ui->label_success->setVisible(false);
    ui->label_failed->setVisible(false);
}

void WizIAPDialog::checkReceiptInfo(const QByteArray& receipt, const QString& strTransationID)
{
    m_transationID = strTransationID;

    QString strPlat;
#ifdef Q_OS_MAC
    strPlat = "macosx";
#else
    strPlat = "linux";
#endif
    QString asServerUrl = WizCommonApiEntry::asServerUrl();
    QString checkUrl = asServerUrl + "/a/pay2/ios";
    //    QString checkUrl = "https://sandbox.itunes.apple.com/verifyReceipt";
    //    QString checkUrl = "https://buy.itunes.apple.com/verifyReceipt";
    WizDatabase& db = WizDatabaseManager::instance()->db();
    QString userID = db.getUserId();
    QString userGUID = db.getUserGuid();
    QString receiptBase64 = receipt.toBase64();
    receiptBase64 = QString(QUrl::toPercentEncoding(receiptBase64));
    QString strExtInfo = QString("client_type=%1&user_id=%2&user_guid=%3&transaction_id=%4&receipt=%5")
            .arg(strPlat).arg(userID).arg(userGUID).arg(strTransationID).arg(receiptBase64);

    qDebug() << "transation id = " << strTransationID;
    //    qDebug() << "check receipt : " << checkUrl << strExtInfo;

    QNetworkRequest request;
    request.setUrl(checkUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    m_net->post(request, strExtInfo.toUtf8());
}

void WizIAPDialog::parseCheckResult(const QString& strResult, const QString& strTransationID)
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
            checkUnfinishedTransation();
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

QStringList WizIAPDialog::getUnfinishedTransations()
{
    QString transation = WizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION);
    return transation.split(';', QString::SkipEmptyParts);
}

void WizIAPDialog::saveUnfinishedTransation(const QString& strTransationID)
{
    QString transation = WizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION);
    QStringList list = transation.split(';', QString::SkipEmptyParts);
    if (!list.contains(strTransationID))
    {
        list.append(strTransationID);
    }
    transation = list.join(';');
    WizDatabaseManager::instance()->db().setMeta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION, transation);
}

void WizIAPDialog::removeTransationFromUnfinishedList(const QString& strTransationID)
{
    QString transation = WizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION);
    QStringList list = transation.split(';', QString::SkipEmptyParts);
    if (!list.contains(strTransationID))
    {
        return;
    }
    list.removeOne(strTransationID);
    transation = list.join(';');
    WizDatabaseManager::instance()->db().setMeta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION, transation);
}

void WizIAPDialog::on_btn_goBack_clicked()
{
    loadUserInfo();
}

void WizIAPDialog::on_btn_month_clicked()
{

    hideInfoLabel();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText(tr("Waiting for AppStore..."));
    m_waitingMsgBox->open();

    //
    m_iAPhelper->purchaseProduct(WIZ_PRODUCT_MONTH);
}

void WizIAPDialog::on_btn_year_clicked()
{
    hideInfoLabel();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText(tr("Waiting for AppStore..."));
    m_waitingMsgBox->open();

    m_iAPhelper->purchaseProduct(WIZ_PRODUCT_YEAR);
}

void WizIAPDialog::loadProducts()
{
    if (m_iAPhelper == 0)
        m_iAPhelper = new WizIAPHelper(this);


    QList<QString> productList;
    productList.append(WIZ_PRODUCT_MONTH);
    productList.append(WIZ_PRODUCT_YEAR);
    m_iAPhelper->requestProducts(productList);
    m_timer.start(2 * 60 * 1000);
    ui->label_downloading->setVisible(true);
}

void WizIAPDialog::stopWaitTimer()
{
    m_timer.stop();
}

void WizIAPDialog::onWaitingTimeOut()
{
    m_timer.stop();
    m_waitingMsgBox->close();    
    m_waitingMsgBox->setStandardButtons(QMessageBox::Ok);
    m_waitingMsgBox->setText(tr("Can not connect to Server, please try again later."));
    m_waitingMsgBox->exec();
    accept();
}

void WizIAPDialog::onCheckReceiptRequest(const QByteArray& receipt, const QString& strTransationID)
{
    m_waitingMsgBox->close();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText(tr("Waiting for Wiz Server..."));
    m_waitingMsgBox->open();

    checkReceiptInfo(receipt, strTransationID);
}

void WizIAPDialog::checkUnfinishedTransation()
{
    qDebug() << "Check unfinished transations";
    QStringList idList = getUnfinishedTransations();
    if (idList.size() == 0 || idList.first().isEmpty())
        return;

    QByteArray receipt;
    m_iAPhelper->loadLocalReceipt(receipt);
    if (receipt.isEmpty())
    {
        WizMessageBox::warning(this, tr("Info"), tr("Can not load receipt!"));
        qDebug() << "local receipt load failed";
        return;
    }

    int result = WizMessageBox::information(this, tr("Info"), tr("You have unfinished transation, continue to process it?"),
                             QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
    if (QMessageBox::Ok == result)
    {
        onCheckReceiptRequest(receipt, idList.first());
    }
}

void WizIAPDialog::checkReceiptFinished(QNetworkReply* reply)
{
    m_waitingMsgBox->done(0);

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        on_purchase_failed(tr("Network error : %1").arg(reply->errorString()));
        return;
    }

    QString strResult = reply->readAll();
    reply->deleteLater();

    parseCheckResult(strResult, m_transationID);
}

void WizIAPDialog::on_purchase_successed()
{
    hideInfoLabel();
    ui->label_success->setVisible(true);
}

void WizIAPDialog::on_purchase_failed(const QString& errorMsg)
{
    hideInfoLabel();
    ui->label_failed->setVisible(true);
    ui->label_failed->setText(QString("<html><head/><body><p align=\"center\"><span style=\""
                                      " font-size:16pt; color:#ff6666;\">%1</span></p></body></html>").arg(errorMsg));
}

#endif
