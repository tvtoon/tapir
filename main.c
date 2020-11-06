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

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
/*#include <ruby.h>*/
#include <unistd.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "rubyfill.h"
#include "sdl_misc.h"
#include "Audio.h"
#include "Bitmap.h"
#include "BitmapArray.h"
#include "Color.h"
#include "Font.h"
#include "Graphics.h"
#include "Input.h"
#include "Plane.h"
#include "RGSSError.h"
#include "RGSSReset.h"
#include "Rect.h"
#include "Sprite.h"
#include "Table.h"
#include "Tilemap.h"
#include "Tone.h"
#include "Viewport.h"
#include "Win32APIFake.h"
#include "Window.h"
#include "font_lookup.h"
#include "ini.h"
#include "main_rb.h"
#include "misc.h"
#include "openres.h"
#include "tapir_config.h"

static bool is_test_mode = false;
static bool is_btest_mode = false;
static bool is_console_mode = false;

void Init_zlib(void);
#if RGSS == 3
void Init_single_byte(void);
void Init_utf_16_32(void);
void Init_japanese_sjis(void);
#elif RGSS == 2
void Init_nkf(void);
#endif

#ifdef RUBY_GLOBAL_SETUP
RUBY_GLOBAL_SETUP
#endif

static void Init_RGSS(void) {
#if RGSS >= 2
  rb_gv_set("$TEST", is_test_mode ? Qtrue : Qfalse);
#else
  rb_gv_set("$DEBUG", is_test_mode ? Qtrue : Qfalse);
#endif
  rb_gv_set("$BTEST", is_btest_mode ? Qtrue : Qfalse);
  Init_RGSSError();
  Init_RGSSReset();
  Init_Rect();
  Init_Color();
  Init_Tone();
  Init_Table();
  Init_Font();
  Init_Bitmap();
  Init_Viewport();
  Init_Sprite();
  Init_Window();
  Init_Tilemap();
  Init_BitmapArray();
  Init_Plane();
  Init_Graphics();
  Init_Input();
  Init_Audio();
  Init_Win32APIFake();
  rb_define_global_function("load_data", rb_load_data, 1);
}

int main(int argc, char **argv)
{
// VALUE excdata;
  bool help = false;
/*
char *ruby_argv_array[] = {
    (char*)"ruby",
    NULL
  };
  char **ruby_argv = ruby_argv_array;
*/
 const char *game_title = 0, *game_script = 0;
 int argpos = 1, i = 0/*, ruby_argc = 2*/, state = 0;
 struct ini *ini_data = 0;
 struct ini_section *game_section = 0;

/* Ruby does not answer well for this...
 atexit(tapir_atexit);
*/
 init_tapir_config();

 while(argpos < argc)
{

  if(!strcmp(argv[argpos], "btest"))
{
   is_test_mode = is_btest_mode = true;
//#if RGSS > 1
}
  else if(!strcmp(argv[argpos], "test"))
{
/*
#else
}
  else if(!strcmp(argv[argpos], "debug"))
{
#endif
*/
   is_test_mode = true;
#if RGSS == 3
}
  else if(!strcmp(argv[argpos], "console"))
{
   is_console_mode = true;
#endif
}
  else if(!strcmp(argv[argpos], "-d") && argpos+1 < argc)
{
   ++argpos;
   i = chdir(argv[argpos]);

   if ( i != 0 )
{
    fprintf(stderr, "Cannot chdir: \"%s\".\n", strerror(errno));
    return(1);
}

}
  else if(!strcmp(argv[argpos], "-h") || !strcmp(argv[argpos], "--help"))
{
   help = true;
}
  else
{
   fprintf(stderr, "warning: unrecognized argument: %s\n", argv[argpos]);
}
  ++argpos;
}

 if (help)
{
  fprintf(stderr,
#if RGSS == 3
   "Tapir-accordion: RGSS3 (RPG Maker VX Ace) compatible game engine\n"
#elif RGSS == 2
   "Tapir-violin: RGSS2 (RPG Maker VX) compatible game engine\n"
#else
   "Tapir-xylophone: RGSS (RPG Maker XP) compatible game engine\n"
#endif
   "\n"
   "Usage: ./tapir [-h] [-d DIR] [test] [btest] [console]\n"
   "    -h      show this help\n"
   "    -d DIR  move to DIR before running the game\n"
   "    test    enable test mode\n"
   "    btest   enable battle test mode\n"
#if RGSS == 3
   "    console show console (not yet implemented)\n"
#endif
);

  return 0;
}

 if (is_console_mode)
{
  fprintf(stderr, "warning: ignored console flag\n");
}

 ini_data = load_ini("Game.ini", 0);

 if ( ini_data == 0 )
{
  fprintf( stderr, "Can't read the game INI!\n" );
  return(1);
}

 unix_inifix_forGJ( ini_data );
 game_section = find_ini_section(ini_data, "Game");

 if ( game_section == 0 )
{
  fprintf( stderr, "GAME section not inside INI!\n" );
  return(1);
}

 game_script = find_ini_entry(game_section, "Scripts");

 if ( game_script == 0 )
{
  fprintf( stderr, "\"Scripts\" key not inside INI!\n" );
  return(1);
}

 game_title = find_ini_entry(game_section, "Title");

 if (game_title == 0) game_title = "tapir";

 i = configure_rtp_path(game_section);

 if ( i != 0 )
{
  fprintf( stderr, "RTP configuration error!\n" );
  return(1);
}

 i = initSDL(game_title);

 if ( i != 0 )
{
  fprintf(stderr, "Error: couldn't init SDL.\n");
  return(1);
}

 i = initFontLookup();

 if ( i != 0 )
{
  fprintf(stderr, "Error: couldn't init fontconfig.\n");
  return(1);
}

 printf( "Begin RGSS...\n" );
#ifdef RUBY_INIT_STACK
//  ruby_sysinit(&ruby_argc, &ruby_argv);
 RUBY_INIT_STACK;
 fprintf( stderr, "RUBY INIT STACK!\n" );
#else
/*
    (void) ruby_argc;
    (void) ruby_argv;
*/
 fprintf( stderr, "RUBY WITHOUT STACK!\n" );
#endif
 ruby_init();
 Init_zlib();
#if RGSS == 3
 Init_single_byte();
 Init_utf_16_32();
 Init_japanese_sjis();
#endif
#if RGSS == 2
 Init_nkf();
#endif
 Init_RGSS();

#ifndef RUBY_INIT_STACK
 extern void Init_stack(void *addr);
 Init_stack(__builtin_frame_address(0));
#endif
 rb_protect(main_rb, /*Qnil*/rb_str_new2(game_script ), &state);
/*
 if ( state != 0 )
{
  excdata = rb_errinfo();
  rb_set_errinfo(Qnil);
  fprintf( stderr, "%s", RSTRING_PTR(excdata) );
}
*/
/* THE END */
 uninitFontLookup();
 cleanupSDL();
 deinit_tapir_config();
  if(ini_data) free_ini(ini_data);
// ruby_stop(0);
// ruby_cleanup(0);
// ruby_finalize();
 printf( "Max Plane: %u.\nMax Sprite: %u.\nMax Tilemap: %u.\nMax Viewport: %u.\nMax Window: %u.\n", maxplanec, maxspritec, maxtmapc, maxvportqc, maxwindowc );

 fprintf( stderr, "End state!\n" );
 return state;
}
