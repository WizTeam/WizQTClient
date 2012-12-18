/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Matt J. Weinstein
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef lucene_util_dirent_H
#define lucene_util_dirent_H


#if !defined(_CL_HAVE_DIRENT_H) && !defined(_CL_HAVE_SYS_NDIR_H) && !defined(_CL_HAVE_SYS_DIR_H) && !defined(_CL_HAVE_NDIR_H)

#ifdef  _WIN64
	typedef __int64             intptr_t;
#else
	typedef int intptr_t;
#endif
#include <io.h>


/**
 * dirent.c
 *
 * Derived from DIRLIB.C by Matt J. Weinstein 
 * This note appears in the DIRLIB.H
 * DIRLIB.H by M. J. Weinstein   Released to public domain 1-Jan-89
 *
 * Updated by Jeremy Bettis <jeremy@hksys.com>
 * Significantly revised and rewinddir, seekdir and telldir added by Colin
 * Cut down again & changed by Ben van Klinken
 * Peters <colin@fu.is.saga-u.ac.jp>
 *
 */
 
/** dirent structure - used by the dirent.h directory iteration functions */
struct CLUCENE_SHARED_INLINE_EXPORT dirent
{
	unsigned short	d_namlen;	/* Length of name in d_name. */
	char *d_name;		/* File name. */
};

/** DIR structure - used by the dirent.h directory iteration functions*/
struct CLUCENE_SHARED_INLINE_EXPORT DIR
{
	/** disk transfer area for this dir */
	struct _finddata_t dd_dta;

	/* dirent struct to return from dir (NOTE: this makes this thread
	 * safe as long as only one thread uses a particular DIR struct at
	 * a time) */
	struct dirent		dd_dir;

	/** _findnext handle */
	intptr_t			dd_handle;

	/**
         * Status of search:
	 *   0 = not started yet (next entry to read is first entry)
	 *  -1 = off the end
	 *   positive = 0 based index of next entry
	 */
	int32_t			dd_stat;
	
	/** given path for dir with search pattern (struct is extended) */
	char			dd_name[CL_MAX_DIR];

};

#define DIRENT_SEARCH_SUFFIX "*"
#define DIRENT_SLASH PATH_DELIMITERA


/**
* Returns a pointer to a DIR structure appropriately filled in to begin
* searching a directory.
*/
CLUCENE_SHARED_EXPORT DIR* opendir (const char* filespec);

/**
* Return a pointer to a dirent structure filled with the information on the
* next entry in the directory.
*/
CLUCENE_SHARED_EXPORT struct dirent*	readdir (DIR* dir);

/**
* Frees up resources allocated by opendir.
*/
CLUCENE_SHARED_EXPORT int32_t	closedir (DIR* dir);


#elif defined (_CL_HAVE_DIRENT_H)
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)

#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if defined(_CL_HAVE_SYS_NDIR_H)
#  include <sys/ndir.h>
# endif
# if defined(_CL_HHAVE_SYS_DIR_H)
#  include <sys/dir.h>
# endif
# if defined(_CL_HHAVE_NDIR_H)
#  include <ndir.h>
# endif

#endif //HAVE_DIRENT_H 
#endif
