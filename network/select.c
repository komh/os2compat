/*
 * select() supporting files, pipes and sockets for OS/2 kLIBC
 *
 * Copyright (C) 2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_KBD
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <process.h>
#include <io.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>

#include <sys/builtin.h>

#define PIPE_NAME_BASE "\\PIPE\\OS2COMPAT\\NETWORK\\SELECT"

int _std_fcntl( int handle, int request, ... );

/* override libc fcntl() */
int fcntl( int handle, int request, ... )
{
    va_list argPtr;
    int     arg;
    ULONG   ulState;

    va_start( argPtr, request );
    arg = va_arg( argPtr, int );
    va_end( argPtr );

    /* named pipe */
    if( !DosQueryNPHState( handle, &ulState ))
    {
        if( request == F_GETFL )
            return ( ulState & NP_NOWAIT ) ? O_NONBLOCK : 0;

        if( request == F_SETFL )
        {
            if( arg == O_NONBLOCK )
                ulState |= NP_NOWAIT;
            else
                ulState &= ~NP_NOWAIT;

            /* extract relevant flags */
            ulState &= NP_NOWAIT | NP_READMODE_MESSAGE;

            return DosSetNPHState( handle, ulState ) ? -1 : 0;
        }
    }

    return _std_fcntl( handle, request, arg );
}

/* override libc pipe() */
int pipe( int *ph )
{
    static volatile unsigned pipes = 0;

    HPIPE hpipe;
    HFILE hpipeWrite;
    ULONG ulAction;

    char szPipeName[ 80 ];

    __atomic_increment( &pipes );
    snprintf( szPipeName, sizeof( szPipeName ),
              "%s\\%x\\%x", PIPE_NAME_BASE, ( unsigned )getpid(), pipes );

    /* NP_NOWAIT should be specified, otherwise DosConnectNPipe() blocks.
     * If you want to change pipes to blocking-mode, then use DosSetNPHState()
     * after DosConnectNPipe()
     */
    DosCreateNPipe( szPipeName,
                    &hpipe,
                    NP_ACCESS_INBOUND | NP_INHERIT,
                    NP_NOWAIT | NP_TYPE_BYTE | NP_READMODE_BYTE | 1,
                    32768, 32768, 0 );

    DosConnectNPipe( hpipe );

    DosOpen( szPipeName, &hpipeWrite, &ulAction, 0, FILE_NORMAL,
             OPEN_ACTION_OPEN_IF_EXISTS,
             OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_WRITEONLY,
             NULL );

    /* _imphandle() is not required specifically, because kLIBC imports
     * native handles automatically if needed. But here use _imphandle()
     * specifically.
     */
    ph[ 0 ] = _imphandle( hpipe );
    ph[ 1 ] = _imphandle( hpipeWrite );

    /* default to blocking mode */
    DosSetNPHState( ph[ 0 ], NP_WAIT | NP_READMODE_BYTE );
    DosSetNPHState( ph[ 1 ], NP_WAIT | NP_READMODE_BYTE );

    return 0;
}

/* alias */
int _std_select( int, fd_set *, fd_set *, fd_set *, struct timeval * );

#define ENABLE_CONSOLE 0

#if ENABLE_CONSOLE
static HEV hevKbd;

__attribute__((constructor))
static void init( void )
{
    DosCreateEventSem( NULL, &hevKbd, DC_SEM_SHARED, FALSE );
}

__attribute__((destructor))
static void fini( void )
{
    DosCloseEventSem( hevKbd );
}

static int kbhit( void )
{
    KBDKEYINFO key;
    ULONG ulCount;

    if( KbdPeek( &key, 0 ) == 0 && ( key.fbStatus & KBDTRF_FINAL_CHAR_IN ))
    {
        DosPostEventSem( hevKbd );

        return 1;
    }

    DosResetEventSem( hevKbd, &ulCount );

    return 0;
}
#endif

/* select type */
#define ST_CONSOLE  0
#define ST_FILE     1
#define ST_PIPE     2
#define ST_SOCKET   3
#define ST_END      ST_SOCKET

/* select parameter */
typedef struct SELECTPARM
{
    int nmaxfds;
    int nfds[ 3 ];
    fd_set fdset[ 3 ];
} SELECTPARM, *PSELECTPARM;

/* FDSET operation */
#define FDSET_READ      0
#define FDSET_WRITE     1
#define FDSET_EXCEPT    2

static int setfd( int fd, PSELECTPARM parms, int op )
{
    struct stat st;
#if ENABLE_CONSOLE
    ULONG ulType, ulAttr;
#endif
    ULONG ulState;
    int type;

    if( fstat( fd, &st ) == -1 )
        return -1;

    /* console is not supported, yet */
#if ENABLE_CONSOLE
    if( S_ISCHR( st.st_mode ) &&
        DosQueryHType( fd, &ulType, &ulAttr ) == 0 &&
        LOBYTE( ulType ) == 1 /* character device */ &&
        ( ulAttr == 0x8083 /* CON */ || ulAttr == 0xC981 /* KBD */ ))
        type = ST_CONSOLE;
    else
#endif
    if( S_ISREG( st.st_mode ))
        type = ST_FILE;
    else if( S_ISFIFO( st.st_mode ) &&
             DosQueryNPHState( fd, &ulState ) == 0 /* named pipe */ &&
             (( op == FDSET_READ && ( ulState & NP_END_SERVER )) ||
              ( op == FDSET_WRITE && !( ulState & NP_END_SERVER ))))
        type = ST_PIPE;
    else if( S_ISSOCK( st.st_mode ))
        type = ST_SOCKET;
    else
        return errno = EINVAL, -1;

    /* accept exception fds if a file or a socket */
    if( type == ST_FILE || type == ST_SOCKET || op != FDSET_EXCEPT )
    {
        PSELECTPARM parm = &parms[ type ];

        if( parm->nmaxfds <= fd )
            parm->nmaxfds = fd + 1;

        if( parm->nfds[ op ] <= fd )
            parm->nfds[ op ] = fd + 1;

        FD_SET( fd, &parm->fdset[ op ]);
    }

    return 0;
}

/* pipe semaphore */
typedef struct PIPESEM
{
    int fd;
    HEV hev;
} PIPESEM, *PPIPESEM;

static int initpipesems( PSELECTPARM parm, PPIPESEM pipesems, int op )
{
    HEV hev;
    int nfds = parm->nfds[ op ];
    fd_set *fdset = &parm->fdset[ op ];
    char semname[ 260 ];
    PPIPEINFO pinfo;
    int n;
    int i;

    pinfo = alloca( sizeof( *pinfo ) + 260 );

    for( n = 0, i = 0; i < nfds; i++ )
    {
        if( FD_ISSET( i, fdset ))
        {
            DosQueryNPipeInfo( i, 1, pinfo, sizeof( *pinfo ) + 260 );
            snprintf( semname, sizeof( semname ),
                      "\\SEM32\\%s\\%d", pinfo->szName, op );

            hev = NULLHANDLE;
            if(( DosCreateEventSem( semname, &hev, 0, FALSE ) &&
                 DosOpenEventSem( semname, &hev )) ||
                DosSetNPipeSem( i, ( HSEM )hev, op ))
            {
                if( hev != NULLHANDLE )
                    DosCloseEventSem( hev );

                while( n > 0 )
                    DosCloseEventSem( pipesems[ --n ].hev );

                return errno = ENOMEM, -1;
            }

            pipesems[ n ].fd = i;
            pipesems[ n ].hev = hev;
            n++;
        }
    }

    return n;
}

static void termpipesems( int nfds, PPIPESEM pipesems )
{
    int i;

    for( i = 0; i < nfds; i++ )
        DosCloseEventSem( pipesems[ i ].hev );
}

static int checkpipesems( int nfds, PPIPESEM pipesems, fd_set *fdset,
                          int pending )
{
    ULONG ulCount;
    HEV hev;
    int fd;
    PIPESEMSTATE state[ 3 ] = {{ 0, }, };
    int n;
    int i, j;

    FD_ZERO( fdset );

    for( n = 0, i = 0; i < nfds; i++ )
    {
        fd = pipesems[ i ].fd;
        hev = pipesems[ i ].hev;

        if( pending )
        {
            DosQueryNPipeSemState(( HSEM )hev, state, sizeof( state ));
            DosResetEventSem( hev, &ulCount );

            for( j = 0; state[ j ].fStatus != NPSS_EOI; j++ )
            {
                if(( state[ j ].fStatus == NPSS_WSPACE &&
                     state[ j ].usKey == FDSET_WRITE ) ||
                   ( state[ j ].fStatus == NPSS_RDATA &&
                     state[ j ].usKey == FDSET_READ ))
                {
                    if( state[ j ].usAvail > 0 )
                        DosPostEventSem( hev );

                    break;
                }
            }
        }

        if( DosWaitEventSem( hev, SEM_IMMEDIATE_RETURN ) == 0 )
        {
            n++;
            FD_SET( fd, fdset );
        }
    }

    return n;
}

/* waitsocks() arguments */
typedef struct WAITSOCKSARGS
{
    PSELECTPARM parm;
    struct timeval *timeout;
    HEV hev;
} WAITSOCKSARGS, *PWAITSOCKSARGS;

static void waitsocks( void *arg )
{
    WAITSOCKSARGS wsa = *( PWAITSOCKSARGS )arg;
    int n;

    n = _std_select( wsa.parm->nmaxfds, &wsa.parm->fdset[ FDSET_READ ],
                     &wsa.parm->fdset[ FDSET_WRITE ],
                     &wsa.parm->fdset[ FDSET_EXCEPT ], wsa.timeout );

    if( n > 0 )
        DosPostEventSem( wsa.hev );
}

int select( int nfds, fd_set *rdset, fd_set *wrset, fd_set *exset,
            struct timeval *timeout )
{
    SELECTPARM parms[ ST_END + 1 ] = {{ 0, }, };
    int nowaitmode = timeout && timeout->tv_sec == 0 && timeout->tv_usec == 0;
    struct timeval nowait = { 0, 0 };
    int nrpipesems = 0, nwpipesems = 0;
    PIPESEM rpipesems[ nfds ], wpipesems[ nfds ];
    HEV hevSock = NULLHANDLE;
    int cancelsock = -1;
    int n = -1;
    int i, j;

    /* wait mode ? */
    if( !rdset && !wrset && !exset )
        return _std_select( 0, NULL, NULL, NULL, timeout );

    /* categorize and check fds */
    for( i = 0; i < nfds; i++ )
    {
        if( rdset && FD_ISSET( i, rdset ) &&
            setfd( i, parms, FDSET_READ ) == -1 )
            return -1;

        if( wrset && FD_ISSET( i, wrset ) &&
            setfd( i, parms, FDSET_WRITE ) == -1 )
            return -1;

        if( exset && FD_ISSET( i, exset ) &&
            setfd( i, parms, FDSET_EXCEPT ) == -1 )
            return -1;
    }

    /* check pending events */

#if ENABLE_CONSOLE
    /* check consoles */
    if(( parms[ ST_CONSOLE ].nfds[ FDSET_READ ] > 0 && kbhit()) ||
       parms[ ST_CONSOLE ].nfds[ FDSET_WRITE ] > 0 )
        timeout = &nowait;
#endif

    /* check the regular files which are always ready in any state */
    if( parms[ ST_FILE ].nmaxfds > 0 )
        timeout = &nowait;

    /* check pipes */
    if( parms[ ST_PIPE ].nmaxfds > 0 )
    {
        PSELECTPARM parm = &parms[ ST_PIPE ];
        fd_set rset, *prset = &rset;
        fd_set wset, *pwset = &wset;

        nrpipesems = initpipesems( parm, rpipesems, FDSET_READ );
        if( nrpipesems == -1 )
            return -1;

        nwpipesems = initpipesems( parm, wpipesems, FDSET_WRITE );
        if( nwpipesems == -1 )
            goto cleanup;

        FD_ZERO( &rset );
        FD_ZERO( &wset );

        if( nowaitmode )
        {
            prset = &parm->fdset[ FDSET_READ ];
            pwset = &parm->fdset[ FDSET_WRITE ];
        }

        if( checkpipesems( nrpipesems, rpipesems, prset, 1 ) +
            checkpipesems( nwpipesems, wpipesems, pwset, 1 ) > 0 )
        {
            if( !nowaitmode )
            {
                parm->fdset[ FDSET_READ ] = *prset;
                parm->fdset[ FDSET_WRITE ] = *pwset;
                /* exception is not supported */
            }

            timeout = &nowait;
        }
    }

    /* check sockets */
    if( parms[ ST_SOCKET ].nmaxfds > 0 )
    {
        PSELECTPARM parm = &parms[ ST_SOCKET ];
        fd_set rset, *prset = &rset;
        fd_set wset, *pwset = &wset;
        fd_set eset, *peset = &eset;

        if( DosCreateEventSem( NULL, &hevSock, DC_SEM_SHARED, FALSE ))
        {
            errno = ENOMEM;
            goto cleanup;
        }

        cancelsock = socket( AF_LOCAL, SOCK_STREAM, 0 );
        if( cancelsock == -1 )
            goto cleanup;

        if( nowaitmode )
        {
            prset = &parm->fdset[ FDSET_READ ];
            pwset = &parm->fdset[ FDSET_WRITE ];
            peset = &parm->fdset[ FDSET_EXCEPT ];
        }
        else
        {
            rset = parm->fdset[ FDSET_READ ];
            wset = parm->fdset[ FDSET_WRITE ];
            eset = parm->fdset[ FDSET_EXCEPT ];
        }

        if( _std_select( parm->nmaxfds, prset, pwset, peset, &nowait ) > 0 )
        {
            if( !nowaitmode )
            {
                parm->fdset[ FDSET_READ ] = rset;
                parm->fdset[ FDSET_WRITE ] = wset;
                parm->fdset[ FDSET_EXCEPT ] = eset;
            }

            timeout = &nowait;
        }
    }

    /* no pending events ? */
    if( timeout != &nowait )
    {
        PSELECTPARM parm = &parms[ ST_PIPE ];
        HMUX hmux;
        SEMRECORD sr;
        TID tidSock = -1;
        ULONG ulTimeout;
        ULONG ulUser;
        int err = 0;
        ULONG rc;

        /* no wait mode ? */
        if( nowaitmode )
        {
            /* then, timed-out */
            n = 0;

            goto cleanup;
        }

        if( DosCreateMuxWaitSem( NULL, &hmux, 0, NULL, DCMW_WAIT_ANY ))
        {
            errno = ENOMEM;
            goto cleanup;
        }

        for( i = 0; i < nrpipesems; i++ )
        {
            sr.hsemCur = ( HSEM )rpipesems[ i ].hev;
            sr.ulUser = rpipesems[ i ].fd;
            if( DosAddMuxWaitSem( hmux, &sr ))
            {
                err = ENOMEM;
                goto cleanup_mux;
            }
        }

        for( i = 0; i < nwpipesems; i++ )
        {
            sr.hsemCur = ( HSEM )wpipesems[ i ].hev;
            sr.ulUser = rpipesems[ i ].fd;
            if( DosAddMuxWaitSem( hmux, &sr ))
            {
                err = ENOMEM;
                goto cleanup_mux;
            }
        }

        if( hevSock != NULLHANDLE )
        {
            PSELECTPARM parmsock = &parms[ ST_SOCKET ];
            WAITSOCKSARGS wsa;

            sr.hsemCur = ( HSEM )hevSock;
            sr.ulUser = ( ULONG )hevSock;
            if( DosAddMuxWaitSem( hmux, &sr ))
            {
                err = ENOMEM;
                goto cleanup_mux;
            }

            /* add cancelsock */
            if( parmsock->nmaxfds <= cancelsock )
                parmsock->nmaxfds = cancelsock + 1;
            if( parmsock->nfds[ FDSET_READ ] <= cancelsock )
                parmsock->nfds[ FDSET_READ ] = cancelsock + 1;
            FD_SET( cancelsock, &parmsock->fdset[ FDSET_READ ]);

            wsa.parm = parmsock;
            wsa.timeout = timeout;
            wsa.hev = hevSock;

            /* no pipes ? */
            if( nrpipesems == 0 && nwpipesems == 0 )
            {
                /* no need to use a thread */
                waitsocks( &wsa );

                /* no more wait */
                timeout = &nowait;
            }
            else
            {
                /* use a thread */
                tidSock = _beginthread( waitsocks, NULL, 1024 * 1024, &wsa );
                if( tidSock == -1 )
                {
                    err = errno;
                    goto cleanup_mux;
                }
            }
        }

        if( timeout == NULL )
            ulTimeout = SEM_INDEFINITE_WAIT;
        else
            ulTimeout = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;

        rc = DosWaitMuxWaitSem( hmux, ulTimeout, &ulUser );
        switch( rc )
        {
            case ERROR_TIMEOUT:
                n = 0;
                break;

            case ERROR_INTERRUPT:
                err = EINTR;
                break;

            case 0:
                checkpipesems( nrpipesems, rpipesems,
                               &parm->fdset[ FDSET_READ ], 0 );
                checkpipesems( nwpipesems, wpipesems,
                               &parm->fdset[ FDSET_WRITE ], 0 );

                if( hevSock != NULLHANDLE )
                {
                    if( DosWaitEventSem( hevSock, SEM_IMMEDIATE_RETURN ) != 0 )
                    {
                        PSELECTPARM parmsock = &parms[ ST_SOCKET ];

                        /* clear events */
                        FD_ZERO( &parmsock->fdset[ FDSET_READ ]);
                        FD_ZERO( &parmsock->fdset[ FDSET_WRITE ]);
                        FD_ZERO( &parmsock->fdset[ FDSET_EXCEPT ]);

                        /* unblock on select() */
                        so_cancel( cancelsock );
                    }
#if 0
                    DosWaitThread( &tidSock, DCWW_WAIT );
#endif
                }
                break;

            default:
                err = ENOMEM;
                break;
        }

cleanup_mux:
        DosCloseMuxWaitSem( hmux );

        if( n == 0 || err != 0 )
        {
            if( err != 0 )
                errno = err;

            goto cleanup;
        }
    }

    if( rdset )
        FD_ZERO( rdset );

    if( wrset )
        FD_ZERO( wrset );

    if( exset )
        FD_ZERO( exset );

    n = 0;

    /* merge fds */
    for( i = 0; i < nfds; i++ )
    {
        for( j = 0; j <= ST_END; j++ )
        {
            PSELECTPARM parm = &parms[ j ];

            if( rdset && FD_ISSET( i, &parm->fdset[ FDSET_READ ] ))
                n++, FD_SET( i, rdset );

            if( wrset && FD_ISSET( i, &parm->fdset[ FDSET_WRITE ] ))
                n++, FD_SET( i, wrset );

            if( exset && FD_ISSET( i, &parm->fdset[ FDSET_EXCEPT ] ))
                n++, FD_SET( i, exset );
        }
    }

cleanup:
    if( parms[ ST_SOCKET].nmaxfds > 0 )
    {
        close( cancelsock );
        DosCloseEventSem( hevSock );
    }

    if( parms[ ST_PIPE ].nmaxfds > 0 )
    {
        termpipesems( nwpipesems, wpipesems );
        termpipesems( nrpipesems, rpipesems );
    }

    return n;
}
