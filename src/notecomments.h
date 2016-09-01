#ifndef WIZSERVICE_NOTECOMMENTS_H
#define WIZSERVICE_NOTECOMMENTS_H

#include <QObject>

class NoteComments : public QObject
{
    Q_OBJECT

    explicit NoteComments();

public:
    static void init();

};


#endif // WIZSERVICE_NOTECOMMENTS_H
