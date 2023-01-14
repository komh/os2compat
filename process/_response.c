/* response.c (emx+gcc) -- Copyright (c) 1990-1996 by Eberhard Mattes
                        -- Copyright (c) 2016-2022 by KO Myung-Hun         */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klibc/startup.h>

/* This should be same as one in spawn.c */
#define RSP_ENV_FILE_KEY    "@__KLIBC_ENV_RESPONSE__@"

/**
 * Read environment strings from a response file without a size limitation
 *
 * @remark After calling this function, do not use \a envp of main(). Use
 *         environ instead.
 */
static void response_env( void )
{
    char **env;
    int key_len;
    FILE *f;
    long filesize;
    char *line, *p;

    key_len = strlen( RSP_ENV_FILE_KEY );

    /* RSP_ENV_FILE_KEY should be the last environmental varaible. */
    for( env = environ; *env && *( env + 1 ); env++ )
        /* nothing */;

    if( !*env   /* environment is empty */
        || strncmp( *env, RSP_ENV_FILE_KEY, key_len ) != 0
        || ( *env )[ key_len ] != '='
        || ( *env )[ key_len + 1 ] != '@'
        || ( f = fopen( *env + key_len + 2, "rt")) == NULL)
        return;                     /* do nothing */

    fseek( f, 0, SEEK_END );
    filesize = ftell( f );
    fseek( f, 0, SEEK_SET );

    if( filesize == 0 )
        return;

    line = malloc( filesize + 1 ); /* 1 for NUL */
    if( !line )
        goto out_of_memory;

    while( fgets( line, filesize + 1, f ) != NULL )
    {
        p = strchr( line, '\n');
        if( p != NULL )
            *p = 0;
        p = strdup( line );
        if( p == NULL )
            goto out_of_memory;

        putenv( p );
    }

    free( line );

    if( ferror( f ))
    {
        fputs("Cannot read response file for environment\n", stderr );
        exit( 255 );
    }

    fclose( f );

    /* remove RSP_ENV_FILE_KEY from environment variables */
    putenv( RSP_ENV_FILE_KEY );

    return;

out_of_memory:
    fputs("Out of memory while reading response file for environment\n", stderr );
    exit( 255 );
}

#define RPUT(x) do \
  { \
    if (new_argc >= new_alloc) \
      { \
        new_alloc += 20; \
        new_argv = (char **)realloc (new_argv, new_alloc * sizeof (char *)); \
        if (new_argv == NULL) \
          goto out_of_memory; \
      } \
    new_argv[new_argc++] = x; \
  } while (0)

/**
 * Read arguments from a response file without a size limitation
 */
static void response_arg (int *argcp, char ***argvp)
{
  int i, old_argc, new_argc, new_alloc;
  char **old_argv, **new_argv;
  char *line, *p;
  FILE *f;

  old_argc = *argcp; old_argv = *argvp;
  for (i = 1; i < old_argc; ++i)
    if (old_argv[i] != NULL
        && !(old_argv[i][-1] & (__KLIBC_ARG_DQUOTE | __KLIBC_ARG_WILDCARD | __KLIBC_ARG_SHELL))
        && old_argv[i][0] == '@')
      break;
  if (i >= old_argc)
    return;                     /* do nothing */
  new_argv = NULL; new_alloc = 0; new_argc = 0;
  for (i = 0; i < old_argc; ++i)
    {
      if (i == 0 || old_argv[i] == NULL
          || (old_argv[i][-1] & (__KLIBC_ARG_DQUOTE | __KLIBC_ARG_WILDCARD | __KLIBC_ARG_SHELL))
          || old_argv[i][0] != '@'
          || (f = fopen (old_argv[i]+1, "rt")) == NULL)
        RPUT (old_argv[i]);
      else
        {
          long filesize;

          fseek(f, 0, SEEK_END);
          filesize = ftell(f);
          fseek(f, 0, SEEK_SET);

          if (filesize == 0)
            continue;

          line = malloc(filesize+1+1); /* 1 for type, 1 for NUL */
          if (!line)
            goto out_of_memory;

          line[0] = __KLIBC_ARG_NONZERO | __KLIBC_ARG_RESPONSE;
          while (fgets (line+1, filesize+1, f) != NULL)
            {
              p = strchr (line+1, '\n');
              if (p != NULL) *p = 0;
              p = strdup (line);
              if (p == NULL)
                goto out_of_memory;
              RPUT (p+1);
            }
          free (line);
          if (ferror (f))
            {
              fputs ("Cannot read response file for arguments\n", stderr);
              exit (255);
            }
          fclose (f);
        }
    }
  RPUT (NULL); --new_argc;
  *argcp = new_argc; *argvp = new_argv;
  return;

out_of_memory:
  fputs ("Out of memory while reading response file for arguments\n", stderr);
  exit (255);
}

/**
 * Replacement of _response() of kLIBC
 *
 * Support to read arguments and environment strings from a response file
 * without a size limitation.
 *
 * @remark After calling this function, do not use \a envp of main(). Use
 *         environ instead.
 */
void _response( int *argcp, char ***argvp )
{
    response_env();
    response_arg( argcp, argvp );
}
