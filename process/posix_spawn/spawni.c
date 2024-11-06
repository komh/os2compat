/* $Id$ */
/** @file
 *
 * System dependent worker for POSIX spawn.
 *
 * Copyright (c) 2004 knut st. osmundsen <bird-srcspam@anduin.net>
 * Copyright (c) 2024 KO Myung-Hun <komh78@gmail.com>
 *
 *
 * This file is part of Innotek LIBC.
 *
 * Innotek LIBC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Innotek LIBC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Innotek LIBC; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Dependencies: process/spawn2.c
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <spawn.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <process.h>
#include <emx/io.h>
#include <emx/umalloc.h>
#include <InnoTekLIBC/backend.h>
#include <dlfcn.h>

#include "../spawn2.h"

#include "spawn_int.h"

/*******************************************************************************
*   Structures and Typedefs                                                    *
*******************************************************************************/
typedef struct RestoreAction
{
    int tag;                            /* tag for restore action for spawn_do_close and spawn_do_open. */
    int fd;                             /* file descriptor. */
    int fFlags;                         /* the original flags of fd. */
} RESTOREACTION, *PRESTOREACTION;

static int (*libc_posix_spawn)(pid_t *, const char *,
                               const posix_spawn_file_actions_t *,
                               const posix_spawnattr_t *, char *const [],
                               char *const []) = (void *)-1L;

static int (*libc_posix_spawnp)(pid_t *, const char *,
                                const posix_spawn_file_actions_t *,
                                const posix_spawnattr_t *, char *const [],
                                char *const []) = (void *)-1L;

/**
 * Load symbols of LIBC.
 *
 * @returns a pointer to a symbol of LIBC dll on success.
 * @returns NULL on failure.
 * @param   sym     Symbol name to load
 */
static void *load_libc_sym(const char *sym)
{
    static void *libc_handle = (void *)-1L;

    if (libc_handle == (void *)-1L)
    {
        libc_handle = dlopen("libcn0", RTLD_LAZY);
        if (libc_handle == NULL)
            libc_handle = dlopen("libc066", RTLD_LAZY);
    }

    if (libc_handle == NULL)
        return NULL;

    return dlsym(libc_handle, sym);
}

/**
 * Load posix_spawn() and posix_spawnp() of LIBC.
 *
 * @returns 0 on success.
 * @returns -1 on failure.
 */
static int load_posix_spawn(void)
{
    if (libc_posix_spawn == (void *)-1L)
    {
        libc_posix_spawn = load_libc_sym("_posix_spawn");
        libc_posix_spawnp = load_libc_sym("_posix_spawnp");
    }

    if (libc_posix_spawn == NULL || libc_posix_spawnp == NULL)
        return -1;

    return 0;
}

/**
 * Save the handle fd for spawn_do_close and spawn_do_open.
 *
 * @returns 0 on success.
 * @returns -1 on failure.
 * @param   tag         Tag for restore action.
 * @param   fd          Handle to save.
 * @param   pRAction    Where to store data for restore action.
 */
static int save_handle(int tag, int fd, PRESTOREACTION pRAction)
{
    int fFlags;

    /* init restore action. */
    pRAction->tag    = -1;
    pRAction->fd     = -1;
    pRAction->fFlags = -1;

    if (!__libc_FH(fd))
        return -1;                      /* no such handle. */

    if (tag == spawn_do_close)
    {
        /* Get close on exec flag. */
        fFlags = fcntl(fd, F_GETFD);
        if (fFlags < 0)
            return -1;

        /* already marked close on exit? */
        if (fFlags & FD_CLOEXEC)
            return 0;

        /* mark it close on exit. */
        if (fcntl(fd, F_SETFD, fFlags | FD_CLOEXEC) < 0)
            return -1;

        pRAction->fFlags = fFlags;
    }

    pRAction->tag = tag;
    pRAction->fd  = fd;

    return 0;
}


/**
 * Restore a handle saved with save_handle().
 *
 * @param   pRAction    Restore action structure for spawn_do_close and spawn_do_open
 */
static void restore_handle(PRESTOREACTION pRAction)
{
    int saved_errno = errno;

    if (pRAction->tag == -1)
        return;                 /* nothing to do */

    if (pRAction->tag == spawn_do_close)
    {
        /* restore original flags */
        fcntl(pRAction->fd, F_SETFD, pRAction->fFlags);
    }
    else /* if (pRActino->tag == spawn_do_open) */
    {
        /* close temporarily opened fd */
        close(pRAction->fd);
    }

    /* reset restore action */
    pRAction->tag    = -1;
    pRAction->fd     = -1;
    pRAction->fFlags = -1;

    errno = saved_errno;
}


/* Internal OS dependent worker. */
int __os2compat_spawni(
    pid_t *pid,
    const char *path,
    const posix_spawn_file_actions_t *file_actions,
    const posix_spawnattr_t *attrp,
    const char *const argv[],
    const char *const envp[],
    int use_path)
{
    char            cwd[_MAX_PATH] = {0,};
    PRESTOREACTION  paRestore = NULL;
    PRESTOREACTION  pRAction = NULL;
    int            *paiRedirFds = NULL; /* fd pairs for redirection. */
    int            *piRedirFd = NULL;
    int             rc;                 /* result. */
    pid_t           pidChild;
    unsigned        fFlags = attrp ? attrp->__flags : 0;

    if (os2compat_load_spawn2() < 0)
    {
        int err;

        if (load_posix_spawn() < 0)
            return ENOSYS;

        if (use_path)
            err = libc_posix_spawnp(pid, path, file_actions, attrp, (char * const *)argv, (char * const *)envp);
        else
            err = libc_posix_spawn(pid, path, file_actions, attrp, (char * const *)argv, (char * const *)envp);

        /*
         * kLIBC returns -1 on failure with setting errno, but POSIX says
         * posix_spawn() and posix_spawnp() should return an error code on
         * failure.
         */
        if (err < 0)
            return errno;

        /* 0 or error code */
        return err;
    }

    if (fFlags & (POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF))
        return ENOSYS;

    if (fFlags & (POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER))
        return ENOSYS;

    if (fFlags & (POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_RESETIDS))
        return ENOSYS;

    /*
     * Apply file actions
     */
    rc = 0;
    if (file_actions)
    {
        char merged_path[_MAX_PATH];
        const char *action_path;
        int i;

        /* TODO: lock the file handle table for other threads. */

        paiRedirFds = _hmalloc(sizeof(int) * file_actions->__used * 2 + 1);
        if (!paiRedirFds)
        {
            /* TODO: release the file handle table lock. */

            return errno;
        }
        piRedirFd = paiRedirFds;

        paRestore = _hmalloc(sizeof(RESTOREACTION) * file_actions->__used);
        if (!paRestore)
        {
            free(paiRedirFds);

            /* TODO: release the file handle table lock. */

            return errno;
        }
        pRAction = paRestore;

        /* Execute the actions. */
        for (i = rc = 0; !rc && i < file_actions->__used; ++i)
        {
            struct __spawn_action  *pAction = (struct __spawn_action *)&file_actions->__actions[i]; /* nasty - const */

            switch (pAction->tag)
            {
                case spawn_do_close:
                    rc = save_handle(spawn_do_close, pAction->action.close_action.fd, pRAction);
                    if (rc == 0)
                        pRAction++;
                    break;

                case spawn_do_dup2:
                    if (!__libc_FH(pAction->action.dup2_action.fd))
                    {
                        rc = -1;
                        break;
                    }

                    *piRedirFd++ = pAction->action.dup2_action.fd;
                    *piRedirFd++ = pAction->action.dup2_action.newfd;
                    break;

                case spawn_do_open:
                    action_path = pAction->action.open_action.path;
                    if (cwd[0] && _fnisrel(action_path))
                    {
                        /* FIXME: _makepath() truncates a path exceeding _MAX_PATH. */
                        _makepath(merged_path, NULL, cwd, action_path, NULL);
                        action_path = merged_path;
                    }

                    rc = open(action_path,
                              pAction->action.open_action.oflag | O_NOINHERIT,
                              pAction->action.open_action.mode);
                    if (rc < 0)
                        break;

                    if (save_handle(spawn_do_open, rc, pRAction) < 0)
                    {
                        close(rc);
                        rc = -1;    /* set error */
                        break;
                    }
                    pRAction++;

                    *piRedirFd++ = rc;
                    *piRedirFd++ = pAction->action.open_action.fd;

                    rc = 0;         /* no error */
                    break;

                case spawn_do_chdir:
                    action_path = pAction->action.chdir_action.path;
                    if (cwd[0] && _fnisrel(action_path))
                    {
                        /* FIXME: _makepath() truncates a path exceeding _MAX_PATH. */
                        _makepath(merged_path, NULL, cwd, action_path, NULL);
                        action_path = merged_path;
                    }

                    /* FIXME: a path exceeding _MAX_PATH is truncated! */
                    strncpy(cwd, action_path, sizeof(cwd) - 1);
                    cwd[sizeof(cwd) - 1] = 0;
                    break;

                case spawn_do_fchdir:
                    if (__libc_Back_ioFHToPath(pAction->action.fchdir_action.fd, cwd, sizeof(cwd)) < 0)
                        rc = -1;
                    break;
            }
        }

        *piRedirFd = -1;        /* mark end of array */
    } /* fileactions */

    /*
     * Spawn the child process (if all went well so far).
     */
    if (!rc)
    {
        const char *dir = cwd[0] ? cwd : NULL;
        int flags = P_NOWAIT | P_2_XREDIR | (dir ? P_2_THREADSAFE : 0);

        if (use_path)
            pidChild = os2compat_spawn2vpe(flags, path, argv, dir, envp, paiRedirFds);
        else
            pidChild = os2compat_spawn2ve(flags, path, argv, dir, envp, paiRedirFds);

        if (pidChild >= 0)
        {
            /* Set return */
            *pid = pidChild;
        }
        else
            rc = errno;
    }
    else
    {
        errno = EINVAL;                 /* error in file_actions or attrp. */
        rc = errno;
    }

    /*
     * Restore the file handles.
     */
    if (paRestore)
    {
        int saved_errno = errno;
        while (--pRAction >= paRestore)
            restore_handle(pRAction);
        free(paRestore);
        errno = saved_errno;
    }

    free(paiRedirFds);

    /* TODO: release file handle table. */

    return rc;
}
