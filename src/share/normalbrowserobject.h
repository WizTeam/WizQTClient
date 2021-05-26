#ifndef NORMALBROWSEROBJECT_H
#define NORMALBROWSEROBJECT_H

#include <QObject>

class QWebEngineView;
class WizMainWindow;

class NormalBrowserObject: public QObject
{
    Q_OBJECT
public:
    NormalBrowserObject(WizMainWindow* mainWindow, QWebEngineView* web, QObject* parent);
protected:
    WizMainWindow* m_mainWindow;
    QWebEngineView* m_web;
public:
    Q_INVOKABLE void GetToken(const QString& strFunctionName);
};

#endif // NORMALBROWSEROBJECT_H
