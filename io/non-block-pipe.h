/*
 * pipe() with a non-blocking mode for OS/2
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_NON_BLOCK_PIPE_H
#define OS2COMPAT_NON_BLOCK_PIPE_H

#ifdef __cplusplus
extern "C" {
#endif

int named_pipe( int *ph );
int sock_pipe( int *sv );

#ifdef __cplusplus
}
#endif

#endif
