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
#include <SDL.h>
#include <SDL_mixer.h>

#include "rubyfill.h"
#include "Audio.h"
#include "RGSSError.h"
#include "ini.h"
#include "misc.h"
#include "openres.h"
#include "sdl_misc.h"

#if RGSS == 3
#define ARGCMAX 4
#else
#define ARGCMAX 3
#endif

struct chunk_cache_entry {
  char name[PATH_MAX + 1];
  Mix_Chunk *chunk;
};

// TODO: properly close these
static Mix_Music *bgm;
static VALUE rb_mAudio;
/* REALLOC YOUR MOM! */
static const size_t cachesize = 64;
static size_t chunki = 0;
static struct chunk_cache_entry *chunk_cache = 0;

static const char extensions[6][5] = { ".ogg", ".mid", ".wma", ".mp3", ".wav", "\0" };
/*
#if RGSS == 3
static VALUE rb_audio_s_setup_midi(VALUE klass);
static VALUE rb_audio_s_bgm_pos(VALUE klass);
static VALUE rb_audio_s_bgs_pos(VALUE klass);
#endif
*/
#if RGSS == 3
static VALUE rb_audio_s_bgm_pos(VALUE klass) {
  (void) klass;
  WARN_UNIMPLEMENTED("Audio.bgm_pos");
  return INT2NUM(0);
}

static VALUE rb_audio_s_bgs_pos(VALUE klass) {
  (void) klass;
  WARN_UNIMPLEMENTED("Audio.bgs_pos");
  return INT2NUM(0);
}

static VALUE rb_audio_s_setup_midi(VALUE klass) {
  (void) klass;
  WARN_UNIMPLEMENTED("Audio.setup_midi");
  return Qnil;
}
#endif

static VALUE rb_audio_s_bgm_play(int argc, VALUE *argv, VALUE klass)
{
/* Should be exported to ruby!
 VALUE filename = 0;
*/
 char filen[PATH_MAX + 1] = "\0", pato[PATH_MAX + 1] = "\0";
 int pitch = 0, pos = 0, volume = 0;
 size_t filens = 0;

 (void) klass;

 if(argc <= 0 || argc > ARGCMAX)
{
  rb_raise(rb_eArgError, "wrong number of arguments (%d for 1..%i)", argc, ARGCMAX );
  return(Qnil);
}

 filens = RSTRING_LEN(argv[0]);
/* Windows PATH_MAX less extension. */
 if ( filens > 251 )
{
  rb_raise(rb_eRGSSError, "File \"%s\" size is too large.", StringValueCStr(argv[0]) );
  return(Qnil);
}

 strncpy( filen, RSTRING_PTR(argv[0]), filens );
// filename = rb_str_new( filen, filens );
 volume = argc > 1 ? clamp_int32(NUM2INT(argv[1]), 0, 100) : 100;
 pitch = argc > 2 ? clamp_int32(NUM2INT(argv[2]), 50, 150) : 100;
 pos = argc > 3 ? NUM2INT(argv[3]) : 0;

 if (pitch != 100)
{
  WARN_UNIMPLEMENTED("Audio.bgm_play with pitch");
}

 if (pos != 0)
{
  WARN_UNIMPLEMENTED("Audio.bgm_play with pos");
}

 if(bgm)
{
  Mix_HaltMusic();
  Mix_FreeMusic(bgm);
  bgm = NULL;
}

 filens = loadfile_withrtp( pato, filen, extensions, filens, 5, 2 );

 if ( filens != 0 )
{
  bgm = Mix_LoadMUS(pato);

  if ( bgm == 0 )
{
// TODO: check error handling
   rb_raise(rb_eRGSSError, "Error loading %s: %s", pato, Mix_GetError());
   return(Qnil);
}

  Mix_VolumeMusic(volume * MIX_MAX_VOLUME / 100);
  // TODO: LOOPSTART from ogg
  Mix_PlayMusic(bgm, -1);
}
 else
{
// TODO: check error handling
  rb_raise(rb_eRGSSError, "File not found: \"%s\".", filen );
}

 return Qnil;
}

static VALUE rb_audio_s_bgm_stop(VALUE klass) {
  (void) klass;
  if(bgm) {
    Mix_HaltMusic();
    Mix_FreeMusic(bgm);
    bgm = NULL;
  }
  return Qnil;
}

static VALUE rb_audio_s_bgm_fade(VALUE klass, VALUE time) {
  (void) klass;
  int time_i = NUM2INT(time);
  if(bgm) {
    Mix_FadeOutMusic(time_i);
    Mix_FreeMusic(bgm);
    bgm = NULL;
  }
  return Qnil;
}

static VALUE rb_audio_s_bgs_play(int argc, VALUE *argv, VALUE klass) {
  (void) klass;

  if(argc <= 0 || argc > ARGCMAX)
  {
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 1..%i)", argc, ARGCMAX );
  }

  (void) argv;
  WARN_UNIMPLEMENTED("Audio.bgs_play");
  return Qnil;
}

static VALUE rb_audio_s_bgs_stop(VALUE klass) {
  (void) klass;
  WARN_UNIMPLEMENTED("Audio.bgs_stop");
  return Qnil;
}

static VALUE rb_audio_s_bgs_fade(VALUE klass, VALUE time) {
  (void) klass;
  (void) time;
  WARN_UNIMPLEMENTED("Audio.bgs_fade");
  return Qnil;
}

static VALUE rb_audio_s_me_play(int argc, VALUE *argv, VALUE klass) {
  (void) klass;
  if(argc <= 0 || argc > 3) {
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 1..3)", argc);
  }
  (void) argv;
  WARN_UNIMPLEMENTED("Audio.me_play");
  return Qnil;
}

static VALUE rb_audio_s_me_stop(VALUE klass) {
  (void) klass;
  WARN_UNIMPLEMENTED("Audio.me_stop");
  return Qnil;
}

static VALUE rb_audio_s_me_fade(VALUE klass, VALUE time) {
  (void) klass;
  (void) time;
  WARN_UNIMPLEMENTED("Audio.me_fade");
  return Qnil;
}

static VALUE rb_audio_s_se_play(int argc, VALUE *argv, VALUE klass) {
/* Should be exported to ruby?
 VALUE filename = 0;
*/
 Mix_Chunk *chunk = 0;
 char filen[PATH_MAX + 1] = "\0", pato[PATH_MAX + 1] = "\0";
 int pitch = 0, volume = 0;
 size_t filens = 0, paths = 0, ui = 0;

 (void) klass;

 if(argc <= 0 || argc > 3)
{
  rb_raise(rb_eArgError, "wrong number of arguments (%d for 1..3)", argc);
  return(Qnil);
}

 filens = RSTRING_LEN(argv[0]);
/* Windows PATH_MAX less extension. */
 if ( filens > 251 )
{
  rb_raise(rb_eRGSSError, "File %s size is too large.", StringValueCStr(argv[0]) );
  return(Qnil);
}

 strncpy( filen, RSTRING_PTR(argv[0]), filens );
// filename = rb_str_new( filen, filens );
 volume = argc > 1 ? clamp_int32(NUM2INT(argv[1]), 0, 100) : 80;
 pitch = argc > 2 ? clamp_int32(NUM2INT(argv[2]), 50, 150) : 100;

 if (pitch != 100)
{
  WARN_UNIMPLEMENTED("Audio.se_play with pitch");
}


 for ( ; ui < cachesize; ui++)
{

  if ( strncmp( chunk_cache[ui].name, filen, filens ) == 0 )
{
   chunk = chunk_cache[ui].chunk;
}

}

 if ( ui == cachesize )
{
  paths = loadfile_withrtp( pato, filen, extensions, filens, 5, 2 );

  if ( paths != 0 )
{
   chunk = Mix_LoadWAV(pato);

   if ( chunk != 0 )
{
    chunki++;

    if ( chunki == cachesize )
{
#ifdef __DEBUG__
     printf( "REALOC INDEX: %lu!\n", chunki );
#endif
     chunki = 0;
}

    chunk_cache[chunki].chunk = chunk;
    strncpy( chunk_cache[chunki].name, filen, filens );
    chunk_cache[chunki].name[filens] = '\0';
}
   else
{
    rb_raise(rb_eRGSSError, "Error loading %s: %s", pato, Mix_GetError());
    return(Qnil);
}

}
  else
{
// TODO: check error handling
   rb_raise(rb_eRGSSError, "File not found: \"%s\".", filen );
   return(Qnil);
}

}

  Mix_VolumeChunk(chunk, volume * MIX_MAX_VOLUME / 100);
  Mix_PlayChannel(-1, chunk, 0);
  return Qnil;
}

static VALUE rb_audio_s_se_stop(VALUE klass) {
  (void) klass;
  WARN_UNIMPLEMENTED("Audio.se_stop");
  return Qnil;
}

/* static END */

void Init_Audio() {
  rb_mAudio = rb_define_module("Audio");
#if RGSS == 3
  rb_define_singleton_method(rb_mAudio, "setup_midi", rb_audio_s_setup_midi, 0);
  rb_define_singleton_method(rb_mAudio, "bgm_pos", rb_audio_s_bgm_pos, 0);
  rb_define_singleton_method(rb_mAudio, "bgs_pos", rb_audio_s_bgs_pos, 0);
#endif
  rb_define_singleton_method(rb_mAudio, "bgm_play", rb_audio_s_bgm_play, -1);
  rb_define_singleton_method(rb_mAudio, "bgm_stop", rb_audio_s_bgm_stop, 0);
  rb_define_singleton_method(rb_mAudio, "bgm_fade", rb_audio_s_bgm_fade, 1);
  rb_define_singleton_method(rb_mAudio, "bgs_play", rb_audio_s_bgs_play, -1);
  rb_define_singleton_method(rb_mAudio, "bgs_stop", rb_audio_s_bgs_stop, 0);
  rb_define_singleton_method(rb_mAudio, "bgs_fade", rb_audio_s_bgs_fade, 1);
  rb_define_singleton_method(rb_mAudio, "me_play", rb_audio_s_me_play, -1);
  rb_define_singleton_method(rb_mAudio, "me_stop", rb_audio_s_me_stop, 0);
  rb_define_singleton_method(rb_mAudio, "me_fade", rb_audio_s_me_fade, 1);
  rb_define_singleton_method(rb_mAudio, "se_play", rb_audio_s_se_play, -1);
  rb_define_singleton_method(rb_mAudio, "se_stop", rb_audio_s_se_stop, 0);
}

void initAudioSDL(void) {
//  chunk_cache_size = 0;
//  chunk_cache_capacity = 10;
  chunk_cache = malloc(sizeof(*chunk_cache) * cachesize );
}

void deinitAudioSDL(void)
{
 size_t ui = 0;

 if (bgm) Mix_FreeMusic(bgm);
 Mix_HaltChannel(-1);

 for ( ; ui < cachesize; ui++ )
{
  Mix_FreeChunk(chunk_cache[ui].chunk);
}

 free(chunk_cache);
}
