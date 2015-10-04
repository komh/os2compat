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
CFLAGS := -Wall

# specify g++ compiler flags for all the programs
CXXFLAGS := -Wall

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
                spawnvpe-1 \
                spawnvpe-1-child \
                non-block-pipe-1 \
                non-block-pipe-2 \
                freopen-1 \
                setmode-1 \
                poll-1

mmap-1_SRCS   := mmap-1.c ../memory/mmap.c
mmap-1_CFLAGS := -I../memory

mmap-2_SRCS   := mmap-2.c ../memory/mmap.c
mmap-2_CFLAGS := -I../memory

mmap-2-child_SRCS   := mmap-2-child.c ../memory/mmap.c
mmap-2-child_CFLAGS := -I../memory

mmap-3_SRCS   := mmap-3.c ../memory/mmap.c
mmap-3_CFLAGS := -I../memory

spawnvpe-1_SRCS   := spawnvpe-1.c ../process/spawnvpe.c
spawnvpe-1_CFLAGS := -I../process

spawnvpe-1-child_SRCS    := spawnvpe-1-child.c
spawnvpe-1-child_LDFLAGS := -Zargs-resp

non-block-pipe-1_SRCS := non-block-pipe-1.c ../io/non-block-pipe.c
non-block-pipe-1_CFLAGS := -I../io

non-block-pipe-2_SRCS := non-block-pipe-1.c ../io/non-block-pipe.c
non-block-pipe-2_CFLAGS := -I../io

freopen-1_SRCS := freopen-1.c ../io/freopen.c

setmode-1_SRCS := setmode-1.c ../io/setmode.c

poll-1_SRCS := poll-1.c ../network/poll.c
poll-1_CFLAGS := -I../network

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
