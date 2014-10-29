/*
 * setmode() workarounded for OS/2 kLIBC
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <io.h>
#include <fcntl.h>

int _std_setmode( int handle, int mode );

int setmode( int handle, int mode )
{
    /* set a console to O_TEXT unconditionally */
    if( isatty( handle ))
        mode = O_TEXT;

    return _std_setmode( handle, mode );
}
