// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <math.h>
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
#include "Sprite.h"
#include "Tone.h"
#include "Viewport.h"
#include "gl_misc.h"
#include "misc.h"

struct Sprite
{
 VALUE viewport, bitmap, src_rect, color, tone, flash_color;
 VALUE bdispose;
// Rect src_rect;
 bool visible, mirror;
 int x, y, z, ox, oy;
 int bush_depth, opacity, blend_type;
 int flash_duration, flash_count;
 bool flash_is_nil;
 double zoom_x, zoom_y, angle;
//#if RGSS > 1
 double wave_phase;
 int wave_amp, wave_length, wave_speed, bush_opacity;
//#endif
 unsigned short vportid;
 unsigned short rendid;
 unsigned short bitmapid;
};

static GLuint shader;

static VALUE rb_cSprite;

static struct Sprite *spritespa[512] = {
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
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static unsigned short cminindex = 0;
static unsigned short spritec = 0;
unsigned short maxspritec = 0;

/*
 * A graphic object containing a bitmap.
 */

static void sprite_mark(struct Sprite *ptr)
{
  rb_gc_mark(ptr->viewport);
  rb_gc_mark(ptr->bitmap);
  rb_gc_mark(ptr->src_rect);
  rb_gc_mark(ptr->color);
  rb_gc_mark(ptr->tone);
  rb_gc_mark(ptr->flash_color);
  rb_gc_mark(ptr->bdispose);
}

static const struct Sprite *rb_sprite_data(VALUE obj)
{
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))sprite_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Sprite",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Sprite *ret;
  Data_Get_Struct(obj, struct Sprite, ret);
  return ret;
}

static struct Sprite *rb_sprite_data_mut(VALUE obj) {
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Sprite");
  return (struct Sprite *)rb_sprite_data(obj);
}

void prepareRenderSprite( const unsigned short index, const unsigned short rindex )
{
 struct Sprite *ptr = spritespa[index];
 struct RenderJob job;

 if ( ptr == 0 )
{
  rb_raise( rb_eRGSSError, "Sprite NULL pointer at index %u!\n", index );
  return;
}

 if(!ptr->visible) return;
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
 job.reg = 2;
 job.rindex = index;
 queueRenderJob( job, ptr->vportid );
}

void renderSprite( const unsigned short index, const int vportox, const int vportoy )
{
 struct Sprite *ptr = spritespa[index];
 const struct Color *color_ptr = rb_color_data(ptr->color);
 const struct Color *flash_color_ptr = rb_color_data(ptr->flash_color);

 if(ptr->flash_duration > 0 && ptr->flash_is_nil) return;

 double flash_opacity = ptr->flash_duration <= 0 ? 0.0 : 1.0 - (double)ptr->flash_count / ptr->flash_duration;

 const struct Tone *tone_ptr = rb_tone_data(ptr->tone);

#ifdef __DEBUG__
 if ( ( rgssver > 1 ) && ( ptr->wave_amp != 0 ) ) WARN_UNIMPLEMENTED("Sprite#wave_amp");

 if (ptr->bush_depth) WARN_UNIMPLEMENTED("Sprite#bush_depth");
#endif

 if ( ptr->bitmap == Qnil ) return;

//  const struct Bitmap *bitmap_ptr = rb_bitmap_data(ptr->bitmap);
 const struct Bitmap *bitmap_ptr = rb_getbitmaps(ptr->bitmapid);
 SDL_Surface *surface = bitmap_ptr->surface;

  if(!surface) return;

  const struct Rect *src_rect = rb_rect_data(ptr->src_rect);

  glEnable(GL_BLEND);

  if(ptr->blend_type == 1) {
    glBlendFuncSeparate(
        GL_ONE, GL_ONE,
        GL_ZERO, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
  } else if(ptr->blend_type == 2) {
    glBlendFuncSeparate(
        GL_ONE, GL_ONE,
        GL_ZERO, GL_ONE);
    glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
  } else {
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
  }

  int src_left = src_rect->x;
  int src_top = src_rect->y;
  int src_right = src_rect->x + src_rect->width;
  int src_bottom = src_rect->y + src_rect->height;
  if(src_left < 0) src_left = 0;
  if(src_top < 0) src_top = 0;
  if(src_right > surface->w) src_right = surface->w;
  if(src_bottom > surface->h) src_bottom = surface->h;

  double angle_rad = ptr->angle * (3.1415926535897932384 / 180.0);
  double angle_cos = cos(angle_rad);
  double angle_sin = sin(angle_rad);
  double zoom_x_inv = 1.0 / ptr->zoom_x;
  double zoom_y_inv = 1.0 / ptr->zoom_y;

  GLfloat zoom_angle[4] = {
    zoom_x_inv * angle_cos, zoom_x_inv * -angle_sin,
    zoom_y_inv * angle_sin, zoom_y_inv * angle_cos
  };

  glUseProgram(shader);
  glUniform1i(glGetUniformLocation(shader, "tex"), 0);
  glUniform2f(glGetUniformLocation(shader, "resolution"), window_width, window_height);
  glUniform2f(glGetUniformLocation(shader, "src_size"),
      surface->w, surface->h);
  glUniform2f(glGetUniformLocation(shader, "src_topleft"),
      src_left, src_top);
  glUniform2f(glGetUniformLocation(shader, "src_bottomright"),
      src_right, src_bottom);
  glUniform2f(glGetUniformLocation(shader, "dst_translate"),
      ptr->x, ptr->y);
  glUniform2f(glGetUniformLocation(shader, "src_translate"),
      ptr->ox + src_rect->x, ptr->oy + src_rect->y);
  glUniformMatrix2fv(glGetUniformLocation(shader, "zoom_angle"),
      1, true, zoom_angle);
  glUniform1i(glGetUniformLocation(shader, "mirror"), ptr->mirror);
  glUniform1f(glGetUniformLocation(shader, "opacity"),
      ptr->opacity / 255.0);
  if(flash_color_ptr->alpha * flash_opacity > color_ptr->alpha) {
    glUniform4f(glGetUniformLocation(shader, "sprite_color"),
        flash_color_ptr->red / 255.0,
        flash_color_ptr->green / 255.0,
        flash_color_ptr->blue / 255.0,
        flash_color_ptr->alpha / 255.0 * flash_opacity);
  } else {
    glUniform4f(glGetUniformLocation(shader, "sprite_color"),
        color_ptr->red / 255.0,
        color_ptr->green / 255.0,
        color_ptr->blue / 255.0,
        color_ptr->alpha / 255.0);
  }
  glUniform4f(glGetUniformLocation(shader, "sprite_tone"),
      tone_ptr->red / 255.0,
      tone_ptr->green / 255.0,
      tone_ptr->blue / 255.0,
      tone_ptr->gray / 255.0);

  glActiveTexture(GL_TEXTURE0);
  bitmapBindTexture((struct Bitmap *)bitmap_ptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  gl_draw_rect(
      0.0, 0.0, window_width, window_height,
      vportox,
      vportoy,
      vportox + window_width,
      vportoy + window_height );

  glUseProgram(0);
}

static void sprite_free(struct Sprite *ptr)
{
 unsigned short cindex = 0;

 if ( ptr->bdispose == Qfalse )
{
  cindex = NEWdisposeRenderable( ptr->rendid );
  ptr->bdispose = Qtrue;
  spritespa[cindex] = 0;
  spritec--;

  if ( cminindex > cindex )
{
   cminindex = cindex;
}

}

 xfree(ptr);
}

static VALUE sprite_alloc(VALUE klass)
{
 VALUE ret = Qnil;
 struct Sprite *ptr = 0;

 if ( cminindex == 512 )
{
  rb_raise( rb_eRGSSError, "Reached maximum sprite count of 512!\n" );
}
 else
{
#ifdef __DEBUG__
 printf( "Allocating sprite %u!\n", cminindex );
#endif
  ptr = ALLOC(struct Sprite);
  ptr->z = 0;
  ptr->viewport = Qnil;
  ptr->bitmap = Qnil;
  ptr->src_rect = Qnil;
  ptr->bdispose = Qfalse;
  ptr->visible = true;
  ptr->x = 0;
  ptr->y = 0;
  ptr->ox = 0;
  ptr->oy = 0;
  ptr->zoom_x = 1.0;
  ptr->zoom_y = 1.0;
  ptr->angle = 0.0;
  ptr->mirror = false;
  ptr->bush_depth = 0;
//#if RGSS > 1
  ptr->bush_opacity = 128;
  ptr->wave_amp = 0;
  ptr->wave_length = 0;
  ptr->wave_speed = 0;
  ptr->wave_phase = 0.0;
//#endif
  ptr->opacity = 255;
  ptr->blend_type = 0;
  ptr->color = Qnil;
  ptr->tone = Qnil;
  ptr->flash_color = Qnil;
  ptr->flash_duration = 0;
  ptr->flash_count = 0;
  ptr->flash_is_nil = false;
  ret = Data_Wrap_Struct(klass, sprite_mark, sprite_free, ptr);
  ptr->src_rect = rb_rect_new2();
  ptr->color = rb_color_new2();
  ptr->tone = rb_tone_new2();
  ptr->flash_color = rb_color_new2();
  ptr->vportid = 255;
  ptr->bitmapid = 1024;
  ptr->rendid = NEWregisterRenderable( cminindex, 2 );
  spritespa[cminindex] = ptr;

  for ( cminindex++; cminindex < 512; cminindex++ )
{
   if ( spritespa[cminindex] == 0 ) break;
}

  spritec++;

  if ( spritec > maxspritec ) maxspritec = spritec;
}

 return ret;
}

/*
 * call-seq:
 *   Sprite.new
 *   Sprite.new(viewport)
 *
 * Creates new sprite object, possibly with viewport.
 */
static VALUE rb_sprite_m_initialize(int argc, VALUE *argv, VALUE self)
{
 struct Sprite *ptr = rb_sprite_data_mut(self);

 if ( argc == 1 )
{
// rb_viewport_data(argv[0]);
  if ( argv[0] != Qnil )
{
   ptr->vportid = rb_viewport_data(argv[0])->ownid;
   ptr->viewport = argv[0];
}

}
 else
{
  if ( argc != 0 ) rb_raise(rb_eArgError, "wrong number of arguments (%d for 0..1)", argc);
}

 return Qnil;
}

static VALUE rb_sprite_m_initialize_copy(VALUE self, VALUE orig) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  const struct Sprite *orig_ptr = rb_sprite_data(orig);
  ptr->z = orig_ptr->z;
  ptr->viewport = orig_ptr->viewport;
  ptr->vportid = orig_ptr->vportid;
  ptr->bitmap = orig_ptr->bitmap;
  ptr->bitmapid = orig_ptr->bitmapid;
  rb_rect_set2(ptr->src_rect, orig_ptr->src_rect);
  rb_color_set2(ptr->color, orig_ptr->color);
  rb_tone_set2(ptr->tone, orig_ptr->tone);
  ptr->visible = orig_ptr->visible;
  ptr->mirror = orig_ptr->mirror;
  ptr->x = orig_ptr->x;
  ptr->y = orig_ptr->y;
  ptr->ox = orig_ptr->ox;
  ptr->oy = orig_ptr->oy;
  ptr->bush_depth = orig_ptr->bush_depth;
  ptr->opacity = orig_ptr->opacity;
  ptr->blend_type = orig_ptr->blend_type;
  ptr->zoom_x = orig_ptr->zoom_x;
  ptr->zoom_y = orig_ptr->zoom_y;
  ptr->angle = orig_ptr->angle;
//#if RGSS > 1
  ptr->wave_amp = orig_ptr->wave_amp;
  ptr->wave_length = orig_ptr->wave_length;
  ptr->wave_speed = orig_ptr->wave_speed;
  ptr->bush_opacity = orig_ptr->bush_opacity;
  ptr->wave_phase = orig_ptr->wave_phase;
//#endif
  rb_color_set2(ptr->flash_color, orig_ptr->flash_color);
  ptr->flash_duration = orig_ptr->flash_duration;
  ptr->flash_count = orig_ptr->flash_count;
  ptr->flash_is_nil = orig_ptr->flash_is_nil;
  return Qnil;
}

static VALUE rb_sprite_m_dispose(VALUE self) {
 struct Sprite *ptr = rb_sprite_data_mut(self);
 unsigned short cindex = 0;

 if ( ptr->bdispose == Qfalse )
{
  cindex = NEWdisposeRenderable( ptr->rendid );
  ptr->bdispose = Qtrue;
  spritespa[cindex] = 0;
  spritec--;

  if ( cminindex > cindex )
{
   cminindex = cindex;
}

#ifdef __DEBUG__
  printf( "Disposing sprite %u!\n", cindex );
#endif
}

 return Qnil;
}

static VALUE rb_sprite_m_disposed_p(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
 return ptr->bdispose;
}

static VALUE rb_sprite_m_flash(VALUE self, VALUE color, VALUE duration) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  if(color == Qnil) {
    ptr->flash_is_nil = true;
  } else {
    ptr->flash_is_nil = false;
    rb_color_set2(ptr->flash_color, color);
  }
  ptr->flash_duration = NUM2INT(duration);
  ptr->flash_count = 0;
  return Qnil;
}

static VALUE rb_sprite_m_update(VALUE self)
{
 struct Sprite *ptr = rb_sprite_data_mut(self);

 if ( rgssver > 1 ) ptr->wave_phase += (double)ptr->wave_speed / ptr->wave_length;

 ++ptr->flash_count;

 if(ptr->flash_count >= ptr->flash_duration)
{
  ptr->flash_count = 0;
  ptr->flash_duration = 0;
}

 return Qnil;
}

static VALUE rb_sprite_m_bitmap(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return ptr->bitmap;
}

static VALUE rb_sprite_m_set_bitmap(VALUE self, VALUE newval)
{
 struct Sprite *ptr = rb_sprite_data_mut(self);
 const struct Bitmap *bitmap_ptr;

 if (newval != Qnil)
{
  bitmap_ptr = rb_bitmap_data(newval);

  if (ptr->bitmap != newval)
{
   ptr->bitmap = newval;
   ptr->bitmapid = bitmap_ptr->ownid;

   if ( bitmap_ptr->surface )
{
//    rb_rect_set2(ptr->src_rect, rb_bitmap_rect(newval));
    rb_rect_set2( ptr->src_rect, bitmap_ptr->rect );
}
   else
{
    rb_raise(rb_eRGSSError, "disposed bitmap");
}

}

}

 return newval;
}

static VALUE rb_sprite_m_src_rect(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return ptr->src_rect;
}

static VALUE rb_sprite_m_set_src_rect(VALUE self, VALUE newval)
{
 struct Sprite *ptr = rb_sprite_data_mut(self);

 if ( ptr->src_rect != newval ) rb_rect_set2(ptr->src_rect, newval);

 return newval;
}

static VALUE rb_sprite_m_viewport(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return ptr->viewport;
}

static VALUE rb_sprite_m_visible(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return ptr->visible ? Qtrue : Qfalse;
}

static VALUE rb_sprite_m_set_visible(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->visible = RTEST(newval);
  return newval;
}

static VALUE rb_sprite_m_x(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->x);
}

static VALUE rb_sprite_m_set_x(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->x = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_y(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->y);
}

static VALUE rb_sprite_m_set_y(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->y = NUM2INT(newval);
/*
 if ( ptr->y < 0 )
{
  ptr->y = -ptr->y;
  newval = INT2NUM(ptr->y);
}
*/
  return newval;
}

static VALUE rb_sprite_m_z(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->z);
}

static VALUE rb_sprite_m_set_z(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->z = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_ox(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->ox);
}

static VALUE rb_sprite_m_set_ox(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->ox = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_oy(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->oy);
}

static VALUE rb_sprite_m_set_oy(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->oy = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_zoom_x(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return DBL2NUM(ptr->zoom_x);
}

static VALUE rb_sprite_m_set_zoom_x(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->zoom_x = NUM2DBL(newval);
  return newval;
}

static VALUE rb_sprite_m_zoom_y(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return DBL2NUM(ptr->zoom_y);
}

static VALUE rb_sprite_m_set_zoom_y(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->zoom_y = NUM2DBL(newval);
  return newval;
}

static VALUE rb_sprite_m_angle(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return DBL2NUM(ptr->angle);
}

static VALUE rb_sprite_m_set_angle(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->angle = NUM2DBL(newval);
  return newval;
}

//#if RGSS > 1
static VALUE rb_sprite_m_width(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(rb_rect_data(ptr->src_rect)->width);
}

static VALUE rb_sprite_m_height(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(rb_rect_data(ptr->src_rect)->height);
}

static VALUE rb_sprite_m_set_viewport(VALUE self, VALUE newval)
{
 struct Sprite *ptr = rb_sprite_data_mut(self);

 if ( ( newval != ptr->viewport ) && ( newval != Qnil ) )
{
// rb_viewport_data(newval);
  ptr->vportid = rb_viewport_data(newval)->ownid;
  ptr->viewport = newval;
}

 return newval;
}

static VALUE rb_sprite_m_wave_amp(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->wave_amp);
}

static VALUE rb_sprite_m_set_wave_amp(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->wave_amp = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_wave_length(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->wave_length);
}

static VALUE rb_sprite_m_set_wave_length(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->wave_length = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_wave_speed(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->wave_speed);
}

static VALUE rb_sprite_m_set_wave_speed(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->wave_speed = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_wave_phase(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return DBL2NUM(ptr->wave_phase);
}

static VALUE rb_sprite_m_set_wave_phase(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->wave_phase = NUM2DBL(newval);
  return newval;
}

static VALUE rb_sprite_m_bush_opacity(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->bush_opacity);
}

static VALUE rb_sprite_m_set_bush_opacity(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->bush_opacity = clamp_int32(NUM2INT(newval), 0, 255);
  return newval;
}
//#endif RGSS > 1

static VALUE rb_sprite_m_mirror(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return ptr->mirror ? Qtrue : Qfalse;
}

static VALUE rb_sprite_m_set_mirror(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->mirror = RTEST(newval);
  return newval;
}

static VALUE rb_sprite_m_bush_depth(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->bush_depth);
}

static VALUE rb_sprite_m_set_bush_depth(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->bush_depth = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_opacity(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->opacity);
}

static VALUE rb_sprite_m_set_opacity(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  ptr->opacity = clamp_int32(NUM2INT(newval), 0, 255);
  return newval;
}

static VALUE rb_sprite_m_blend_type(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return INT2NUM(ptr->blend_type);
}

static VALUE rb_sprite_m_set_blend_type(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  /* TODO: check range */
  ptr->blend_type = NUM2INT(newval);
  return newval;
}

static VALUE rb_sprite_m_color(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return ptr->color;
}

static VALUE rb_sprite_m_set_color(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  rb_color_set2(ptr->color, newval);
  return newval;
}
static VALUE rb_sprite_m_tone(VALUE self) {
  const struct Sprite *ptr = rb_sprite_data(self);
  return ptr->tone;
}

static VALUE rb_sprite_m_set_tone(VALUE self, VALUE newval) {
  struct Sprite *ptr = rb_sprite_data_mut(self);
  rb_tone_set2(ptr->tone, newval);
  return newval;
}

/* static END */

int initSpriteSDL()
{

  static const char *fsh_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform vec2 resolution;\n"
    "uniform sampler2D tex;\n"
    "uniform vec2 dst_translate;\n"
    "uniform vec2 src_translate;\n"
    "uniform vec2 src_topleft;\n"
    "uniform vec2 src_bottomright;\n"
    "uniform vec2 src_size;\n"
    "uniform mat2 zoom_angle;\n"
    "uniform bool mirror;\n"
    "uniform float opacity;\n"
    "uniform vec4 sprite_color;\n"
    "uniform vec4 sprite_tone;\n"
    "\n"
    "void main(void) {\n"
    "    vec4 color;\n"
    "    vec2 coord = gl_TexCoord[0].xy;\n"
    "    coord = coord - dst_translate;\n"
    "    coord = zoom_angle * coord;\n"
    "    coord = coord + src_translate;\n"
    "    if(src_topleft.x <= coord.x && src_topleft.y <= coord.y && coord.x <= src_bottomright.x && coord.y <= src_bottomright.y) {\n"
    "      if(mirror) {\n"
    "        coord.x = src_topleft.x + src_bottomright.x - coord.x;\n"
    "      }\n"
    "      color = texture2D(tex, vec2(coord.x / src_size.x, coord.y / src_size.y));\n"
    "    } else {\n"
    "      color = vec4(0.0, 0.0, 0.0, 0.0);\n"
    "    }\n"
    "    /* Grayscale */\n"
    "    float gray = color.r * 0.298912 + color.g * 0.586611 + color.b * 0.114478;\n"
    "    color.rgb *= 1.0 - sprite_tone.a;\n"
    "    color.rgb += vec3(gray, gray, gray) * sprite_tone.a;\n"
    "    /* tone blending */\n"
    "    color.rgb = min(max(color.rgb + sprite_tone.rgb, 0.0), 1.0);\n"
    "    /* color blending */\n"
    "    color.rgb *= 1.0 - sprite_color.a;\n"
    "    color.rgb += sprite_color.rgb * sprite_color.a;\n"
    "    color.a *= opacity;\n"
    "    gl_FragColor = color;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

 shader = compileShaders(fsh_source);

 if (shader == 0) return(1);

 return(0);
}

void Init_Sprite(void)
{
 rb_cSprite = rb_define_class("Sprite", rb_cObject);
 rb_define_alloc_func(rb_cSprite, sprite_alloc);
 rb_define_private_method(rb_cSprite, "initialize", rb_sprite_m_initialize, -1);
 rb_define_private_method(rb_cSprite, "initialize_copy", rb_sprite_m_initialize_copy, 1);
 rb_define_method(rb_cSprite, "dispose", rb_sprite_m_dispose, 0);
 rb_define_method(rb_cSprite, "disposed?", rb_sprite_m_disposed_p, 0);
 rb_define_method(rb_cSprite, "flash", rb_sprite_m_flash, 2);
 rb_define_method(rb_cSprite, "update", rb_sprite_m_update, 0);
 rb_define_method(rb_cSprite, "bitmap", rb_sprite_m_bitmap, 0);
 rb_define_method(rb_cSprite, "bitmap=", rb_sprite_m_set_bitmap, 1);
 rb_define_method(rb_cSprite, "src_rect", rb_sprite_m_src_rect, 0);
 rb_define_method(rb_cSprite, "src_rect=", rb_sprite_m_set_src_rect, 1);
 rb_define_method(rb_cSprite, "viewport", rb_sprite_m_viewport, 0);
 rb_define_method(rb_cSprite, "visible", rb_sprite_m_visible, 0);
 rb_define_method(rb_cSprite, "visible=", rb_sprite_m_set_visible, 1);
 rb_define_method(rb_cSprite, "x", rb_sprite_m_x, 0);
 rb_define_method(rb_cSprite, "x=", rb_sprite_m_set_x, 1);
 rb_define_method(rb_cSprite, "y", rb_sprite_m_y, 0);
 rb_define_method(rb_cSprite, "y=", rb_sprite_m_set_y, 1);
 rb_define_method(rb_cSprite, "z", rb_sprite_m_z, 0);
 rb_define_method(rb_cSprite, "z=", rb_sprite_m_set_z, 1);
 rb_define_method(rb_cSprite, "ox", rb_sprite_m_ox, 0);
 rb_define_method(rb_cSprite, "ox=", rb_sprite_m_set_ox, 1);
 rb_define_method(rb_cSprite, "oy", rb_sprite_m_oy, 0);
 rb_define_method(rb_cSprite, "oy=", rb_sprite_m_set_oy, 1);
 rb_define_method(rb_cSprite, "zoom_x", rb_sprite_m_zoom_x, 0);
 rb_define_method(rb_cSprite, "zoom_x=", rb_sprite_m_set_zoom_x, 1);
 rb_define_method(rb_cSprite, "zoom_y", rb_sprite_m_zoom_y, 0);
 rb_define_method(rb_cSprite, "zoom_y=", rb_sprite_m_set_zoom_y, 1);
 rb_define_method(rb_cSprite, "angle", rb_sprite_m_angle, 0);
 rb_define_method(rb_cSprite, "angle=", rb_sprite_m_set_angle, 1);
 rb_define_method(rb_cSprite, "mirror", rb_sprite_m_mirror, 0);
 rb_define_method(rb_cSprite, "mirror=", rb_sprite_m_set_mirror, 1);
 rb_define_method(rb_cSprite, "bush_depth", rb_sprite_m_bush_depth, 0);
 rb_define_method(rb_cSprite, "bush_depth=", rb_sprite_m_set_bush_depth, 1);
 rb_define_method(rb_cSprite, "opacity", rb_sprite_m_opacity, 0);
 rb_define_method(rb_cSprite, "opacity=", rb_sprite_m_set_opacity, 1);
 rb_define_method(rb_cSprite, "blend_type", rb_sprite_m_blend_type, 0);
 rb_define_method(rb_cSprite, "blend_type=", rb_sprite_m_set_blend_type, 1);
 rb_define_method(rb_cSprite, "color", rb_sprite_m_color, 0);
 rb_define_method(rb_cSprite, "color=", rb_sprite_m_set_color, 1);
 rb_define_method(rb_cSprite, "tone", rb_sprite_m_tone, 0);
 rb_define_method(rb_cSprite, "tone=", rb_sprite_m_set_tone, 1);

 if ( rgssver > 1 )
{
  rb_define_method(rb_cSprite, "bush_opacity", rb_sprite_m_bush_opacity, 0);
  rb_define_method(rb_cSprite, "bush_opacity=", rb_sprite_m_set_bush_opacity, 1);
  rb_define_method(rb_cSprite, "wave_amp", rb_sprite_m_wave_amp, 0);
  rb_define_method(rb_cSprite, "wave_amp=", rb_sprite_m_set_wave_amp, 1);
  rb_define_method(rb_cSprite, "wave_length", rb_sprite_m_wave_length, 0);
  rb_define_method(rb_cSprite, "wave_length=", rb_sprite_m_set_wave_length, 1);
  rb_define_method(rb_cSprite, "wave_speed", rb_sprite_m_wave_speed, 0);
  rb_define_method(rb_cSprite, "wave_speed=", rb_sprite_m_set_wave_speed, 1);
  rb_define_method(rb_cSprite, "wave_phase", rb_sprite_m_wave_phase, 0);
  rb_define_method(rb_cSprite, "wave_phase=", rb_sprite_m_set_wave_phase, 1);
  rb_define_method(rb_cSprite, "width", rb_sprite_m_width, 0);
  rb_define_method(rb_cSprite, "height", rb_sprite_m_height, 0);
  rb_define_method(rb_cSprite, "viewport=", rb_sprite_m_set_viewport, 1);
}

}

void deinitSpriteSDL() {
  if(shader) glDeleteProgram(shader);
}
