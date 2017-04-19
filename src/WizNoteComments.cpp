#include "WizNoteComments.h"
#include "WizNoteComments_p.h"

#include <QDebug>

#include "WizDocumentView.h"
#include "share/WizObject.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"

static WizNoteCommentsPrivate* g_comments = 0;

WizNoteCommentsPrivate::WizNoteCommentsPrivate()
{
    //connect(WizGlobal::instance(), SIGNAL(viewNoteLoaded(CWizDocumentView*, const WIZDOCUMENTDATA&)),
    //        SLOT(onViewNoteLoaded(CWizDocumentView*, const WIZDOCUMENTDATA&)));
    //connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)));
}

WizNoteCommentsPrivate::~WizNoteCommentsPrivate()
{
    g_comments = 0;
}

void WizNoteCommentsPrivate::onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& note)
{
    m_map.insert(view, note);

    WizToken::requestToken();
}

void WizNoteCommentsPrivate::onTokenAcquired(const QString& strToken)
{
    if (strToken.isEmpty())
        return;

    loadComments(strToken);
}

void WizNoteCommentsPrivate::onCommentsButtonClicked()
{
}

void WizNoteCommentsPrivate::loadComments(const QString& strToken)
{
    QMap<WizDocumentView*, WIZDOCUMENTDATAEX>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++) {
        qDebug() << WizCommonApiEntry::commentUrl(strToken, it.value().strKbGUID, it.value().strGUID);
    }
}



WizNoteComments::WizNoteComments()
{
}

void WizNoteComments::init()
{
    if (!g_comments) {
        g_comments = new WizNoteCommentsPrivate();
    }
}


