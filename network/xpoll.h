/*
 * xpoll() implementation similar to kqueue() and epoll() for OS/2 kLIBC
 *
 * Copyright (C) 2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_XPOLL_H
#define OS2COMPAT_XPOLL_H

#include "poll.h"

#ifdef __cplusplus
extern "C" {
#endif

struct os2compat_xpollset;

/**
 * Create a xpollset instance
 *
 * @returns A new xpollset intance
 * @returns NULL on error
 */
struct os2compat_xpollset *os2compat_xpoll_create( void );

/**
 * Destroy a xpollset instance
 *
 * @param[in] xpset A xpollset instance
 * @return 0
 */
int os2compat_xpoll_destroy( struct os2compat_xpollset *xpset );

/**
 * Add a fd and an event
 *
 * @param xpset A xpollset instance
 * @param[in] fd A file descriptor to add to @a xpset
 * @param[in] event An event to poll. POLLIN, POLLOUT and POLLPRI.
 * @returns 0 on success, -1 on error with setting errno
 * @remark if @a fd
 */
int os2compat_xpoll_add( struct os2compat_xpollset *xpset,
                         int fd, unsigned event );

/**
 * Delete a fd
 *
 * @param xpset A xpollset instance
 * @param[in] fd A file descriptor to delete from @a xpset
 * @return 0 on success, -1 on error with setting errno
 */
int os2compat_xpoll_del( struct os2compat_xpollset *xpset, int fd );

/**
 * Delete closed fds after added by xpoll_add() but not deleted by xpoll_del()
 *
 * @param xpset A xpollset instance
 * @return 0 on success, -1 on error with setting errno
 */
int os2compat_xpoll_del_closed( struct os2compat_xpollset *xpset );

/**
 * Query a fd in a xpollset instance
 *
 * @param xpset A xpollset instance
 * @param[in] fd A file descriptor to query
 * @param[out] pfd The place to store the result
 * @return 0 on success, -1 on error with setting errno
 */
int os2compat_xpoll_query( struct os2compat_xpollset *xpset,
                           int fd, struct pollfd *pfd );

/**
 * Wait events
 *
 * @param xpset A xpollset instance
 * @param[out] fds The place to store the result
 * @param[in] maxfds Maximum count of file descriptors that @a fds can hold
 * @param[in] timeout Time in milli-seconds to wait
 * @returns Count of ready file descriptors up to @a maxfds
 * @returns -1 on error with setting errno
 */
int os2compat_xpoll_wait( struct os2compat_xpollset *xpset,
                          struct pollfd *fds, int maxfds, int timeout );

#define xpollset os2compat_xpollset

#define xpoll_create    os2compat_xpoll_create
#define xpoll_destroy   os2compat_xpoll_destroy
#define xpoll_add       os2compat_xpoll_add
#define xpoll_del       os2compat_xpoll_del
#define xpoll_del_closed os2compat_xpoll_del_closed
#define xpoll_query     os2compat_xpoll_query
#define xpoll_wait      os2compat_xpoll_wait

#ifdef __cplusplus
}
#endif

#endif
