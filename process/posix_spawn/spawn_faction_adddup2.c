/* Copyright (C) 2000, 2009-2024 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Specification.  */
#include <spawn.h>

#include <errno.h>
#include <unistd.h>

#define __sysconf(open_max) getdtablesize ()

#include "spawn_int.h"

/* Add an action to FILE-ACTIONS which tells the implementation to call
   'dup2' for the given file descriptors during the 'spawn' call.  */
int
posix_spawn_file_actions_adddup2 (posix_spawn_file_actions_t *file_actions,
                                  int fd, int newfd)
{
  int maxfd = __sysconf (_SC_OPEN_MAX);

  /* Test for the validity of the file descriptor.  */
  if (fd < 0 || newfd < 0 || fd >= maxfd || newfd >= maxfd)
    return EBADF;

  /* Allocate more memory if needed.  */
  if (file_actions->__used == file_actions->__allocated
      && __posix_spawn_file_actions_realloc (file_actions) != 0)
    /* This can only mean we ran out of memory.  */
    return ENOMEM;

  {
    struct __spawn_action *rec;

    /* Add the new value.  */
    rec = &file_actions->__actions[file_actions->__used];
    rec->tag = spawn_do_dup2;
    rec->action.dup2_action.fd = fd;
    rec->action.dup2_action.newfd = newfd;

    /* Account for the new entry.  */
    ++file_actions->__used;

    return 0;
  }
}
