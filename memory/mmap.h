/*
 * mmap() simple implementation for OS/2 kLIBC
 *
 * Copyright (C) 2014-2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_MMAP_H
#define OS2COMPAT_MMAP_H

#include <sys/mman.h>
#include <sys/types.h>

/*
 * Protections are chosen from these bits, or-ed together
 */
#ifndef PROT_NONE
#define PROT_NONE   0x00 /* no permissions */
#endif
#ifndef PROT_READ
#define PROT_READ   0x01 /* pages can be read */
#endif
#ifndef PROT_WRITE
#define PROT_WRITE  0x02 /* pages can be written */
#endif
#ifndef PROT_EXEC
#define PROT_EXEC   0x04 /* pages can be executed */
#endif

/*
 * Flags contain sharing type and options.
 * Sharing types; choose one.
 */
#ifndef MAP_SHARED
#define MAP_SHARED  0x0001  /* share changes */
#endif
#ifndef MAP_PRIVATE
#define MAP_PRIVATE 0x0002  /* changes are private */
#endif
#ifndef MAP_FIXED
#define MAP_FIXED   0x0010  /* map addr must be exactly as requested */
#endif

/*
 * Mapping type
 */
#ifndef MAP_FILE
#define MAP_FILE        0x0000  /* map from file (default) */
#endif
#ifndef MAP_ANON
#define MAP_ANON        0x1000  /* allocated from memory, swap space */
#endif
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS   MAP_ANON
#endif

/* Return value of `mmap' in case of an error.  */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

/*
 * Synchronization flags
 */
#ifndef MS_SYNC
#define MS_SYNC         0x0000  /* Perform synchronous writes */
#endif
#ifndef MS_ASYNC
#define MS_ASYNC        0x0001  /* Perform asynchronous writes */
#endif
#ifndef MS_INVALIDATE
#define MS_INVALIDATE   0x0002  /* Invalidate cached data */
#endif

#ifdef __cplusplus
extern "C" {
#endif

void *os2compat_mmap( void *addr, size_t len, int prot, int flags, int fildes,
                      off_t off );
int   os2compat_munmap( void *addr, size_t len );
int   os2compat_mprotect( void *addr, size_t len, int prot );
void *os2compat_mmap_anon( void *addr, size_t len, int prot, int flags,
                           off_t off );
int   os2compat_msync( void *addr, size_t len, int flags );

#define mmap        os2compat_mmap
#define munmap      os2compat_munmap
#define mprotect    os2compat_mprotect
#define mmap_anon   os2compat_mmap_anon
#define msync       os2compat_msync

#ifdef __cplusplus
}
#endif

#endif
