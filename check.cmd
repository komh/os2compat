/*
 * check.cmd to test the compilation and the linkage of os2compat
 * implementation
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

G.sNl = x2c('0d') || x2c('0a');
G.sTmpDir = value('TMPDIR',, 'OS2ENVIRONMENT');

call checkCC 'network/socklen_t.h',, 'socklen_t socklen; (void)socklen;';
call checkCC 'network/shutdown.h'
call checkCC 'network/poll.h', 'network/poll.c', 'poll( 0, 0, 0 );';
call checkCC 'network/getaddrinfo.h', 'network/getaddrinfo.c', ,
             'gai_strerror( 0 );' || g.sNl ||,
             'getaddrinfo( 0, 0, 0, 0 );' || g.sNl ||,
             'freeaddrinfo( 0 );' || g.sNl ||,
             'getnameinfo( 0, 0, 0, 0, 0, 0, 0 );';
call checkCC 'memory/mmap.h', 'memory/mmap.c', ,
             'mmap( 0, 0, 0, 0, 0, 0);' || g.sNl ||,
             'mprotect( 0, 0, 0);' || g.sNl ||,
             'munmap( 0, 0 );' || g.sNl ||,
             'mmap_anon( 0, 0, 0, 0, 0);';
call checkCC 'process.h', 'process/spawnvpe.c', 'spawnvpe( 0, 0, 0, 0 );';
call checkCC 'stdio.h', 'io/freopen.c', 'freopen( 0, 0, 0 );';
call checkCC 'io.h fcntl.h', 'io/setmode.c', 'setmode( 0, 0 );';
call checkCC 'thread/semaphore.h', 'thread/semaphore.c', ,
             'sem_init( 0, 0, 0 );' || g.sNl ||,
             'sem_destroy( 0 );' || g.sNl ||,
             'sem_post( 0 );' || g.sNl ||,
             'sem_wait( 0 );' || g.sNl ||,
             'sem_trywait( 0 );';
call checkCC 'io/scandir.h', 'io/scandir.c', 'scandir( 0, 0, 0, alphasort );';
call checkCC 'network/cmsg.h',, 'return CMSG_LEN( 0 ) + CMSG_SPACE( 0 );';
call checkCC 'network/if_nameindex.h', 'network/if_nameindex.c', ,
             'struct if_nameindex *nis; (void)nis;' || g.sNL ||,
             'if_nameindex();' || g.sNL ||,
             'if_freenameindex( 0 );' || g.sNL ||,
             'if_indextoname( 0, 0 );' || g.sNL ||,
             'if_nametoindex( 0 );';
call checkCC 'stdlib.h', 'process/_response.c', '_response( 0, 0 );';
call checkCC 'sys/types.h sys/resource.h', 'process/getrusage.c', ,
             'getrusage( 0, 0 );';
call checkCC 'sys/select.h', 'network/select.c', 'select( 0, 0, 0, 0, 0 );';
call checkCC 'fcntl.h', 'io/fcntl.c', 'fcntl( 0, 0, 0 );';
call checkCC 'io.h', 'io/pipe.c', 'pipe( 0 );';

say 'Check completed';

exit 0;

checkCC: procedure expose G.
    parse arg sHeaders, sSources, sTests;

    say 'Checking with:';
    say '    headers =' sHeaders;
    say '    sources =' sSources;
    say '    tests =' sTests;

    sTestFileBase = G.sTmpDir;
    if right(sTestFileBase, 1) = '\' then
        sTestFileBase = delstr(sTestFileBase, length(sTestFileBase), 1);
    sTestFileBase = sTestFileBase || '\os2compat_test';

    sTestFile = sTestFileBase || '.c';
    sTestFileExe = sTestFileBase || '.exe';

    address cmd 'del' sTestFile '>nul 2>&1';

    call lineout sTestFile,, 1;

    sHeaderList = sHeaders;
    do while words(sHeaderList) > 0
        parse var sHeaderList sHeader sHeaderList;
        call lineout sTestFile, '#include "' || sHeader || '"';
    end;

    call lineout sTestFile, '';
    call lineout sTestFile, 'int main( void )';
    call lineout sTestFile, '{';
    call lineout sTestFile, sTests;
    call lineout sTestFile, '    return 0;';
    call lineout sTestFile, '}';

    call lineout sTestFile;

    sCmd = 'gcc -Wall -DOS2EMX_PLAIN_CHAR -funsigned-char -o' sTestFileExe,
           sTestFile sSources;

    address cmd sCmd;

    if rc <> 0 then
    do
        say 'Check failed:';
        say '    headers =' sHeaders;
        say '    sources =' sSources;
        say '    tests =' sTests;
        say '    cmd =' sCmd;
        exit 1;
    end;

    say 'Check passed';

    return;
