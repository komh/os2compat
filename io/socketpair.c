/*
 * socketpair() workaround for OS/2 kLIBC
 *
 * Copyright (C) 2024 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

/*
 * Dependencies: None
 */

#include <stdio.h>

/* OS/2 kLIBC quits a program without flushing a stream associated with a
 * socket. This leads to loss of buffered data of a stream. So flush before
 * a program ends.
 */
__attribute__((destructor))
static void flush( void )
{
    flushall();
}
