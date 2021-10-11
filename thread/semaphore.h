/*
 * POSIX semaphore implementation for OS/2 kLIBC
 *
 * Copyright (C) 2016-2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_SEMAPHORE_H
#define OS2COMPAT_SEMAPHORE_H

#define SEM_VALUE_MAX   32767

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure for semaphore
 */
typedef struct {
    unsigned long/*HEV*/ hev;        /**< event semaphore    */
    unsigned long/*HMTX*/ hmtxWait;  /**< mutex for waiting  */
    unsigned long/*HMTX*/ hmtxCount; /**< mutex for counting */
    int count;                       /**< count of semaphore */
} os2compat_sem_t;

int os2compat_sem_init( os2compat_sem_t *sem, int pshared,
                        unsigned int value );
int os2compat_sem_destroy( os2compat_sem_t *sem );
int os2compat_sem_post( os2compat_sem_t *sem );
int os2compat_sem_wait( os2compat_sem_t *sem );
int os2compat_sem_trywait( os2compat_sem_t *sem );

#define sem_t os2compat_sem_t

#define sem_init        os2compat_sem_init
#define sem_destroy     os2compat_sem_destroy
#define sem_post        os2compat_sem_post
#define sem_wait        os2compat_sem_wait
#define sem_trywait     os2compat_sem_trywait

#ifdef __cplusplus
}
#endif

#endif
