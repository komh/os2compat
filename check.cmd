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

call checkCC 'network/socklen_t.h',, 'socklen_t socklen;';
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

    address cmd 'del' sTestFile '>nul';

    call lineout sTestFile,, 1;

    do until words(sHeaders) = 0
        parse var sHeaders sHeader sHeaders;
        call lineout sTestFile, '#include "' || sHeader || '"';
    end;

    call lineout sTestFile, '';
    call lineout sTestFile, 'int main( void )';
    call lineout sTestFile, '{';
    call lineout sTestFile, sTests;
    call lineout sTestFile, '    return 0;';
    call lineout sTestFile, '}';

    call lineout sTestFile;

    sCmd = 'gcc -o' sTestFileExe sTestFile sSources;

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
