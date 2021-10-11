/*
 * scandir() and alphasort() implementation for OS/2 kLIBC
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <dirent.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "scandir.h"

/**
 * scandir()
 *
 * @remark OS/2 kLIBC declares scandir() differently from POSIX
 */
int scandir( const char *dir, struct dirent ***namelist,
             int ( *sel )(/* const */ struct dirent * ),
             int ( *compare )( const /* struct dirent ** */ void *,
                               const /* struct dirent ** */ void *))
{
    DIR *dp;
    struct dirent **list = NULL;
    size_t list_size = 0;
    int list_count = 0;
    struct dirent *d;
    int saved_errno;

    dp = opendir( dir );
    if( dp == NULL )
        return -1;

    /* Save original errno */
    saved_errno = errno;

    /* Clear errno to test later */
    errno = 0;

    while(( d = readdir( dp )) != NULL )
    {
        int selected = !sel || sel( d );

        /* Clear errno modified by sel() */
        errno = 0;

        if( selected )
        {
            struct dirent *d1;
            size_t len;

            /* List full ? */
            if( list_count == list_size )
            {
                struct dirent **new_list;

                if( list_size == 0 )
                    list_size = 20;
                else
                    list_size += list_size;

                new_list = ( struct dirent ** )realloc( list,
                                                        list_size *
                                                            sizeof( *list ));
                if( !new_list )
                {
                    errno = ENOMEM;
                    break;
                }

                list = new_list;
            }

            /* On OS/2 kLIBC, d_name is not the last member of struct dirent.
             * So just allocate as many as the size of struct dirent. */
            len = sizeof( struct dirent );
            d1 = ( struct dirent * )malloc( len );
            if( !d1 )
            {
                errno = ENOMEM;

                break;
            }

            memcpy( d1, d, len );

            list[ list_count++ ] = d1;
        }
    }

    /* Error ? */
    if( errno )
    {
        /* Store errno to use later */
        saved_errno = errno;

        /* Free a directory list */
        while( list_count > 0 )
            free( list[ --list_count ]);
        free( list );

        /* Indicate an error */
        list_count = -1;
    }
    else
    {
        /* If compare is present, sort */
        if( compare != NULL )
            qsort( list, list_count, sizeof( *list ), compare );

        *namelist = list;
    }

    /* Ignore error of closedir() */
    closedir( dp );

    errno = saved_errno;

    return list_count;
}

/**
 * alphasort()
 *
 * @remark OS/2 kLIBC requires parameters of compare function differently
 * from POSIX
 */
int alphasort( const /* struct dirent **d1 */ void *p1,
               const /* struct dirent **d2 */ void *p2 )
{
    struct dirent **d1 = ( struct dirent ** )p1;
    struct dirent **d2 = ( struct dirent ** )p2;

    return strcoll(( *d1 )->d_name, ( *d2 )->d_name);
}

