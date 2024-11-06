/*
 * spawn2ve() and spawn2vpe()
 *
 * Copyright (C) 2024 KO Myung-Hun <komh78@gmail.com>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <emx/io.h>
#include <emx/umalloc.h>

#include <dlfcn.h>
#include <sys/wait.h>

#include "spawn2.h"

static int ( *libcx_spawn2 )( int, const char *, const char * const [],
                              const char *, const char * const [],
                              const int []) = ( void * )-1L;

/* aliases */
pid_t _std_wait( int * );
pid_t _std_waitpid( pid_t, int *, int );
pid_t _std_wait3( int *, int, struct rusage * );
pid_t _std_wait4( pid_t, int *, int, struct rusage * );
pid_t _std_waitid (idtype_t, id_t, siginfo_t *, int );

/**
 * Load symbols of LIBC.
 *
 * @returns a pointer to a symbol of LIBC dll on success.
 * @returns NULL on failure.
 * @param   sym     Symbol name to load
 */
static void *load_libc_sym( const char *sym )
{
    static void *libc_handle = ( void * )-1L;

    if( libc_handle == ( void * )-1L )
    {
        libc_handle = dlopen( "libcn0", RTLD_LAZY );
        if( libc_handle == NULL )
            libc_handle = dlopen( "libc066", RTLD_LAZY );
    }

    if( libc_handle == NULL )
        return NULL;

    return dlsym( libc_handle, sym );
}

/**
 * Load symbols of LIBCx.
 *
 * @returns a pointer to a symbol of LIBCx dll on success.
 * @returns NULL on failure.
 * @param   sym     Symbol name to load
 */
static void *load_libcx_sym( const char *sym )
{
    static void *libcx_handle = ( void * )-1L;

    if( libcx_handle == ( void * )-1L )
        libcx_handle = dlopen( "libcx0", RTLD_LAZY );
    if( libcx_handle == NULL )
        return NULL;

    return dlsym( libcx_handle, sym );
}

/**
 * Load symbols of DOSCALLS.
 *
 * @returns a pointer to a symbol of DOSCALLS dll on success.
 * @returns NULL on failure.
 * @param   ord     Symbol ordinal to load
 */
static void *load_doscalls_sym( ULONG ordinal )
{
    static HMODULE doscalls_handle = ( HMODULE )-1L;
    char szErrorName[ CCHMAXPATH ];
    PFN sym;

    if( doscalls_handle == ( HMODULE )-1L )
    {
        if( DosLoadModule( szErrorName, sizeof( szErrorName ),
                           "doscalls", &doscalls_handle ) != 0 )
            doscalls_handle = NULLHANDLE;
    }

    if( doscalls_handle == NULLHANDLE )
        return NULL;

    if( DosQueryProcAddr( doscalls_handle, ordinal, NULL, &sym ) != 0 )
        return NULL;

    return sym;
}

/**
 * Replacement of LIBC wait() to support a pid returned by spawn2().
 */
pid_t wait( int *statusp )
{
    static pid_t ( *os2compat_wait )( int * ) = NULL;

    if( os2compat_wait == NULL )
    {
        os2compat_wait = load_libcx_sym( "_wait" );
        if( os2compat_wait == NULL )
            os2compat_wait = _std_wait;
    }

    return os2compat_wait( statusp );
}

/**
 * Replacement of LIBC waitpid() to support a pid returned by spawn2().
 */
pid_t waitpid( pid_t pid, int *statusp, int options )
{
    static pid_t ( *os2compat_waitpid )( pid_t, int *, int ) = NULL;

    if( os2compat_waitpid == NULL )
    {
        os2compat_waitpid = load_libcx_sym( "_waitpid" );
        if( os2compat_waitpid == NULL )
            os2compat_waitpid = _std_waitpid;
    }

    return os2compat_waitpid( pid, statusp, options );
}

/**
 * Replacement of LIBC wait3() to support a pid returned by spawn2().
 */
pid_t wait3( int *statusp, int options, struct rusage *rusage )
{
    static pid_t ( *os2compat_wait3 )( int *, int, struct rusage * ) = NULL;

    if( os2compat_wait3 == NULL )
    {
        os2compat_wait3 = load_libcx_sym( "_wait3" );
        if( os2compat_wait3 == NULL )
            os2compat_wait3 = _std_wait3;
    }

    return os2compat_wait3( statusp, options, rusage );
}

/**
 * Replacement of LIBC wait4() to support a pid returned by spawn2().
 */
pid_t wait4( pid_t pid, int *statusp, int options, struct rusage *rusage )
{
    static pid_t ( *os2compat_wait4 )( pid_t, int *, int, struct rusage * )
        = NULL;

    if( os2compat_wait4 == NULL )
    {
        os2compat_wait4 = load_libcx_sym( "_wait4" );
        if( os2compat_wait4 == NULL )
            os2compat_wait4 = _std_wait4;
    }

    return os2compat_wait4( pid, statusp, options, rusage );
}

/**
 * Replacement of LIBC waitid() to support a pid returned by spawn2().
 */
pid_t waitid( idtype_t idtype, id_t id, siginfo_t *infop, int options )
{
    static int ( *os2compat_waitid )( idtype_t, id_t, siginfo_t *, int )
        = NULL;

    if( os2compat_waitid == NULL )
    {
        os2compat_waitid = load_libcx_sym( "_waitid" );
        if( os2compat_waitid == NULL )
            os2compat_waitid = _std_waitid;
    }

    return os2compat_waitid( idtype, id, infop, options );
}

/**
 * Replacement of LIBC __waitpid() to support a pid returned by spawn2().
 */
int __waitpid(int pid, int *status, int options)
{
    static int ( *os2compat___waitpid )( int, int *, int ) = ( void * )-1L;

    if( os2compat___waitpid == ( void * )-1L )
    {
        os2compat___waitpid = load_libcx_sym( "___waitpid" );
        if( os2compat___waitpid == NULL )
            os2compat___waitpid = load_libc_sym( "___waitpid" );
    }

    if( os2compat___waitpid == NULL )
    {
        errno = ENOSYS;
        return -1;
    }

    return os2compat___waitpid( pid, status, options );
}

ULONG APIENTRY DosWaitChild( ULONG action, ULONG option, PRESULTCODES pres,
                             PPID ppid, PID pid )
{
    static int ( *os2compat_DosWaitChild )(
        ULONG, ULONG, PRESULTCODES, PPID, PID ) = ( void * )-1L;

    if( os2compat_DosWaitChild == ( void * )-1L )
    {
        os2compat_DosWaitChild = load_libcx_sym( "DosWaitChild" );
        if( os2compat_DosWaitChild == NULL )
            os2compat_DosWaitChild = load_doscalls_sym( 280 );
    }

    if( os2compat_DosWaitChild == NULL )
        return ERROR_CALL_NOT_IMPLEMENTED;

    return os2compat_DosWaitChild( action, option, pres, ppid, pid );
}

/**
 * Load spawn2() of LIBCx.
 *
 * @returns 0 on success.
 * @returns -1 on failure.
 */
int os2compat_load_spawn2( void )
{
    if( libcx_spawn2 == ( void * )-1L )
        libcx_spawn2 = load_libcx_sym( "_spawn2" );

    if( libcx_spawn2 == NULL )
        return -1;

    return 0;
}

/**
 *
 * Spawn a child process without searching in PATH
 *
 * @remark see libcx/spawn2.h for details.
 */
int os2compat_spawn2ve( int mode, const char *name, const char * const argv[],
                        const char *cwd, const char * const envp[],
                        const int stdfds[])
{
    char path[ _MAX_PATH ];
    int rc;

    if( cwd && _fnisrel( name ))
    {
        /* FIXME: _makepath() truncates a path exceeding _MAX_PATH. */
        _makepath( path, NULL, cwd, name, NULL );
    }
    else
    {
        /* FIXME: a path exceeding _MAX_PATH is truncated! */
        strncpy( path, name, sizeof( path ) - 1 );
        path[ sizeof( path ) - 1 ] = 0;
    }

    /*
     * spawn2() always search in PATH if the executable given by name is
     * not found in current dir. Disable such a behavior here.
     */
    rc = _searchenv2_one_file( path, sizeof( path ), strlen( path ),
                               _SEARCHENV2_F_EXEC_FILE, ".exe" );
    if( rc < 0 && errno == EISDIR )
        errno = ENOENT;     /* hide EISDIR */
    if( rc < 0 )
        return -1;

    return os2compat_spawn2vpe( mode, name, argv, cwd, envp, stdfds );
}

/**
 *
 * Spawn a child process with searching in PATH.
 *
 * @remark see libcx/spawn2.h for details.
 */
int os2compat_spawn2vpe( int mode, const char *name, const char * const argv[],
                         const char *cwd, const char * const envp[],
                         const int stdfds[])
{
    int fd;
    int *pfd;
    int *extended_fds = NULL;
    int *redir_fds = NULL;
    int rc;

    if( os2compat_load_spawn2() < 0 )
    {
        errno = ENOSYS;
        return -1;
    }

    /*
     * If P_2_NOINHERIT is not given explicitly, then pass open file handles
     * without FD_CLOEXEC implicitly unlike spawn2().
     */
    if( stdfds && !( mode & P_2_NOINHERIT ))
    {
        /* Convert simple mode to extended mode */
        if( !( mode & P_2_XREDIR ))
        {
            /* simple mode has always 3 handles */
            extended_fds = malloc( sizeof( stdfds[ 0 ]) * 3 * 2 + 1 );
            if( extended_fds == NULL )
            {
                errno = ENOMEM;
                return -1;
            }

            for( pfd = extended_fds, fd = 0; fd < 3; fd++ )
            {
                *pfd++ = stdfds[ fd ] == 0 ? fd : stdfds[ fd ];
                *pfd++ = fd;
            }

            *pfd = -1;      /* mark end of array */

            mode |= P_2_XREDIR;
            stdfds = extended_fds;
        }

        if( mode & P_2_XREDIR )
        {
            int redirs;
            int *pfd_end;
            int *rfd;
            LONG lDelta = 0;
            ULONG ulCurMaxFH = 0;

            for( redirs = 0, pfd = ( int * )stdfds; *pfd != -1; pfd += 2 )
                redirs++;

            DosSetRelMaxFH( &lDelta, &ulCurMaxFH );
            /* fallback to safe value on error. */
            if (ulCurMaxFH == 0)
                ulCurMaxFH = 64;
            /* make it nearest to multiple of 4 as LIBC does */
            ulCurMaxFH = ( ulCurMaxFH + 3 ) & ~3;

            redir_fds = _hmalloc( sizeof( stdfds[ 0 ])
                                  * ( redirs + ulCurMaxFH ) * 2 + 1 );
            if( redir_fds == NULL )
            {
                free( extended_fds );

                errno = ENOMEM;
                return -1;
            }

            memcpy( redir_fds, stdfds, sizeof( stdfds[ 0 ]) * redirs * 2 );

            pfd_end = redir_fds + redirs * 2;
            rfd = pfd_end;

            for( fd = 0; fd < ulCurMaxFH; fd++ )
            {
                if( !__libc_FH( fd ) || ( fcntl( fd, F_GETFD ) & FD_CLOEXEC ))
                    continue;

                /* pass file handles not used by a child. */
                for( pfd = redir_fds + 1; pfd < pfd_end; pfd += 2 )
                {
                    if( *pfd == fd )
                        break;
                }

                if( pfd < pfd_end )
                    continue;

                /* pass to a child. */
                *rfd++ = fd;
                *rfd++ = fd;
            }

            *rfd = -1;      /* mark end of array */

            stdfds = redir_fds;
        }
    }

    rc = libcx_spawn2( mode, name, argv, cwd, envp, stdfds );

    free( redir_fds );
    free( extended_fds );

    return rc;
}
