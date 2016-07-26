/*
 * POSIX semaphore implementation for OS/2 kLIBC
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
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
} sem_t;

int sem_init( sem_t *sem, int pshared, unsigned int value );
int sem_destroy( sem_t *sem );
int sem_post( sem_t *sem );
int sem_wait( sem_t *sem );
int sem_trywait( sem_t *sem );

#ifdef __cplusplus
}
#endif

#endif
