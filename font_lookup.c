// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL_ttf.h>
#include <fontconfig/fontconfig.h>

#include "rubyfill.h"
#include "RGSSError.h"
#include "font_lookup.h"
#include "ini.h"
#include "openres.h"

static FcConfig *config = NULL;

int initFontLookup(void)
{
 char font_paths[3][PATH_MAX + 1] = { "\0", "\0", "\0" };
 unsigned char rtpc = 0, ui = 0;

 if(!FcInit())
{
  fprintf(stderr, "FcInit failed\n");
  return(1);
}

 config = FcInitLoadConfigAndFonts();
 // TODO case-insensitive paths
 FcConfigAppFontAddDir(config, (const FcChar8 *)"Fonts");
#ifdef __DEBUG__
 printf( "Building the font path...\n" );
#endif
 rtpc = build_rtppath( /*(char **)*/font_paths, "Fonts", 5 );

 if ( rtpc == 0 )
{
  fprintf(stderr, "Font path cat failed.\n");
  return(1);
}

 for ( ; ui < rtpc; ui++ )
{
  FcConfigAppFontAddDir(config, (const FcChar8 *)font_paths[ui] );
}

 return(0);
}

void uninitFontLookup(void) {
  if(config) {
    FcConfigDestroy(config);
    config = NULL;
  }
  FcFini();
}

bool fontExistence(const char *name) {
  // fprintf(stderr, "fontExistence(%s)\n", name);

  FcPattern* pat = FcNameParse((const FcChar8 *)name);
  FcConfigSubstitute(config, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);

  FcResult result;
  FcPattern* font = FcFontMatch(config, pat, &result);
  bool exists = (font != NULL);
  FcPatternDestroy(font);
  FcPatternDestroy(pat);
  return exists;
}

TTF_Font *loadFont(const char *name, int size, bool bold, bool italic)
{
// TODO: cache font here to avoid multiple loading
// fprintf(stderr, "loadFont(%s, %d, %d, %d)\n", name, size, bold, italic);
 FcPattern *font = 0, *pat = FcPatternCreate();
 FcResult result = 0;
 TTF_Font *sdl_font = 0;
 const char *path = '\0';
 int index = 0;

 FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)name);
 FcPatternAddInteger(pat, FC_SIZE, size);
 FcPatternAddInteger(pat, FC_WEIGHT, bold ? FC_WEIGHT_BOLD : FC_WEIGHT_MEDIUM);
 FcPatternAddInteger(pat, FC_SLANT, italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
 FcConfigSubstitute(config, pat, FcMatchPattern);
 FcDefaultSubstitute(pat);
 font = FcFontMatch(config, pat, &result);

 if (!font)
{
  rb_raise(rb_eRGSSError, "No font found!\n");
  return(0);
}

 if (FcPatternGetString(font, FC_FILE, 0, (FcChar8 **)&path) != FcResultMatch)
{
  rb_raise(rb_eRGSSError, "Fontconfig FC_FILE failed!\n" );
  return(0);
}

 if (FcPatternGetInteger(font, FC_INDEX, 0, &index) != FcResultMatch)
{
  rb_raise(rb_eRGSSError, "Fontconfig FC_INDEX failed!\n" );
  return(0);
}
// fprintf(stderr, "font = %s, index = %d\n", path, index);
 sdl_font = TTF_OpenFontIndex(path, size, index);
 TTF_SetFontStyle(sdl_font, (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0));
 FcPatternDestroy(font);
 FcPatternDestroy(pat);
 return sdl_font;
}
