#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "websocketclientwrapper.h"
#include "websockettransport.h"
#include <QWebEngineView>

class QWebChannel;

class WizWebEngineView : public QWebEngineView
{
    Q_OBJECT

public:
    WizWebEngineView(QWidget* parent);
    virtual ~WizWebEngineView();
public:
    void addToJavaScriptWindowObject(QString name, QObject* obj);
public Q_SLOTS:
    void innerLoadFinished(bool);
Q_SIGNALS:
    void loadFinishedEx(bool);
private:
    QWebSocketServer* m_server;
    WebSocketClientWrapper* m_clientWrapper;
    QWebChannel* m_channel;
    QString m_objectNames;
};

#endif // MAINWINDOW_H
