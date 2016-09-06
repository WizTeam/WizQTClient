#ifndef WIZSERVICE_NOTECOMMENTS_H
#define WIZSERVICE_NOTECOMMENTS_H

#include <QObject>

class WizNoteComments : public QObject
{
    Q_OBJECT

    explicit WizNoteComments();

public:
    static void init();

};


#endif // WIZSERVICE_NOTECOMMENTS_H
