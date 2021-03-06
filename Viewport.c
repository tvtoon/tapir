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
#include <SDL_opengl.h>

#include "rubyfill.h"
#include "sdl_misc.h"
#include "Color.h"
#include "RGSSError.h"
#include "Rect.h"
#include "Tone.h"
#include "Viewport.h"
#include "misc.h"

static VALUE rb_cViewport;

static struct Viewport *vportspa[64] = { 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 };
static unsigned short cminindex = 0;
static unsigned short vportqc = 0;
unsigned short maxvportqc = 0;


/*
 * A graphic object container.
 */

static void viewport_mark(struct Viewport *ptr)
{
  rb_gc_mark(ptr->rect);
  rb_gc_mark(ptr->color);
  rb_gc_mark(ptr->tone);
  rb_gc_mark(ptr->bdispose);
}

static struct Viewport *rb_viewport_data_mut(VALUE obj)
{
// Note: original RGSS doesn't check frozen.
 if(OBJ_FROZEN(obj)) rb_error_frozen("Viewport");
 return (struct Viewport *)rb_viewport_data(obj);
}

static void viewport_free(struct Viewport *ptr)
{
 const unsigned short theind = ptr->ownid;

 if ( ptr->bdispose == Qfalse )
// if ( vportspa[theind] != 0 )
{
#ifdef __DEBUG__
  printf( "Freeing viewport %u!\n", theind );
#endif
//  cindex = NEWdisposeRenderable( ptr->rendid );
  vportspa[theind][0].bdispose = Qtrue;
  vportspa[theind][0].ownid = 64;
  vportqc--;

  if ( cminindex > theind )
{
   cminindex = theind;
}

  vportspa[theind] = 0;
}

 xfree(ptr);
}

static VALUE viewport_alloc(VALUE klass)
{
 VALUE ret = Qnil;
 struct Viewport *ptr = 0;

 if ( cminindex == 64 )
{
  rb_raise( rb_eRGSSError, "Reached maximum viewport count of 64!\n" );
}
 else
{
#ifdef __DEBUG__
  printf( "Allocating viewport %u!\n", cminindex );
#endif
  ptr = ALLOC(struct Viewport);
//  ptr->viewport_queue.capacity = 64;
//  initRenderQueue(&ptr->viewport_queue);
  ptr->rect = Qnil;
  ptr->color = Qnil;
  ptr->tone = Qnil;
  ptr->visible = true;
  ptr->ox = 0;
  ptr->oy = 0;
  ptr->z = 0;
  ptr->bdispose = Qfalse;
  ret = Data_Wrap_Struct(klass, viewport_mark, viewport_free, ptr);
  ptr->rect = rb_rect_new2();
  ptr->color = rb_color_new2();
  ptr->tone = rb_tone_new2();
//  ptr->rendid = NEWregisterRenderable( cminindex, 3 );
  ptr->ownid = cminindex;
  vportspa[cminindex] = ptr;

  for ( cminindex++; cminindex < 64; cminindex++ )
{
   if ( vportspa[cminindex] == 0 ) break;
}

  vportqc++;

  if ( vportqc > maxvportqc ) maxvportqc = vportqc;
}

  return ret;
}

/*
 * call-seq:
 *   Viewport.new(x, y, width, height)
 *   Viewport.new(rect)
 *   Viewport.new
 *
 * Creates a new viewport.
 */
static VALUE rb_viewport_m_initialize(int argc, VALUE *argv, VALUE self)
{
 struct Viewport *ptr = rb_viewport_data_mut(self);

 if ( argc == 1 )
{
  rb_rect_set2(ptr->rect, argv[0]);
}
 else if ( argc == 4 )
{
  rect_set( rb_rect_data_mut(ptr->rect), NUM2INT(argv[0]), NUM2INT(argv[1]), NUM2INT(argv[2]), NUM2INT(argv[3]) );
}
 else
{

  if ( rgssver == 3 )
{
   if ( argc == 0 ) rect_set(rb_rect_data_mut(ptr->rect), 0, 0, window_width, window_height);
   else rb_raise(rb_eArgError, "wrong number of arguments (%d for 0..1 or 4)", argc);
}
  else
{
   rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 4)", argc);
}

}

 return Qnil;
}

static VALUE rb_viewport_m_initialize_copy(VALUE self, VALUE orig) {
  struct Viewport *ptr = rb_viewport_data_mut(self);
  const struct Viewport *orig_ptr = rb_viewport_data(orig);
  rb_rect_set2(ptr->rect, orig_ptr->rect);
  rb_color_set2(ptr->color, orig_ptr->color);
  rb_tone_set2(ptr->tone, orig_ptr->tone);
  ptr->visible = orig_ptr->visible;
  ptr->ox = orig_ptr->ox;
  ptr->oy = orig_ptr->oy;
  ptr->z = orig_ptr->z;
  return Qnil;
}

static VALUE rb_viewport_m_dispose(VALUE self)
{
// struct Viewport *ptr = rb_viewport_data_mut(self);
 const struct Viewport *ptr = rb_viewport_data(self);
 const unsigned short theind = ptr->ownid;

 if ( ptr->bdispose == Qfalse )
// if ( vportspa[theind] != 0 )
{
#ifdef __DEBUG__
  printf( "Freeing viewport %u!\n", theind );
#endif
//  cindex = NEWdisposeRenderable( ptr->rendid );
  vportspa[theind][0].bdispose = Qtrue;
  vportspa[theind][0].ownid = 64;
  vportqc--;

  if ( cminindex > theind )
{
   cminindex = theind;
}

  vportspa[theind] = 0;
}

 return Qnil;
}

static VALUE rb_viewport_m_disposed_p(VALUE self)
{
 const struct Viewport *ptr = rb_viewport_data(self);
 return ptr->bdispose;
}

static VALUE rb_viewport_m_flash(VALUE self, VALUE color, VALUE duration) {
  (void) self;
  (void) color;
  (void) duration;
  WARN_UNIMPLEMENTED("Viewport#flash");
  return Qnil;
}

static VALUE rb_viewport_m_update(VALUE self) {
  (void) self;
  WARN_UNIMPLEMENTED("Viewport#update");
  return Qnil;
}

static VALUE rb_viewport_m_rect(VALUE self) {
  const struct Viewport *ptr = rb_viewport_data(self);
  return ptr->rect;
}

static VALUE rb_viewport_m_set_rect(VALUE self, VALUE newval)
{
 struct Viewport *ptr = rb_viewport_data_mut(self);

 if ( newval != ptr->rect )
{
  rb_rect_set2(ptr->rect, newval);
}

 return newval;
}

static VALUE rb_viewport_m_visible(VALUE self) {
  const struct Viewport *ptr = rb_viewport_data(self);
  return ptr->visible ? Qtrue : Qfalse;
}

static VALUE rb_viewport_m_set_visible(VALUE self, VALUE newval) {
  struct Viewport *ptr = rb_viewport_data_mut(self);
  ptr->visible = RTEST(newval);
//printf( "New visible on Viewport %u: %i.\n", ptr->ownid, ptr->visible );
  return newval;
}

static VALUE rb_viewport_m_z(VALUE self) {
  const struct Viewport *ptr = rb_viewport_data(self);
  return INT2NUM(ptr->z);
}

static VALUE rb_viewport_m_set_z(VALUE self, VALUE newval) {
  struct Viewport *ptr = rb_viewport_data_mut(self);
  ptr->z = NUM2INT(newval);
//printf( "New Z on Viewport %u: %i.\n", ptr->ownid, ptr->z );
  return newval;
}

static VALUE rb_viewport_m_ox(VALUE self) {
  const struct Viewport *ptr = rb_viewport_data(self);
  return INT2NUM(ptr->ox);
}

static VALUE rb_viewport_m_set_ox(VALUE self, VALUE newval) {
  struct Viewport *ptr = rb_viewport_data_mut(self);
  ptr->ox = NUM2INT(newval);
  return newval;
}

static VALUE rb_viewport_m_oy(VALUE self) {
  const struct Viewport *ptr = rb_viewport_data(self);
  return INT2NUM(ptr->oy);
}

static VALUE rb_viewport_m_set_oy(VALUE self, VALUE newval) {
  struct Viewport *ptr = rb_viewport_data_mut(self);
  ptr->oy = NUM2INT(newval);
  return newval;
}

static VALUE rb_viewport_m_color(VALUE self) {
  const struct Viewport *ptr = rb_viewport_data(self);
  return ptr->color;
}

static VALUE rb_viewport_m_set_color(VALUE self, VALUE newval)
{
 struct Viewport *ptr = rb_viewport_data_mut(self);

 if ( newval != ptr->color )
{
  rb_color_set2(ptr->color, newval);
}

 return newval;
}

static VALUE rb_viewport_m_tone(VALUE self) {
  const struct Viewport *ptr = rb_viewport_data(self);
  return ptr->tone;
}

static VALUE rb_viewport_m_set_tone(VALUE self, VALUE newval)
{
 struct Viewport *ptr = rb_viewport_data_mut(self);

 if ( newval != ptr->tone )
{
  rb_tone_set2(ptr->tone, newval);
}

 return newval;
}

/* static END */

const struct Viewport *rb_viewport_data(VALUE obj) {
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))viewport_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Viewport",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Viewport *ret;
  Data_Get_Struct(obj, struct Viewport, ret);
  return ret;
}

struct Viewport *rb_getvports( const unsigned short id )
{
/*
 vports->ox = vportspa[id][0].ox;
 vports->oy = vportspa[id][0].oy;
 vports->z = vportspa[id][0].z;
 vports->visible = vportspa[id][0].visible;
// vports-> = vportspa[id][0].ownid;
 vports->rendid = vportspa[id][0].rendid;
*/
 return( vportspa[id] );
}

void Init_Viewport(void) {
  rb_cViewport = rb_define_class("Viewport", rb_cObject);
  rb_define_alloc_func(rb_cViewport, viewport_alloc);
  rb_define_private_method(rb_cViewport, "initialize",
      rb_viewport_m_initialize, -1);
  rb_define_private_method(rb_cViewport, "initialize_copy",
      rb_viewport_m_initialize_copy, 1);
  rb_define_method(rb_cViewport, "dispose", rb_viewport_m_dispose, 0);
  rb_define_method(rb_cViewport, "disposed?", rb_viewport_m_disposed_p, 0);
  rb_define_method(rb_cViewport, "flash", rb_viewport_m_flash, 2);
  rb_define_method(rb_cViewport, "update", rb_viewport_m_update, 0);
  rb_define_method(rb_cViewport, "rect", rb_viewport_m_rect, 0);
  rb_define_method(rb_cViewport, "rect=", rb_viewport_m_set_rect, 1);
  rb_define_method(rb_cViewport, "visible", rb_viewport_m_visible, 0);
  rb_define_method(rb_cViewport, "visible=", rb_viewport_m_set_visible, 1);
  rb_define_method(rb_cViewport, "z", rb_viewport_m_z, 0);
  rb_define_method(rb_cViewport, "z=", rb_viewport_m_set_z, 1);
  rb_define_method(rb_cViewport, "ox", rb_viewport_m_ox, 0);
  rb_define_method(rb_cViewport, "ox=", rb_viewport_m_set_ox, 1);
  rb_define_method(rb_cViewport, "oy", rb_viewport_m_oy, 0);
  rb_define_method(rb_cViewport, "oy=", rb_viewport_m_set_oy, 1);
  rb_define_method(rb_cViewport, "color", rb_viewport_m_color, 0);
  rb_define_method(rb_cViewport, "color=", rb_viewport_m_set_color, 1);
  rb_define_method(rb_cViewport, "tone", rb_viewport_m_tone, 0);
  rb_define_method(rb_cViewport, "tone=", rb_viewport_m_set_tone, 1);
}
