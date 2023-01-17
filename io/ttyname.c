/*
 * ttyname() implementation for OS/2 kLIBc
 *
 * Copyright (C) 2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#include <os2.h>

#include <unistd.h>
#include <errno.h>

#define DEV_CON 0
#define DEV_NUL 1
#define DEV_CLK 2

/* ttyname() of OS/2 kLIBC is just a stub. */
char *ttyname( int fd )
{
    static char *ttynames[] = { "/dev/con", "/dev/nul", "/dev/clock$" };

    ULONG type;
    ULONG attr;
    ULONG rc;

    rc = DosQueryHType( fd, &type, &attr );
    if( rc )
    {
        errno = EBADF;
        return NULL;
    }

    if( type == HANDTYPE_DEVICE )
    {
        if( attr & 3 )     /* 1 = KBD$, 2 = SCREEN$ */
            return ttynames[ DEV_CON ];

        if( attr & 4 )     /* 4 = NUL */
            return ttynames[ DEV_NUL ];

        if( attr & 8 )     /* 8 = CLOCK$ */
            return ttynames[ DEV_CLK ];

        errno = ENODEV;
        return NULL;
    }

    errno = ENOTTY;
    return NULL;
}
