/*
 * poll() implementation for OS/2 kLIBC
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_POLL_H
#define OS2COMPAT_POLL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef POLLIN
#define POLLIN      0x0001
#endif
#ifndef POLLPRI
#define POLLPRI     0x0002
#endif
#ifndef POLLOUT
#define POLLOUT     0x0004
#endif
#ifndef POLLERR
#define POLLERR     0x0008  /* not supported */
#endif
#ifndef POLLHUP
#define POLLHUP     0x0010  /* not supported */
#endif
#ifndef POLLNVAL
#define POLLNVAL    0x0020
#endif

struct pollfd
{
    int fd;
    unsigned events;
    unsigned revents;
};

int poll( struct pollfd *fds, unsigned nfds, int timeout );

#ifdef __cplusplus
}
#endif

#endif
