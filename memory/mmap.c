/*
 * mmap() simple implementation for OS/2 kLIBC
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_EXAPIS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include <InnoTekLIBC/backend.h>
#include <InnoTekLIBC/fork.h>

#include "mmap.h"

#define MAP_ALLOCATED   0x0020  /* memory allocated for MAP_FIXED */

#define SHARED_NAME_PREFIX      "\\SHAREMEM\\OS2MMAP\\"
#define SHARED_NAME_PREFIX_LEN  18
#define SHARED_NAME_MAX_LEN     ( SHARED_NAME_PREFIX_LEN + CCHMAXPATH )

typedef struct os2_mmap_s
{
    void    *addr;              /**< address of mapped memory */
    size_t  len;                /**< length of mapped memory */
    int     flags;              /**< protection flags of mapped memory */

    /* members for MAP_SHARED */
    int     fd;             /**< file descriptor of a mapped file */
    off_t   off;            /**< offset of a mapped file */
    void    *base;          /**< base address of a shared mapped memory */
    int     prot;           /**< protection flags of a shared mapped memory */
    char    shared_name[ SHARED_NAME_MAX_LEN ];
                            /**< name of a named shared memory */

    struct os2_mmap_s *prev;    /**< previous list of os2_mmap */
    struct os2_mmap_s *next;    /**< next list of os2_mmap */
} os2_mmap;
static os2_mmap *m_mmap = NULL;

/**
 * Read a file into a memory.
 * @param[in]  fd  file descriptor
 * @param[in]  off offset of a file
 * @param[out] buf a memory to be transferred from a file
 * @param[in]  len length to read in bytes. -1 for a file length
 * @return 0 on success, or -1 on error
 */
static int readFromFile( int fd, off_t off, void *buf, size_t len )
{
    int pos = lseek( fd, 0, SEEK_CUR ); /* Get a current position */

    if( pos == -1 )
        return -1;

    /* Seek to offset off */
    if( lseek( fd, off, SEEK_SET ) == -1)
        return -1;

    if( len == -1 )
        len = filelength( fd );

    /* Read in a file */
    if( read( fd, buf, len ) == -1 )
        return -1;

    /* Restore the position */
    if( lseek( fd, pos, SEEK_SET ) == -1 )
        return -1;

    return 0;
}

/**
 * Write a memory to a file.
 * @param[in] fd  file descriptor
 * @param[in] off offset of a file
 * @param[in] buf a memory to be transferred from a file
 * @param[in] len length to read in bytes
 * @return 0 on success, or -1 on error
 */
static int writeToFile( int fd, off_t off, void *buf, size_t len )
{
    int pos = lseek( fd, 0, SEEK_CUR ); /* Get a current position */

    if( pos == -1 )
        return -1;

    /* Seek to ofset off */
    if( lseek( fd, off, SEEK_SET ) == -1)
        return -1;

    /* Write to a file */
    if( write( fd, buf, len ) == -1 )
        return -1;

    /* Restore the position */
    if( lseek( fd, pos, SEEK_SET ) == -1 )
        return -1;

    return 0;
}

/**
 * Remove os2_mmap entry from a list.
 * @param[in] mm pointer to os2_mmap entry
 */
static void mmapRemoveMmap( os2_mmap *mm )
{
    if( mm->next )
        mm->next->prev = mm->prev;

    if( mm->prev )
        mm->prev->next = mm->next;

    if( m_mmap == mm )
        m_mmap = mm->prev;

    free( mm );
}

/* forward declaration */
static int mmapRemoveAnonMem( void *addr );

/**
 * Free OS/2 memory allocated by mmap()
 * @param[in] addr  address returned by mmap()
 * @param[in] base  address of a shared memory allocated by mmap()
 * @param[in] flags flags used by mmap()
 */
static void mmapFreeMem( void *addr, void *base, int flags )
{
    if( !( flags & MAP_FIXED ))
    {
        DosFreeMem( addr );

        if( flags & MAP_SHARED )
        {
            if( flags & MAP_ANON )
                mmapRemoveAnonMem( addr );
            else
            {
                PULONG pulRefCount = base;

                /* Free a shared memory only once. Otherwise all the aliases
                * will be invalidated.
                */
                if( --( *pulRefCount ) == 0 )
                    while( DosFreeMem( base ) == 0 )
                        /* nothing */;
            }
        }
    }
    else if( flags & MAP_ALLOCATED )
        DosFreeMemEx( addr );
}

/**
 * Set protection flags of OS/2 memory with flags used by mmap().
 * @param[in] addr address of OS/2 memory
 * @param[in] len  length of OS/2 memory
 * @param[in] prot proection flags used by mmap()
 * @return 0 on success, or -1 on error
 */
static int mmapSetMem( void *addr, size_t len, int prot )
{
    ULONG fl = 0;

    if( prot & PROT_READ )
        fl |= PAG_READ;

    if( prot & PROT_WRITE )
        fl |= PAG_WRITE;

    if( prot & PROT_EXEC )
        fl |= PAG_EXECUTE;

    if( !fl )            /* PROT_NONE ? */
        fl |= PAG_READ;  /* Set READ flag if PROT_NONE */

    if( DosSetMem( addr, len, fl ) == 0 )
        return 0;

    return -1;
}

#define USE_SHARED_MEM_FOR_ANON 0

#if USE_SHARED_MEM_FOR_ANON
#define SHARED_NAME_MMAP_ANON       "\\SHAREMEM\\OS2MMAP\\MMAP\\ANON\\LIST"
#define SHARED_NAME_MMAP_ANON_SIZE  ( 64 * 1024 )

typedef struct MMAPANONLIST
{
    PVOID addr;     /**< address of MAP_ANON memory */
    ULONG len;      /**< length of MAP_ANON memory */
    ULONG prot;     /**< protection flags of MAP_ANON memory */
    ULONG refCount; /**< reference count of MAP_ANON memory */
} MMAPANONLIST, *PMMAPANONLIST;

/**
 * Add MAP_ANON memory to a global anon list.
 * @param[in] addr pointer to memory to add
 * @param[in] len  length of memory
 * @param[in] prot protection flags of memory
 * @return 0 on success, or -1 on error
 * @bug cannot add entries more than 4095
 * = ( 65536 - sizeof( ULONG )) / (sizeof( PVOID ) + sizeof( ULONG ) * 3 )
 */
static int mmapAddAnonMem( void *addr, size_t len, int prot )
{
    PVOID         pAnonList;
    PULONG        pulRefCount;
    PMMAPANONLIST pmal;
    PMMAPANONLIST pmalEnd;

    ULONG ulFlag = fPERM | PAG_COMMIT;

    if( !addr )
        return -1;

    if( DosAllocSharedMem( &pAnonList, SHARED_NAME_MMAP_ANON,
                           SHARED_NAME_MMAP_ANON_SIZE, ulFlag | OBJ_ANY )
        && DosAllocSharedMem( &pAnonList, SHARED_NAME_MMAP_ANON,
                              SHARED_NAME_MMAP_ANON_SIZE, ulFlag )
        && DosGetNamedSharedMem( &pAnonList, SHARED_NAME_MMAP_ANON, fPERM ))
        return -1;

    /* DosAllocSharedMem() initializes a whole pAnonList with 0 */
    pulRefCount = pAnonList;

    pmal    = ( PMMAPANONLIST )( pulRefCount + 1 );
    pmalEnd = ( PMMAPANONLIST )
              (( PBYTE )pAnonList + SHARED_NAME_MMAP_ANON_SIZE);

    /* find an empty slot */
    while( pmal + 1 <= pmalEnd && pmal->addr )
        pmal++;

    /* found ? */
    if( pmal + 1 <= pmalEnd )
    {
        pmal->addr     = addr;
        pmal->len      = len;
        pmal->prot     = prot;
        pmal->refCount = 1;

        ( *pulRefCount )++; /* increase global reference count */

        return 0;
    }

    DosFreeMem( pAnonList );

    return -1;
}

/**
 * Remove MAP_ANON memory from a global anon list.
 * @param[in] addr pointer to memory to add
 * @return 0 on success, or -1 on error
 */
static int mmapRemoveAnonMem( void *addr )
{
    PVOID         pAnonList;
    PULONG        pulRefCount;
    PMMAPANONLIST pmal;
    PMMAPANONLIST pmalEnd;

    if( DosGetNamedSharedMem( &pAnonList, SHARED_NAME_MMAP_ANON, fPERM ))
        return -1;

    pulRefCount = pAnonList;

    pmal    = ( PMMAPANONLIST )( pulRefCount + 1 );
    pmalEnd = ( PMMAPANONLIST )
              (( PBYTE )pAnonList + SHARED_NAME_MMAP_ANON_SIZE);

    /* find an addr */
    while( pmal + 1 <= pmalEnd && pmal->addr != addr )
        pmal++;

    if( pmal + 1 <= pmalEnd )
    {
        /* decrease reference count for a mmaped memory  */
        if( --pmal->refCount == 0 ) /* no reference ? */
            pmal->addr = NULL;      /* clear slot */

        /* decrease global reference count */
        if( --( *pulRefCount ) == 0 )               /* no reference ? */
            while( DosFreeMem( pAnonList ) == 0 )   /* free shared mem */
                /* nothing */;

        return 0;
    }

    DosFreeMem( pAnonList );

    return -1;
}

/**
 * Get access to all the shared memories created with MAP_ANON | MAP_SHARED.
 * @return 0 on success, or -1 on error
 */
static int mmapGetAnonMem( void )
{
    PVOID         pAnonList;
    PULONG        pulRefCount;
    PMMAPANONLIST pmal;
    PMMAPANONLIST pmalEnd;

    if( !DosGetNamedSharedMem( &pAnonList, SHARED_NAME_MMAP_ANON, fPERM ))
    {
        pulRefCount = pAnonList;

        pmal    = ( PMMAPANONLIST )( pulRefCount + 1 );
        pmalEnd = ( PMMAPANONLIST )
                  (( PBYTE )pAnonList + SHARED_NAME_MMAP_ANON_SIZE);

        /* find MAP_ANON memory and get access to it */
        for(; pmal + 1 <= pmalEnd; pmal++ )
        {
            if( pmal->addr )
            {
                if( DosGetSharedMem( pmal->addr, PAG_READ )
                    || mmapSetMem( pmal->addr, pmal->len, pmal->prot ) == -1 )
                    break;

                /* increase global reference count */
                ( *pulRefCount )++;

                /* increase reference count for addr */
                pmal->refCount++;
            }
        }

        DosFreeMem( pAnonList );

        return ( pmal + 1 > pmalEnd ) ? 0 : -1;
    }

    return 0;
}
#else
/**
 * Add MAP_ANON memory to a global anon list.
 * @param[in] addr pointer to memory to add
 * @param[in] len  length of memory
 * @param[in] prot protection flags of memory
 * @return 0 on success, or -1 on error
 */
static int mmapAddAnonMem( void *addr, size_t len, int prot )
{
    return 0;
}

/**
 * Remove MAP_ANON memory from a global anon list.
 * @param[in] addr pointer to memory to add
 * @return 0 on success, or -1 on error
 */
static int mmapRemoveAnonMem( void *addr )
{
    return 0;
}

/**
 * Get access to all the shared memories created with MAP_ANON | MAP_SHARED.
 * @return 0 on success, or -1 on error
 */
static int mmapGetAnonMem( void )
{
    os2_mmap *mm;

    for( mm = m_mmap; mm; mm = mm->prev )
    {
        if(( mm->flags & MAP_ANON ) && ( mm->flags & MAP_SHARED ))
        {
            if( DosGetSharedMem( mm->addr, PAG_READ )
                || mmapSetMem( mm->addr, mm->len, mm->prot ) == -1 )
                break;
        }
    }

    return mm ? -1 : 0;
}
#endif

/**
 * Alias memory at a given address.
 *
 * @remark  This algorithm is an improoved version of the one Odin uses in
 *          it's Ring-3 PeLdr.
 */
static int aliasAtAddress( void *pMem, ULONG cbReq, void *pvReq, ULONG fReq )
{
    PVOID   apvTmps[ 512 ]; /* 512MB */
    ULONG   cbTmp;
    ULONG   fTmp;
    int     iTmp;
    int     rcRet = ERROR_NOT_ENOUGH_MEMORY;

    /*
     * Adjust flag and size.
     */
    fReq &= ~OBJ_LOCATION;
    cbReq = ( cbReq + 0xfff ) & ~0xfff;

    /*
     * Allocation loop.
     * This algorithm is not optimal!
     */
    fTmp  = fPERM;
    cbTmp = 1 * 1024 * 1024; /* 1MB */
    for( iTmp = 0; iTmp < sizeof( apvTmps ) / sizeof( apvTmps[ 0 ]); iTmp++ )
    {
        PVOID   pvNew = NULL;
        int     rc;

        /* Allocate chunk. */
        rc = DosAllocMem( &pvNew, cbTmp, fTmp );
        apvTmps[ iTmp ] = pvNew;
        if( rc )
            break;

        /*
         * Passed it?
         * Then retry with the requested size.
         */
        if( pvNew > pvReq )
        {
            if( cbTmp <= cbReq )
                break;
            DosFreeMem( pvNew );
            cbTmp = cbReq;
            iTmp--;
            continue;
        }

        /*
         * Does the allocated object touch into the requested one?
         */
        if(( char * )pvNew + cbTmp > ( char * )pvReq )
        {
            /*
             * Yes, we've found the requested address!
             */
            apvTmps[ iTmp ] = NULL;
            DosFreeMem( pvNew );

            /*
             * Adjust the allocation size to fill the gap between the
             * one we just got and the requested one.
             * If no gap we'll attempt the real allocation.
             */
            cbTmp = ( uintptr_t )pvReq - ( uintptr_t )pvNew;
            if( cbTmp )
            {
                iTmp--;
                continue;
            }

            rc = DosAliasMem( pMem, cbReq, &pvNew, fReq );
            if( rc || ( char * )pvNew > ( char * )pvReq )
                break; /* we failed! */
            if( pvNew == pvReq )
            {
                rcRet = 0;
                break;
            }

            /*
             * We've got an object which start is below the one we
             * requested. This is probably caused by the requested object
             * fitting in somewhere our tmp objects didn't.
             * So, we'll have loop and retry till all such holes are filled.
             */
            apvTmps[ iTmp ] = pvNew;
        }
    }

    /*
     * Cleanup reserved memory and return.
     */
    while( iTmp-- > 0 )
        if( apvTmps[ iTmp ])
            DosFreeMem( apvTmps[ iTmp ]);

    return rcRet;
}

/**
 * Get access to all the shared memories created with MAP_SHARED only.
 *
 * @return 0 on success, or -1 on error
 *
 */
static int mmapGetSharedMem( void )
{
    os2_mmap *mm;
    void *shared_mem;
    int pagesize = getpagesize();

    for( mm = m_mmap; mm; mm = mm->prev )
    {
        if(( mm->flags & ( MAP_ANON | MAP_SHARED )) == MAP_SHARED )
        {
            if( DosGetNamedSharedMem( &shared_mem, mm->shared_name, fPERM ))
                return -1;

            /* Right ? However, fails without this */
            DosFreeMem( mm->addr );

            if( aliasAtAddress((char * )mm->base + pagesize + mm->off,
                               mm->len, mm->addr, OBJ_LOCATION )
                || mmapSetMem( mm->addr, mm->len, mm->prot ) == -1 )
                return -1;

            ( *( PULONG )mm->base )++;
        }
    }

    return 0;
}

/**
 * Inherit all the shared memories created with MAP_SHARED.
 * @return 0 on success, or -1 on error
 */
static int mmapInherit( void )
{
    os2_mmap *mm;

    for( mm = m_mmap; mm; mm = mm->prev )
    {
        /* remove entries without MAP_SHARED */
        if( !( mm->flags & MAP_SHARED ))
            mmapRemoveMmap( mm );
    }

    return ( mmapGetAnonMem() ||  mmapGetSharedMem()) ? -1 : 0;
}

/**
 * fork() callback
 */
static int forkChildCallback( __LIBC_PFORKHANDLE pForkHandle,
                              __LIBC_FORKOP enumOp )
{
    int rc;

    switch( enumOp )
    {
        case __LIBC_FORK_OP_FORK_CHILD:
            rc = mmapInherit();
            break;

        default:
            rc = 0;
            break;
    }

    return rc;
}

_FORK_CHILD1( 0xFFFF00FF, forkChildCallback );

/**
 * Get a name for a shared memory from a file descriptor.
 * @param[in]  fd   file descriptor
 * @param[out] name the place to store a file name
 * @param[in]  size maximum size of name
 * @return 0 on success, or -1 error
 */
int mmapGetSharedNameFromFd( int fd, char *name, size_t size )
{
    if( size <= SHARED_NAME_PREFIX_LEN )
        return -1;

    strcpy( name, SHARED_NAME_PREFIX );

    /* get a filename from a file descriptor */
    if( __libc_Back_ioFHToPath( fd, name + SHARED_NAME_PREFIX_LEN,
                                size - SHARED_NAME_PREFIX_LEN ))
        return -1;

    /* replace ':' with '_' */
    name[ SHARED_NAME_PREFIX_LEN + 1 ] = '_';

    /* convert '/' to '\' */
    for( name += SHARED_NAME_PREFIX_LEN; *name; name++ )
        if( *name == '/')
            *name = '\\';

    return 0;
}

/**
 * Map a file to a memory.
 * @remark MAP_FIXED works only with MAP_PRIVATE.
 * @warning If a translation mode of fildes is a text mode, len and a real
 * length read may be different.
 * @todo demand paging.
 */
void *mmap( void *addr, size_t len, int prot, int flags, int fildes, off_t off )
{
    os2_mmap *new_mmap;
    int pagesize;

    ULONG rc;

    void  *ret;
    void  *shared_base = NULL;
    char   shared_name_buf[ SHARED_NAME_MAX_LEN ] = "";
    int    read_file = !( flags & MAP_ANON );

    pagesize = getpagesize();

    /* addr should be multiple of a page size if MAP_FIXED */
    if(( flags & MAP_FIXED ) && (( uintptr_t )addr % pagesize ))
        return MAP_FAILED;

    /* off should be multiple of a page size */
    if( off % pagesize)
        return MAP_FAILED;

    if(( flags & ( MAP_SHARED | MAP_PRIVATE )) == ( MAP_SHARED | MAP_PRIVATE )
       || !( flags & ( MAP_SHARED | MAP_PRIVATE )))
        return MAP_FAILED;

    /* MAP_FIXED with MAP_SHARED is not supported */
    if(( flags & ( MAP_FIXED | MAP_SHARED )) == ( MAP_FIXED | MAP_SHARED ))
        return MAP_FAILED;

    /* check fd has a write access flag if PROT_WRITE with MAP_SHARED */
    if( !( flags & MAP_ANON ) && ( flags & MAP_SHARED )
        && ( prot & PROT_WRITE )
        && !( fcntl( fildes, F_GETFL ) & ( O_WRONLY | O_RDWR )))
        return MAP_FAILED;

    if( flags & MAP_FIXED )
    {
        ULONG cb;
        ULONG fl;

        cb = 1;
        rc = DosQueryMem( addr, &cb, &fl );
        if( rc || ( fl & PAG_FREE ))
        {
            ULONG ulFlag = fPERM | PAG_COMMIT | OBJ_LOCATION;

            rc = DosAllocMemEx( &addr, len, ulFlag );
            if( !rc )
                flags |= MAP_ALLOCATED;
        }
        else /* Committed or reserved pages */
            rc = DosSetMem( addr, len,
                            fPERM | (( fl & PAG_COMMIT ) ^ PAG_COMMIT ));

        if( rc )
            return MAP_FAILED;

        ret = addr;
    }
    else
    {
        ULONG ulFlag = fPERM | PAG_COMMIT;

        if( flags & MAP_SHARED )
        {
            char *shared_name;
            off_t shared_mem_len;

            if( flags & MAP_ANON )
            {
                shared_name = NULL;
                shared_mem_len = len;
                ulFlag |= fSHARE;
            }
            else
            {
                if( mmapGetSharedNameFromFd( fildes, shared_name_buf,
                                             sizeof( shared_name_buf )) == -1 )
                    return MAP_FAILED;

                shared_name = shared_name_buf;
                shared_mem_len = pagesize + filelength( fildes );
            }

            rc = DosAllocSharedMem( &shared_base, shared_name,
                                    shared_mem_len, ulFlag | OBJ_ANY );
            if( rc )
                rc = DosAllocSharedMem( &shared_base, shared_name,
                                        shared_mem_len, ulFlag );

            if( rc && shared_name )
            {
                read_file = 0;

                rc = DosGetNamedSharedMem( &shared_base, shared_name, fPERM );
            }

            if( !rc )
            {
                if( flags & MAP_ANON )
                {
                    rc = mmapAddAnonMem( shared_base, len, prot );
                    if( rc == -1 )
                        DosFreeMem( shared_base );

                    ret = shared_base;
                }
                else
                {
                    /* duplicate fildes to use later */
                    fildes = dup( fildes );

                    /* prevent inheritance of dup()ed fildes */
                    fcntl( fildes, F_SETFD,
                           fcntl( fildes, F_GETFD ) | FD_CLOEXEC );

                    rc = DosAliasMem(( char * )shared_base + pagesize + off,
                                     len, &ret, 0 );
                    if( fildes == -1 || rc )
                    {
                        if( !rc )
                        {
                            DosFreeMem( ret );

                            rc = -1;
                        }

                        DosFreeMem( shared_base );
                    }
                    else
                    {
                        PULONG pulRefCount = shared_base;

                        if( read_file )
                            *pulRefCount = 1;
                        else
                            ( *pulRefCount )++;
                    }
                }
            }
        }
        else    /* MAP_PRIVATE ? */
        {
            /* First, try to allocate in high memory */
            rc = DosAllocMem( &ret, len, ulFlag | OBJ_ANY );
            if( rc ) /* Failed ? Try to allocate in low memory */
                rc = DosAllocMem( &ret, len, ulFlag );
        }

        if( rc )
            return MAP_FAILED;
    }

    new_mmap = malloc( sizeof( os2_mmap ));
    if( !new_mmap )
    {
        mmapFreeMem( ret, shared_base, flags );

        return MAP_FAILED;
    }

    new_mmap->addr  = ret;
    new_mmap->len   = len;
    new_mmap->flags = flags;
    new_mmap->fd    = fildes;
    new_mmap->off   = off;
    new_mmap->base  = shared_base;
    new_mmap->prot  = prot;
    strcpy( new_mmap->shared_name, shared_name_buf );
    new_mmap->prev  = m_mmap;
    new_mmap->next  = NULL;

    if( m_mmap )
        m_mmap->next = new_mmap;
    m_mmap = new_mmap;

    if( read_file )
    {
        off_t read_off;
        void *read_buf;
        size_t read_len;

        if( flags & MAP_SHARED )
        {
            read_off = 0;
            read_buf = ( char * )new_mmap->base + pagesize;
            read_len = -1;
        }
        else
        {
            read_off = off;
            read_buf = ret;
            read_len = len;
        }

        if( readFromFile( fildes, read_off, read_buf, read_len ) == -1 )
        {
            munmap( ret, len );

            return MAP_FAILED;
        }
    }

    if( mprotect( ret, len, prot ))
    {
        munmap( ret, len );

        return MAP_FAILED;
    }

    return ret;
}

/**
 * Unmap mapped memory by mmap().
 * @remark Partial unmapping is not supported.
 */
int munmap( void *addr, size_t len )
{
    os2_mmap *mm;

    if(( uintptr_t )addr % getpagesize())
        return -1;

    for( mm = m_mmap; mm; mm = mm->prev )
    {
        if( mm->addr == addr && mm->len == len )
            break;
    }

    if( mm )
    {
        if(( mm->flags & ( MAP_ANON | MAP_SHARED )) == MAP_SHARED
           && ( mm->prot & PROT_WRITE )
           && msync( mm->addr, mm->len, MS_SYNC ) == -1 )
            return -1;

        if( mm->flags & MAP_SHARED )
            close( mm->fd );

        mmapFreeMem( addr, mm->base, mm->flags );

        mmapRemoveMmap( mm );

        return 0;
    }

    return -1;
}

/**
 * Set memory protection flags.
 * @bug Set READ flag if PROT_NONE. OS/2 has no equivalent attributes to it.
 */
int mprotect( void *addr, size_t len, int prot )
{
    os2_mmap *mm;

    if(( uintptr_t )addr % getpagesize())
        return -1;

    for( mm = m_mmap; mm; mm = mm->prev )
    {
        if( mm->addr <= addr
            && ( char * )addr + len <= ( char * )mm->addr + mm->len )
            break;
    }

    if( mm && mmapSetMem( addr, len, prot ) == 0 )
        return 0;

    return -1;
}

/**
 * Anonymous mmap().
 * @see mmap().
 */
void *mmap_anon( void *addr, size_t len, int prot, int flags, off_t off )
{
    return mmap( addr, len, prot, flags | MAP_ANON, -1, off );
}

/**
 * Synchronize memory with a file
 */
int msync( void *addr, size_t len, int flags )
{
    os2_mmap *mm;

    if( !( flags & ( MS_ASYNC | MS_SYNC | MS_INVALIDATE )))
        return -1;

    if(( flags & MS_ASYNC ) && ( flags & MS_SYNC ))
        return -1;

    if(( uintptr_t )addr % getpagesize())
        return -1;

    /* find addr */
    for( mm = m_mmap; mm; mm = mm->prev )
    {
        if( mm->addr <= addr
            && ( char * )addr + len <= ( char * )mm->addr + mm->len )
            break;
    }

    /* not found */
    if( !mm )
        return -1;

    /* nothing to do for MAP_ANON */
    if( mm->flags & MAP_ANON )
        return 0;

    /* nothing to do for MS_ASYNC */

    /* write to a file */
    if(( flags & MS_SYNC )
       && ( mm->flags & MAP_SHARED ) && ( mm->prot & PROT_WRITE )
       && writeToFile( mm->fd, mm->off + ( char * )addr - ( char * )mm->addr,
                       addr, len ) == -1 )
        return -1;

    /* read from a file if MAP_SHARED */
    if(( flags & MS_INVALIDATE ) && ( flags & MAP_SHARED ))
    {
        char   name[ SHARED_NAME_MAX_LEN ];
        void  *base;

        if( mmapGetSharedNameFromFd( mm->fd, name, sizeof( name )) == -1 )
            return -1;

        if( DosGetNamedSharedMem( &base, name, PAG_READ ) != 0 )
            return -1;

        return readFromFile( mm->fd, 0, ( char * )base + getpagesize(), -1 );
    }

    return 0;
}
