/*
 RGSSAD extractor.
*/
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <tunalib/caractere.h>
#include <tunalib/matsimples.h>

#include "rgssad.h"

int copyofmy_mkpath( char *path, const size_t pathl, const mode_t defperm )
{
 struct stat sb;
 int mkdir_errno = 0, ti = 0;
 size_t curlen = 0, postot = 0, sumcred = 0;
/* Absolute path. */
 if ( path[0] == '/' ) sumcred = 1;

 for ( ; path[postot] != '\0'; sumcred = 1 )
{
  postot += sumcred;
  curlen = strcspn( &path[postot], "/" );
  postot += curlen;
  path[postot] = '\0';
  ti = mkdir( path, defperm );

/* The directory exists? */
  if ( ti != 0 )
{
   mkdir_errno = errno;
/* Not there. */
   ti = stat(path, &sb);

   if ( ti != 0 )
{
    errno = mkdir_errno;
    return(1);
}

   ti = S_ISDIR(sb.st_mode);

   if ( ti == 0 )
{
    errno = ENOTDIR;
    return(1);
}

}

  if ( postot != pathl ) path[postot] = '/';
}

 return (0);
}

int rgssad_createhierarchy( const rgssa_subhdr *rgsssh, const size_t filec )
{
/*
 char *testp = '\0';
*/
 char **dirusa = '\0';
 const mode_t defperm = ( 0777 & (~umask(0)) ) | S_IRWXU;
 int errc = 0;
 size_t *availi = 0, *finali = 0, *pathsa = 0;
 size_t freeu = 0, difnow = 0, ui = 0;

 pathsa = (size_t *)malloc( sizeof(size_t) * filec );

 if ( pathsa == 0 )
{
  fprintf( stderr, "Cannot allocate path size array!\n" );
  return(1);
}

 availi = (size_t *)malloc( sizeof(size_t) * filec );

 if ( availi == 0 )
{
  fprintf( stderr, "Cannot allocate base index array!\n" );
  free(pathsa);
  return(1);
}
/* Diretórios aqui. */
 ui = rgssad_dirpos( pathsa, rgsssh, filec );

 if ( ui != filec)
{
  free(pathsa);
  free(availi);
  return(2);
}

/* Índices seqüenciais. */
 difnow = rgssad_difseqst( availi, pathsa, rgsssh, filec );

/* Processa os índices exclusivos aqui. */
 finali = (size_t *)malloc( sizeof(size_t) * difnow );

 if ( finali == 0 )
{
  fprintf( stderr, "Cannot allocate final index array!\n" );
  free(pathsa);
  free(availi);
  return(2);
}

 difnow = rgssad_difpara( finali, availi, difnow );

 if ( difnow == 0 )
{
  fprintf( stderr, "Zero parallel array!\n" );
  free(finali);
  free(availi);
  free(pathsa);
  return(2);
}
/* Resultado final e criação de diretórios aqui. */
 dirusa = (char **)malloc( sizeof( char * ) * difnow );

 if ( dirusa == 0 )
{
  fprintf( stderr, "Cannot allocate base directory array!\n" );
  free(finali);
  free(availi);
  free(pathsa);
  return(2);
}

 for ( ui = 0; ui < difnow; ui++ )
{
  dirusa[ui] = ( char * )malloc( sizeof( char ) * ( PATH_MAX + 1 ) );

  if ( dirusa[ui] == 0 )
{
   break;
}

}
// Break, use ui to free stuff...
 if ( ui != difnow )
{
  errc = 1;
  freeu = ui;
  difnow = 0;
}
 else
{
  freeu = difnow;
}

#ifdef __DEBUG__
 printf( "Difnow e errc: %lu e %i.\n", difnow, errc );
#endif

 for ( ui = 0; (ui < difnow) && (errc == 0); ui++ )
{
//  printf( "%lu em %lu: \"%s\" (%u).\n", ui, finali[ui], rgsssh[finali[ui]].filen, rgsssh[finali[ui]].filens );
  strncpy( dirusa[ui], rgsssh[finali[ui]].filen, pathsa[finali[ui]] );
  dirusa[ui][pathsa[finali[ui]]] = '\0';
/*
#ifdef __DEBUG__
  printf( "%s\n", dirusa[ui] );
#endif
*/
  errc = copyofmy_mkpath( dirusa[ui], pathsa[finali[ui]], defperm );
}
// CLEAR
 for ( ui = 0; ui < freeu; ui++ ) free(dirusa[ui]);

 free(dirusa);
 free(finali);
 free(availi);
 free(pathsa);
 return(errc);
}

int rgssad_extractf( FILE *rgssarc, const rgssa_subhdr *rgsssh, long int curpos, const size_t maxbsize, const size_t filec )
{
 FILE *savef = 0;
 const size_t breakc = filec - 1;
 int reti = 0;
 size_t curfiles = 0, readt = 0, ui = 0, uj = 0;
 unsigned char *bufmod = 0, *buforg = 0;

 buforg = (unsigned char *)malloc( sizeof(unsigned char) * maxbsize );

 if ( buforg == 0 )
{
#ifdef __DEBUG__
  fprintf( stderr, "Memory error for original!\n" );
#endif
  return(1);
}

 bufmod = (unsigned char *)malloc( sizeof(unsigned char) * maxbsize );

 if ( bufmod == 0 )
{
#ifdef __DEBUG__
  fprintf( stderr, "Memory error for unencrypted!\n" );
#endif
  free(buforg);
  return(1);
}

 for ( ui = 0, uj = curpos; ui < filec; ui++, uj += readt + curpos )
{
  curfiles = rgsssh[ui].files;
  curpos = rgsssh[ui].pos - uj;
#ifdef __DEBUG__
  printf( "Move and position: %lX (%lX).\n", curpos, ftell(rgssarc) );
#endif
  reti = fseek( rgssarc, curpos, SEEK_CUR );

  if ( reti != 0 )
{
#ifdef __DEBUG__
   fprintf( stderr, "Error seeking %lu!\n", ui );
#endif
   ui = breakc;
   reti = 2;
   curfiles = 0;
}

  savef = fopen( rgsssh[ui].filen, "wb" );

  if ( savef == 0 )
{
#ifdef __DEBUG__
   fprintf( stderr, "Error opening file to save!\n" );
#endif
   ui = breakc;
   reti = 2;
   curfiles = 0;
}

  readt = fread( buforg, 1, curfiles, rgssarc );

  if ( readt != curfiles )
{
#ifdef __DEBUG__
   fprintf( stderr, "Memory error for unencrypted!\n" );
#endif
   ui = breakc;
   reti = 2;
   curfiles = 0;
}

//  rgssad_decriptestem3( bufmod, buforg, rgsssh[ui] );
  rgssad_decriptestem3( bufmod, buforg, curfiles, rgsssh[ui].datakey );
  fwrite( bufmod, 1, curfiles, savef );
  fclose(savef);
}

 free(buforg);
 free(bufmod);
 return(reti);
}

size_t rgssad_difpara( size_t *finali, size_t *availi, const size_t arrayc )
{
/* size_t *maska = 0;*/
 size_t difac = arrayc, diffin = 0, difpar = 0, maska = 0, ui = 0, valnow = (size_t)-1;
/*
 maska = (size_t *)malloc( sizeof(size_t) * arrayc );

 if ( maska == 0 )
{
  fprintf( stderr, "Cannot allocate replace index array!\n" );
  return(0);
}
*/
 for ( ; difac > 0; difac = difpar, difpar = 0 )
{
/*
  for ( ui = 0; ui < difac; ui++ )
{
   maska[ui] = availi[ui];
}
*/
  for ( ui = 0; ui < difac; ui++ )
{
   maska = availi[ui];

/*   if ( valnow != maska[ui] ) */
   if ( valnow != maska )
{
/*    availi[difpar] = maska[ui];*/
    availi[difpar] = maska;
    difpar++;
}

}

  if ( difpar > 0 )
{
   finali[diffin] = valnow = availi[0];
#ifdef __DEBUG__
   printf( "%lu final index: %lu.\n", diffin, availi[0] );
#endif
   diffin++;
}

}

/* free(maska);*/
 return(diffin);
}

size_t rgssad_difseqst( size_t *availi, const size_t *pathsa, const rgssa_subhdr *rgsssh, const size_t filec )
{
 char patati[PATH_MAX + 1] = "\0";
 int i = 0;
 size_t difseq = 0, ui = 0;

 for ( ui = 0; ui < filec; ui++ )
{
  i = strncmp( patati, rgsssh[ui].filen, pathsa[ui] );

  if ( i != 0 )
{
   availi[difseq] = ui;
#ifdef __DEBUG__
   printf( "%lu index: %lu.\n", difseq, ui );
#endif
   difseq++;
   strncpy( patati, rgsssh[ui].filen, pathsa[ui] );
   patati[pathsa[ui]] = '\0';
}

}

 return(difseq);
}

size_t rgssad_dirpos( size_t *pathsa, const rgssa_subhdr *rgsssh, const size_t filec )
{
 char patati[PATH_MAX + 1] = "\0";
 size_t ui = 0;

 for ( ui = 0; ui < filec; ui++ )
{
//  testp = dirname( rgsssh->filen[ui] );
  pathsa[ui] = char_rpos( rgsssh[ui].filen, "/", rgsssh[ui].filens, 1 );

  if ( pathsa[ui] == 0 )
{

   if ( rgsssh[ui].filen[pathsa[ui]] == '/' )
{
#ifdef __DEBUG__
    fprintf( stderr, "Invalid absolute path: \"%s\".\n", rgsssh[ui].filen );
#endif
    break;
}

}

#ifdef __DEBUG__
  printf( "Path: \"%s\" (%u).\n", rgsssh[ui].filen, rgsssh[ui].filens );
  strncpy( patati, rgsssh[ui].filen, pathsa[ui] );
  patati[pathsa[ui]] = '\0';
  printf( "Copied path: \"%s\" (%lu).\n", patati, pathsa[ui] );
#endif
}

 return(ui);
}

size_t rgssad_getheadf( FILE *rgssf, rgssa_hdr *rgssh )
{
 size_t count = 0;

 count += fread( &rgssh->magic, 1, 7, rgssf );
 count += fread( &rgssh->version, 1, 1, rgssf );
 count += fread( &rgssh->key, 1, 4, rgssf );
 return(count);
}

size_t rgssad1_countsubheadf( FILE *rgssf, const unsigned int key )
{
 int i = 0;
 rgssa_subhdr rgsssh = { 0,0,key,0,"\0" };
 size_t count = 0, testr = RGSS1_MINISHDR + 1, totalr = 0;

 for ( rgsssh.pos = 1; ( i == 0 ) && ( testr > RGSS1_MINISHDR ); count++, totalr += testr + rgsssh.files, rgsssh.files = 0 )
{
  testr = rgssad1_getsubheadf( rgssf, &rgsssh, rgsssh.datakey );
  i = fseek( rgssf, rgsssh.files, SEEK_CUR );
//  rgsssh->pos;
}

 i += fseek( rgssf, -(totalr), SEEK_CUR );

 if ( ( testr != 0 ) || ( i != 0 ) )
{
  count = 0;
}
 else
{
  count--;
}

#ifdef __DEBUG__
 printf( "Total reading: %lu (%i).\n", totalr, i );
#endif
 return(count);
}

size_t rgssad1_getsubheadf( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key )
{
 size_t count = 0, ui = 0;

 count += fread( &rgsssh->filens, 1, 4, rgssf );
 rgsssh->filens ^= key;
 rgsssh->datakey = key * 7 + 3;

 if ( rgsssh->filens < ( PATH_MAX + 1 ) )
{
  count += fread( rgsssh->filen, 1, rgsssh->filens, rgssf );
  rgsssh->filen[rgsssh->filens] = '\0';

  for ( ; ui < rgsssh->filens; ui++ )
{
   rgsssh->datakey = rgsssh->datakey * 7 + 3;
}

  count += fread( &rgsssh->files, 1, 4, rgssf );
  rgsssh->files ^= rgsssh->datakey;
  rgsssh->datakey = rgsssh->datakey * 7 + 3;
}
#ifdef __DEBUG__
 else
{
  fprintf( stderr, "EFileNameSize: %u.\n", rgsssh->filens );
}
#endif

 return(count);
}

size_t rgssad1_getsubheadfv( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key )
{
 size_t count = 0, ui = 0;

 count += fread( &rgsssh->filens, 1, 4, rgssf );
 rgsssh->filens ^= key;
 rgsssh->datakey = key * 7 + 3;
 count += fread( rgsssh->filen, 1, rgsssh->filens, rgssf );

 for ( ; ui < rgsssh->filens; ui++ )
{
  rgsssh->filen[ui] ^= rgsssh->datakey;
  rgsssh->datakey = rgsssh->datakey * 7 + 3;
}

 rgsssh->filen[ui] = '\0';
 count += fread( &rgsssh->files, 1, 4, rgssf );
 rgsssh->files ^= rgsssh->datakey;
 rgsssh->datakey = rgsssh->datakey * 7 + 3;
 return(count);
}

size_t rgssad3_countsubheadf( FILE *rgssf, const unsigned int key )
{
 int i = 0;
 rgssa_subhdr rgsssh;
 size_t count = 0, testr = RGSS3_MINISHDR + 1, totalr = 0;

 for ( rgsssh.pos = 1; testr > RGSS3_MINISHDR; count++, totalr += testr )
{
  testr = rgssad3_getsubheadf( rgssf, &rgsssh, key );
}

 i = fseek( rgssf, -(totalr), SEEK_CUR );

 if ( ( rgsssh.pos != 0 ) || ( i != 0 ) )
{
  count = 0;
}
 else
{
  count--;
}

#ifdef __DEBUG__
 printf( "Total reading: %lu (%i).\n", totalr, i );
#endif
 return(count);
}

size_t rgssad3_getsubheadf( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key )
{
 size_t count = 0;

 count += fread( &rgsssh->pos, 1, 4, rgssf );
 count += fread( &rgsssh->files, 1, 4, rgssf );
 count += fread( &rgsssh->datakey, 1, 4, rgssf );
 count += fread( &rgsssh->filens, 1, 4, rgssf );

 rgsssh->pos ^= key;
 rgsssh->filens ^= key;

 if ( rgsssh->pos != 0 )
{

  if ( rgsssh->filens < ( PATH_MAX + 1 ) )
{
   count += fread( rgsssh->filen, 1, rgsssh->filens, rgssf );
   rgsssh->filen[rgsssh->filens] = '\0';
}

}

 return(count);
}

size_t rgssad3_getsubheadfv( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key )
{
 size_t count = 0, ui = 0;

 count += fread( &rgsssh->pos, 1, 4, rgssf );
 count += fread( &rgsssh->files, 1, 4, rgssf );
 count += fread( &rgsssh->datakey, 1, 4, rgssf );
 count += fread( &rgsssh->filens, 1, 4, rgssf );

 rgsssh->pos ^= key;
 rgsssh->files ^= key;
 rgsssh->datakey ^= key;
 rgsssh->filens ^= key;

 count += fread( rgsssh->filen, 1, rgsssh->filens, rgssf );

 for ( ; ui < rgsssh->filens; ui++ )
{
  rgsssh->filen[ui] ^= key >> ((ui & 3) << 3);
}

 rgsssh->filen[ui] = '\0';
 return(count);
}

void rgssad_inihead( rgssa_hdr *rgssh )
{
 rgssh->magic[0] = '\0';
 rgssh->version = 0;
 rgssh->key = 0;
}

void rgssad_inisubhead( rgssa_subhdr *rgsssh )
{
 rgsssh->pos = 0;
 rgsssh->files = 0;
 rgsssh->datakey = 0;
 rgsssh->filens = 0;
 rgsssh->filen[0] = '\0';
}

void rgssad_decryptrealm( unsigned char *bufmod, const unsigned char *buforg, const unsigned int curpos, const unsigned int files, unsigned int datakey )
{
 unsigned int align = files, aloop = 0, mul = 0, nopost = 0, postr = 0, prer = 0, remain = curpos % 4, value = 0xCCCCCCCC;
 unsigned int bufpos = 0, ui = 0;
/* The mask */
 if ( remain != 0 )
{
  prer = minimo( 4 - remain, files );
/*  fread( &value, 1, prer, rgssf );*/
  memcpy( &value, &buforg[bufpos], prer );
  mul = 8 * remain;
  value <<= mul;
  value ^= datakey;
  value >>= mul;
  align -= prer;
/*
  fwrite( &value, 1, prer, savef );
*/
  memcpy( &bufmod[bufpos], &value, prer );
  bufpos += prer;
/*
 Only advance the magic if we actually reached the next alignment???
 if ( ( rgsssh.pos + prer ) % 4 == 0 ) rgsssh->datakey = rgsssh->datakey * 7 + 3;
*/
  datakey = datakey * 7 + 3;
}

 aloop = align / 4;
 nopost = prer + ( aloop * 4 );
 postr = files - nopost;
/**/
 for ( ui = 0; ui < aloop; ui++, bufpos += 4 )
{
/*  fread( &value, 1, 4, rgssf );*/
  memcpy( &value, &buforg[bufpos], 4 );
  value ^= datakey;
/*  fwrite( &value, 1, 4, savef );*/
  memcpy( &bufmod[bufpos], &value, 4 );
  datakey = datakey * 7 + 3;
}

/* fread( &value, 1, postr, rgssf );*/
 memcpy( &value, &buforg[bufpos], postr );
 value ^= datakey;
/* fwrite( &value, 1, postr, savef );*/
 memcpy( &bufmod[bufpos], &value, postr );
#ifdef __DEBUG__
 printf( "Remainder: %u.\n", remain );
 printf( "Prereading: %u.\n", prer );
 printf( "Align bytes: %u.\n", align );
 printf( "Loop count: %u.\n", aloop );
 printf( "Not post count: %u.\n", nopost );
 printf( "Only post: %u.\n", postr );
 printf( "Value multiplied: %X (%u).\n", value, mul );
#endif
}

void rgssad_decriptestem2( unsigned char *bufmod, const unsigned char *buforg, const unsigned int files, unsigned int curpos, unsigned int datakey )
{
 unsigned int ui = 0;

 for ( ; ui < files; ui++)
{
  bufmod[ui] = buforg[ui] ^ ( datakey >> ( (curpos & 3) << 3 ) );
  curpos++;
  if( ( curpos & 3 ) == 0) datakey = datakey * 7 + 3;
}

}

void rgssad_decriptestem3( unsigned char *bufmod, const unsigned char *buforg, const unsigned int files, unsigned int datakey )
{
 unsigned int aloop = files / 4, postr = 0, value = 0xCCCCCCCC;
 unsigned int bufpos = 0, ui = 0;

 postr = files - ( aloop * 4 );
/**/
 for ( ui = 0; ui < aloop; ui++, bufpos += 4 )
{
  memcpy( &value, &buforg[bufpos], 4 );
  value ^= datakey;
  memcpy( &bufmod[bufpos], &value, 4 );
  datakey = datakey * 7 + 3;
}

 memcpy( &value, &buforg[bufpos], postr );
 value ^= datakey;
 memcpy( &bufmod[bufpos], &value, postr );
#ifdef __DEBUG__
 printf( "Loop count: %u.\n", aloop );
 printf( "Only post: %u.\n", postr );
 printf( "Last value multiplied: %X.\n", value );
#endif
}

/* UNIX GAMBIT! */
void rgssad_fixpath( char *filen, const size_t filens )
{
 unsigned int ui = 0;

 for ( ; ui < filens; ui++ )
{
  if ( filen[ui] == '\\' ) filen[ui] = '/';
}

}

void rgssad_print( const rgssa_subhdr rgsssh )
{
 printf( "Position: %X.\n", rgsssh.pos );
 printf( "Data size: %u.\n", rgsssh.files );
 printf( "Data Key: %X.\n", rgsssh.datakey );
 printf( "Name size: %u.\n", rgsssh.filens );
 printf( "Name: \"%s\".\n", rgsssh.filen );
}

int main( int argc, char **argv )
{
 FILE *rgssarc = 0;
 int i = 2, j = 0, w = 0;
 long int headpos = 0;
 rgssa_hdr rgsst;
 rgssa_subhdr subtest;
 size_t (* countshf[3])( FILE *rgssf, const unsigned int key ) = { rgssad1_countsubheadf, rgssad1_countsubheadf, rgssad3_countsubheadf };
 size_t filec = 0, mastest = 0, rgss1tr = 0, ui = 0, uj = 0;
/* unsigned char *bufmod = 0, *buforg = 0;*/
 unsigned char optver = 1;
 unsigned int realkey = 0;

 if ( argc < 3 )
{
  fprintf( stderr, "Three or more!\n" );
  return(1);
}

 optver = argv[1][0] - 0x30;

 if ( ( optver == 0 ) || ( optver > 3 ) )
{
  fprintf( stderr, "One, two, three!\n" );
  return(1);
}

 optver--;
 rgssad_inihead( &rgsst );
 rgssad_inisubhead( &subtest );

 for ( ; i < argc; i++ )
{
  rgssarc = fopen( argv[i], "rb" );

  if ( rgssarc == 0 )
{
   fprintf( stderr, "Bomb on file \"%s\"!\n", argv[i] );
}
  else
{
   mastest = rgssad_getheadf( rgssarc, &rgsst );

   if ( mastest == RGSS_MAINHDR )
{
    j = strncmp( "RGSSAD\0", rgsst.magic, 7 );

    if ( j != 0 )
{
     fprintf( stderr, "Not an archive: \"%s\"!\n", rgsst.magic );
     return(1);
}

    if ( ( rgsst.version != 1 ) && ( rgsst.version != 3 ) )
{
     fprintf( stderr, "Unknown version: %u!\n", rgsst.version );
     return(1);
}
#ifdef __DEBUG__
    printf( "File \"%s\".\n", argv[i] );
#endif
    if ( optver == 2 )
{
     realkey = rgsst.key * 9 + 3;
}
    else
{
     realkey = 0xDEADCAFE;
     fseek( rgssarc, -4, SEEK_CUR );
}

    filec = countshf[optver]( rgssarc, realkey );

    if ( filec != 0 )
{
     printf( "Subheader file count of %lu!\n", filec );
     rgsst.suba = (rgssa_subhdr *)malloc( sizeof(rgssa_subhdr) * filec );

     if ( rgsst.suba == 0 )
{
      fprintf( stderr, "Not enough memory!\n" );
      w = 3;
      break;
}
     else
{

      if ( optver != 2 )
{
       headpos = rgss1tr = 8;
       mastest = RGSS1_MINISHDR + 1;

       for ( ui = 0; (ui < filec) && (mastest > RGSS1_MINISHDR); realkey = rgsst.suba[ui].datakey, ui++ )
{
        mastest = rgssad1_getsubheadfv( rgssarc, &rgsst.suba[ui], realkey );
        rgss1tr += mastest;
        rgsst.suba[ui].pos = rgss1tr;
        rgss1tr += rgsst.suba[ui].files;
        j = fseek( rgssarc, rgsst.suba[ui].files, SEEK_CUR );

        if ( j != 0 )
{
         fprintf( stderr, "Error seeking at %lu!\n", ui );
         w = 3;
         break;
}
/*
#ifdef __DEBUG__
        printf( "Sub %lu.\n", ui );
        rgssad_print( rgsst.suba[ui] );
//        rgssad_decripteste( rgsst.suba[ui] );
#endif
*/
}

       fseek( rgssarc, -(rgss1tr - 8), SEEK_CUR );
}
      else
{
       rgss1tr = 12;
       mastest = RGSS3_MINISHDR + 1;

       for ( ui = 0; (ui < filec) && (mastest > RGSS3_MINISHDR); ui++ )
{
        mastest = rgssad3_getsubheadfv( rgssarc, &rgsst.suba[ui], realkey );
        rgss1tr += mastest;
/*
#ifdef __DEBUG__
        printf( "Sub %lu.\n", ui );
        rgssad_print( rgsst.suba[ui] );
//        rgssad_decripteste( rgsst.suba[ui] );
#endif
*/
}

       headpos = rgss1tr;
}

/* Decrypt testing. */
      printf( "Total read: %lu.\n", rgss1tr );

      if ( ui == filec )
{
/* Greater buffer wins. */
       for ( ui = 0; ui < filec; ui++ )
{
        if ( rgsst.suba[ui].files > uj ) uj = rgsst.suba[ui].files;

        rgssad_fixpath( rgsst.suba[ui].filen, rgsst.suba[ui].filens );
}
#ifdef __DEBUG__
       printf( "Largest buffer size: %lu bytes.", uj );
#endif
       j = rgssad_createhierarchy( rgsst.suba, filec );

       if ( j != 0 )
{
        fprintf( stderr, "Couldn't create hierarchy!\n" );
        w = 3;
        break;
}
       else
{
        j = rgssad_extractf( rgssarc, rgsst.suba, headpos, uj, filec );

        if ( j != 0 )
{
         fprintf( stderr, "Couldn't extract files!\n" );
         w = 3;
}

}


}

      free(rgsst.suba);
}

}
    else
{
     fprintf( stderr, "Error getting subheader count!\n" );
     w = 2;
     break;
}

}
   else
{
    fprintf( stderr, "Invalid archive size of %lu!\n", mastest );
    w = 1;
}

}

  fclose(rgssarc);
}

 return(w);
}
