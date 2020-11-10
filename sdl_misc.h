// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

struct RenderViewport {
  int width, height;
  int ox, oy;
};

/*
struct RenderJob;

struct Renderable {
  void (*clear)(struct Renderable *renderable);
  void (*prepare)(struct Renderable *renderable, int t);
  void (*render)(struct Renderable *renderable, const struct RenderJob *job, const struct RenderViewport *viewport);
  bool disposed;
};
*/
struct RenderJob {
//  struct Renderable *renderable;
 int z, y, t, aux[3];
 unsigned short reg;
};

struct RenderQueue
{
 unsigned short size, capacity;
 struct RenderJob *queue;
};

extern int window_width;
extern int window_height;
extern int window_brightness;
/* One use in "Graphics.c". */
extern SDL_Window *window;

int initSDL(const char *window_title);
void initRenderQueue(struct RenderQueue *queue);
void capturedRenderSDL(SDL_Surface *surface);
void cleanupSDL(void);
void clearRenderQueue(struct RenderQueue *queue);
void defreeze_screen(void);
void deinitRenderQueue(struct RenderQueue *queue);
void event_loop(void);
void freeze_screen(void);
void ini_transition( void );
void load_transition_image( const char *filename, const size_t filenso, const int vagueness );
void queueRenderJob(VALUE viewport, struct RenderJob job);
void renderSDL(void);
void renderQueue(struct RenderQueue *queue, const struct RenderViewport *viewport);
/*
void registerRenderable(struct Renderable *renderable);
void disposeRenderable(struct Renderable *renderable);
void disposeAll(void);
*/
unsigned short NEWregisterRenderable( const unsigned short index, const unsigned char type );
unsigned short NEWdisposeRenderable( const unsigned short index );
