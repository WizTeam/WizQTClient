#ifndef WIZFRAMELESSWEBDIALOG_H
#define WIZFRAMELESSWEBDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>

class QWebFrame;

class CWizFramelessWebDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CWizFramelessWebDialog(QWidget *parent = 0);

    void loadAndShow(const QString& strUrl);

signals:
    void doNotShowThisAgain(bool bAgain);

protected:
    void timerEvent(QTimerEvent *event);

public slots:
    void Execute(const QString& strFunction, QVariant param1, QVariant param2,
                                  QVariant param3, QVariant param4);
    void onJavaScriptWindowObjectCleared();

    void onPageLoadFinished(bool ok);


private:
    QWebFrame *m_frame;
    QString m_url;
    QList<int> m_timerIDList;
};

#endif // WIZFRAMELESSWEBDIALOG_H
