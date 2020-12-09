// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <stdbool.h>

#include <SDL.h>

#include "rubyfill.h"
#include "sdl_misc.h"
#include "RGSSReset.h"
#include "main_rb.h"
#include "main_rb1.h"
#include "main_rb2.h"
#include "main_rb3.h"
#include "misc.h"

/* 323 + 256 (Script name) + 17 */
static char execscript[596 + 1] = "scripts_fname = \"";
/* 253 + ifdef */
static const char scriptend3s[323 + 1] =
"\"\n$RGSS_SCRIPTS = load_data(scripts_fname)\n"
"$RGSS_SCRIPTS.each do|section|\n"
"  section.insert(3, Zlib::Inflate::inflate(section[2]))\n"
"end\n"
"$RGSS_SCRIPTS.each_with_index do|section, idx|\n"
"  script = section[3].dup\n"
"  script.force_encoding(\"utf-8\")\n"
"  filename = sprintf(\"{%04d}\", idx)\n"
"  eval(script, TOPLEVEL_BINDING, filename)\n"
"end\n";

static const char scriptend12s[295 + 1] =
"\"\n$RGSS_SCRIPTS = load_data(scripts_fname)\n"
"$RGSS_SCRIPTS.each do|section|\n"
"  section.insert(3, Zlib::Inflate::inflate(section[2]))\n"
"end\n"
"$RGSS_SCRIPTS.each_with_index do|section, idx|\n"
"  script = section[3].dup\n"
"  filename = sprintf(\"Section%03d\", idx)\n"
"  eval(script, TOPLEVEL_BINDING, filename)\n"
"end\n";

static size_t scriptsize = 17;
static size_t scriptsufs = 323;

/*
static VALUE rescue_any(VALUE data, VALUE e) {
  (void) data;
  (void) e;
  rb_eval_string(
      "  $stderr.print(\"#{$!.class}: #{$!.message}\\n\")\n"
      "  $!.backtrace.each do|frame|\n"
      "    $stderr.print(\"\\tfrom #{frame}\\n\")\n"
      "  end\n"
  );
  return Qnil;
}
*/
static VALUE rescue_reset(VALUE data, VALUE e)
{
 VALUE algo;
 algo = rb_errinfo();
 rb_set_errinfo(Qnil);
 fprintf( stderr, "%s; %s\n", RSTRING_PTR(e), RSTRING_PTR(algo) );

 disposeAll();
 rb_gc_start();
 *(bool*)data = true;
 return Qnil;
}

static VALUE rb_rgss_main(VALUE self)
{
 (void) self;
 bool retry = true;

 while (retry)
{
  retry = false;
//rb_rgss_main2
  rb_rescue2( rb_yield, Qnil, rescue_reset, (VALUE)&retry, rb_eRGSSReset, NULL);
}

 return Qnil;
}

VALUE main_rb(VALUE data)
{
 VALUE excdata;
 const char *scriptn = RSTRING_PTR(data);
 const char *scriptends = scriptend3s;
 int retval = 0;
 size_t argsize = RSTRING_LEN(data);

 if ( rgssver != 3 )
{
  scriptsufs = 295;
  scriptends = scriptend12s;
}

#ifdef __DEBUG__
 printf( "Using script \"%s\" (%lu characters).\n", scriptn, argsize );
#endif
 strncpy( &execscript[scriptsize], scriptn, argsize );
 scriptsize += argsize;
 strncpy( &execscript[scriptsize], scriptends, scriptsufs );
 scriptsize += scriptsufs;
 execscript[scriptsize] = '\0';
#ifdef __DEBUG__
 printf( "%s", execscript );
#endif

 if ( rgssver == 3 )
{
  rb_define_global_function("rgss_main", rb_rgss_main, 0);
  rb_eval_string(rgss3_exe);
}
 else if ( rgssver == 2 )
{
  rb_eval_string(rgss2_exe);
}
 else
{
  rb_eval_string(rgss1_exe);
}

// load_libs();
 rb_eval_string_protect( execscript, &retval );

 if ( retval != 0 )
{
  excdata = rb_errinfo();

  rb_eval_string(
      "  $stderr.print(\"#{$!.class}: #{$!.message}\\n\")\n"
      "  $!.backtrace.each do|frame|\n"
      "    $stderr.print(\"\\tfrom #{frame}\\n\")\n"
      "  end\n"
  );

  rb_set_errinfo(Qnil);
  fprintf( stderr, "%s", RSTRING_PTR(excdata) );
}


/*
 while(retry)
{
  retry = false;
  rb_rescue2( uselesswrapper, Qnil, rescue_reset, (VALUE)&retry, rb_eRGSSReset, NULL, rescue_any, Qnil, rb_eException, NULL );
//rb_rescue2( , load_scripts, Qnil, );
}
*/
 return(retval);
}
