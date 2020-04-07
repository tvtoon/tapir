/*
 Copyright 2017 Masaki Hara;
           2020 Tibério Vítor
 See the COPYRIGHT file at the top-level directory of this distribution.

 Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
 http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
 <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
 option. This file may not be copied, modified, or distributed
 except according to those terms.
*/

#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <SDL.h>

#include "rubyfill.h"
#include "RGSSError.h"
#include "ini.h"
#include "openres.h"
#include "tapir_config.h"

static char rtp_paths[3][PATH_MAX + 1] = { "\0", "\0", "\0" };
static size_t rtp_pathsiza[3] = { 0,0,0 };
static unsigned char rtp_slotc = 0;

static bool script_loading_flag = false;

static size_t rtp_loadinistandard( char *rtp_fullp, const char *rtp_name, const size_t rtp_paths )
{
//const char rtp_snamea[3][9] = { "Standard", "RPGVX", "RPGVXAce" };
 const char rtp_fullna[3][16] = { "RGSS/Standard/", "RGSS3/RPGVXAce/", "RGSS2/RPGVX/" };
 const size_t rtp_fullnsa[3] = { 14, 15, 12 };
 const size_t rtp_fullsnpa[3] = { 5, 6, 6 };
 const size_t rtp_fullsnsa[3] = { 8, 8, 5 };
 const size_t pathlim = PATH_MAX - 3;
 int i = 0, testi = 0;
 size_t inis = 0, rets = rtp_paths;

/* Check for standard matching version. */
 for ( i = 0; i < 3; i++ )
{
  testi = strncmp( rtp_name, &rtp_fullna[i][rtp_fullsnpa[i]], rtp_fullsnsa[i] );

  if ( testi == 0 ) break;
}

 if ( i != 3 )
{
  rtp_name = rtp_fullna[i];
  inis = rtp_fullnsa[i];
}
 else
{
  inis = strlen(rtp_name) + 1;
}

 inis += rtp_paths;

 if ( inis > pathlim )
{
#ifdef __DEBUG__
  fprintf( stderr, "RTP path is too large!" );
#endif
}
 else
{
  rets = inis;
  strncpy( &rtp_fullp[rtp_paths], rtp_name, rets );
  rtp_fullp[rets] = '\0';
//  rtp_slotc++;
}

 return(rets);
}

static unsigned char rtp_pathloop_adjust( char (* pathn)[PATH_MAX + 1], const char *opath, const size_t paths )
{
 char *carn = '\0';
 int testi = 0;
 size_t carpp = 0, carps = 0, ui = 0;
 unsigned char loopc = 1;

 strncpy( pathn[0], opath, paths );
 pathn[0][paths] = '\0';

 carn = strrchr( pathn[0], '/' );

 if ( carn != 0 )
{
  carn = &carn[1];
  carps = strlen(carn);

  for ( ui = 0; ui < carps; ui++ )
{
   testi = isalpha(carn[ui]);

   if ( testi == 1 )
{
    break;
}

}
/* Only count them if alphabet is there. */
  if ( ui != carps )
{
   strncpy( pathn[1], opath, paths );
   strncpy( pathn[2], opath, paths );
   pathn[1][paths] = pathn[2][paths] = '\0';
   carn[ui] = toupper(carn[ui]);
   carpp = paths - ( carps - ui );
   loopc = 3;

   for ( ui = carpp; ui < paths; ui++ )
{
    pathn[2][ui] = toupper(pathn[2][ui]);
}

}

}

#ifdef __DEBUG__
 printf( "Pointer: \"%s\".\nOriginal: \"%s\".\n1 up: \"%s\".\nAll up: \"%s\".\nPosition %lu proof: \"%s\".\n", carn, pathn[0], pathn[1], pathn[2], carpp, &pathn[0][carpp] );
#endif
 return(loopc);
}

/* static END */

VALUE rb_load_data(VALUE self, VALUE path)
{
 FILE *file = 0;
 VALUE rb_mMarshal = 0, str = 0;
 char buf[BUFSIZ + 1] = "\0";
 int script_loading = script_loading_flag;
 size_t numread = BUFSIZ;

 (void) self;
 script_loading_flag = 0;

 if ( script_loading == 0 )
{
  file = fopen( StringValueCStr(path), "rb" );
}

 if ( file == 0 )
{
  errno = ENOENT;
  rb_sys_fail(StringValueCStr(path));
  return(Qnil);
}
 else
{
  str = rb_str_new(0, 0);

  while ( numread == BUFSIZ )
{
   numread = fread( buf, 1, BUFSIZ, file );
   rb_str_cat(str, buf, numread);
}

  fclose(file);
  rb_mMarshal = rb_const_get_at(rb_cObject, rb_intern("Marshal"));
}

 return rb_funcall(rb_mMarshal, rb_intern("load"), 1, str);
}

int configure_rtp_path(struct ini_section *game_section)
{
 char testpath[PATH_MAX + 1] = "\0";
 const char *rtp_base_path = get_rtp_base_config(), *rtp_name = 0;
 const char rtp_inia[3][5] = { "RTP1", "RTP2", "RTP3" }, rtp_base_candidates[4][29] = { "/opt/Enterbrain/", "/usr/local/share/Enterbrain/", "/usr/share/Enterbrain/", "" };
 const size_t rtp_basesa[4] = { 16, 28, 22, 0 };
 int i = 0, reti = 0, testi = 0;
 size_t paths = 0, pathlim = PATH_MAX - 3;
 struct stat dirsta;
 unsigned char rtpbi = 3;
/* What to copy pasta. */
 if ( rtp_base_path != 0 )
{
  paths = strlen(rtp_base_path);
/* Reduce and fix this bullshit later. */
  if ( paths > pathlim )
{
   paths = 0;
}
  else
{

   if ( ( paths > 0 ) && ( rtp_base_path[paths - 1] != '/' ) )
{
    if ( ( paths + 1 ) > pathlim )
{
     paths = 0;
}
    else
{
     strncpy( testpath, rtp_base_path, paths );
     testpath[paths] = '/';
     paths++;
     testpath[paths] = '\0';
}

}
   else
{
    strncpy( testpath, rtp_base_path, paths );
    testpath[paths] = '\0';
}

}

  rtp_base_path = testpath;
}
 else
{

  for ( i = 0; i < 3; i++ )
{
   testi = stat( rtp_base_candidates[i], &dirsta );

   if ( testi == 0 )
{
    testi = S_ISDIR( dirsta.st_mode );

    if ( testi != 0 ) break;
}

}

  rtpbi = i;
  rtp_base_path = rtp_base_candidates[rtpbi];
  paths = rtp_basesa[rtpbi];
}

 for ( i = 0; i < 3; i++ )
{
  strncpy( rtp_paths[i], rtp_base_path, paths );
  rtp_paths[i][paths] = '\0';
  rtp_pathsiza[i] += paths;
#ifdef __DEBUG__
  printf( "RTP half %i: \"%s\" (%lu).\n", i, rtp_paths[i], rtp_pathsiza[i] );
#endif
}

/* INI */
 rtp_name = find_ini_entry(game_section, "RTP");

 if ( rtp_name == 0 )
{

  for ( i = 0; i < 3; i++ )
{
   rtp_name = find_ini_entry(game_section, rtp_inia[i] );
   pathlim = strlen(rtp_name);

   if ( pathlim != 0 )
{
#ifdef __DEBUG__
    printf( "Entering loop checkup with \"%s\" (%lu).\n", rtp_name, pathlim );
#endif
    paths = rtp_loadinistandard( rtp_paths[rtp_slotc], rtp_name, rtp_pathsiza[rtp_slotc] );

    if ( paths != rtp_pathsiza[rtp_slotc] )
{
     rtp_pathsiza[rtp_slotc] = paths;
     rtp_slotc++;
}
    else
{
#ifdef __DEBUG__
     fprintf( stderr, "RTP path is too large!" );
#endif
     reti = 1;
}

}

}

}
 else
{
/* Check for standard matching version. */
  paths = rtp_loadinistandard( rtp_paths[rtp_slotc], rtp_name, rtp_pathsiza[rtp_slotc] );

  if ( paths != rtp_pathsiza[rtp_slotc] )
{
   rtp_pathsiza[rtp_slotc] = paths;
   rtp_slotc++;
}
  else
{
#ifdef __DEBUG__
   fprintf( stderr, "RTP path is too large!" );
#endif
   reti = 1;
}

}

#ifdef __DEBUG__
 for ( i = 0; i < rtp_slotc; i++ )
{
  printf( "RTP %i: \"%s\" (%lu).\n", i, rtp_paths[i], rtp_pathsiza[i] );
}
#endif

 return(reti);
}

size_t loadfile_withrtp( char *pato, const char *filen, const char (*extsa)[5], const size_t filens, const size_t extotal, const size_t extrtp )
{
 FILE *arq = 0;
 char cart[3][PATH_MAX + 1] = { "\0", "\0", "\0" };
 const size_t pathlim = PATH_MAX + 1;
 size_t ui = 0, uj = 0, uk = 0;
 size_t pathps = 0, paths = filens + 4, patrs = 0;
 unsigned char caseloop = 1;

 strncpy( pato, filen, filens );

 for ( ; ui < extotal; ui++ )
{
  strncpy( &pato[filens], extsa[ui], 4 );
  pato[paths] = '\0';
  arq = fopen( pato, "rb" );

  if (arq != 0)
{
   patrs = paths;
   fclose(arq);
   break;
}

}

 if ( ui == extotal )
{

  for ( ui = 0; ui < rtp_slotc; ui++ )
{
   pathps = rtp_pathsiza[ui] + filens;
   paths = pathps + 4;

   if ( paths < pathlim )
{
    strncpy( pato, rtp_paths[ui], rtp_pathsiza[ui] );
    caseloop = rtp_pathloop_adjust( cart, filen, filens );

    for ( ; uk < caseloop; uk++ )
{
     strncpy( &pato[rtp_pathsiza[ui]], cart[uk], filens );
/* Limit to stuff like OGG or MIDI only */
     for ( uj = 0; uj < extrtp; uj++ )
{
      strncpy( &pato[pathps], extsa[uj], 4 );
      pato[paths] = '\0';
//printf( "TLRTP %lu: \"%s\".\n", uj, pato );
      arq = fopen( pato, "rb" );

      if (arq != 0)
{
       patrs = paths;
       ui = rtp_slotc - 1;
       uk = caseloop - 1;
       fclose(arq);
       break;
}

}

}

}
#ifdef __DEBUG__
   else
{
    fprintf( stderr, "Larger than PATH_MAX fail for \"%s\"!\n", pato );
}
#endif

}

}

 return(patrs);
}

unsigned char build_rtppath( /*char ** */char (*patob)[PATH_MAX + 1], const char *newdir, const size_t newdirs )
{
 size_t paths = 0;
 unsigned char ui = 0;

 for ( ; ui < rtp_slotc; ui++ )
{
  paths = newdirs + rtp_pathsiza[ui];

  if ( paths > PATH_MAX )
{
   ui = 0;
   break;
}
  else
{
   strncpy( patob[ui], rtp_paths[ui], rtp_pathsiza[ui] );
   strncpy( &patob[ui][rtp_pathsiza[ui]], newdir, newdirs );
   patob[ui][paths] = '\0';
}

}

 return(ui);
}

void flag_script_loading(void) {
  script_loading_flag = true;
}
