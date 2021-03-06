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

#ifndef OS2COMPAT_MMAP_H
#define OS2COMPAT_MMAP_H

#include <sys/types.h>

/*
 * Protections are chosen from these bits, or-ed together
 */
#define PROT_NONE   0x00 /* no permissions */
#define PROT_READ   0x01 /* pages can be read */
#define PROT_WRITE  0x02 /* pages can be written */
#define PROT_EXEC   0x04 /* pages can be executed */

/*
 * Flags contain sharing type and options.
 * Sharing types; choose one.
 */
#define MAP_SHARED  0x0001  /* share changes */
#define MAP_PRIVATE 0x0002  /* changes are private */
#define MAP_FIXED   0x0010  /* map addr must be exactly as requested */

/*
 * Mapping type
 */
#define MAP_FILE        0x0000  /* map from file (default) */
#define MAP_ANON        0x1000  /* allocated from memory, swap space */
#define MAP_ANONYMOUS   MAP_ANON

/* Return value of `mmap' in case of an error.  */
#define MAP_FAILED ((void *)-1)

/*
 * Synchronization flags
 */
#define MS_ASYNC        0x0001  /* Perform asynchronous writes */
#define MS_SYNC         0x0002  /* Perform synchronous writes */
#define MS_INVALIDATE   0x0004  /* Invalidate cached data */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MMAP_DECLARED
#define _MMAP_DECLARED
void *mmap( void *addr, size_t len, int prot, int flags, int fildes, off_t off );
#endif
int   munmap( void *addr, size_t len );
int   mprotect( void *addr, size_t len, int prot );
void *mmap_anon( void *addr, size_t len, int prot, int flags, off_t off );
int   msync( void *addr, size_t len, int flags );

#ifdef __cplusplus
}
#endif

#endif
