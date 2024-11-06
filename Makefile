#
#   Configuration parts of GNU Make/GCC build system.
#   Copyright (C) 2022 by KO Myung-Hun <komh@chollian.net>
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
CFLAGS := -Wall -DOS2EMX_PLAIN_CHAR -funsigned-char -O3

# specify g++ compiler flags for all the programs
CXXFLAGS := -Wall -DOS2EMX_PLAIN_CHAR -funsigned-char -O3

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

# set if you want not to compress resources
NO_COMPRESS_RES :=

# specify BLDLEVEL VENDOR string
BLDLEVEL_VENDOR := OS/2 Factory

# specify a macro defining version, and a file including that macro
# to generate BLDLEVEL version string
BLDLEVEL_VERSION_MACRO :=
BLDLEVEL_VERSION_FILE :=

# specify BLDLEVEL VERSION string if you want to set VERSION string manually,
# default is generated with BLDLEVEL_VERSION_MACRO and BLDLEVEL_VERSION_FILE
# if unset
BLDLEVEL_VERSION := $(shell date +%y.%m.%d)

# Variables for programs
#
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
#   program_DESC        for a BLDLEVEL description string
#   program_OUTDIR      for the place where to store .EXE

BIN_PROGRAMS :=

# Variables for libraries
#
# 1. specify a list of libraries without an extension with
#
#   BIN_LIBRARIES
#
# Now, assume
#
#   BIN_LIBRARIES := library
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
#   library_NO_EXPORT   if set, no symbols are exported in .def file
#   library_EXTRADEPS   for extra dependencies
#   library_NO_IMPLIB   if set, implib is not generated.
#   library_DESC        for a BLDLEVEL description string
#   library_OUTDIR      for the place where to store .DLL and so on
#
# the above is only for DLL

BIN_LIBRARIES := os2compat

os2compat_DIRS := io memory network process thread process/posix_spawn
os2compat_SRCS := $(foreach d,$(os2compat_DIRS),$(wildcard $(d)/*.c))
os2compat_LIB  := 1
os2compat_DLL  := 1
os2compat_DLLNAME := os2comp0
os2compat_DESC := Supplementary library for missing features in OS/2 kLIBC
os2compat_OUTDIR := lib

include Makefile.common

# additional stuffs

PREFIX := /usr/local
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include

INSTALL := ginstall

.PHONY: install install_libs install_headers

install: all install_libs install_headers

LIB_FILES := $(os2compat_OUTDIR)/$(os2compat_DLLNAME)$(DLL_EXT) \
             $(os2compat_OUTDIR)/os2compat$(LIB_EXT) \
             $(os2compat_OUTDIR)/os2compat$(OMF_LIB_EXT) \
             $(os2compat_OUTDIR)/os2compat_dll$(LIB_EXT) \
             $(os2compat_OUTDIR)/os2compat_dll$(OMF_LIB_EXT)

install_libs: $(LIB_FILES)
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL) $(LIB_FILES) $(DESTDIR)$(LIBDIR)

HEADER_FILES := include/os2compat/dirent.h \
                include/os2compat/ifaddrs.h \
                include/os2compat/netdb.h \
                include/os2compat/poll.h \
                include/os2compat/semaphore.h \
                include/os2compat/sched.h \
                include/os2compat/spawn.h \
                include/os2compat/spawn2.h \
                include/os2compat/net/if.h \
                include/os2compat/sys/mman.h \
                include/os2compat/sys/socket.h \
                include/os2compat/sys/xpoll.h

PRIV_HEADER_FILES := io/scandir.h \
                     memory/mmap.h \
                     network/cmsg.h \
                     network/getaddrinfo.h \
                     network/getifaddrs.h \
                     network/if_nameindex.h \
                     network/poll.h \
                     network/shutdown.h \
                     network/socklen_t.h \
                     network/xpoll.h \
                     process/spawn2.h \
                     process/posix_spawn/posix_spawn.h \
                     thread/semaphore.h \
                     thread/sched_yield.h

install_headers: $(HEADER_FILES) $(PRIV_HEADER_FILES )
	for h in $(subst include/,,$(HEADER_FILES)); do \
	  $(INSTALL) -D include/$$h $(DESTDIR)$(INCLUDEDIR)/$$h; \
	done
	for h in $(PRIV_HEADER_FILES); do \
	  $(INSTALL) -D $$h $(DESTDIR)$(INCLUDEDIR)/os2compat/priv/$$h; \
	done
