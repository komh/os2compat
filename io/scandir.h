/*
 * scandir() and alphasort() implementation for OS/2 kLIBC
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_SCANDIR_H
#define OS2COMPAT_SCANDIR_H

#include <dirent.h> /* struct dirent, scandir() */

#ifdef __cplusplus
extern "C" {
#endif

int scandir( const char *dir, struct dirent ***namelist,
             int ( *sel )(/* const */ struct dirent * ),
             int ( *compare )( const /* struct dirent ** */ void *,
                               const /* struct dirent ** */ void *));

int alphasort( const /* struct dirent **d1 */ void *p1,
               const /* struct dirent **d2 */ void *p2);

#ifdef __cplusplus
}
#endif

#endif

