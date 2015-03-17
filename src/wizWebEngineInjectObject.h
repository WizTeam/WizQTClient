#ifndef CWIZWEBENGINEINJECTOBJECT_H
#define CWIZWEBENGINEINJECTOBJECT_H

#include <QObject>

class WizCodeEditorDialog;
class CWizCodeExternal : public QObject
{
    Q_OBJECT
public:
    explicit CWizCodeExternal(WizCodeEditorDialog* editor, QObject* parent);

    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();
    Q_INVOKABLE void insertHtml(const QString& strResult);
    Q_INVOKABLE QString getLastCodeType();
    Q_INVOKABLE void saveLastCodeType(const QString& codeType);

private:
    WizCodeEditorDialog* m_editor;
};

class CWizCommentsExternal : public QObject
{
    Q_OBJECT
public:
    explicit CWizCommentsExternal(QObject* parent);

    Q_INVOKABLE void openUrl(const QString& strUrl);
};

#endif // CWIZWEBENGINEINJECTOBJECT_H
