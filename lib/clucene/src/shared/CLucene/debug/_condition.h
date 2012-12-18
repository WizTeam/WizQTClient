/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef __CONDITION_H
#define __CONDITION_H

//todo: this is a hack
#undef CND_PRECONDITION

/*
To enable condition debugging uncomment _CND_DEBUG in CLConfig.h
*/

#ifdef _CL__CND_DEBUG	/* Don't include the debug code */
    /* _CL__CND_DEBUG defined, include debug code */

	#ifdef _CND_NODEBUGTEXT
		#define CND_PRECONDITION(cond,usermessage)				CND__EXITCONDITION(cond,__FILE__,__LINE__,CND_STR_PRECONDITION,NULL)
		#define CND_CONDITION(cond,usermessage)					CND__EXITCONDITION(cond,__FILE__,__LINE__,CND_STR_CONDITION,NULL)
		#define CND_WARNING(cond,usermessage)					CND__CONDITION(cond,__FILE__,__LINE__,CND_STR_WARNING,NULL)
		#define CND_MESSAGE(cond,usermessage)					CND__CONDITION(cond,__FILE__,__LINE__,CND_STR_MESSAGE,NULL)
		#define CND_DEBUGMESSAGE(usermessage)					CND__MESSAGE(__FILE__,__LINE__,CND_STR_DEBUGMESSAGE,NULL)
	#else
		#define CND_PRECONDITION(cond,usermessage)				CND__EXITCONDITION(cond,__FILE__,__LINE__,CND_STR_PRECONDITION,usermessage)
		#define CND_CONDITION(cond,usermessage)					CND__EXITCONDITION(cond,__FILE__,__LINE__,CND_STR_CONDITION,usermessage)
		#define CND_WARNING(cond,usermessage)					CND__CONDITION(cond,__FILE__,__LINE__,CND_STR_WARNING,usermessage)
		#define CND_MESSAGE(cond,usermessage)					CND__CONDITION(cond,__FILE__,__LINE__,CND_STR_MESSAGE,usermessage)
		#define CND_DEBUGMESSAGE(usermessage)					CND__MESSAGE(__FILE__,__LINE__,CND_STR_DEBUGMESSAGE,usermessage)
	#endif

	//if _CND_DEBUG_DONTIMPLEMENT_OUTDEBUG is defined, then you must implement
	//this routine in the client application. The debug callback can then
	//be better customised to the host application.
	//Here is the default implementation:
	void _Cnd_OutDebug( const char* FormattedMsg, const char* StrTitle, const char* File, int32_t Line, int32_t Title, const char* Mes2, int32_t fatal );

	void CLUCENE_SHARED_EXPORT __cnd_FormatDebug( const char* File, int32_t Line, int32_t Title, const char* Mes2, int32_t fatal );
	#define CND__EXIT(file,line,title,mes2)						{__cnd_FormatDebug(file,line,title,mes2,1);}
	#define CND__EXITCONDITION(cond,file,line,title,mes2)		{if(!(cond)){__cnd_FormatDebug(file,line,title,mes2,1);}}
	#define CND__CONDITION(cond,file,line,title,mes2)			{if(!(cond)){__cnd_FormatDebug(file,line,title,mes2,0);}}
	#define CND__MESSAGE(file,line,title,mes2)					{__cnd_FormatDebug(file,line,title,mes2,0);}
#else
    #define CND_PRECONDITION(cond, usermessage)
    #define CND_CONDITION(cond, usermessage)
    #define CND_WARNING(cond,usermessage)
    #define CND_MESSAGE(cond,usermessage)
    #define CND_DEBUGMESSAGE(usermessage)
#endif

#ifndef CND_STR_DEFINES
    #define CND_STR_DEFINES
    #define CND_STR_PRECONDITION								1
    #define CND_STR_CONDITION									2
    #define CND_STR_WARNING										3
    #define CND_STR_MESSAGE										4
    #define CND_STR_DEBUGMESSAGE								5
    #define CND_STR_EXIT										6
#endif

//cnd-debug exit command
#ifndef debugFatalExit
 #define debugFatalExit(ret) exit(ret)
#endif


#endif
