#ifndef WIZSERVICE_NOTECOMMENTS_H
#define WIZSERVICE_NOTECOMMENTS_H

#include <QObject>

namespace WizService {

class NoteComments : public QObject
{
    Q_OBJECT

    explicit NoteComments();

public:
    static void init();

};

} // namespace WizService

#endif // WIZSERVICE_NOTECOMMENTS_H
