#include "WizNoteManager.h"
#include <QFileInfo>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDir>
#include <fstream>
#include <QDebug>

#include "share/jsoncpp/json/json.h"
#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include "sync/WizApiEntry.h"
#include "sync/WizToken.h"
#include "share/WizObject.h"
#include "share/WizDatabase.h"
#include "share/WizDatabaseManager.h"
#include "share/WizThreads.h"
#include "share/WizEventLoop.h"
#include "WizDocTemplateDialog.h"
#include "share/WizObjectDataDownloader.h"

void WizNoteManager::createIntroductionNoteForNewRegisterAccount()
{
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        //get local note
        QDir dir(Utils::WizPathResolve::introductionNotePath());
        QStringList introductions = dir.entryList(QDir::Files);
        if (introductions.isEmpty())
            return;

        QSettings settings(Utils::WizPathResolve::introductionNotePath() + "settings.ini", QSettings::IniFormat);
        //copy note to new account
        WizDatabase& db = m_dbMgr.db();
        for (QString fileName : introductions)
        {
            QString filePath = Utils::WizPathResolve::introductionNotePath() + fileName;
            QFileInfo info(filePath);
            if (info.suffix() == "ini")
                continue;
            settings.beginGroup("Location");
            QString location = settings.value(info.baseName(), "/My Notes/").toByteArray();
            settings.endGroup();
            WIZTAGDATA tag;
            WIZDOCUMENTDATA doc;
            settings.beginGroup("Title");
            doc.strTitle = settings.value(info.baseName()).toString();
            settings.endGroup();
            if (!db.createDocumentByTemplate(filePath, location, tag, doc))
            {
                qCritical() << "create introduction note failed : " << filePath;
            }
        }
    });
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data)
{
    return createNote(data, "", QObject::tr("Untitled"), "<p><br/></p>", "");
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", "");
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strLocation)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", strLocation);
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const WIZTAGDATA& tag)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", "", tag);
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strLocation, const WIZTAGDATA& tag)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", strLocation, tag);
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml)
{
    return createNote(data, strKbGUID, strTitle, strHtml, "");
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml,
                                 const QString& strLocation)
{
    QString location = strLocation;
    if (location.isEmpty())
    {
        location = m_dbMgr.db(strKbGUID).getDefaultNoteLocation();
    }

    if (data.strType.isEmpty())
    {
        data.strType = WIZ_DOCUMENT_TYPE_NORMAL;
    }

    if (!m_dbMgr.db(strKbGUID).createDocumentAndInit(strHtml, "", 0, strTitle, "newnote", location, "", data))
    {
        qCritical() << "Failed to new document!";
        return false;
    }

    return true;
}


bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml,
                                 const WIZTAGDATA& tag)
{
    QString location = m_dbMgr.db(strKbGUID).getDefaultNoteLocation();
    return createNote(data, strKbGUID, strTitle, strHtml, location, tag);
}

bool WizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml,
                                 const QString& strLocation, const WIZTAGDATA& tag)
{
    if (!createNote(data, strKbGUID, strTitle, strHtml, strLocation))
        return false;

    if (!tag.strGUID.isEmpty())
    {
        WizDocument doc(m_dbMgr.db(strKbGUID), data);
        doc.addTag(tag);
    }

    return true;
}

bool WizNoteManager::createNoteByTemplate(WIZDOCUMENTDATA& data, const WIZTAGDATA& tag, const QString& strZiw)
{
    //通过模板创建笔记时，如果模板文件不存在则创建一篇空笔记
    if (!QFile::exists(strZiw))
    {
        qDebug() << "Template file not exists : " << strZiw;
        return createNote(data, data.strKbGUID, data.strTitle, "<p><br></p>", data.strLocation, tag);
    }

    if (!m_dbMgr.db(data.strKbGUID).createDocumentByTemplate(strZiw, data.strLocation, tag, data))
    {
        qDebug() << "Failed to new document! " << strZiw;
        return false;
    }
    return true;
}

void WizNoteManager::updateTemplateJS(const QString& local)
{
    //软件启动之后获取模板信息，检查template.js是否存在、是否是最新版。需要下载时进行下载
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=]() {
        //NOTE:现在编辑器依赖template.js文件。需要确保该文件存在。如果文件不存在则拷贝
        WizEnsurePathExists(Utils::WizPathResolve::customNoteTemplatesPath());
        if (!QFile::exists(Utils::WizPathResolve::wizTemplateJsFilePath()))
        {
            QString localJs = Utils::WizPathResolve::resourcesPath() + "files/wizeditor/wiz_template.js";
            WizCopyFile(localJs, Utils::WizPathResolve::wizTemplateJsFilePath(), true);
        }

        QNetworkAccessManager manager;
        QString url = WizCommonApiEntry::asServerUrl() + "/a/templates?language_type=" + local;
#ifdef Q_OS_MAC
        url.append("&client_type=macosx");
#else
        url.append("&client_type=linux");
#endif
//        qDebug() << "get templates message from url : " << url;
        //
        QByteArray ba;
        {
            QNetworkReply* reply = manager.get(QNetworkRequest(url));
            WizAutoTimeOutEventLoop loop(reply);
            loop.exec();
            //
            if (loop.error() != QNetworkReply::NoError || loop.result().isEmpty())
                return;

            ba = loop.result();
        }

        //根据线上的内容来判断本地的模板文件是否需要更新
        if (!updateLocalTemplates(ba, manager))
            return;

        //更新成功之后将数据保存到本地
        QString jsonFile = Utils::WizPathResolve::wizTemplateJsonFilePath();
        std::ofstream logFile(jsonFile.toUtf8().constData(), std::ios::out | std::ios::trunc);
        logFile << ba.constData();
    });
}

void WizNoteManager::downloadTemplatePurchaseRecord()
{
    //下载用户购买的模板列表
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=]() {
        WizEnsurePathExists(Utils::WizPathResolve::customNoteTemplatesPath());
        //
        QNetworkAccessManager manager;
        QString url = WizCommonApiEntry::asServerUrl() + "/a/templates/record?token=" + WizToken::token();
//        qDebug() << "get templates record from url : " << url;
        //
        QByteArray ba;
        {
            QNetworkReply* reply = manager.get(QNetworkRequest(url));
            WizAutoTimeOutEventLoop loop(reply);
            loop.exec();
            //
            if (loop.error() != QNetworkReply::NoError || loop.result().isEmpty())
                return;

            ba = loop.result();
            QString jsonFile = Utils::WizPathResolve::wizTemplatePurchaseRecordFile();
            std::ofstream recordFile(jsonFile.toUtf8().constData(), std::ios::out | std::ios::trunc);
            recordFile << ba.constData();
        }
    });
}

bool WizNoteManager::updateLocalTemplates(const QByteArray& newJsonData, QNetworkAccessManager& manager)
{
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(newJsonData.constData(), d))
        return false;

    QString localFile = Utils::WizPathResolve::wizTemplateJsonFilePath();
    bool needUpdateJs = true;
    QMap<int, TemplateData> localTmplMap;
    QFile file(localFile);
    if (file.open(QFile::ReadOnly))
    {
        QTextStream stream(&file);
        QString jsonData = stream.readAll();
        Json::Value localD;
        if (reader.parse(jsonData.toUtf8().constData(), localD))
        {
            if (localD.isMember("template_js_version") && d.isMember("template_js_version"))
            {
                needUpdateJs = (localD["template_js_version"].asString() !=
                        d["template_js_version"].asString());
            }
        }

        getTemplatesFromJsonData(jsonData.toUtf8(), localTmplMap);
    }

    //
    if (needUpdateJs)
    {
        QString link;
        if (d.isMember("template_js_link"))
        {
            link = QString::fromStdString(d["template_js_link"].asString());
        }
        if (!link.isEmpty())
        {
            qDebug() << "get templates js file from url : " << link;
            QString file = Utils::WizPathResolve::wizTemplateJsFilePath();
            QNetworkReply* reply = manager.get(QNetworkRequest(link));
            //
            WizAutoTimeOutEventLoop loop(reply);
            loop.exec();
            //
            if (loop.error() != QNetworkReply::NoError || loop.result().isEmpty())
                return false;

            QByteArray ba = loop.result();
            std::ofstream jsFile(file.toUtf8().constData(), std::ios::out | std::ios::trunc);
            jsFile << ba.constData();
        }
    }

    //
    QMap<int, TemplateData> serverTmplMap;
    getTemplatesFromJsonData(newJsonData, serverTmplMap);

    //下载服务器上有更新的模板
    for (auto it = serverTmplMap.begin(); it != serverTmplMap.end(); ++it)
    {
        auto iter = localTmplMap.find(it.key());
        if (iter == localTmplMap.end())
            continue;

        if (iter.value().strVersion != it.value().strVersion || !QFile::exists(it.value().strFileName))
        {
            QString strUrl = WizCommonApiEntry::asServerUrl() + "/a/templates/download/" + QString::number(it.value().id);
            QFileInfo info(it.value().strFileName);
            WizFileDownloader* downloader = new WizFileDownloader(strUrl, info.fileName(), info.absolutePath() + "/", false);
            downloader->startDownload();
        }
    }

    //删除服务器上不存在的模板
    for (auto it = localTmplMap.begin(); it != localTmplMap.end(); ++it)
    {
        auto iter = serverTmplMap.find(it.key());
        if (iter == localTmplMap.end())
        {
            WizDeleteFile(it.value().strFileName);
        }
    }

    return true;
}

bool WizNoteManager::downloadTemplateBlocked(const TemplateData& tempData)
{
    QString strUrl = WizCommonApiEntry::asServerUrl() + "/a/templates/download/" + QString::number(tempData.id);
    return WizURLDownloadToFile(strUrl, tempData.strFileName, false);
}


WizNoteManager::WizNoteManager(WizDatabaseManager& dbMgr)
    : m_dbMgr(dbMgr)
{
}

