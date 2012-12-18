/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
//msvc test for memory leaks:
#ifdef _MSC_VER
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include "test.h"
#include <stdlib.h>

#include <fcntl.h>
#ifdef _CL_HAVE_DIRECT_H
#include <direct.h>
#endif
#ifdef _CL_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef _CL_HAVE_IO_H
#include <io.h>
#endif

const char* cl_tempDir;
bool cl_quiet;
char clucene_data_location[1024];

int main(int argc, char *argv[]) {
#ifdef _MSC_VER
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF );
    _crtBreakAlloc = -1;
#endif
#endif
    int ret_result = 0;
    int i = 0;
    int exclude = 0;
    int list_provided = 0;
    CuSuiteList *alltests = NULL;
    CuString *output = CuStringNew();
    bool silent = false;
    bool verbose = false;
    bool times = true;
    uint64_t startTime = 0;

    cl_tempDir = NULL;
    if (Misc::dir_Exists("/tmp"))
        cl_tempDir = "/tmp";
    if (getenv("TEMP") != NULL)
        cl_tempDir = getenv("TEMP");
    else if (getenv("TMP") != NULL)
        cl_tempDir = getenv("TMP");

    char* tmp = _CL_NEWARRAY(char, strlen(cl_tempDir) + 9);
    strcpy(tmp, cl_tempDir);
    strcat(tmp, "/clucene");
    _mkdir(tmp);
    if (Misc::dir_Exists(tmp))
        cl_tempDir = tmp;

    clucene_data_location[0] = 0;
    if (CL_NS(util)::Misc::dir_Exists(CLUCENE_DATA_LOCATION1 "/reuters-21578-index/segments"))
        strcpy(clucene_data_location, CLUCENE_DATA_LOCATION1);
    else if (CL_NS(util)::Misc::dir_Exists(CLUCENE_DATA_LOCATION2 "/reuters-21578-index/segments"))
        strcpy(clucene_data_location, CLUCENE_DATA_LOCATION2);
    else if (CL_NS(util)::Misc::dir_Exists(CLUCENE_DATA_LOCATION3 "/reuters-21578-index/segments"))
        strcpy(clucene_data_location, CLUCENE_DATA_LOCATION3);
    else if (getenv(CLUCENE_DATA_LOCATIONENV) != NULL) {
        strcpy(clucene_data_location, getenv(CLUCENE_DATA_LOCATIONENV));
        strcat(clucene_data_location, "/data/reuters-21578-index/segments");
        if (CL_NS(util)::Misc::dir_Exists(clucene_data_location)) {
            strcpy(clucene_data_location, getenv(CLUCENE_DATA_LOCATIONENV));
            strcat(clucene_data_location, "/data");
        } else
            clucene_data_location[0] = 0;
    }

    /* first check that we are running the test for the correct position */
    //todo: make this configurable
    if (!*clucene_data_location) {
        fprintf(stderr, "%s must be run from a subdirectory of the application's root directory\n", argv[0]);
        fprintf(stderr, "ensure that the test data exists in %s or %s or %s\n", CLUCENE_DATA_LOCATION1, CLUCENE_DATA_LOCATION2, CLUCENE_DATA_LOCATION3);
        if (getenv(CLUCENE_DATA_LOCATIONENV) != NULL)
            fprintf(stderr, "%s/data was also checked because of the " CLUCENE_DATA_LOCATIONENV " environment variable", getenv(CLUCENE_DATA_LOCATIONENV));
        ret_result = 1;
        goto exit_point;
    }

    CuInit(argc, argv);

    /* see if we're in exclude mode, see if list of testcases provided */
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "/?")) {
            printf("%s [-l list] [-q quiet] [-v verbose] [-t show times] [-p printf messages immediatelly] [-x exclude] [tests...]\n", argv[0]);
            goto exit_point;
        }
        if (!strcmp(argv[i], "-v")) {
            verbose = true;
            continue;
        }
        if (!strcmp(argv[i], "-p")) { //used in CuInit
            continue;
        }
        if (!strcmp(argv[i], "-q")) {
            silent = true;
            continue;
        }
        if (!strcmp(argv[i], "-x")) {
            exclude = 1;
            continue;
        }
        if (!strcmp(argv[i], "-t")) {
            times = true;
            continue;
        }
        if (!strcmp(argv[i], "-l")) {
            for (i = 0; tests[i].func != NULL; i++) {
                printf("%s\n", tests[i].testname);
            }
            ret_result = 0;
            goto exit_point;
        }
        if (argv[i][0] == '-') {
            fprintf(stderr, "invalid option: `%s'\n", argv[i]);
            ret_result = 1;
            goto exit_point;
        }
        list_provided = 1;
    }

    if (!list_provided) {
        /* add everything */
        alltests = CuSuiteListNew(_T("All CLucene Tests"));
        for (i = 0; tests[i].func != NULL; i++) {
            CuSuiteListAdd(alltests, tests[i].func());
        }
    } else if (exclude) {
        /* add everything but the tests listed */
        alltests = CuSuiteListNew(_T("Partial CLucene Tests"));
        for (i = 0; tests[i].func != NULL; i++) {
            int this_test_excluded = 0;
            int j;

            for (j = 1; j < argc && !this_test_excluded; j++) {
                if (!strcmp(argv[j], tests[i].testname)) {
                    this_test_excluded = 1;
                }
            }
            if (!this_test_excluded) {
                CuSuiteListAdd(alltests, tests[i].func());
            }
        }
    } else {
        /* add only the tests listed */
        alltests = CuSuiteListNew(_T("Partial CLucene Tests"));
        for (i = 1; i < argc; i++) {
            int j;
            int found = 0;

            if (argv[i][0] == '-') {
                continue;
            }
            for (j = 0; tests[j].func != NULL; j++) {
                if (!strcmp(argv[i], tests[j].testname)) {
                    CuSuiteListAdd(alltests, tests[j].func());
                    found = 1;
                }
            }
            if (!found) {
                fprintf(stderr, "invalid test name: `%s'\n", argv[i]);
                ret_result = 1;
                goto exit_point;
            }
        }
    }

    startTime = Misc::currentTimeMillis();

    printf("Key: .= pass N=not implemented F=fail\n");
    if (silent)
        CuSuiteListRun(alltests);
    else
        CuSuiteListRunWithSummary(alltests, verbose, times);
    i = CuSuiteListDetails(alltests, output);
    _tprintf(_T("%s\n"), output->buffer);

    if (times)
        printf("Tests run in %dms\n\n", (int32_t) (CL_NS(util)::Misc::currentTimeMillis() - startTime));

exit_point:
    if (alltests != NULL)
        CuSuiteListDelete(alltests);
    CuStringFree(output);
    _CLDELETE_LCaARRAY(const_cast<char *>(cl_tempDir));
    cl_tempDir = NULL;

    _lucene_shutdown(); //clears all static memory
    //print lucenebase debug

    if (ret_result != 0)
        return ret_result;
    else
        return i > 0 ? 1 : 0;

    //Debuggin techniques:
    //For msvc, use this for breaking on memory leaks:
    //	_crtBreakAlloc
    //for linux, use valgrind
}

