/*
 * POSIX sched_yield() implementation for OS/2 kLIBC
 *
 * Copyright (C) 2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_SCHED_YIELD_H
#define OS2COMPAT_SCHED_YIELD_H

#ifdef __cplusplus
extern "C" {
#endif

int os2compat_sched_yield( void );

#define sched_yield     os2compat_sched_yield

#ifdef __cplusplus
}
#endif

#endif
