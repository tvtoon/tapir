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
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "rubyfill.h"
#include "sdl_misc.h"
#include "Bitmap.h"
#include "Color.h"
#include "RGSSError.h"
#include "Rect.h"
#include "Tone.h"
#include "Viewport.h"
#include "Window.h"
#include "gl_misc.h"
#include "misc.h"

struct Window
{
 VALUE viewport, windowskin, contents, cursor_rect;
 VALUE bdispose;
 bool visible, active, pause;
 int x, y, z, width, height;
 int ox, oy;
 int opacity, back_opacity, contents_opacity;
//#if RGSS == 3
 VALUE tone;
 bool arrows_visible;
 int padding, padding_bottom;
//#endif
//#if RGSS > 1
 int openness;
//#endif
 int cursor_tick;
 int pause_tick;
 unsigned short rendid;
 unsigned short vportid;
 unsigned short contid;
 unsigned short wskinid;
/* Save ruby from itself?
 unsigned char pad;
 */
/* RGSS1 */
 unsigned char task;
 unsigned char stretch;
};

static GLuint shader1;
static GLuint shader2;
static GLuint shader3;
static GLuint shader4;
static GLuint cursor_shader;

static VALUE rb_cWindow;

static struct Window *windowspa[256] =
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/*
 Width is always "x + 32", while height is "y * 4".
*/
static struct Tplan
{
 double x;
 double y;
 double width;
 double height;
} plandim = { 96, 32, 128, 128 };

/* Inside the pause rendering, always "y * 2" for some reason... */
static const double pdpausey = 64;
static unsigned short cminindex = 0;
static unsigned short windowc = 0;
unsigned short maxwindowc = 0;

/*
 * A graphic object containing a bitmap.
 */

static void window_mark(struct Window *ptr)
{
  rb_gc_mark(ptr->viewport);
  rb_gc_mark(ptr->windowskin);
  rb_gc_mark(ptr->contents);
  rb_gc_mark(ptr->cursor_rect);
  rb_gc_mark(ptr->tone);
  rb_gc_mark(ptr->bdispose);
}

static const struct Window *rb_window_data(VALUE obj) {
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))window_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Window",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Window *ret;
  Data_Get_Struct(obj, struct Window, ret);
  return ret;
}

static struct Window *rb_window_data_mut(VALUE obj)
{
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Window");
  return (struct Window *)rb_window_data(obj);
}


void prepareRenderWindow( const unsigned short index, const unsigned short rindex )
{
 struct Window *ptr = windowspa[index];
 struct RenderJob job;

 if ( ptr == 0 )
{
#ifdef __DEBUG__
  fprintf( stderr, "Window NULL pointer at index %u!\n", index );
#endif
  rb_raise( rb_eRGSSError, "Window NULL pointer at index %u!\n", index );
  return;
}

 if (!ptr->visible) return;
/*
 if ( ptr->viewport != Qnil )
{
  job.z = vppw->z;
}
 else
{
  job.z = ptr->z;
}
*/

 job.z = ptr->z;
 job.y = ptr->y;
 job.t = rindex;
 job.reg = 3;
 job.rindex = index;
/*
 Only for RGSS1...
 job.aux[0] = 0;
*/
 queueRenderJob( job, ptr->vportid );

 if ( rgssver == 1 )
{
  job.z = ptr->z + 2;
// job.aux[0] = 1;
  queueRenderJob(job, ptr->vportid );
}

}

void renderWindow( const unsigned short index, const int vportox, const int vportoy )
{
// SDL_Surface *contents_surface = 0;
 const SDL_Surface *skinsurf = 0;
 const struct Rect *cursor_rect_ptr = 0;
 const struct Tone *tone_ptr = 0;
 const struct Window *ptr = windowspa[index];
 struct Bitmap *contents_bitmap_ptr = 0, *skin_bitmap_ptr = 0;
 struct Tone tonela = { 0.0, 0.0, 0.0, 0.0 };
 const int xrpos = -vportox + ptr->x, yrpos = -vportoy + ptr->y;
 int adjusted_x = 0, adjusted_y = 0, cursor_opacity = 128, open_height = 0, open_y = 0;

 if ( ptr->openness != 0 )
{
  cursor_rect_ptr = rb_rect_data(ptr->cursor_rect);

  if ( rgssver == 3 )
{
   adjusted_x = ptr->x + cursor_rect_ptr->x + ptr->padding - ptr->ox;
   adjusted_y = ptr->y + cursor_rect_ptr->y + ptr->padding - ptr->oy;
   tone_ptr = rb_tone_data(ptr->tone);
   tonela.red = tone_ptr->red / 255.0;
   tonela.green = tone_ptr->green / 255.0;
   tonela.blue = tone_ptr->blue / 255.0;
   tonela.gray = tone_ptr->gray / 255.0;
}
  else
{
   adjusted_x = ptr->x + cursor_rect_ptr->x + ptr->padding;
   adjusted_y = ptr->y + cursor_rect_ptr->y + ptr->padding;
}

  open_height = ptr->height * ptr->openness / 255;
  open_y = ptr->y + (ptr->height - open_height) / 2;

  if ( ptr->windowskin != Qnil )
{
/*
 GCC or ruby bug: the pointer is not initialized at the correct timing, it will segfault at the branching bellow if used directly.

   skin_bitmap_ptr = rb_bitmap_data_mut(ptr->windowskin);
   contents_bitmap_ptr = rb_bitmap_data_mut(ptr->contents);
*/
   skin_bitmap_ptr = rb_getbitmaps(ptr->wskinid);
   contents_bitmap_ptr =  rb_getbitmaps(ptr->contid);
   skinsurf = skin_bitmap_ptr->surface;
}

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);

  if ( skinsurf != 0 )
//  if ( skin_bitmap_ptr->surface != 0 )
{
   glActiveTexture(GL_TEXTURE0);
   bitmapBindTexture(skin_bitmap_ptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//   if ( stretchback )
   glUseProgram(shader1);
   glUniform1i(glGetUniformLocation(shader1, "windowskin"), 0);
   glUniform2f(glGetUniformLocation(shader1, "resolution"), window_width, window_height);
   glUniform1f(glGetUniformLocation(shader1, "opacity"), ptr->opacity * ptr->back_opacity / (255.0 * 255.0));
   glUniform4f( glGetUniformLocation(shader1, "window_tone"), tonela.red, tonela.green, tonela.blue, tonela.gray );
   gl_draw_rect( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0.0, 0.0, 1.0, 1.0);
//    gl_draw_recti( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0, 0, 1, 1 );

//   if ( tiledback )
   glUseProgram(shader2);
   glUniform1i(glGetUniformLocation(shader2, "windowskin"), 0);
   glUniform2f(glGetUniformLocation(shader2, "resolution"), window_width, window_height);
   glUniform1f(glGetUniformLocation(shader2, "opacity"), ptr->opacity * ptr->back_opacity / (255.0 * 255.0));
   glUniform4f( glGetUniformLocation(shader2, "window_tone"), tonela.red, tonela.green, tonela.blue, tonela.gray );
   gl_draw_rect( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0.0, 0.0, (ptr->width - 4) / 64.0, (open_height - 4) / 64.0);
//    gl_draw_recti( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0, 0, (ptr->width - 4) / 64, (open_height - 4) / 64 );

   glUseProgram(shader3);
   glUniform1i(glGetUniformLocation(shader3, "windowskin"), 0);
   glUniform2f(glGetUniformLocation(shader3, "resolution"), window_width, window_height);
   glUniform1f(glGetUniformLocation(shader3, "opacity"), ptr->opacity / 255.0);
   glUniform2f(glGetUniformLocation(shader3, "bg_size"), ptr->width, open_height);
   gl_draw_rect( -vportox + ptr->x, -vportoy + open_y, xrpos + ptr->width, -vportoy + open_y + open_height, 0.0, 0.0, ptr->width, open_height);
//   gl_draw_recti( -vportox + ptr->x, -vportoy + open_y, xrpos + ptr->width, -vportoy + open_y + open_height, 0, 0, ptr->width, open_height);

   if ( ptr->openness == 255 )
{

    if ( ( cursor_rect_ptr->width > 0 ) && ( cursor_rect_ptr->height > 0 ) )
{
// TODO: clipping?
    if (ptr->active) cursor_opacity = 255 - (20 - abs(ptr->cursor_tick - 20)) * 8;

    glActiveTexture(GL_TEXTURE0);
    bitmapBindTexture(skin_bitmap_ptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUseProgram(cursor_shader);
    glUniform1i(glGetUniformLocation(cursor_shader, "windowskin"), 0);
    glUniform2f(glGetUniformLocation(cursor_shader, "resolution"), window_width, window_height);
    glUniform1f(glGetUniformLocation(cursor_shader, "opacity"), ptr->contents_opacity * cursor_opacity / (255.0 * 255.0));
    glUniform2f(glGetUniformLocation(cursor_shader, "cursor_size"), cursor_rect_ptr->width, cursor_rect_ptr->height);
    gl_draw_rect( -vportox + adjusted_x, -vportoy + adjusted_y, -vportox + adjusted_x + cursor_rect_ptr->width, -vportoy + adjusted_y + cursor_rect_ptr->height, 0.0, 0.0, cursor_rect_ptr->width, cursor_rect_ptr->height);
//    gl_draw_recti( -vportox + adjusted_x, -vportoy + adjusted_y, -vportox + adjusted_x + cursor_rect_ptr->width, -vportoy + adjusted_y + cursor_rect_ptr->height, 0, 0, cursor_rect_ptr->width, cursor_rect_ptr->height );
}

    glUseProgram(shader4);
    glUniform1i(glGetUniformLocation(shader4, "contents"), 0);
    glUniform2f(glGetUniformLocation(shader4, "resolution"), window_width, window_height);
    glUniform1f(glGetUniformLocation(shader4, "opacity"), ptr->opacity / 255.0);

    if ( ( contents_bitmap_ptr != 0 ) && ptr->arrows_visible )
{

     if ( ptr->ox > 0 )
{
      gl_draw_rect( xrpos + 4, yrpos + ptr->height * 0.5 - 8, xrpos + 12, yrpos + ptr->height * 0.5 + 8, (plandim.x - 16) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height);
//      gl_draw_recti( xrpos + 4, yrpos + ptr->height / 2 - 8, xrpos + 12, yrpos + ptr->height / 2 + 8, (plandim.x - 16) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height );
}

     if ( ptr->oy > 0 )
{
      gl_draw_rect( xrpos + ptr->width * 0.5 - 8, yrpos + 4, xrpos + ptr->width * 0.5 + 8, yrpos + 12, (plandim.x - 8) / plandim.width, (plandim.y - 16) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height);
//      gl_draw_recti( xrpos + ptr->width / 2 - 8, yrpos + 4, xrpos + ptr->width / 2 + 8, yrpos + 12, (plandim.x - 8) / plandim.width, (plandim.y - 16) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height );
}

     if ( contents_bitmap_ptr->surface->w - ptr->ox > ptr->width - ptr->padding * 2 )
{
      gl_draw_rect( xrpos + ptr->width - 12, yrpos + ptr->height * 0.5 - 8, xrpos + ptr->width - 4, yrpos + ptr->height * 0.5 + 8, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x + 16) / plandim.width, (plandim.y + 8) / plandim.height);
//      gl_draw_recti( xrpos + ptr->width - 12, yrpos + ptr->height / 2 - 8, xrpos + ptr->width - 4, yrpos + ptr->height / 2 + 8, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x + 16) / plandim.width, (plandim.y + 8) / plandim.height );
}

     if ( contents_bitmap_ptr->surface->h - ptr->oy > ptr->height - ptr->padding - ptr->padding_bottom )
{
      gl_draw_rect( xrpos + ptr->width * 0.5 - 8, yrpos + ptr->height - 12, xrpos + ptr->width * 0.5 + 8, yrpos + ptr->height - 4, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y + 16) / plandim.height);
//      gl_draw_recti( xrpos + ptr->width / 2 - 8, yrpos + ptr->height - 12, xrpos + ptr->width / 2 + 8, yrpos + ptr->height - 4, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y + 16) / plandim.height);
}

}

    if ( ptr->pause )
{
     int pause_opacity = ptr->pause_tick > 16 ? 16 : ptr->pause_tick;
     int pause_anim = ptr->pause_tick % 64 / 16;
     double src_x2 = pause_anim % 2 * 16;
     double src_y2 = pause_anim / 2 * 16;

     glUniform1f(glGetUniformLocation(shader4, "opacity"), ptr->opacity * pause_opacity / (255.0 * 16.0));
     gl_draw_rect( xrpos + ptr->width * 0.5 - 8, yrpos + ptr->height - 16, xrpos + ptr->width * 0.5 + 8, yrpos + ptr->height, (plandim.x + src_x2) / plandim.width, (pdpausey + src_y2) / plandim.height, (plandim.x + src_x2 + 16) / plandim.width, (pdpausey + src_y2 + 16) / plandim.height);
//     gl_draw_recti( xrpos + ptr->width / 2 - 8, yrpos + ptr->height - 16, xrpos + ptr->width / 2 + 8, yrpos + ptr->height, (plandim.x + src_x2) / plandim.width, (pdpausey + src_y2) / plandim.height, (plandim.x + src_x2 + 16) / plandim.width, (pdpausey + src_y2 + 16) / plandim.height );
}

   if ( contents_bitmap_ptr->surface )
{
    int wcontent_width = ptr->width - ptr->padding * 2;
    int wcontent_height = ptr->height - ptr->padding - ptr->padding_bottom;
    int content_width = contents_bitmap_ptr->surface->w;
    int content_height = contents_bitmap_ptr->surface->h;
    int clip_left = ptr->ox;
    int clip_top = ptr->oy;
    int clip_right = ptr->ox + wcontent_width;
    int clip_bottom = ptr->oy + wcontent_height;

    if(clip_left < 0) clip_left = 0;
    if(clip_top < 0) clip_top = 0;
    if(clip_right > content_width) clip_right = content_width;
    if(clip_bottom > content_height) clip_bottom = content_height;
/*
    glUseProgram(shader4);
    glUniform1i(glGetUniformLocation(shader4, "contents"), 0);
    glUniform2f(glGetUniformLocation(shader4, "resolution"), window_width, window_height);
*/
    glUniform1f(glGetUniformLocation(shader4, "opacity"), ptr->contents_opacity / 255.0);
    glActiveTexture(GL_TEXTURE0);
    bitmapBindTexture(contents_bitmap_ptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl_draw_rect( xrpos + ptr->padding + (clip_left - ptr->ox), yrpos + ptr->padding + (clip_top - ptr->oy), xrpos + ptr->padding + (clip_right - ptr->ox), yrpos + ptr->padding + (clip_bottom - ptr->oy), (double)clip_left / content_width, (double)clip_top / content_height, (double)clip_right / content_width, (double)clip_bottom / content_height);
//    gl_draw_recti( xrpos + ptr->padding + (clip_left - ptr->ox), yrpos + ptr->padding + (clip_top - ptr->oy), xrpos + ptr->padding + (clip_right - ptr->ox), yrpos + ptr->padding + (clip_bottom - ptr->oy), clip_left / content_width, clip_top / content_height, clip_right / content_width, clip_bottom / content_height );
}

}

}

  glUseProgram(0);
}

}

static void window_free(struct Window *ptr)
{
 unsigned short cindex = 0;

 if ( ptr->bdispose == Qfalse )
{
  cindex = NEWdisposeRenderable( ptr->rendid );
  ptr->bdispose = Qtrue;
  windowspa[cindex] = 0;
  windowc--;

  if ( cminindex > cindex )
{
   cminindex = cindex;
}

}

 xfree(ptr);
}

static VALUE window_alloc(VALUE klass)
{
 VALUE ret = Qnil;
 struct Bitmap *bptr = 0;
 struct Window *ptr = 0;

 if ( cminindex == 256 )
{
#ifdef __DEBUG__
  fprintf( stderr, "Reached maximum window count of 256!\n" );
#endif
  rb_raise( rb_eRGSSError, "Reached maximum window count of 256!\n" );
}
 else
{
#ifdef __DEBUG__
  printf( "Allocating window %u!\n", cminindex );
#endif
  ptr = ALLOC(struct Window);
  ptr->windowskin = Qnil;
  ptr->contents = Qnil;
  ptr->stretch = true;
  ptr->cursor_rect = Qnil;
  ptr->viewport = Qnil;
  ptr->active = true;
  ptr->visible = true;
  ptr->pause = false;
  ptr->x = 0;
  ptr->y = 0;
  ptr->width = 0;
  ptr->height = 0;
  ptr->ox = 0;
  ptr->oy = 0;
  ptr->arrows_visible = true;
// TODO: In RGSS1, content Z and background Z differ.
  if ( rgssver == 3 )
{
   ptr->back_opacity = 192;
   ptr->padding = 12;
   ptr->padding_bottom = 12;
   ptr->tone = rb_tone_new2();
   ptr->z = 100;
}
  else
{
   ptr->back_opacity = 255;
   ptr->padding = 16;
   ptr->padding_bottom = 16;
   ptr->tone = Qnil;
   ptr->z = 0;
}

  ptr->opacity = 255;
  ptr->contents_opacity = 255;
  ptr->openness = 255;
  ptr->cursor_tick = 0;
  ptr->pause_tick = 0;
  ptr->bdispose = Qfalse;
  ret = Data_Wrap_Struct(klass, window_mark, window_free, ptr);
  ptr->contents = rb_bitmap_new( bptr, 1, 1 );
  ptr->cursor_rect = rb_rect_new2();
  ptr->rendid = NEWregisterRenderable( cminindex, 3 );
  ptr->vportid = 255;
  ptr->wskinid = 1024;
/* Seems silly, but is another case of rather strange bug, at allocation timing up there. */
  if ( bptr != 0 ) ptr->contid = bptr->ownid;
  else ptr->contid = 1024;

  ptr->task = 0;
  windowspa[cminindex] = ptr;

  for ( cminindex++; cminindex < 256; cminindex++ )
{
   if ( windowspa[cminindex] == 0 ) break;
}

  windowc++;

  if ( windowc > maxwindowc ) maxwindowc = windowc;
}

 return ret;
}

/*
 * call-seq:
 *   Window.new
 *   Window.new(x)
 *   Window.new(x, y)
 *   Window.new(x, y, width)
 *   Window.new(x, y, width, height)
 *
 * Creates a new window.
 */
static VALUE rb_window_m_initialize(int argc, VALUE *argv, VALUE self)
{
 struct Window *ptr = rb_window_data_mut(self);

 if ( ( argc == 0 ) || ( argc > 4 ) )
{
  rb_raise(rb_eArgError, "wrong number of arguments (%d for 4)", argc);
}
 else
{
/*
 This is done by allocation...
 ptr->x = argc > 0 ? NUM2INT(argv[0]) : 0;
 ptr->y = argc > 1 ? NUM2INT(argv[1]) : 0;
 ptr->width = argc > 2 ? NUM2INT(argv[2]) : 0;
 ptr->height = argc > 3 ? NUM2INT(argv[3]) : 0;
*/
  ptr->x = NUM2INT(argv[0]);

  if ( argc > 1 )
{
   ptr->y = NUM2INT(argv[1]);

   if ( argc > 2 )
{
    ptr->width = NUM2INT(argv[2]);

    if ( argc > 3 )
{
     ptr->height = NUM2INT(argv[3]);
}

}

}

}

 return Qnil;
}

static VALUE rb_window_m_initialize_copy(VALUE self, VALUE orig) {
  struct Window *ptr = rb_window_data_mut(self);
  const struct Window *orig_ptr = rb_window_data(orig);
  ptr->windowskin = orig_ptr->windowskin;
  ptr->contents = orig_ptr->contents;
  ptr->wskinid = orig_ptr->wskinid;
  ptr->contid = orig_ptr->contid;
  ptr->stretch = orig_ptr->stretch;
  rb_rect_set2(ptr->cursor_rect, orig_ptr->cursor_rect);
  ptr->viewport = orig_ptr->viewport;
  ptr->vportid = orig_ptr->vportid;
  ptr->active = orig_ptr->active;
  ptr->visible = orig_ptr->visible;
  ptr->arrows_visible = orig_ptr->arrows_visible;
  ptr->pause = orig_ptr->pause;
  ptr->x = orig_ptr->x;
  ptr->y = orig_ptr->y;
  ptr->width = orig_ptr->width;
  ptr->height = orig_ptr->height;
  ptr->z = orig_ptr->z;
  ptr->ox = orig_ptr->ox;
  ptr->oy = orig_ptr->oy;
  ptr->padding = orig_ptr->padding;
  ptr->padding_bottom = orig_ptr->padding_bottom;
  ptr->opacity = orig_ptr->opacity;
  ptr->back_opacity = orig_ptr->back_opacity;
  ptr->contents_opacity = orig_ptr->contents_opacity;
  ptr->openness = orig_ptr->openness;
  ptr->tone = orig_ptr->tone;
  ptr->cursor_tick = orig_ptr->cursor_tick;
  ptr->pause_tick = orig_ptr->pause_tick;
  return Qnil;
}

static VALUE rb_window_m_dispose(VALUE self)
{
 struct Window *ptr = rb_window_data_mut(self);
 unsigned short cindex = 0;

 if ( ptr->bdispose == Qfalse )
{
  cindex = NEWdisposeRenderable( ptr->rendid );
  ptr->bdispose = Qtrue;
  windowspa[cindex] = 0;
  windowc--;

  if ( cminindex > cindex )
{
   cminindex = cindex;
}
#ifdef __DEBUG__
  printf( "Disposing window %u!\n", cindex );
#endif
}

 return Qnil;
}

static VALUE rb_window_m_disposed_p(VALUE self)
{
 const struct Window *ptr = rb_window_data(self);
 return ptr->bdispose;
}

static VALUE rb_window_m_update(VALUE self)
{
 struct Window *ptr = rb_window_data_mut(self);
 ptr->cursor_tick = (ptr->cursor_tick + 1) % 40;

 if(ptr->pause)
{
  ++ptr->pause_tick;
  if(ptr->pause_tick >= 64 + 16) ptr->pause_tick -= 64;
}

 return Qnil;
}

//#if RGSS == 3
static VALUE rb_window_m_move( VALUE self, VALUE x, VALUE y, VALUE width, VALUE height)
{
 struct Window *ptr = rb_window_data_mut(self);
 ptr->x = NUM2INT(x);
 ptr->y = NUM2INT(y);
 ptr->width = NUM2INT(width);
 ptr->height = NUM2INT(height);
 return Qnil;
}

static VALUE rb_window_m_open_p(VALUE self)
{
 const struct Window *ptr = rb_window_data(self);
 return ptr->openness == 255 ? Qtrue : Qfalse;
}

static VALUE rb_window_m_close_p(VALUE self)
{
 const struct Window *ptr = rb_window_data(self);
 return ptr->openness == 0 ? Qtrue : Qfalse;
}
//#endif

static VALUE rb_window_m_windowskin(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->windowskin;
}

static VALUE rb_window_m_set_windowskin(VALUE self, VALUE newval)
{
 struct Window *ptr = rb_window_data_mut(self);

 if ( ( newval != ptr->windowskin ) && ( newval != Qnil ) )
{
  ptr->wskinid = rb_bitmap_data(newval)->ownid;
  ptr->windowskin = newval;
}
/*
 else
{
  ptr->wskinid = 1024;
}
*/

 return newval;
}

static VALUE rb_window_m_contents(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->contents;
}

static VALUE rb_window_m_set_contents(VALUE self, VALUE newval)
{
 struct Window *ptr = rb_window_data_mut(self);

 if ( ( newval != ptr->contents ) && ( newval != Qnil ) )
{
  ptr->contid = rb_bitmap_data(newval)->ownid;
  ptr->contents = newval;
}
/*
 else
{
  ptr->contid = 1024;
}
*/
 return newval;
}

//#if RGSS == 1
static VALUE rb_window_m_stretch(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->stretch ? Qtrue : Qfalse;
}

static VALUE rb_window_m_set_stretch(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->stretch = RTEST(newval);
  return newval;
}
//#endif

static VALUE rb_window_m_cursor_rect(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->cursor_rect;
}

static VALUE rb_window_m_set_cursor_rect(VALUE self, VALUE newval)
{
 struct Window *ptr = rb_window_data_mut(self);

 if ( newval != ptr->cursor_rect )
{
  rb_rect_set2(ptr->cursor_rect, newval);
}

 return newval;
}

static VALUE rb_window_m_viewport(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->viewport;
}

//#if RGSS > 1
static VALUE rb_window_m_set_viewport(VALUE self, VALUE newval)
{
 struct Window *ptr = rb_window_data_mut(self);

 if ( ( newval != ptr->viewport) && ( newval != Qnil ) )
{
  ptr->vportid = rb_viewport_data(newval)->ownid;
  ptr->viewport = newval;
}
//   ptr->vportid = 255;

 return newval;
}
//#endif

static VALUE rb_window_m_active(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->active ? Qtrue : Qfalse;
}

static VALUE rb_window_m_set_active(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->active = RTEST(newval);
  return newval;
}

static VALUE rb_window_m_visible(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->visible ? Qtrue : Qfalse;
}

static VALUE rb_window_m_set_visible(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->visible = RTEST(newval);
  return newval;
}

//#if RGSS == 3
static VALUE rb_window_m_arrows_visible(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->arrows_visible ? Qtrue : Qfalse;
}

static VALUE rb_window_m_set_arrows_visible(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->arrows_visible = RTEST(newval);
  return newval;
}
//#endif

static VALUE rb_window_m_pause(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->pause ? Qtrue : Qfalse;
}

static VALUE rb_window_m_set_pause(VALUE self, VALUE newval)
{
 struct Window *ptr = rb_window_data_mut(self);

 ptr->pause = RTEST(newval);
 if(!ptr->pause) ptr->pause_tick = 0;

 return newval;
}

static VALUE rb_window_m_x(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->x);
}

static VALUE rb_window_m_set_x(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->x = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_y(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->y);
}

static VALUE rb_window_m_set_y(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->y = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_width(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->width);
}

static VALUE rb_window_m_set_width(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->width = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_height(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->height);
}

static VALUE rb_window_m_set_height(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->height = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_z(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->z);
}

static VALUE rb_window_m_set_z(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->z = NUM2INT(newval);
//printf( "New Z on Window with Viewport %u: %i.\n", ptr->vportid, ptr->z );
  return newval;
}

static VALUE rb_window_m_ox(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->ox);
}

static VALUE rb_window_m_set_ox(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->ox = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_oy(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->oy);
}

static VALUE rb_window_m_set_oy(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->oy = NUM2INT(newval);
  return newval;
}

//#if RGSS == 3
static VALUE rb_window_m_padding(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->padding);
}

static VALUE rb_window_m_set_padding(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->padding_bottom = ptr->padding = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_padding_bottom(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->padding_bottom);
}

static VALUE rb_window_m_set_padding_bottom(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->padding_bottom = NUM2INT(newval);
  return newval;
}
//#endif

static VALUE rb_window_m_opacity(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->opacity);
}

static VALUE rb_window_m_set_opacity(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->opacity = clamp_int32(NUM2INT(newval), 0, 255);
  return newval;
}

static VALUE rb_window_m_back_opacity(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->back_opacity);
}

static VALUE rb_window_m_set_back_opacity(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->back_opacity = clamp_int32(NUM2INT(newval), 0, 255);
  return newval;
}

static VALUE rb_window_m_contents_opacity(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->contents_opacity);
}

static VALUE rb_window_m_set_contents_opacity(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->contents_opacity = clamp_int32(NUM2INT(newval), 0, 255);
  return newval;
}

//#if RGSS > 1
static VALUE rb_window_m_openness(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->openness);
}

static VALUE rb_window_m_set_openness(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->openness = clamp_int32(NUM2INT(newval), 0, 255);
  return newval;
}
//#endif

//#if RGSS == 3
static VALUE rb_window_m_tone(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->tone;
}

static VALUE rb_window_m_set_tone(VALUE self, VALUE newval)
{
 struct Window *ptr = rb_window_data_mut(self);

 if ( newval != ptr->tone )
{
  rb_tone_set2(ptr->tone, newval);
}

 return newval;
}
//#endif

/* static END */

int initWindowSDL()
{

  static const char *fsh1_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "uniform float opacity;\n"
    "uniform vec4 window_tone;\n"
    "\n"
    "void main(void) {\n"
#if RGSS >= 2
    "    vec4 color = texture2D(windowskin, vec2(gl_TexCoord[0].x * 0.5, gl_TexCoord[0].y * 0.5));\n"
#else
    "    vec4 color = texture2D(windowskin, vec2(gl_TexCoord[0].x * (2.0 / 3.0), gl_TexCoord[0].y));\n"
#endif
    "    /* Grayscale */\n"
    "    float gray = color.r * 0.298912 + color.g * 0.586611 + color.b * 0.114478;\n"
    "    color.rgb *= 1.0 - window_tone.a;\n"
    "    color.rgb += vec3(gray, gray, gray) * window_tone.a;\n"
    "    /* tone blending */\n"
    "    color.rgb = min(max(color.rgb + window_tone.rgb, 0.0), 1.0);\n"
    "    color.a *= opacity;\n"
    "    gl_FragColor = color;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

  static const char *fsh2_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "uniform float opacity;\n"
    "uniform vec4 window_tone;\n"
    "\n"
    "void main(void) {\n"
    "    vec2 coord = mod(gl_TexCoord[0].xy, 1.0);\n"
#if RGSS >= 2
    "    vec4 color = texture2D(windowskin, vec2(coord.x * 0.5, coord.y * 0.5 + 0.5));\n"
#else
    "    vec4 color = texture2D(windowskin, vec2(coord.x * (2.0 / 3.0), coord.y));\n"
#endif
    "    /* Grayscale */\n"
    "    float gray = color.r * 0.298912 + color.g * 0.586611 + color.b * 0.114478;\n"
    "    color.rgb *= 1.0 - window_tone.a;\n"
    "    color.rgb += vec3(gray, gray, gray) * window_tone.a;\n"
    "    /* tone blending */\n"
    "    color.rgb = min(max(color.rgb + window_tone.rgb, 0.0), 1.0);\n"
    "    color.a *= opacity;\n"
    "    gl_FragColor = color;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

  static const char *fsh3_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "uniform float opacity;\n"
    "uniform vec2 bg_size;\n"
    "\n"
    "void main(void) {\n"
    "    vec2 coord = gl_TexCoord[0].xy;\n"
    "    vec2 reverse_coord = bg_size - coord;\n"
    "    vec2 src_coord;\n"
    "    bool draw = false;\n"
    "    if(coord.x < 16.0) {\n"
    "      src_coord.x = coord.x;\n"
    "      draw = true;\n"
    "    } else if(reverse_coord.x < 16.0) {\n"
    "      src_coord.x = 64.0 - reverse_coord.x;\n"
    "      draw = true;\n"
    "    } else {\n"
    "      src_coord.x = mod(coord.x - 16.0, 32.0) + 16.0;\n"
    "    }\n"
    "    if(coord.y < 16.0) {\n"
    "      src_coord.y = coord.y;\n"
    "      draw = true;\n"
    "    } else if(reverse_coord.y < 16.0) {\n"
    "      src_coord.y = 64.0 - reverse_coord.y;\n"
    "      draw = true;\n"
    "    } else {\n"
    "      src_coord.y = mod(coord.y - 16.0, 32.0) + 16.0;\n"
    "    }\n"
#if RGSS >= 2
    "    src_coord.x = (src_coord.x + 64.0) / 128.0;\n"
    "    src_coord.y = src_coord.y / 128.0;\n"
#else
    "    src_coord.x = (src_coord.x + 128.0) / 192.0;\n"
    "    src_coord.y = src_coord.y / 128.0;\n"
#endif
    "    if(draw) {\n"
    "      gl_FragColor = texture2D(windowskin, src_coord);\n"
    "    } else {\n"
    "      gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
    "    }\n"
    "    gl_FragColor.a *= opacity;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

  static const char *fsh4_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "uniform float opacity;\n"
    "\n"
    "void main(void) {\n"
    "    vec4 color = texture2D(windowskin, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));\n"
    "    color.a *= opacity;\n"
    "    gl_FragColor = color;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

  static const char *cursor_fsh_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "uniform float opacity;\n"
    "uniform vec2 cursor_size;\n"
    "\n"
    "void main(void) {\n"
    "    vec2 coord = gl_TexCoord[0].xy;\n"
    "    vec2 reverse_coord = cursor_size - coord;\n"
    "    vec2 src_coord;\n"
    "    if(coord.x < 2.0) {\n"
    "      src_coord.x = coord.x;\n"
    "    } else if(reverse_coord.x < 2.0) {\n"
    "      src_coord.x = 32.0 - reverse_coord.x;\n"
    "    } else {\n"
    "      src_coord.x = (coord.x - 2.0) / (cursor_size.x - 4.0) * 28.0 + 2.0;\n"
    "    }\n"
    "    if(coord.y < 2.0) {\n"
    "      src_coord.y = coord.y;\n"
    "    } else if(reverse_coord.y < 2.0) {\n"
    "      src_coord.y = 32.0 - reverse_coord.y;\n"
    "    } else {\n"
    "      src_coord.y = (coord.y - 2.0) / (cursor_size.y - 4.0) * 28.0 + 2.0;\n"
    "    }\n"
#if RGSS >= 2
    "    src_coord.x = (src_coord.x + 64.0) / 128.0;\n"
    "    src_coord.y = (src_coord.y + 64.0) / 128.0;\n"
#else
    "    src_coord.x = (src_coord.x + 128.0) / 192.0;\n"
    "    src_coord.y = (src_coord.y + 64.0) / 128.0;\n"
#endif
    "    gl_FragColor = texture2D(windowskin, src_coord);\n"
    "    gl_FragColor.a *= opacity;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

  shader1 = compileShaders( fsh1_source);
  if ( shader1 == 0 ) return(1);

  shader2 = compileShaders( fsh2_source);
  if ( shader2 == 0 ) return(1);

  shader3 = compileShaders( fsh3_source);
  if ( shader3 == 0 ) return(1);

  shader4 = compileShaders( fsh4_source);
  if ( shader4 == 0 ) return(1);

  cursor_shader = compileShaders(cursor_fsh_source);
  if ( cursor_shader == 0 ) return(1);

  return(0);
}

void Init_Window(void) {
  rb_cWindow = rb_define_class("Window", rb_cObject);
  rb_define_alloc_func(rb_cWindow, window_alloc);
  rb_define_private_method(rb_cWindow, "initialize", rb_window_m_initialize, -1);
  rb_define_private_method(rb_cWindow, "initialize_copy", rb_window_m_initialize_copy, 1);
  rb_define_method(rb_cWindow, "dispose", rb_window_m_dispose, 0);
  rb_define_method(rb_cWindow, "disposed?", rb_window_m_disposed_p, 0);
  rb_define_method(rb_cWindow, "update", rb_window_m_update, 0);
  rb_define_method(rb_cWindow, "windowskin", rb_window_m_windowskin, 0);
  rb_define_method(rb_cWindow, "windowskin=", rb_window_m_set_windowskin, 1);
  rb_define_method(rb_cWindow, "contents", rb_window_m_contents, 0);
  rb_define_method(rb_cWindow, "contents=", rb_window_m_set_contents, 1);
  rb_define_method(rb_cWindow, "cursor_rect", rb_window_m_cursor_rect, 0);
  rb_define_method(rb_cWindow, "cursor_rect=", rb_window_m_set_cursor_rect, 1);
  rb_define_method(rb_cWindow, "viewport", rb_window_m_viewport, 0);
  rb_define_method(rb_cWindow, "active", rb_window_m_active, 0);
  rb_define_method(rb_cWindow, "active=", rb_window_m_set_active, 1);
  rb_define_method(rb_cWindow, "visible", rb_window_m_visible, 0);
  rb_define_method(rb_cWindow, "visible=", rb_window_m_set_visible, 1);
  rb_define_method(rb_cWindow, "pause", rb_window_m_pause, 0);
  rb_define_method(rb_cWindow, "pause=", rb_window_m_set_pause, 1);
  rb_define_method(rb_cWindow, "x", rb_window_m_x, 0);
  rb_define_method(rb_cWindow, "x=", rb_window_m_set_x, 1);
  rb_define_method(rb_cWindow, "y", rb_window_m_y, 0);
  rb_define_method(rb_cWindow, "y=", rb_window_m_set_y, 1);
  rb_define_method(rb_cWindow, "width", rb_window_m_width, 0);
  rb_define_method(rb_cWindow, "width=", rb_window_m_set_width, 1);
  rb_define_method(rb_cWindow, "height", rb_window_m_height, 0);
  rb_define_method(rb_cWindow, "height=", rb_window_m_set_height, 1);
  rb_define_method(rb_cWindow, "z", rb_window_m_z, 0);
  rb_define_method(rb_cWindow, "z=", rb_window_m_set_z, 1);
  rb_define_method(rb_cWindow, "ox", rb_window_m_ox, 0);
  rb_define_method(rb_cWindow, "ox=", rb_window_m_set_ox, 1);
  rb_define_method(rb_cWindow, "oy", rb_window_m_oy, 0);
  rb_define_method(rb_cWindow, "oy=", rb_window_m_set_oy, 1);
  rb_define_method(rb_cWindow, "opacity", rb_window_m_opacity, 0);
  rb_define_method(rb_cWindow, "opacity=", rb_window_m_set_opacity, 1);
  rb_define_method(rb_cWindow, "back_opacity", rb_window_m_back_opacity, 0);
  rb_define_method(rb_cWindow, "back_opacity=", rb_window_m_set_back_opacity, 1);
  rb_define_method(rb_cWindow, "contents_opacity", rb_window_m_contents_opacity, 0);
  rb_define_method(rb_cWindow, "contents_opacity=", rb_window_m_set_contents_opacity, 1);

 if ( rgssver > 1 )
{
  rb_define_method(rb_cWindow, "viewport=", rb_window_m_set_viewport, 1);
  rb_define_method(rb_cWindow, "openness", rb_window_m_openness, 0);
  rb_define_method(rb_cWindow, "openness=", rb_window_m_set_openness, 1);

  if ( rgssver == 3 )
{
   rb_define_method(rb_cWindow, "move", rb_window_m_move, 4);
   rb_define_method(rb_cWindow, "open?", rb_window_m_open_p, 0);
   rb_define_method(rb_cWindow, "close?", rb_window_m_close_p, 0);
   rb_define_method(rb_cWindow, "arrows_visible", rb_window_m_arrows_visible, 0);
   rb_define_method(rb_cWindow, "arrows_visible=", rb_window_m_set_arrows_visible, 1);
   rb_define_method(rb_cWindow, "padding", rb_window_m_padding, 0);
   rb_define_method(rb_cWindow, "padding=", rb_window_m_set_padding, 1);
   rb_define_method(rb_cWindow, "padding_bottom", rb_window_m_padding_bottom, 0);
   rb_define_method(rb_cWindow, "padding_bottom=", rb_window_m_set_padding_bottom, 1);
   rb_define_method(rb_cWindow, "tone", rb_window_m_tone, 0);
   rb_define_method(rb_cWindow, "tone=", rb_window_m_set_tone, 1);
}

}
 else
{
  plandim.x = 160;
  plandim.width = 192;
  rb_define_method(rb_cWindow, "stretch", rb_window_m_stretch, 0);
  rb_define_method(rb_cWindow, "stretch=", rb_window_m_set_stretch, 1);
}

}

void deinitWindowSDL() {
  if(cursor_shader) glDeleteProgram(cursor_shader);
  if(shader4) glDeleteProgram(shader4);
  if(shader3) glDeleteProgram(shader3);
  if(shader2) glDeleteProgram(shader2);
  if(shader1) glDeleteProgram(shader1);
}

void renderWindowRGSS1( const unsigned short index, const int vportox, const int vportoy )
{
// SDL_Surface *contents_surface = 0;
 const SDL_Surface *skinsurf = 0;
 const struct Rect *cursor_rect_ptr = 0;
 struct Bitmap *contents_bitmap_ptr = 0, *skin_bitmap_ptr = 0;
 struct Tone tonela = { 0.0, 0.0, 0.0, 0.0 };
 struct Window *ptr = windowspa[index];
 const int xrpos = -vportox + ptr->x, yrpos = -vportoy + ptr->y;
 int adjusted_x = 0, adjusted_y = 0, cursor_opacity = 128, open_height = 0, open_y = 0;

 cursor_rect_ptr = rb_rect_data(ptr->cursor_rect);
 adjusted_x = ptr->x + cursor_rect_ptr->x + ptr->padding;
 adjusted_y = ptr->y + cursor_rect_ptr->y + ptr->padding;
 open_height = ptr->height;
 open_y = ptr->y / 2;

 if ( ptr->windowskin != Qnil )
{
/*
 GCC or ruby bug: the pointer is not initialized at the correct timing, it will segfault at the branching bellow if used directly.

  skin_bitmap_ptr = rb_bitmap_data_mut(ptr->windowskin);
  contents_bitmap_ptr = rb_bitmap_data_mut(ptr->contents);
*/

  skin_bitmap_ptr = rb_getbitmaps(ptr->wskinid);
  contents_bitmap_ptr =  rb_getbitmaps(ptr->contid);
  skinsurf = skin_bitmap_ptr->surface;
}

 glEnable(GL_BLEND);
 glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
 glBlendEquation(GL_FUNC_ADD);

 if ( skinsurf != 0 )
// if ( skin_bitmap_ptr->surface != 0 )
{

  if ( ptr->task == 0 )
{
   glActiveTexture(GL_TEXTURE0);
   bitmapBindTexture(skin_bitmap_ptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   if ( ptr->stretch )
{
    glUseProgram(shader1);
    glUniform1i(glGetUniformLocation(shader1, "windowskin"), 0);
    glUniform2f(glGetUniformLocation(shader1, "resolution"), window_width, window_height);
    glUniform1f(glGetUniformLocation(shader1, "opacity"), ptr->opacity * ptr->back_opacity / (255.0 * 255.0));
    glUniform4f( glGetUniformLocation(shader1, "window_tone"), tonela.red, tonela.green, tonela.blue, tonela.gray );
    gl_draw_rect( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0.0, 0.0, 1.0, 1.0);
//    gl_draw_recti( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0, 0, 1, 1 );
}
   else
{
    glUseProgram(shader2);
    glUniform1i(glGetUniformLocation(shader2, "windowskin"), 0);
    glUniform2f(glGetUniformLocation(shader2, "resolution"), window_width, window_height);
    glUniform1f(glGetUniformLocation(shader2, "opacity"), ptr->opacity * ptr->back_opacity / (255.0 * 255.0));
    glUniform4f( glGetUniformLocation(shader2, "window_tone"), tonela.red, tonela.green, tonela.blue, tonela.gray );
    gl_draw_rect( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0.0, 0.0, (ptr->width - 4) / 64.0, (open_height - 4) / 64.0);
//    gl_draw_recti( xrpos + 2, -vportoy + open_y + 2, xrpos + ptr->width - 2, -vportoy + open_y + open_height - 2, 0, 0, (ptr->width - 4) / 64, (open_height - 4) / 64 );
}

   glUseProgram(shader3);
   glUniform1i(glGetUniformLocation(shader3, "windowskin"), 0);
   glUniform2f(glGetUniformLocation(shader3, "resolution"), window_width, window_height);
   glUniform1f(glGetUniformLocation(shader3, "opacity"), ptr->opacity / 255.0);
   glUniform2f(glGetUniformLocation(shader3, "bg_size"), ptr->width, open_height);
   gl_draw_rect( -vportox + ptr->x, -vportoy + open_y, xrpos + ptr->width, -vportoy + open_y + open_height, 0.0, 0.0, ptr->width, open_height);
//   gl_draw_recti( -vportox + ptr->x, -vportoy + open_y, xrpos + ptr->width, -vportoy + open_y + open_height, 0, 0, ptr->width, open_height);
}

  if ( ( ptr->task == 1 ) && ( cursor_rect_ptr->width > 0 ) && ( cursor_rect_ptr->height > 0 ) )
{
// TODO: clipping?
   if (ptr->active) cursor_opacity = 255 - (20 - abs(ptr->cursor_tick - 20)) * 8;

   glActiveTexture(GL_TEXTURE0);
   bitmapBindTexture(skin_bitmap_ptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glUseProgram(cursor_shader);
   glUniform1i(glGetUniformLocation(cursor_shader, "windowskin"), 0);
   glUniform2f(glGetUniformLocation(cursor_shader, "resolution"), window_width, window_height);
   glUniform1f(glGetUniformLocation(cursor_shader, "opacity"), ptr->contents_opacity * cursor_opacity / (255.0 * 255.0));
   glUniform2f(glGetUniformLocation(cursor_shader, "cursor_size"), cursor_rect_ptr->width, cursor_rect_ptr->height);
   gl_draw_rect( -vportox + adjusted_x, -vportoy + adjusted_y, -vportox + adjusted_x + cursor_rect_ptr->width, -vportoy + adjusted_y + cursor_rect_ptr->height, 0.0, 0.0, cursor_rect_ptr->width, cursor_rect_ptr->height);
//    gl_draw_recti( -vportox + adjusted_x, -vportoy + adjusted_y, -vportox + adjusted_x + cursor_rect_ptr->width, -vportoy + adjusted_y + cursor_rect_ptr->height, 0, 0, cursor_rect_ptr->width, cursor_rect_ptr->height );
}

  glUseProgram(shader4);
  glUniform1i(glGetUniformLocation(shader4, "contents"), 0);
  glUniform2f(glGetUniformLocation(shader4, "resolution"), window_width, window_height);
  glUniform1f(glGetUniformLocation(shader4, "opacity"), ptr->opacity / 255.0);

  if ( contents_bitmap_ptr != 0 )
{

   if ( ptr->ox > 0 )
{
    gl_draw_rect( xrpos + 4, yrpos + ptr->height * 0.5 - 8, xrpos + 12, yrpos + ptr->height * 0.5 + 8, (plandim.x - 16) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height);
//     gl_draw_recti( xrpos + 4, yrpos + ptr->height / 2 - 8, xrpos + 12, yrpos + ptr->height / 2 + 8, (plandim.x - 16) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height );
}

   if ( ptr->oy > 0 )
{
    gl_draw_rect( xrpos + ptr->width * 0.5 - 8, yrpos + 4, xrpos + ptr->width * 0.5 + 8, yrpos + 12, (plandim.x - 8) / plandim.width, (plandim.y - 16) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height);
//      gl_draw_recti( xrpos + ptr->width / 2 - 8, yrpos + 4, xrpos + ptr->width / 2 + 8, yrpos + 12, (plandim.x - 8) / plandim.width, (plandim.y - 16) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height );
}

   if ( contents_bitmap_ptr->surface->w - ptr->ox > ptr->width - ptr->padding * 2 )
{
    gl_draw_rect( xrpos + ptr->width - 12, yrpos + ptr->height * 0.5 - 8, xrpos + ptr->width - 4, yrpos + ptr->height * 0.5 + 8, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x + 16) / plandim.width, (plandim.y + 8) / plandim.height);
//      gl_draw_recti( xrpos + ptr->width - 12, yrpos + ptr->height / 2 - 8, xrpos + ptr->width - 4, yrpos + ptr->height / 2 + 8, (plandim.x + 8) / plandim.width, (plandim.y - 8) / plandim.height, (plandim.x + 16) / plandim.width, (plandim.y + 8) / plandim.height );
}

   if ( contents_bitmap_ptr->surface->h - ptr->oy > ptr->height - ptr->padding - ptr->padding_bottom )
{
    gl_draw_rect( xrpos + ptr->width * 0.5 - 8, yrpos + ptr->height - 12, xrpos + ptr->width * 0.5 + 8, yrpos + ptr->height - 4, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y + 16) / plandim.height);
//    gl_draw_recti( xrpos + ptr->width / 2 - 8, yrpos + ptr->height - 12, xrpos + ptr->width / 2 + 8, yrpos + ptr->height - 4, (plandim.x - 8) / plandim.width, (plandim.y + 8) / plandim.height, (plandim.x + 8) / plandim.width, (plandim.y + 16) / plandim.height);
}

}

  if ( ptr->pause )
{
   int pause_opacity = ptr->pause_tick > 16 ? 16 : ptr->pause_tick;
   int pause_anim = ptr->pause_tick % 64 / 16;
   double src_x2 = pause_anim % 2 * 16;
   double src_y2 = pause_anim / 2 * 16;

   glUniform1f(glGetUniformLocation(shader4, "opacity"), ptr->opacity * pause_opacity / (255.0 * 16.0));
   gl_draw_rect( xrpos + ptr->width * 0.5 - 8, yrpos + ptr->height - 16, xrpos + ptr->width * 0.5 + 8, yrpos + ptr->height, (plandim.x + src_x2) / plandim.width, (pdpausey + src_y2) / plandim.height, (plandim.x + src_x2 + 16) / plandim.width, (pdpausey + src_y2 + 16) / plandim.height);
//   gl_draw_recti( xrpos + ptr->width / 2 - 8, yrpos + ptr->height - 16, xrpos + ptr->width / 2 + 8, yrpos + ptr->height, (plandim.x + src_x2) / plandim.width, (pdpausey + src_y2) / plandim.height, (plandim.x + src_x2 + 16) / plandim.width, (pdpausey + src_y2 + 16) / plandim.height );
}

  if ( contents_bitmap_ptr->surface )
{
   int wcontent_width = ptr->width - ptr->padding * 2;
   int wcontent_height = ptr->height - ptr->padding - ptr->padding_bottom;
   int content_width = contents_bitmap_ptr->surface->w;
   int content_height = contents_bitmap_ptr->surface->h;
   int clip_left = ptr->ox;
   int clip_top = ptr->oy;
   int clip_right = ptr->ox + wcontent_width;
   int clip_bottom = ptr->oy + wcontent_height;

   if(clip_left < 0) clip_left = 0;
   if(clip_top < 0) clip_top = 0;
   if(clip_right > content_width) clip_right = content_width;
   if(clip_bottom > content_height) clip_bottom = content_height;
/*
    glUseProgram(shader4);
    glUniform1i(glGetUniformLocation(shader4, "contents"), 0);
    glUniform2f(glGetUniformLocation(shader4, "resolution"), window_width, window_height);
*/
   glUniform1f(glGetUniformLocation(shader4, "opacity"), ptr->contents_opacity / 255.0);
   glActiveTexture(GL_TEXTURE0);
   bitmapBindTexture(contents_bitmap_ptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   gl_draw_rect( xrpos + ptr->padding + (clip_left - ptr->ox), yrpos + ptr->padding + (clip_top - ptr->oy), xrpos + ptr->padding + (clip_right - ptr->ox), yrpos + ptr->padding + (clip_bottom - ptr->oy), (double)clip_left / content_width, (double)clip_top / content_height, (double)clip_right / content_width, (double)clip_bottom / content_height);
//    gl_draw_recti( xrpos + ptr->padding + (clip_left - ptr->ox), yrpos + ptr->padding + (clip_top - ptr->oy), xrpos + ptr->padding + (clip_right - ptr->ox), yrpos + ptr->padding + (clip_bottom - ptr->oy), clip_left / content_width, clip_top / content_height, clip_right / content_width, clip_bottom / content_height );
}

}

 ptr->task ^= 1;
 glUseProgram(0);
}
