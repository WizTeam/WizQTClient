#ifndef WIZFILEREADER_H
#define WIZFILEREADER_H

#include <QThread>
#include <QStringList>

class CWizFileReader : public QThread
{
    Q_OBJECT
public:
    explicit CWizFileReader(QObject *parent = 0);

    void loadFiles(const QStringList& strFiles);

    QString loadHtmlFileToHtml(const QString& strFileName);
    QString loadTextFileToHtml(const QString& strFileName);
    QString loadImageFileToHtml(const QString& strFileName);
    QString loadRtfFileToHtml(const QString& strFileName);

signals:
    void fileLoaded(QString strHtml, QString strTitle);
    void fileLoadFailed(const QString& strFileName);
    void loadFinished();
    void loadProgress(int total,int loaded);
    void htmlFileloaded(const QString &strFileName, const QString& strHtml,
                     const QString& strTitle);
    void richTextFileLoaded(const QString& strHtml, const QString& strTitle,
                         const QString& strFileName);

public slots:

protected:
    void run();

private:
    QStringList m_files;
};

#endif // WIZFILEREADER_H
