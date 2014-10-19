/*
 * spawnvpe() test program
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
#include <string.h>

int main( int argc, char *argv[])
{
    int arglen;
    int i;

    arglen = 0;
    for( i = 0; i < argc; i++ )
    {
        printf("argv[%d] = %s\n", i, argv[ i ]);
        arglen += strlen( argv[ i ]);
    }

    printf("Total length of passed arguments from a parent = %d\n", arglen );

    return 0;
}
