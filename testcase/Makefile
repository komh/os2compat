#
#   Configuration parts of GNU Make/GCC build system.
#   Copyright (C) 2014 by KO Myung-Hun <komh@chollian.net>
#
#   This file is part of GNU Make/GCC build system.
#
#   This program is free software. It comes without any warranty, to
#   the extent permitted by applicable law. You can redistribute it
#   and/or modify it under the terms of the Do What The Fuck You Want
#   To Public License, Version 2, as published by Sam Hocevar. See
#   http://www.wtfpl.net/ for more details.
#

##### Configuration parts that you can modify

# specify sub directories
SUBDIRS :=

# specify gcc compiler flags for all the programs
CFLAGS := -Wall -DOS2EMX_PLAIN_CHAR -funsigned-char

# specify g++ compiler flags for all the programs
CXXFLAGS := -Wall -DOS2EMX_PLAIN_CHAR -funsigned-char

# specify linker flags such as -L option for all the programs
LDFLAGS :=

# specify dependent libraries such as -l option for all the programs
LDLIBS :=

ifdef RELEASE
# specify flags for release mode
CFLAGS   +=
CXXFLAGS +=
LDFLAGS  +=
else
# specify flags for debug mode
CFLAGS   +=
CXXFLAGS +=
LDFLAGS  +=
endif

# specify resource compiler, default is rc if not set
RC :=

# specify resource compiler flags
RCFLAGS :=

# 1. specify a list of programs without an extension with
#
#   BIN_PROGRAMS
#
# Now, assume
#
#   BIN_PROGRAMS := program
#
# 2. specify sources for a specific program with
#
#   program_SRCS
#
# the above is REQUIRED
#
# 3. specify various OPTIONAL flags for a specific program with
#
#   program_CFLAGS      for gcc compiler flags
#   program_CXXFLAGS    for g++ compiler flags
#   program_LDFLAGS     for linker flags
#   program_LDLIBS      for dependent libraries
#   program_RCSRC       for rc source
#   program_RCFLAGS     for rc flags
#   program_DEF         for .def file
#   program_EXTRADEPS   for extra dependencies

BIN_PROGRAMS := mmap-1 \
                mmap-2 \
                mmap-2-child \
                mmap-3 \
                mmap-4 \
                mmap-5 \
                mmap-6 \
                mmap-7 \
                mmap-8 \
                spawnvpe-1 \
                spawnvpe-1-child \
                freopen-1 \
                setmode-1 \
                poll-1 \
                semaphore-1 \
                scandir-1 \
                if_nameindex-1 \
                _response-1 \
                getrusage-1 \
                select-1

mmap-1_SRCS   := mmap-1.c ../memory/mmap.c
mmap-1_CFLAGS := -I../memory

mmap-2_SRCS   := mmap-2.c ../memory/mmap.c
mmap-2_CFLAGS := -I../memory

mmap-2-child_SRCS   := mmap-2-child.c ../memory/mmap.c
mmap-2-child_CFLAGS := -I../memory

mmap-3_SRCS   := mmap-3.c ../memory/mmap.c
mmap-3_CFLAGS := -I../memory

mmap-4_SRCS   := mmap-4.c ../memory/mmap.c
mmap-4_CFLAGS := -I../memory

mmap-5_SRCS   := mmap-5.c ../memory/mmap.c
mmap-5_CFLAGS := -I../memory
mmap-5_LDFLAGS := -Zbin-files

mmap-6_SRCS   := mmap-6.c ../memory/mmap.c
mmap-6_CFLAGS := -I../memory

mmap-7_SRCS   := mmap-7.c ../memory/mmap.c
mmap-7_CFLAGS := -I../memory

mmap-8_SRCS   := mmap-8.c ../memory/mmap.c
mmap-8_CFLAGS := -I../memory

spawnvpe-1_SRCS   := spawnvpe-1.c ../process/spawnvpe.c
spawnvpe-1_CFLAGS := -I../process

spawnvpe-1-child_SRCS    := spawnvpe-1-child.c
spawnvpe-1-child_LDFLAGS := -Zargs-resp

freopen-1_SRCS := freopen-1.c ../io/freopen.c

setmode-1_SRCS := setmode-1.c ../io/setmode.c

poll-1_SRCS := poll-1.c ../network/poll.c
poll-1_CFLAGS := -I../network

semaphore-1_SRCS := semaphore-1.c ../thread/semaphore.c
semaphore-1_CFLAGS := -I../thread

scandir-1_SRCS := scandir-1.c ../io/scandir.c
scandir-1_CFLAGS := -I../io

if_nameindex-1_SRCS := if_nameindex-1.c ../network/if_nameindex.c
if_nameindex-1_CFLAGS := -I../network

_response-1_SRCS := _response-1.c ../process/_response.c

getrusage-1_SRCS := getrusage-1.c ../process/getrusage.c
getrusage-1_CFLAGS := -I../process

select-1_SRCS := select-1.c ../network/select.c ../io/pipe.c
select-1_CFLAGS := -I../network

# 1. specify a list of libraries without an extension with
#
#   BIN_LIBRARIES
#
# Now, assume
#
#   BIN_PROGRAMS := library
#
# 2. specify sources for a specific library with
#
#   library_SRCS
#
# the above is REQUIRED
#
# 3. set library type flags for a specific library to a non-empty value
#
#   library_LIB         to create a static library
#   library_DLL         to build a DLL
#
# either of the above SHOULD be SET
#
# 4. specify various OPTIONAL flags for a specific library with
#
#   library_CFLAGS      for gcc compiler flags
#   library_CXXFLAGS    for g++ compiler flags
#
# the above is common for LIB and DLL
#
#   library_DLLNAME     for customized DLL name without an extension
#   library_LDFLAGS     for linker flags
#   library_LDLIBS      for dependent libraries
#   library_RCSRC       for rc source
#   library_RCFLAGS     for rc flags
#   library_DEF         for .def file, if not set all the symbols are exported
#   library_EXTRADEPS   for extra dependencies
#
# the above is only for DLL

BIN_LIBRARIES :=

include Makefile.common

# additional stuffs
