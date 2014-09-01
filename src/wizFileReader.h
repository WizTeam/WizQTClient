#ifndef WIZFILEREADER_H
#define WIZFILEREADER_H

#include <QThread>
#include <QStringList>

class CWizFileReader : public QThread
{
    Q_OBJECT
public:
    explicit CWizFileReader(QObject *parent = 0);

    void loadFiles(QStringList strFiles);

    QString loadTextFileToHtml(QString strFileName);
    QString loadImageFileToHtml(QString strFileName);

signals:
    void fileLoaded(QString strHtml, QString strTitle);
    void loadFinished();
    void loadProgress(int total,int loaded);

public slots:

protected:
    void run();

private:
    QStringList m_files;
};

#endif // WIZFILEREADER_H
