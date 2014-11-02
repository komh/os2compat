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
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <unistd.h>
#include <sys/types.h>

#include "mmap.h"

typedef struct os2_mmap_s
{
    void    *addr;              /**< address of mapped memory */
    size_t  len;                /**< length of mapped memory */
    int     flags;              /**< protection flags of mapped memory */
    struct os2_mmap_s *prev;    /**< previous list of os2_mmap */
    struct os2_mmap_s *next;    /**< next list of os2_mmap */
} os2_mmap;
static os2_mmap *m_mmap = NULL;

/**
 * Read a file into a memory.
 * @param[in]  fd  file descriptor
 * @param[in]  off offset of a file
 * @param[out] buf a memory to be transferred from a file
 * @param[in]  len length to read in bytes
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

    /* Read in a file */
    if( read( fd, buf, len ) == -1 )
        return -1;

    /* Restore the position */
    if( lseek( fd, pos, SEEK_SET ) == -1 )
        return -1;

    return 0;
}

/**
 * Map a file to a memory.
 * @remark MAP_FIXED will succeed only if [addr, addr + len) is already
 * allocated. MAP_SHARED is not supported.
 * @warning If a translation mode of fildes is a text mode, len and a real
 * length read may be different.
 */
void *mmap( void *addr, size_t len, int prot, int flags, int fildes, off_t off )
{
    os2_mmap *new_mmap;
    int pagesize;

    ULONG rc;

    void  *ret;

    pagesize = getpagesize();

    /* addr should be multiple of a page size if MAP_FIXED */
    if(( flags & MAP_FIXED ) && (( uintptr_t )addr % pagesize ))
        return MAP_FAILED;

    /* off should be multiple of a page size */
    if( off % pagesize)
        return MAP_FAILED;

    if( prot & PROT_WRITE )
    {
        if( flags & MAP_SHARED )
            return MAP_FAILED;

        if( !( flags & MAP_PRIVATE ))
            return MAP_FAILED;
    }

    if( flags & MAP_FIXED )
    {
        ULONG cb;
        ULONG fl;

        cb = len;
        rc = DosQueryMem( addr, &cb, &fl );
        if( rc || ( cb < len ))
            return MAP_FAILED;

        rc = DosSetMem( addr, len, fPERM );
        if( rc )
            return MAP_FAILED;

        ret = addr;
    }
    else
    {
        /* First, try to allocate in high memory */
        rc = DosAllocMem( &ret, len, fPERM | OBJ_ANY | PAG_COMMIT );
        if( rc ) /* Failed ? Try to allocate in low memory */
            rc = DosAllocMem( &ret, len, fPERM | PAG_COMMIT );

        if( rc )
            return MAP_FAILED;
    }

    new_mmap = malloc( sizeof( os2_mmap ));
    if( !new_mmap )
    {
        if( !( flags & MAP_FIXED ))
            DosFreeMem( ret );

        return MAP_FAILED;
    }

    new_mmap->addr  = ret;
    new_mmap->len   = len;
    new_mmap->flags = flags;
    new_mmap->prev  = m_mmap;
    new_mmap->next  = NULL;

    if( m_mmap )
        m_mmap->next = new_mmap;
    m_mmap = new_mmap;

    if( !( flags & MAP_ANON ))
    {
        if( readFromFile( fildes, off, ret, len ) == -1 )
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

        if( !( mm->flags & MAP_FIXED ))
            DosFreeMem( addr );

        if( mm->next )
            mm->next->prev = mm->prev;

        if( mm->prev )
            mm->prev->next = mm->next;

        if( m_mmap == mm )
            m_mmap = mm->prev;

        free( mm );

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
        if( mm->addr <= addr && addr + len <= mm->addr + mm->len )
            break;
    }

    if( mm )
    {
        ULONG fl;

        fl = 0;

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
    }

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

