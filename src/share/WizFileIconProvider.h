#ifndef WIZFILEICONPROVIDER_H
#define WIZFILEICONPROVIDER_H

#include <QFileIconProvider>

class WizFileIconProvider : public QFileIconProvider
{
public:
    WizFileIconProvider();
public:
    virtual QIcon icon(const QString& strFilePath) const;
    virtual QString type(const QString& strFileName) const;
    //
    using QFileIconProvider::icon;
    using QFileIconProvider::type;
};

#endif // WIZFILEICONPROVIDER_H
