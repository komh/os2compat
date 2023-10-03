/*
 * Define socklen_t for OS/2 kLIBC
 *
 * Copyright (C) 2014-2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_SOCKLEN_T_H
#define OS2COMPAT_SOCKLEN_T_H

#include <sys/socket.h>

#ifndef _SOCKLEN_T_DECLARED
typedef int socklen_t;
#define _SOCKLEN_T_DECLARED
#endif

#endif
