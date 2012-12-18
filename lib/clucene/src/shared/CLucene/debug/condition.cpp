/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#define _CL__CND_DEBUG
#include "CLucene/_SharedHeader.h"
#include "_condition.h"
#include "CLucene/util/Misc.h"

#define __CND_STR_PRECONDITION    "PRECONDITION"
#define __CND_STR_CONDITION       "CONDITION"
#define __CND_STR_WARNING         "WARNING"
#define __CND_STR_MESSAGE         "MESSAGE"
#define __CND_STR_DEBUGMESSAGE    "DEBUG MESSAGE"
#define __CND_STR_EXIT            "EXIT"

#ifndef _CND_DEBUG_DONTIMPLEMENT_OUTDEBUG
void _Cnd_OutDebug( const char* FormattedMsg, const char* StrTitle, const char* File, int32_t Line, int32_t Title, const char* Mes2, int32_t fatal ){
	#ifdef __WINDOWS_H
			/*Display a standard messagebox*/
 			MessageBox(NULL, FormattedMsg, StrTitle, (fatal==1 ? MB_ICONSTOP:MB_ICONEXCLAMATION) | MB_OK | MB_TASKMODAL);
	#else
			printf("%s\n",FormattedMsg);
	#endif

	#if defined(_CND_DEBUG_WARN_DEBUGGER) /*attempt to signal windows debugger*/
			OutputDebugString(FormattedMsg);
			DebugBreak(); /*Position debugger just before exit program*/
	#endif

	if ( fatal )
		debugFatalExit(1);
}
#endif

void __cnd_FormatDebug( const char* File, int32_t Line, int32_t Title, const char* Mes2, int32_t fatal ) {
	char M[512];
    const char* StrTitle = NULL;

	if( Mes2 )
		_snprintf(M,512,"file:%s line:%d\n%s",File,Line,Mes2);
	else
		_snprintf(M,512,"file:%s line:%d",File,Line);

    /*Determine which title to use*/
    switch( Title ) {
        case CND_STR_PRECONDITION: {
            StrTitle = __CND_STR_PRECONDITION;
            break;
            }
        case CND_STR_CONDITION: {
            StrTitle = __CND_STR_CONDITION;
            break;
            }
        case CND_STR_WARNING: {
            StrTitle = __CND_STR_WARNING;
            break;
            }
        case CND_STR_MESSAGE: {
	        StrTitle = __CND_STR_MESSAGE;
            break;
            }
        case CND_STR_DEBUGMESSAGE: {
            StrTitle = __CND_STR_DEBUGMESSAGE;
            break;
            }
        case CND_STR_EXIT: {
            StrTitle = __CND_STR_EXIT;
            break;
            }
        default:
            break;
        }/*switch*/

	_Cnd_OutDebug(M, StrTitle, File, Line, Title, Mes2, fatal);
}
