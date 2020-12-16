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
#include <limits.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#include <SDL_ttf.h>

#include "rubyfill.h"
#include "Audio.h"
#include "Input.h"
#include "Plane.h"
#include "RGSSError.h"
#include "RGSSReset.h"
#include "Rect.h"
#include "Sprite.h"
#include "Tilemap.h"
#include "Viewport.h"
#include "Window.h"
#include "gl_misc.h"
#include "ini.h"
#include "misc.h"
#include "openres.h"
#include "sdl_misc.h"
#include "surface_misc.h"

int window_width = 544;
int window_height = 416;
int window_brightness = 255;

SDL_Window *window = NULL;

static GLuint transition_shader;
static GLuint transition_texture;
static GLuint transition_texture2;
static SDL_GLContext glcontext = NULL;
//static struct RenderQueue main_queue;
static const unsigned short mq_capacity = 512, registry_capacity = 1024;
static struct Rect nullrect = { 0, 0, 0, 0 };
static struct RenderJob job_queuea[512];
static int transition_vagueness = 255;
static unsigned char newreg = 1;
static unsigned short mq_size = 0, registry_size = 0, regminfi = 0;

/* The new deal: type and index for rendering. */
typedef struct
{
 unsigned char rendta;
 unsigned short rendia;
} newqueue;

static newqueue tnewqa[1024];

static void ( *preparefuna[4])( const unsigned short index, const unsigned short rindex ) = { prepareRenderPlane, prepareRenderTilemap, prepareRenderSprite, /*prepareRenderViewport,*/ prepareRenderWindow };
static void ( *renderfuna[4])( const unsigned short index, const int vportox, const int vportoy ) = { renderPlane, renderTilemap, renderSprite, /*renderViewport,*/ renderWindow };

static int initTransition(void)
{
  static const char *fsh_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D tex;\n"
    "uniform sampler2D tex2;\n"
    "uniform float brightness;\n"
    "uniform float vagueness;\n"
    "\n"
    "void main(void) {\n"
    "    float tr = texture2D(tex2, gl_TexCoord[0].xy).r;\n"
    "    float br = \n"
    "      (brightness - (tr - vagueness)) / \n"
    "      (vagueness + 0.0000001);\n"
    "    br = max(min(br, 1.0), 0.0);\n"
    "    gl_FragColor = texture2D(tex, gl_TexCoord[0].xy);\n"
    "    gl_FragColor.a *= 1.0 - br;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

 transition_shader = compileShaders(fsh_source);

 if ( transition_shader == 0 ) return(1);

 glGenTextures(1, &transition_texture);
 glGenTextures(1, &transition_texture2);

 defreeze_screen();
/* load_transition_image(NULL, 255);*/
 ini_transition();
 return(0);
}

static void deinitTransition(void)
{
  if(transition_texture2) glDeleteTextures(1, &transition_texture2);
  if(transition_texture) glDeleteTextures(1, &transition_texture);
  if(transition_shader) glDeleteProgram(transition_shader);
}

int initSDL(const char *window_title)
{
 int img_flags = IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF, mix_init_flags = 0, shaderrc = 0;
 unsigned short ui = 0;

 if ( rgssver == 1 )
{
  window_width = 640;
  window_height = 480;
  preparefuna[1] = prepareRenderTilemapRGSS1;
  renderfuna[1] = renderTilemapRGSS1;
  renderfuna[3] = renderWindowRGSS1;
}

 nullrect.width = window_width;
 nullrect.height = window_height;

 for ( ; ui < registry_capacity; ui++ )
{
  tnewqa[ui].rendta = 0;
  tnewqa[ui].rendia = registry_capacity;
}

 if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
{
  fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
  return(1);
}

 if (IMG_Init(img_flags) != img_flags)
{
  fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
  SDL_Quit();
  return(1);
}

 if (TTF_Init())
{
  fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
  IMG_Quit();
  SDL_Quit();
  return(1);
}

 mix_init_flags = Mix_Init(MIX_INIT_MP3|MIX_INIT_OGG);

 if( ( mix_init_flags & ( MIX_INIT_MP3 | MIX_INIT_OGG ) ) == 0 )
{
  fprintf(stderr, "Mix_Init warning: could not init MP3 and OGG\n");
  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  return(1);
}

// TODO: in some environment, SDL_mixer produces unignorable latency...
 if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
{
  printf("Mix_OpenAudio Error: %s\n", Mix_GetError());
  Mix_CloseAudio();
  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  return(1);
}

 Mix_AllocateChannels(16);

// int mix_frequency, mix_channels;
// Uint16 mix_format;
// Mix_QuerySpec(&mix_frequency, &mix_format, &mix_channels);
// fprintf(stderr, "frequency = %d, format = %d, channels = %d\n",
//     mix_frequency, (int)mix_format, mix_channels);

 SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

 window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL);

 if(!window)
{
  fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
  Mix_CloseAudio();
  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  return(1);
}

// int major, minor;
// SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
// SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
// fprintf(stderr, "GL version: %d.%d\n", major, minor);
 glcontext = SDL_GL_CreateContext(window);

 if(!glcontext)
{
  fprintf(stderr, "SDL_GL_CreateContext error: %s\n", SDL_GetError());
  SDL_DestroyWindow(window);
  Mix_CloseAudio();
  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  return(1);
}

 shaderrc += initTransition();
 shaderrc += initSpriteSDL();
 shaderrc += initWindowSDL();
 shaderrc += initTilemapSDL();
 shaderrc += initPlaneSDL();
 initAudioSDL();

 if ( shaderrc > 0 )
{
  cleanupSDL();
  return(1);
}

 return(0);
}

void cleanupSDL() {
  deinitAudioSDL();
  deinitPlaneSDL();
  deinitTilemapSDL();
  deinitWindowSDL();
  deinitSpriteSDL();
  deinitTransition();
  if(glcontext) SDL_GL_DeleteContext(glcontext);
  if(window) SDL_DestroyWindow(window);
  Mix_CloseAudio();
  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
//  deinitRenderQueue(&main_queue);
//  if(registry) free(registry);
}

static int compare_jobs(const void *o1, const void *o2)
{
 const struct RenderJob *j1 = (const struct RenderJob *)o1;
 const struct RenderJob *j2 = (const struct RenderJob *)o2;
/*
 if ( j1->reg < j2->reg ) return -1;
 else if ( j1->reg > j2->reg ) return 1;
*/
 if(j1->z < j2->z) return -1;
 else if(j1->z > j2->z) return 1;

 if(j1->y < j2->y) return -1;
 else if(j1->y > j2->y) return 1;

 if(j1->t < j2->t) return -1;
 else if(j1->t > j2->t) return 1;

 return 0;
}

void event_loop()
{
 SDL_Event e;
 int quit = 0;

 while(SDL_PollEvent(&e))
{
  switch(e.type)
{
   case SDL_KEYDOWN:
    if(e.key.keysym.sym == SDLK_F12)
{
     rb_raise(rb_eRGSSReset, "RGSS Reset");
}
    if(!e.key.repeat)
{
     keyPressed(e.key.keysym.sym);
}
    break;

   case SDL_KEYUP:
    keyReleased(e.key.keysym.sym);
    break;

   case SDL_QUIT:
    quit = 1;
    break;
}

}

 if ( quit == 1 )
{
  ruby_finalize();
}

}

static void renderScreen()
{
 int recy = 0;
 unsigned short reg = 0, regc = 0;
// const struct RenderViewport viewport = { window_width, window_height, 0, 0 };

 if ( newreg == 1 )
{
  mq_size = 0;

  for (; ( regc < registry_size ) || ( reg < registry_capacity ); reg++ )
{
/* && ( tnewqa[reg].rendta != 3 ) */
   if ( tnewqa[reg].rendia != registry_capacity )
{
    preparefuna[tnewqa[reg].rendta]( tnewqa[reg].rendia, reg );
    regc++;
}

}

  if ( regc != registry_size )
{
   rb_raise(rb_eRGSSError, "Limit reached and the registry count is wrong: %u from %u!\n", regc, registry_size );
   mq_size = 0;
}
  else
{
   qsort( job_queuea, mq_size, sizeof( struct RenderJob ), compare_jobs);
#ifdef __DEBUG__
   for ( reg = 0, regc = 1; reg < mq_size; reg++, regc++ )
{
    printf( "Arj %u: Z=%i, Y=%i, T=%i, R=%u, I=%u.\n", regc, job_queuea[reg].z, job_queuea[reg].y, job_queuea[reg].t, job_queuea[reg].reg, job_queuea[reg].rindex );
}
#endif
}

  newreg = 0;
}

 glEnable(GL_SCISSOR_TEST);

 for ( reg = 0; reg < mq_size; reg++ )
{
  recy = window_height - ( job_queuea[reg].recy[0] + job_queuea[reg].rech[0] );
/*
  glScissor( 0, 0, window_width, window_height );
  glViewport( 0, 0, window_width, window_height );
*/
  glScissor( job_queuea[reg].recx[0], recy, job_queuea[reg].recw[0], job_queuea[reg].rech[0] );
  glViewport( job_queuea[reg].recx[0], recy, job_queuea[reg].recw[0], job_queuea[reg].rech[0] );
  renderfuna[ job_queuea[reg].reg ]( job_queuea[reg].rindex, job_queuea[reg].ox[0], job_queuea[reg].oy[0] );
}

 glDisable(GL_SCISSOR_TEST);
}

void renderSDL()
{
 SDL_GL_MakeCurrent(window, glcontext);
 glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
 glClear(GL_COLOR_BUFFER_BIT);
 glScissor(0, 0, window_width, window_height);
 glViewport(0, 0, window_width, window_height);
 renderScreen();

 if (window_brightness != 255)
{
// transition
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, transition_texture2);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, transition_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glUseProgram(transition_shader);
  glUniform1i(glGetUniformLocation(transition_shader, "tex"), 0);
  glUniform1i(glGetUniformLocation(transition_shader, "tex2"), 1);
  glUniform2f(glGetUniformLocation(transition_shader, "resolution"), window_width, window_height);
  glUniform1f(glGetUniformLocation(transition_shader, "brightness"), window_brightness / 255.0);
  glUniform1f(glGetUniformLocation(transition_shader, "vagueness"), transition_vagueness / 255.0);
  gl_draw_rect( 0, 0, window_width, window_height, 0.0, 0.0, 1.0, 1.0 );
  glUseProgram(0);
}

 SDL_GL_SwapWindow(window);
}

void capturedRenderSDL(SDL_Surface *surface)
{
 unsigned int *pixels = (unsigned int *)surface->pixels;
 unsigned int c0 = 0;
 size_t h = surface->h, pitch = surface->pitch / 4, w = surface->w;
 size_t y0 = 0, y1 = 0, y0pitch = 0, y1pitch = 0;
 size_t x = 0;

 SDL_GL_MakeCurrent(window, glcontext);
 glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
 glClear(GL_COLOR_BUFFER_BIT);
 glScissor(0, 0, window_width, window_height);
 glViewport(0, 0, window_width, window_height);
 renderScreen();
 glReadPixels(0, 0, window_width, window_height, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

// Swap y
 for ( ; y0 * 2 + 1 < h; ++y0 )
{
  y1 = h - 1 - y0;
  y0pitch = y0 * pitch;
  y1pitch = y1 * pitch;

  for ( x = 0; x < w; x++, y0pitch++, y1pitch++ )
{
   c0 = pixels[y0pitch];
   pixels[y0pitch] = pixels[y1pitch];
   pixels[y1pitch] = c0;
}

}

}

unsigned short NEWregisterRenderable( const unsigned short index, const unsigned char type )
{
 const unsigned short ret = regminfi;

 if ( registry_size == registry_capacity )
{
  rb_raise(rb_eRGSSError, "No more register available, maximum of %u!\n", registry_capacity );
}
 else
{
  tnewqa[regminfi].rendta = type;
  tnewqa[regminfi].rendia = index;
  registry_size++;

  for ( regminfi++; regminfi < registry_capacity; regminfi++ )
{
   if ( tnewqa[regminfi].rendia == registry_capacity ) break;
}

  newreg = 1;
}

 return(ret);
}

unsigned short NEWdisposeRenderable( const unsigned short index )
{
 unsigned short ret = registry_capacity;

 if ( registry_size == 0 )
{
  rb_raise(rb_eRGSSError, "Disposing without registers!\n" );
}
 else
{
  ret = tnewqa[index].rendia;
  tnewqa[index].rendia = registry_capacity;

  if ( regminfi > index )
{
   regminfi = index;
}

  registry_size--;
  newreg = 1;
}

 return(ret);
}

void disposeAll(void)
{
/*
 size_t i = 0;
 TODO: dispose all Bitmaps too
 for ( ; i < registry_size; ++i)
{
  registry[i]->disposed = true;
}
*/
 registry_size = 0;
}

void queueRenderJob( struct RenderJob job, const unsigned short vportid )
{
 const struct Rect *rectv = 0;
 struct Rect *recto = &nullrect;
 struct Viewport *vppw = 0;
// ptr->vportid = rb_viewport_data(newval)->ownid;

 if ( mq_size == mq_capacity )
{
  rb_raise(rb_eRGSSError, "Hopeless queue %u!\n", mq_capacity );
}
 else
{
//  if ( vport != 0 ) ( job.ox == 0 ) && ( job.oy == 0 )
  if ( vportid != 255 )
{
   vppw = rb_getvports( vportid );
/*
   if ( vppw->visible )
{
*/
   job.ox = &vppw->ox;
   job.oy = &vppw->oy;

   if ( job.reg != 1 ) job.z = vppw->z;

   rectv = rb_rect_data( vppw->rect );

   if ( ( rectv->width > 0 ) || ( rectv->height > 0 ) )
{
    recto = rectv;
}
/*
  if ( ( recto != 0 ) || ( recto->width > 0 ) || ( recto->height > 0 ) )
{
   recto = &rect;
   job_queuea[mq_size].recx = &recto->x;
   job_queuea[mq_size].recy = &recto->y;
   job_queuea[mq_size].recw = &recto->width;
   job_queuea[mq_size].rech = &recto->height;
}
*/
}
  else
{
   job.ox = &nullrect.x;
   job.oy = &nullrect.y;
}

  job_queuea[mq_size].ox = job.ox;
  job_queuea[mq_size].oy = job.oy;
  job_queuea[mq_size].recx = &recto->x;
  job_queuea[mq_size].recy = &recto->y;
  job_queuea[mq_size].recw = &recto->width;
  job_queuea[mq_size].rech = &recto->height;
  job_queuea[mq_size].z = job.z;
  job_queuea[mq_size].y = job.y;
  job_queuea[mq_size].t = job.t;
  job_queuea[mq_size].reg = job.reg;
  job_queuea[mq_size].rindex = job.rindex;
  mq_size++;
  printf( "Job %u: Z=%i, Y=%i, T=%i, R=%u, I=%u. Rec %i:%i, %i:%i, %i:%i.\n", mq_size, job.z, job.y, job.t, job.reg, job.rindex, job.ox[0], job.oy[0], recto->x, recto->y, recto->width,  recto->height );
}

}

void freeze_screen(void)
{
  SDL_Surface *frozen = create_rgba_surface(window_width, window_height);
  capturedRenderSDL(frozen);
  glBindTexture(GL_TEXTURE_2D, transition_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frozen->w, frozen->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, frozen->pixels);
  SDL_FreeSurface(frozen);
}

void defreeze_screen(void)
{
  SDL_Surface *frozen = create_rgba_surface(window_width, window_height);
  SDL_FillRect(frozen, NULL, SDL_MapRGBA(frozen->format, 0, 0, 0, 255));
  glBindTexture(GL_TEXTURE_2D, transition_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frozen->w, frozen->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, frozen->pixels);
  SDL_FreeSurface(frozen);
}

void ini_transition( void )
{
 SDL_Surface *img = 0;
/*
!strcmp(filename, "")
 if ( !filename || ( filens == 0 ) )
{
*/
 img = create_rgba_surface(window_width, window_height);
 SDL_FillRect(img, NULL, SDL_MapRGBA(img->format, 255, 255, 255, 255));
 glBindTexture(GL_TEXTURE_2D, transition_texture2);
 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
 SDL_FreeSurface(img);
 transition_vagueness = 255;
}

void load_transition_image( const char *filename, const size_t filenso, const int vagueness )
{
 SDL_Surface *img = 0;
 SDL_Surface *transition_image = 0;
 char filen[PATH_MAX + 1] = "\0", pato[PATH_MAX + 1] = "\0";
 const char extensions[2][5] = { ".png", "\0" };
 size_t filens = 0;
/*

 filens = strlen(filename);
 printf( "Loading \"%s\".\n", filename );

 Windows PATH_MAX less extension.
*/

 if ( filenso > 251 )
{
  rb_raise(rb_eRGSSError, "File %s size is too large.", filename );
}
 else
{
  strncpy( filen, filename, filenso );
  filens = loadfile_withrtp( pato, filen, extensions, filenso, 1, 1 );
// TODO: check error handling
  if ( filens == 0 )
{
   rb_raise(rb_eRGSSError, "File not found: \"%s\".", filen );
}
  else
{
   img = IMG_Load(pato);
/* TODO: check error handling */
   if ( img == 0 )
{
    rb_raise(rb_eRGSSError, "Error loading %s: %s", filename, IMG_GetError());
}
   else
{
    transition_image = create_rgba_surface_from(img);
//    printf( "Transition... " );
    glBindTexture(GL_TEXTURE_2D, transition_texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, transition_image->w, transition_image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, transition_image->pixels);
//    printf( "GL!\n" );
    SDL_FreeSurface(transition_image);
    transition_vagueness = vagueness;
}

}

}

}
