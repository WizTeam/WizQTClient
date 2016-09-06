#ifndef WIZSERVICE_NOTECOMMENTS_P_H
#define WIZSERVICE_NOTECOMMENTS_P_H

#include <QObject>
#include <QMap>

class QString;

struct WIZDOCUMENTDATA;

class WizDocumentView;

class WizNoteCommentsPrivate : public QObject
{
    Q_OBJECT

public:
    explicit WizNoteCommentsPrivate();
    ~WizNoteCommentsPrivate();

private:
    QMap<WizDocumentView*, WIZDOCUMENTDATA> m_map;

    void loadComments(const QString& strToken);

public Q_SLOTS:
    void onTokenAcquired(const QString& strToken);
    void onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATA& note);
    void onCommentsButtonClicked();

};



#endif // WIZSERVICE_NOTECOMMENTS_P_H
