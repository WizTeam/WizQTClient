#include "WizSvgEditorDialog.h"
#include <QFile>
#include <QTimer>

#include "share/WizThreads.h"
#include "share/WizMisc.h"
#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include "share/jsoncpp/json/json.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "sync/WizApiEntry.h"
#include "share/WizMessageBox.h"



WizSvgEditorDialog::WizSvgEditorDialog(QString url, QString data, std::function<void(bool changed, std::function<void(bool)> saved)> saveCallback, QString htmlFilePath, QWidget* parent)
    : WizWebSettingsDialog({{"WizQtObject", this}}, url, QSize(), parent)
    , m_error(false)
    , m_data(data)
    , m_saveCallback(saveCallback)
    , m_htmlFilePath(htmlFilePath)
{
    QString resourcePath = Utils::WizMisc::extractFilePath(m_htmlFilePath);
    resourcePath += "index_files/";
    Utils::WizMisc::ensurePathExists(resourcePath);
    m_indexFilesPath = resourcePath;

#ifdef QT_DEBUG
    //m_url = "http://192.168.1.73:8888/static/index.html?clientType=macosx";
#endif
    setWindowTitle(QObject::tr("Handwriting Note"));
    //
    QString type = data.isEmpty() ? "createNote" : "";
    QString a = + "type=" + type;
    m_url = m_url + "&a=" + QUrl::toPercentEncoding(a);
}

void WizSvgEditorDialog::reject()
{
    if (m_error) {
        WizWebSettingsDialog::reject();
        return;
    }
    //
    web()->page()->runJavaScript("wizSvgPainter.save({closeDialog: true})", [=](const QVariant& ret) {
        //
        if (!ret.isValid()) {
            WizWebSettingsDialog::reject();
        }

    });
}

void WizSvgEditorDialog::onLoaded(bool ok)
{
    if (!ok) {
        m_error = true;
        return;
    }
    //
    if (web()->page()->url() == QUrl("about:blank")) {
        return;
    }
    //
    if (m_data.isEmpty()) {
        return;
    }
    //
    QString data = m_data;
    //
    data = data.replace("\\", "\\\\");
    //
    QString script = QString("wizSvgPainter.importData(`%1`)").arg(data);
    web()->page()->runJavaScript(script);
}

void WizSvgEditorDialog::loadData(QString fileName, QString callback)
{
    QString text;
    fileName = m_indexFilesPath + fileName;
    WizLoadUnicodeTextFromFile(fileName, text);
    //
    text = text.replace("\\", "\\\\");
    QString js = QString("%1(`%2`)").arg(callback).arg(text);
    web()->page()->runJavaScript(js);
}

void WizSvgEditorDialog::saveData(QString options, QString svgData, QString htmlData)
{
    Json::Value o;
    Json::Reader reader;
    if (!reader.parse(options.toUtf8().constData(), o))
        return;
    //
    bool changed = false;
    if (!svgData.isEmpty()) {
        //
        QString fileName = QString::fromUtf8(o["fileName"].asString().c_str());
        QString oldFileName = QString::fromUtf8(o["oldFileName"].asString().c_str());
        if (!fileName.isEmpty()) {
            fileName = m_indexFilesPath + fileName;
            ::WizSaveUnicodeTextToUtf8File(fileName, svgData, false);
            //
            if (!oldFileName.isEmpty()) {
                oldFileName = m_indexFilesPath + oldFileName;
                QFile::remove(oldFileName);
            }
        }
        changed = true;
    }

    //
    if (!htmlData.isEmpty()) {
        //
        bool isFullHtml = o["isFullHtml"].asBool();
        if (!isFullHtml) {
            htmlData = QString("<!DOCTYPE html><html><head></head>%1</html>").arg(htmlData);
        }
        ::WizSaveUnicodeTextToUtf8File(m_htmlFilePath, htmlData);
        changed = true;
    }
    //
    bool closeDialog = o["closeDialog"].asBool();
    //
    m_saveCallback(changed, [=] (bool saved) {
        if (!saved) {
            WizMessageBox::warning(this, tr("Info"), tr("Failed to save note."));
            return;
        }
        if (closeDialog) {
            WizWebSettingsDialog::reject();
        }
    });
    //
}

//
void saveSvgCore(WizDatabaseManager& dbMgr, const WIZDOCUMENTDATAEX& doc, QString strHtmlFile, bool changed, std::function<void(bool)> saved)
{

    QString html;
   ::WizLoadUnicodeTextFromFile(strHtmlFile, html);
    //
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dbMgr] {
        //
        WizDocumentDataLocker locker(doc.strGUID);
        //
        bool ret = true;
        if (changed) {
            bool notify = false;    //don't notify
            WizDatabase& db = dbMgr.db(doc.strKbGUID);
            auto note = doc;
            ret = db.updateDocumentData(note, html, strHtmlFile, 0, notify);
        }
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
            saved(ret);
        });
        //
    });
}

void editHandwritingNote(WizDatabaseManager& dbMgr, const WIZDOCUMENTDATAEX& doc, QString strHtmlFile, QString data, QWidget* parent)
{
    auto saveSvgCallback = [=, &dbMgr] (bool changed, std::function<void(bool)> saved) {
        //
        saveSvgCore(dbMgr, doc, strHtmlFile, changed, saved);
        //
    };
    //
    QString url = ::WizCommonApiEntry::makeUpUrlFromCommand("svg_editor");
    WizSvgEditorDialog* dialog = new WizSvgEditorDialog(url, data, saveSvgCallback, strHtmlFile, parent);
    //
    dialog->exec();

    dialog->deleteLater();
}

void createHandwritingNote(WizDatabaseManager& dbMgr, const WIZDOCUMENTDATAEX& doc, QWidget* parent)
{

    QString strHtmlFile;
    WizDatabase& db = dbMgr.db(doc.strKbGUID);
    if (!db.documentToTempHtmlFile(doc, strHtmlFile)) {
        return;
    }
    //
    auto saveSvgCallback = [=, &dbMgr] (bool changed, std::function<void(bool)> saved) {
        //
        saveSvgCore(dbMgr, doc, strHtmlFile, changed, saved);
        //
    };
    //
    QString url = ::WizCommonApiEntry::makeUpUrlFromCommand("svg_editor");
    WizSvgEditorDialog* dialog = new WizSvgEditorDialog(url, "", saveSvgCallback, strHtmlFile, parent);
    //
    dialog->exec();
    //
    //
    dialog->deleteLater();
}

