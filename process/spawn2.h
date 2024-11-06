/*
 * spawn2ve() and spawn2vpe()
 *
 * Copyright (C) 2024 KO Myung-Hun <komh78@gmail.com>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_SPAWN2_H
#define OS2COMPAT_SPAWN2_H

#include <libcx/spawn2.h>

#ifdef __cplusplus
extern "C" {
#endif

int os2compat_load_spawn2( void );
int os2compat_spawn2ve( int mode, const char *name, const char * const argv[],
                        const char *cwd, const char * const envp[],
                        const int stdfds[]);
int os2compat_spawn2vpe( int mode, const char *name, const char * const argv[],
                         const char *cwd, const char * const envp[],
                         const int stdfds[]);

#define load_spawn2     os2compat_load_spawn2
#define spawn2ve        os2compat_spawn2ve
#define spawn2vpe       os2compat_spawn2vpe

#ifdef __cplusplus
}
#endif

#endif
