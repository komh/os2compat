/*
 * Test macros for OS2COMPAT testcase
 *
 * Copyright (C) 2021-2024 KO Myung-Hun <komh78@gmail.com>
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

#define TEST_EQ     TEST_EQUAL
#define TEST_EQ_MSG TEST_EQUAL_MSG
#define TEST_NE     TEST_NOT_EQUAL
#define TEST_NE_MSG TEST_NOT_EQUAL_MSG

#define TEST_EQUAL( expr, expectd )                                         \
    do {                                                                    \
        int TEST_expr = ( int )( expr );                                    \
        int TEST_expected = ( int )( expectd );                             \
        if( TEST_expr == TEST_expected )                                    \
            fprintf( stderr, "PASSED: %d: %s = %d\n",                       \
                     __LINE__,  #expr, TEST_expr );                         \
        else {                                                              \
            fprintf( stderr, "FAILED: %d: %s = %d, Expected = %d\n",        \
                     __LINE__, #expr, TEST_expr, TEST_expected );           \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#define TEST_NOT_EQUAL( expr, expectd )                                     \
    do {                                                                    \
        int TEST_expr = ( int )( expr );                                    \
        int TEST_expected = ( int )( expectd );                             \
        if( TEST_expr != TEST_expected )                                    \
            fprintf( stderr, "PASSED: %d: %s != %d\n",                      \
                     __LINE__,  #expr, TEST_expected );                     \
        else {                                                              \
            fprintf( stderr, "FAILED: %d: %s = %d, Expected != %d\n",       \
                     __LINE__, #expr, TEST_expr, TEST_expected );           \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#define TEST_BOOL( expr, expectd )                                          \
    do {                                                                    \
        int TEST_expr = !!( expr );                                         \
        int TEST_expected = ( int )( expectd );                             \
        if( TEST_expr == TEST_expected )                                    \
            fprintf( stderr, "PASSED: %d: %s = %d\n",                       \
                     __LINE__,  #expr, TEST_expr );                         \
        else {                                                              \
            fprintf( stderr, "FAILED: %d: %s = %d, Expected = %d\n",        \
                     __LINE__, #expr, TEST_expr, TEST_expected );           \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#define TEST_EQUAL_MSG( expr, expectd, msg )                                \
    do {                                                                    \
        int TEST_expr = ( int )( expr );                                    \
        int TEST_expected = ( int )( expectd );                             \
        if( TEST_expr == TEST_expected )                                    \
            fprintf( stderr, "PASSED: %s: %d: %s = %d\n",                   \
                     ( msg ), __LINE__, #expr, TEST_expr );                 \
        else {                                                              \
            fprintf( stderr, "FAILED: %s: %d: %s = %d, Expected = %d\n",    \
                     ( msg ), __LINE__, #expr, TEST_expr, TEST_expected );  \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#define TEST_NOT_EQUAL_MSG( expr, expectd, msg )                            \
    do {                                                                    \
        int TEST_expr = ( int )( expr );                                    \
        int TEST_expected = ( int )( expectd );                             \
        if( TEST_expr != TEST_expected )                                    \
            fprintf( stderr, "PASSED: %s: %d: %s != %d\n",                  \
                     ( msg ), __LINE__,  #expr, TEST_expected );            \
        else {                                                              \
            fprintf( stderr, "FAILED: %s: %d: %s = %d, Expected != %d\n",   \
                     ( msg ), __LINE__, #expr, TEST_expr, TEST_expected );  \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#define TEST_BOOL_MSG( expr, expectd, msg )                                 \
    do {                                                                    \
        int TEST_expr = !!( expr );                                         \
        int TEST_expected = ( int )( expectd );                             \
        if( TEST_expr == TEST_expected )                                    \
            fprintf( stderr, "PASSED: %s: %d: %s = %d\n",                   \
                     ( msg ), __LINE__,  #expr, TEST_expr );                \
        else {                                                              \
            fprintf( stderr, "FAILED: %s: %d: %s = %d, Expected = %d\n",    \
                     ( msg ), __LINE__, #expr, TEST_expr, TEST_expected );  \
            abort();                                                        \
        }                                                                   \
   } while( 0 )

#define TEST_ALL_PASSED()                                                   \
    printf("All tests PASSED.\n")

#endif
