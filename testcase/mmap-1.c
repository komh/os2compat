/*
 * mmap() test program
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
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#include "mmap.h"

int main( void )
{
    const char *name = "mmap.test";
    const char *msg = "This is a mmap() test\n"
                      "First malloc() and read().\n"
                      "Second mmap().\n"
                      "Finally compare each other.\n";

    int fd;
    char *contents_read;
    char *contents_mmap;
    int len;
    int rc = 0;

    fd = open( name, O_WRONLY | O_CREAT | O_BINARY );

    write( fd, msg, strlen( msg ));

    close( fd );

    fd = open( name, O_RDONLY | O_BINARY );

    len = filelength( fd );

    contents_read = malloc( len );
    if(!contents_read)
    {
        fprintf( stderr, "malloc() failed\n");

        rc = 1;
        goto exit;
    }

    read( fd, contents_read, len );

    printf("Contents from malloc() + read()\n");
    printf("-----\n");
    printf("%s", contents_read );
    printf("-----\n");

    contents_mmap = mmap( NULL, len, PROT_READ, MAP_PRIVATE, fd, 0 );
    if( contents_mmap == MAP_FAILED )
    {
        fprintf( stderr, "mmap() failed\n");

        rc = 1;
        goto exit;
    }

    printf("Contents from mmap()\n");
    printf("-----\n");
    printf("%s", contents_mmap );
    printf("-----\n");

    if( memcmp( contents_read, contents_mmap, len ))
        printf("FAILED: Contents are different from each other\n");
    else
        printf("PASSED: Contents are identical each other\n");

    free( contents_read );
    munmap( contents_mmap, len );

exit:
    close( fd );

    remove( name );

    return rc;
}
