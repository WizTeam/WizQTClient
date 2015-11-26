#ifndef CWIZNOTEMANAGER_H
#define CWIZNOTEMANAGER_H

#include "wizdef.h"

/*
 * CWizNoteManager用于管理笔记相关的操作，例如创建、删除等。需要在程序启动时进行初始化
 */
class CWizNoteManager
{
public:
    //before use instance, should create singleton manually
    static CWizNoteManager* instance();
    static bool createSingleton(CWizExplorerApp& app);


    // create note
    void createIntroductionNoteForNewRegisterAccount();








private:
    CWizNoteManager(CWizExplorerApp& app);

    class CGarbo
    {
    public:
        ~CGarbo()
        {
            if(m_instance)
                delete(m_instance);
        }
        CGarbo()
        {}
    };

    static CGarbo garbo;

    //
    CWizExplorerApp& m_app;
    static CWizNoteManager* m_instance;
};

#endif // CWIZNOTEMANAGER_H
