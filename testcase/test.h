/*
 * Test macros for OS2COMPAT testcase
 *
 * Copyright (C) 2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_TESTCASE_TEST_H
#define OS2COMPAT_TESTCASE_TEST_H

#include <stdlib.h>

#define TEST_EQUAL( expr, expectd )                                         \
    do {                                                                    \
        int TEST_expr = ( expr );                                           \
        int TEST_expected = ( expectd );                                    \
        if( TEST_expr == TEST_expected )                                    \
            fprintf( stderr, "PASSED: %d: %s = %d\n",                       \
                     __LINE__,  #expr, TEST_expr );                         \
        else {                                                              \
            fprintf( stderr, "FAILED: %d: %s = %d, Expected = %d\n",        \
                     __LINE__, #expr, TEST_expr, TEST_expected );           \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#define TEST_BOOL( expr, expectd )                                          \
    do {                                                                    \
        int TEST_expr = !!( expr );                                         \
        int TEST_expected = ( expectd );                                    \
        if( TEST_expr == TEST_expected )                                    \
            fprintf( stderr, "PASSED: %d: %s = %d\n",                       \
                     __LINE__,  #expr, TEST_expr );                         \
        else {                                                              \
            fprintf( stderr, "FAILED: %d: %s = %d, Expected = %d\n",        \
                     __LINE__, #expr, TEST_expr, TEST_expected );           \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#endif
