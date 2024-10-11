/*
 * getrusage() implementation for OS/2 kLIBC
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#include <os2.h>

/* OS/2 kLIBC has declarations for getrusage() */
#include <sys/types.h> /* id_t */
#include <sys/resource.h>

#include <errno.h>
#include <string.h>
#include <process.h>

static int getproctime( PID pid, PULONG pulstime, PULONG pulutime )
{
    char buf[ 1024 ];
    QSPTRREC *pstate;
    QSPREC *pproc;
    QSTREC *pthread;
    ULONG ulstime;
    ULONG ulutime;
    int i;

    if( DosQuerySysState( QS_PROCESS, 0, pid, 0, buf, sizeof( buf )))
    {
        errno = EINVAL;

        return -1;
    }

    pstate = ( QSPTRREC * )buf;
    pproc = pstate->pProcRec;
    pthread = pproc->pThrdRec;
    ulstime = 0;
    ulutime = 0;

    /* Accumulate time of all threads */
    for( i = pproc->cTCB; i > 0; i-- )
    {
        ulstime += pthread->systime;
        ulutime += pthread->usertime;

        pthread++;
    }

    *pulstime = ulstime;
    *pulutime = ulutime;

    return 0;
}

/**
 * getrusage()
 *
 * @remark Support RUSAGE_SELF only
 */
int getrusage (int who, struct rusage *r_usage)
{
    if( who != RUSAGE_SELF && who != RUSAGE_CHILDREN )
    {
        errno = EINVAL;

        return -1;
    }

    /* Intialize members of struct rusage */
    memset( r_usage, 0, sizeof( *r_usage ));

    if( who == RUSAGE_SELF )
    {
        ULONG ulstime;
        ULONG ulutime;

        if( getproctime( getpid(), &ulstime, &ulutime ) == -1 )
            return -1;

        r_usage->ru_utime.tv_sec = ulutime / 1000;
        r_usage->ru_utime.tv_usec = ( ulutime % 1000 ) * 1000;
        r_usage->ru_stime.tv_sec = ulstime / 1000;
        r_usage->ru_stime.tv_usec = ( ulstime % 1000 ) * 1000;
    }

    return 0;
}
