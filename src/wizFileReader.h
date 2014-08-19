#ifndef WIZFILEREADER_H
#define WIZFILEREADER_H

#include <QThread>

class CWizFileReader : public QThread
{
    Q_OBJECT
public:
    explicit CWizFileReader(QObject *parent = 0);

    void loadFiles(QStringList strFiles);

    QString loadTextFileToHtml(QString strFileName);
    QString loadImageFileToHtml(QString strFileName);

signals:
    void fileLoaded(QString strHtml);
    void loadProgress(int loaded, int total);

public slots:

protected:
    void run();

private:
    QStringList m_files;
};

#endif // WIZFILEREADER_H
