/*
 * setmode() test program
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

int main( void )
{
    setmode( fileno( stdout ), O_BINARY );
    setmode( fileno( stderr ), O_BINARY );

    fprintf( stderr, "stderr:\nHello, stderr!!!\n");
    fprintf( stdout, "stdout:\nHello, stdout!!!\n");

    return 0;
}
