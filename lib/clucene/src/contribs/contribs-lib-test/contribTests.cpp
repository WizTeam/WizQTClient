/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

unittest tests[] = {
    {"analysis", testanalysis},
    {"snowball", testsnowball},
    {"highlighter",testhighlighter},
    {"streams",teststreams},
    {"utf8",testutf8},
    {"LastTest", NULL}
};
