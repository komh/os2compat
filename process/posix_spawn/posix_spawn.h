/*
 * posix_spawn_file_actions_addchdir() and posix_spawn_file_actions_addfchdir()
 * implementation for OS/2 kLIBC
 *
 * Copyright (C) 2024 KO Myung-Hun <komh78@gmail.com>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_POSIX_SPAWN_H
#define OS2COMPAT_POSIX_SPAWN_H

#include <spawn.h>

#ifdef __cplusplus
extern "C" {
#endif

int os2compat_posix_spawn_file_actions_addchdir(posix_spawn_file_actions_t
       *restrict file_actions, const char *restrict path);

int os2compat_posix_spawn_file_actions_addfchdir(posix_spawn_file_actions_t
       *file_actions, int fildes);

#define posix_spawn_file_actions_addchdir \
    os2compat_posix_spawn_file_actions_addchdir

#define posix_spawn_file_actions_addfchdir \
    os2compat_posix_spawn_file_actions_addfchdir

#ifdef __cplusplus
}
#endif

#endif
