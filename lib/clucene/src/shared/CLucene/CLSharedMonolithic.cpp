/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
/*
* this is a monolithic file that can be used to compile clucene using one source file.
* it simplifies some build processes by avoiding static & dynamic compalation pitfalls.
* 
* note: when creating a project add either this file, or all the other .cpp files, not both!
*/
#include "CLucene/SharedHeader.cpp"
#include "CLucene/config/gunichartables.cpp"
#include "CLucene/config/repl_tcscasecmp.cpp"
#include "CLucene/config/repl_tcslwr.cpp"
#include "CLucene/config/repl_tcstod.cpp"
#include "CLucene/config/repl_lltot.cpp"
#include "CLucene/config/repl_tcstoll.cpp"
#include "CLucene/config/repl_tprintf.cpp"
#include "CLucene/config/threads.cpp"
#include "CLucene/config/utf8.cpp"
#include "CLucene/debug/condition.cpp"
#include "CLucene/util/Misc.cpp"
#include "CLucene/util/dirent.cpp"
#include "CLucene/util/StringBuffer.cpp"
