// Filename:-	files.h
//

#include "stdafx.h"
#include "includes.h"
//#include <io.h>
#include "R_Common.h"
//#include "R_Image.h"
//
#include "files.h"


/*
============
Com_StringContains
============
*/
char *Com_StringContains(char *str1, char *str2, int casesensitive) {
	int len, i, j;

	len = strlen(str1) - strlen(str2);
	for (i = 0; i <= len; i++, str1++) {
		for (j = 0; str2[j]; j++) {
			if (casesensitive) {
				if (str1[j] != str2[j]) {
					break;
				}
			}
			else {
				if (toupper(str1[j]) != toupper(str2[j])) {
					break;
				}
			}
		}
		if (!str2[j]) {
			return str1;
		}
	}
	return NULL;
}

/*
============
Com_Filter
============
*/
int Com_Filter(char *filter, char *name, int casesensitive)
{
	char buf[MAX_TOKEN_CHARS];
	char *ptr;
	int i, found;

	while(*filter) {
		if (*filter == '*') {
			filter++;
			for (i = 0; *filter; i++) {
				if (*filter == '*' || *filter == '?') break;
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if (strlen(buf)) {
				ptr = Com_StringContains(name, buf, casesensitive);
				if (!ptr) return qfalse;
				name = ptr + strlen(buf);
			}
		}
		else if (*filter == '?') {
			filter++;
			name++;
		}
		else if (*filter == '[' && *(filter+1) == '[') {
			filter++;
		}
		else if (*filter == '[') {
			filter++;
			found = qfalse;
			while(*filter && !found) {
				if (*filter == ']' && *(filter+1) != ']') break;
				if (*(filter+1) == '-' && *(filter+2) && (*(filter+2) != ']' || *(filter+3) == ']')) {
					if (casesensitive) {
						if (*name >= *filter && *name <= *(filter+2)) found = qtrue;
					}
					else {
						if (toupper(*name) >= toupper(*filter) &&
							toupper(*name) <= toupper(*(filter+2))) found = qtrue;
					}
					filter += 3;
				}
				else {
					if (casesensitive) {
						if (*filter == *name) found = qtrue;
					}
					else {
						if (toupper(*filter) == toupper(*name)) found = qtrue;
					}
					filter++;
				}
			}
			if (!found) return qfalse;
			while(*filter) {
				if (*filter == ']' && *(filter+1) != ']') break;
				filter++;
			}
			filter++;
			name++;
		}
		else {
			if (casesensitive) {
				if (*filter != *name) return qfalse;
			}
			else {
				if (toupper(*filter) != toupper(*name)) return qfalse;
			}
			filter++;
			name++;
		}
	}
	return qtrue;
}

/*
============
Com_FilterPath
============
*/
int Com_FilterPath(char *filter, char *name, int casesensitive)
{
	int i;
	char new_filter[MAX_QPATH];
	char new_name[MAX_QPATH];

	for (i = 0; i < MAX_QPATH-1 && filter[i]; i++) {
		if ( filter[i] == '\\' || filter[i] == ':' ) {
			new_filter[i] = '/';
		}
		else {
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';
	for (i = 0; i < MAX_QPATH-1 && name[i]; i++) {
		if ( name[i] == '\\' || name[i] == ':' ) {
			new_name[i] = '/';
		}
		else {
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';
	return Com_Filter(new_filter, new_name, casesensitive);
}



void *Z_Malloc(int size)
{
	return malloc(size);
}
void Z_Free(void *mem)
{
	free(mem);
}

void *S_Malloc(int size)
{
	return Z_Malloc(size);
}


/*
=================================================================================

DIRECTORY SCANNING FUNCTIONS

=================================================================================
*/

#define	MAX_FOUND_FILES	0x1000

char *CopyString( const char *in ) {
	char	*out;
	
	out = (char *)S_Malloc (strlen(in)+1);
	strcpy (out, in);
	return out;
}

#include <QtCore/QDir>
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs ) {
	char		search[MAX_QPATH];
	int			nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];

	if ( !extension ) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
	}

	Com_sprintf( search, sizeof(search), "%s/*%s", directory, extension );

	// search
	nfiles = 0;

    QDir root (search);
    if ( !root.exists() )
    {
        *numfiles = 0;
        return NULL;
    }

    QDir::Filters filters = QDir::Files;
    if ( wantsubs )
    {
        filters |= QDir::Dirs;
    }

    QStringList files = root.entryList (filters);
    nfiles = min (files.size(), MAX_FOUND_FILES - 1);
    for ( int i = 0; i < nfiles; i++ )
    {
        list[i] = CopyString (files[i].toLatin1());
    }

	listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
	for ( int i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[nfiles] = NULL;

	return listCopy;
}

void	Sys_FreeFileList( char **flist ) {
	int		i;

	if ( !flist ) {
		return;
	}

	for ( i = 0 ; flist[i] ; i++ ) {
		Z_Free( flist[i] );
	}

	Z_Free( flist );
}
