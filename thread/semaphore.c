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

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <errno.h>

#include "semaphore.h"

#define EPILOGUE                    \
    do {                            \
        int err = rc2errno( rc );   \
        if( err )                   \
        {                           \
            errno = err;            \
            return -1;              \
        }                           \
    } while( 0 );                   \
    return 0

/**
 * Return errno value corresponding to OS/2 error code
 *
 * @param[in] rc OS/2 error code.
 * @return 0 if @a rc is treated as no error, otherwise errno value
 *         corresponding to OS/2 error code.
 */
static int rc2errno( ULONG rc )
{
    int err = 0;

    switch( rc )
    {
        case ERROR_INVALID_HANDLE :
            err = EINVAL;
            break;

        case ERROR_NOT_ENOUGH_MEMORY :
            err = ENOMEM;
            break;

        case ERROR_INVALID_PARAMETER :
            err = EINVAL;
            break;

        case ERROR_INTERRUPT :
            err = EINTR;
            break;

        case ERROR_TOO_MANY_HANDLES :
            err = ENOSPC;
            break;

        case ERROR_SEM_BUSY :
            err = EBUSY;
            break;

        case ERROR_TIMEOUT :
            err = EAGAIN;
            break;
    }

    return err;
}

/**
 * sem_init()
 *
 * @todo Support @a pshared
 */
int sem_init( sem_t *sem, int pshared, unsigned int value )
{
    if( value > SEM_VALUE_MAX )
    {
        errno = EINVAL;

        return -1;
    }

    if( DosCreateEventSem( NULL, &sem->hev, 0, value > 0 ? TRUE : FALSE ) ||
        DosCreateMutexSem( NULL, &sem->hmtxWait, 0, FALSE ) ||
        DosCreateMutexSem( NULL, &sem->hmtxCount, 0, FALSE ))
    {
        errno = ENOSPC;

        sem_destroy( sem );

        return -1;
    }

    sem->count = value;

    return 0;
}

/**
 * sem_destroy()
 */
int sem_destroy( sem_t *sem )
{
    ULONG rc;

    rc = DosCloseEventSem( sem->hev );

    if( !rc )
        rc = DosCloseMutexSem( sem->hmtxWait );

    if( !rc )
        rc = DosCloseMutexSem( sem->hmtxCount );

    EPILOGUE;
}

/**
 * sem_post()
 */
int sem_post( sem_t *sem )
{
    ULONG rc;

    DosRequestMutexSem( sem->hmtxCount, SEM_INDEFINITE_WAIT );

    if( sem->count < SEM_VALUE_MAX )
    {
        sem->count++;

        rc = DosPostEventSem(sem->hev);
    }

    DosReleaseMutexSem(sem->hmtxCount);

    EPILOGUE;
}

static int sem_wait_common( sem_t *sem, ULONG ulTimeout )
{
    ULONG rc;

    DosRequestMutexSem( sem->hmtxWait, SEM_INDEFINITE_WAIT );

    rc = DosWaitEventSem( sem->hev, ulTimeout );

    if( !rc )
    {
        DosRequestMutexSem( sem->hmtxCount, SEM_INDEFINITE_WAIT );

        sem->count--;
        if( sem->count == 0 )
        {
            ULONG ulCount;

            DosResetEventSem( sem->hev, &ulCount );
        }

        DosReleaseMutexSem( sem->hmtxCount );
    }

    DosReleaseMutexSem( sem->hmtxWait );

    EPILOGUE;
}

/**
 * sem_wait()
 */
int sem_wait( sem_t *sem )
{
    return sem_wait_common( sem, SEM_INDEFINITE_WAIT );
}

/**
 * sem_trywait()
 */
int sem_trywait( sem_t *sem )
{
    return sem_wait_common( sem, SEM_IMMEDIATE_RETURN );
}

/* TODO: implement named semaphore support */
