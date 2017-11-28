/*
 * mmap( MAP_FIXED | MAP_PRIVATE ) at MAP_SHARED test program
 *
 * Copyright (C) 2017 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

/*
 * XXX: Currently this test fails.
 * On Linux, mmap( MAP_FIXED | MAP_PRIVATE ) at MAP_SHARED alters the
 * attributes of the region from MAP_SHARED to MAP_PRIVATE. This is not
 * implemented, yet. However, I'm not sure this is portable, that is,
 * conforming to POSIX. Place in lowest order.
 */

/*
 * The following codes were modified from:
 * from https://stackoverflow.com/questions/22416989/change-an-mmapd-memory-region-from-map-shared-to-map-private
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "mmap.h"

#define TESTFILE "mmap-7.test"

int main(void)
{
    int fd = -1;
    void *shared_0 = NULL, *shared_1 = NULL;
    void *private_0 = NULL;
    struct stat st;
    int ret = 1;

    if(( fd = open( TESTFILE, O_CREAT | O_RDWR, S_IREAD | S_IWRITE )) < 0 )
    {
        fprintf( stderr, "Failed to open(): %s\n", strerror( errno ));

        return 1;
    }

    if( write( fd, "0123456789", 10 ) != 10 )
    {
        fprintf( stderr, "Failed to write: %s\n", strerror( errno ));

        goto cleanup;
    }

    if( fstat( fd, &st ) < 0 )
    {
        fprintf( stderr, "Failed fstat(): %s\n", strerror( errno ));

        goto cleanup;
    }

    if(( shared_0 = mmap( NULL, st.st_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0 )) == MAP_FAILED)
    {
        fprintf( stderr, "Failed to mmap( shared_0 ).\n");

        goto cleanup;
    }

    if(( shared_1 = mmap( NULL, st.st_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0 )) == MAP_FAILED )
    {
        fprintf( stderr, "Failed to mmap( shared_1 ).\n");

        goto cleanup;
    }

    if(( private_0 = mmap( shared_0, st.st_size, PROT_READ | PROT_WRITE,
                           MAP_FIXED | MAP_PRIVATE, fd, 0 )) == MAP_FAILED )
    {
        fprintf( stderr, "Failed to mmap( private_0 ).\n");

        goto cleanup;
    }


    if( shared_0 != private_0 )
    {
        fprintf(stderr, "Error: mmap() didn't map to the same region");

        goto cleanup;
    }

    printf("shared_0: %p == private_0: %p\n", shared_0, private_0 );
    printf("shared_1: %p\n", shared_1);

    printf("Shared mapping before write: %d\n", *( char * )shared_1 );
    printf("Private mapping before write: %d\n", *( char * )private_0 );

    /* write to the private COW mapping and sync changes */
    *( char * )private_0 += 0x10;

    if( msync( private_0, 1, MS_SYNC | MS_INVALIDATE ) < 0 )
    {
        fprintf(stderr, "Failed msync(): %s\n", strerror(errno));

        goto cleanup;
    }

    printf("Shared mapping after write: %d\n", *( char * )shared_1 );
    printf("Private mapping after write: %d\n", *( char * )private_0 );

    ret = *( char * )shared_1 == *( char * )private_0;

cleanup:
    if( private_0 && munmap( private_0, st.st_size ))
        fprintf( stderr, "Failed to munmap( private_0 ).\n");

    if( shared_1 && munmap( shared_1, st.st_size ))
        fprintf( stderr, "Failed to munmap( shared_1 ).\n");

    if( shared_0 && munmap( shared_0, st.st_size ))
        fprintf( stderr, "Failed to munmap( shared_0 ).\n");

    close( fd );

    remove(TESTFILE);

    if( ret )
        printf("Not implemented yet.\n");
    else
        printf("PASSED.\n");

    return ret;
}
