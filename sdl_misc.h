// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
/*
struct RenderViewport {
  int width, height;
  int ox, oy;
};
*/
struct RenderJob {
 int z, y, t, aux[3];
 int ox, oy;
 unsigned short reg;
 unsigned short rindex;
};

extern int window_width;
extern int window_height;
extern int window_brightness;
/* One use in "Graphics.c". */
extern SDL_Window *window;

int initSDL(const char *window_title);
void capturedRenderSDL(SDL_Surface *surface);
void cleanupSDL(void);
void defreeze_screen(void);
void event_loop(void);
void freeze_screen(void);
void ini_transition( void );
void load_transition_image( const char *filename, const size_t filenso, const int vagueness );
void queueRenderJob(struct RenderJob job);
void renderSDL(void);
void disposeAll(void);
unsigned short NEWregisterRenderable( const unsigned short index, const unsigned char type );
unsigned short NEWdisposeRenderable( const unsigned short index );
