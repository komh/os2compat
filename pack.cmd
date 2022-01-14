/* pack.cmd to package a project using GNU Make/GCC build system and git */

call setlocal

/****** Configuration parts begin ******/

sPackageName = 'os2compat'
sRepoDir = '.'

sVerMacro = ''
sVerHeader = ''

sDistFiles = 'README' '/',
             'donation.txt' '/'

/****** Configuration parts end ******/

'echo' 'on'

sCmd = 'sh' '-c' '"date +%%y.%%m.%%d"'
sVer = getOutput( sCmd )
sPackageNameVer = sPackageName || '-' || sVer

'sed' '-e' 's/@VER@/' || sVer || '/g',
       sPackageName || '.txt' '>' sPackageNameVer || '.txt'

sDistFiles = sDistFiles,
             sPackageNameVer || '.txt' '/.'

'mkdir' sPackageNameVer

'gmake' 'clean'
'gmake' 'RELEASE=1'
'gmake' 'install' 'PREFIX=/usr' 'DESTDIR=' || sPackageNameVer

do while strip( sDistFiles ) \= ''
    parse value sDistFiles with sSrc sDestDir sDistFiles

    'ginstall' '-d' sPackageNameVer || sDestDir
    'ginstall' sRepoDir || '/' || sSrc sPackageNameVer || sDestDir
end

'git' 'tag' '-d' sVer '>nul' '2>&1'
'git' 'tag' '-a' '-m' '"tag v' || sVer || '"' sVer

'git' 'archive' '--format' 'zip' sVer '--prefix' sPackageNameVer || '/' '>',
       sPackageNameVer || '/' || sPackageNameVer || '-src.zip'

'rm' '-f' sPackageNameVer || '.zip'
'zip' '-rpSm' sPackageNameVer || '.zip' sPackageNameVer

call endlocal

exit 0

/* Get outputs from commands */
getOutput: procedure
    parse arg sCmd

    nl = x2c('d') || x2c('a')

    rqNew = rxqueue('create')
    rqOld = rxqueue('set', rqNew )

    address cmd sCmd '| rxqueue' rqNew

    sResult = ''
    do while queued() > 0
        parse pull sLine
        sResult = sResult || sLine || nl
    end

    call rxqueue 'Delete', rqNew
    call rxqueue 'Set', rqOld

    /* Remove empty lines at end */
    do while right( sResult, length( nl )) = nl
        sResult = delstr( sResult, length( sResult ) - length( nl ) + 1 )
    end

    return sResult
