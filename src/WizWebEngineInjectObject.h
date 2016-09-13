#ifndef CWIZWEBENGINEINJECTOBJECT_H
#define CWIZWEBENGINEINJECTOBJECT_H

#include <QObject>

class WizCodeEditorDialog;
class WizCodeExternal : public QObject
{
    Q_OBJECT
public:
    explicit WizCodeExternal(WizCodeEditorDialog* editor, QObject* parent);

    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();
    Q_INVOKABLE void insertHtml(const QString& strResult);
    Q_INVOKABLE QString getLastCodeType();
    Q_INVOKABLE void saveLastCodeType(const QString& codeType);

private:
    WizCodeEditorDialog* m_editor;
};

class WizCommentsExternal : public QObject
{
    Q_OBJECT
public:
    explicit WizCommentsExternal(QObject* parent);

    Q_INVOKABLE void openUrl(const QString& strUrl);
};

#endif // CWIZWEBENGINEINJECTOBJECT_H
