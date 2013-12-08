#ifndef WIZSERVICE_NOTECOMMENTS_P_H
#define WIZSERVICE_NOTECOMMENTS_P_H

#include <QObject>
#include <QMap>

class QString;

struct WIZDOCUMENTDATA;
class CWizDocumentView;

namespace WizService {
namespace Internal {

class NoteCommentsPrivate : public QObject
{
    Q_OBJECT

public:
    explicit NoteCommentsPrivate();
    ~NoteCommentsPrivate();

private:
    QMap<CWizDocumentView*, WIZDOCUMENTDATA> m_map;

    void loadComments(const QString& strToken);

public Q_SLOTS:
    void onTokenAcquired(const QString& strToken, const QString& strMsg);
    void onViewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& note);
    void onCommentsButtonClicked();

};

} // namespace Internal
} // namespace WizService


#endif // WIZSERVICE_NOTECOMMENTS_P_H
