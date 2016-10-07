/*
 * mmap(MAP_FIXED) test program for not allocated case
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This file was modifed from /usr/share/autoconf/autoconf/functions.m4 of
 * autoconf 2.69.
 *
 * This file should be linked with -Zbin-files linker flag.
 *
 * The following is the original license header.
 *
# This file is part of Autoconf.             -*- Autoconf -*-
# Checking for functions.
# Copyright (C) 2000-2012 Free Software Foundation, Inc.

# This file is part of Autoconf.  This program is free
# software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Under Section 7 of GPL version 3, you are granted additional
# permissions described in the Autoconf Configure Script Exception,
# version 3.0, as published by the Free Software Foundation.
#
# You should have received a copy of the GNU General Public License
# and a copy of the Autoconf Configure Script Exception along with
# this program; see the files COPYINGv3 and COPYING.EXCEPTION
# respectively.  If not, see <http://www.gnu.org/licenses/>.

# Written by David MacKenzie, with help from
# Franc,ois Pinard, Karl Berry, Richard Pixley, Ian Lance Taylor,
# Roland McGrath, Noah Friedman, david d zuhn, and many others.
 *
 */

/* Thanks to Mike Haertel and Jim Avera for this test.
   Here is a matrix of mmap possibilities:
    mmap private not fixed
    mmap private fixed at somewhere currently unmapped
    mmap private fixed at somewhere already mapped
    mmap shared not fixed
    mmap shared fixed at somewhere currently unmapped
    mmap shared fixed at somewhere already mapped
   For private mappings, we should verify that changes cannot be read()
   back from the file, nor mmap's back from the file at a different
   address.  (There have been systems where private was not correctly
   implemented like the infamous i386 svr4.0, and systems where the
   VM page cache was not coherent with the file system buffer cache
   like early versions of FreeBSD and possibly contemporary NetBSD.)
   For shared mappings, we should conversely verify that changes get
   propagated back to all the places they're supposed to be.

   Grep wants private fixed already mapped.
   The main things grep needs to know about mmap are:
   * does it exist and is it safe to write into the mmap'd area
   * how to use it (BSD variants)  */

#include <fcntl.h>

#include "mmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#define RETURN(x)                               \
    do {                                        \
        fprintf (stderr, "Error %d\n", (x));    \
        return (x);                             \
    } while (0)

int
main ()
{
  char *data, *data2, *data3;
  const char *cdata2;
  int i, pagesize;
  int fd, fd2;

  pagesize = getpagesize ();

  /* First, make a file with some known garbage in it. */
  data = (char *) malloc (pagesize);
  if (!data)
    RETURN (1);
  for (i = 0; i < pagesize; ++i)
    *(data + i) = rand ();
  umask (0);
  fd = creat ("conftest.mmap", 0600);
  if (fd < 0)
    RETURN (2);
  if (write (fd, data, pagesize) != pagesize)
    RETURN (3);
  close (fd);

  /* Next, check that the tail of a page is zero-filled.  File must have
     non-zero length, otherwise we risk SIGBUS for entire page.  */
  fd2 = open ("conftest.txt", O_RDWR | O_CREAT | O_TRUNC, 0600);
  if (fd2 < 0)
    RETURN (4);
  cdata2 = "";
  if (write (fd2, cdata2, 1) != 1)
    RETURN (5);
  data2 = (char *) mmap (0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0L);
  if (data2 == MAP_FAILED)
    RETURN (6);
  for (i = 0; i < pagesize; ++i)
    if (*(data2 + i))
      RETURN (7);
  close (fd2);
  if (munmap (data2, pagesize))
    RETURN (8);

  /* Next, try to mmap the file at a fixed address which already has
     something else allocated at it.  If we can, also make sure that
     we see the same garbage.  */
  fd = open ("conftest.mmap", O_RDWR);
  if (fd < 0)
    RETURN (9);
  if (data2 != mmap (data2, pagesize, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_FIXED, fd, 0L))
    RETURN (10);
  for (i = 0; i < pagesize; ++i)
    if (*(data + i) != *(data2 + i))
      RETURN (11);

  /* Finally, make sure that changes to the mapped area do not
     percolate back to the file as seen by read().  (This is a bug on
     some variants of i386 svr4.0.)  */
  for (i = 0; i < pagesize; ++i)
    *(data2 + i) = *(data2 + i) + 1;
  data3 = (char *) malloc (pagesize);
  if (!data3)
    RETURN (12);
  if (read (fd, data3, pagesize) != pagesize)
    RETURN (13);
  for (i = 0; i < pagesize; ++i)
    if (*(data + i) != *(data3 + i))
      RETURN (14);
  close (fd);
  RETURN (0);
}
