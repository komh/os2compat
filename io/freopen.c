/*
 * freopen() workarounded for OS/2 kLIBC
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <errno.h>

/* alias */
FILE *_std_freopen( const char *fname, const char *mode, FILE *stream );

/* freopen() fails when fname is NULL even if it succeeds */
FILE *freopen( const char *fname, const char *mode, FILE *stream )
{
    FILE *result;

    errno = 0;

    result = _std_freopen( fname, mode, stream );

    if( !fname && !result && !errno )
        result = stream;

    return result;
}
