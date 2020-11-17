// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <stdbool.h>
#include <stdint.h>

#include "rubyfill.h"
#include "Rect.h"
#include "misc.h"

// Emulation of INT2FIX in x86-32.
#define INT2FIX_E(x) INT2NUM((int32_t)(((uint32_t)(x) + 0x40000000U) & 0x7FFFFFFFU) - 0x40000000)

static VALUE rb_cRect;
//static unsigned int rectc = 0;
unsigned int maxrectc = 0;

/*
 * A rectangle has four <tt>int32_t</tt> values in its RDATA,
 * each representing x, y, width and height values.
 *
 * == Update Hook
 *
 * == Bugs
 *
 * - Rect#x, Rect#y, Rect#width and Rect#height wrongly chops the MSB
 *   in the value. Therefore, 32bit values larger than 1073741824 or
 *   smaller than -1073741825 will only appear as a result of Rect#_dump
 *   or Rect#to_s.
 * - In RGSS1 and RGSS2, Rect#==, Rect#=== or Rect#eql? raises TypeError
 *   when objects other than Rect is given.
 */

static void rect_mark(struct Rect *ptr) {
  (void) ptr;
}

static VALUE rect_alloc(VALUE klass) {
  struct Rect *ptr = ALLOC(struct Rect);
  ptr->x = 0;
  ptr->y = 0;
  ptr->width = 0;
  ptr->height = 0;
  VALUE ret = Data_Wrap_Struct(klass, rect_mark, -1, ptr);
/*
 rectc++;

 if ( rectc > maxrectc ) maxrectc = rectc;
*/

  return ret;
}

/*
 * call-seq:
 *   Rect.new(x, y, width, height)
 *   Rect.new (RGSS3 only)
 *
 * Returns a new rectangle. In the second form, it initializes all fields by 0.
 */
static VALUE rb_rect_m_initialize(int argc, VALUE *argv, VALUE self) {
  switch(argc) {
    case 4:
      rect_set(
          rb_rect_data_mut(self),
          NUM2INT(argv[0]), NUM2INT(argv[1]),
          NUM2INT(argv[2]), NUM2INT(argv[3]));
      break;
#if RGSS == 3
    case 0:
      rect_set(rb_rect_data_mut(self), 0, 0, 0, 0);
      break;
#endif
    default:
      // Note: original RGSS flips numbers.
      rb_raise(rb_eArgError,
          "wrong number of arguments (%d for 4)", argc);
      break;
  }
  return Qnil;
}

static VALUE rb_rect_m_initialize_copy(VALUE self, VALUE orig) {
  if (TYPE(self) != TYPE(orig) || rb_obj_class(self) != rb_obj_class(orig)) {
    rb_raise(rb_eTypeError, "wrong argument class");
  }
  rb_rect_set2(self, orig);
  return Qnil;
}

/*
 * call-seq:
 *    rectangle == other -> bool
 *    rectangle === other -> bool
 *    rectangle.eql?(other) -> bool
 *
 * Compares it with another rectangle.
 */
static VALUE rb_rect_m_equal(VALUE self, VALUE other) {
#if RGSS == 3
  if(!rb_rect_data_p(other)) return Qfalse;
#else
  // RGSS <= 2 fails comparison when different objects are given.
  rb_rect_data(other);
#endif
  const struct Rect *ptr = rb_rect_data(self);
  const struct Rect *other_ptr = rb_rect_data(other);
  bool equal =
    ptr->x == other_ptr->x &&
    ptr->y == other_ptr->y &&
    ptr->width == other_ptr->width &&
    ptr->height == other_ptr->height;
  return equal ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *    rectangle.set(x, y, width, height) -> rectangle
 *    rectangle.set(other) -> rectangle (RGSS3 only)
 *
 * Sets all fields. In the second form, it copies all fields from
 * <code>other</code>.
 *
 * It returns the rectangle itself.
 */
static VALUE rb_rect_m_set(int argc, VALUE *argv, VALUE self) {
  switch(argc) {
    case 4:
      rect_set(
          rb_rect_data_mut(self),
          NUM2INT(argv[0]), NUM2INT(argv[1]),
          NUM2INT(argv[2]), NUM2INT(argv[3]));
      break;
#if RGSS == 3
    case 1:
      rb_rect_set2(self, argv[0]);
      break;
#endif
    default:
      // Note: original RGSS flips numbers.
      rb_raise(rb_eArgError,
          "wrong number of arguments (%d for 4)", argc);
      break;
  }
  return self;
}

/*
 * call-seq:
 *    rectangle.empty -> rectangle
 *
 * Initializes all fields by 0.
 *
 * It returns the rectangle itself.
 */
static VALUE rb_rect_m_empty(VALUE self) {
  struct Rect *ptr = rb_rect_data_mut(self);
  rect_set(ptr, 0, 0, 0 ,0);
  return self;
}

/*
 * call-seq:
 *    rectangle.x -> integer
 *
 * Returns the x value of the rectangle.
 */
static VALUE rb_rect_m_x(VALUE self) {
  const struct Rect *ptr = rb_rect_data(self);
  // Note: RGSS wrongly wraps it with 31bit.
  return INT2FIX_E(ptr->x);
}

/*
 * call-seq:
 *    rectangle.x = newval -> newval
 *
 * Sets the x value of the rectangle.
 */
static VALUE rb_rect_m_set_x(VALUE self, VALUE newval) {
  struct Rect *ptr = rb_rect_data_mut(self);
  ptr->x = NUM2INT(newval);
  return newval;
}

/*
 * call-seq:
 *    rectangle.y -> integer
 *
 * Returns the y value of the rectangle.
 */
static VALUE rb_rect_m_y(VALUE self) {
  const struct Rect *ptr = rb_rect_data(self);
  // Note: RGSS wrongly wraps it with 31bit.
  return INT2FIX_E(ptr->y);
}

/*
 * call-seq:
 *    rectangle.y = newval -> newval
 *
 * Sets the y value of the rectangle.
 */
static VALUE rb_rect_m_set_y(VALUE self, VALUE newval) {
  struct Rect *ptr = rb_rect_data_mut(self);
  ptr->y = NUM2INT(newval);
  return newval;
}

/*
 * call-seq:
 *    rectangle.width -> integer
 *
 * Returns the width value of the rectangle.
 */
static VALUE rb_rect_m_width(VALUE self) {
  const struct Rect *ptr = rb_rect_data(self);
  // Note: RGSS wrongly wraps it with 31bit.
  return INT2FIX_E(ptr->width);
}

/*
 * call-seq:
 *    rectangle.width = newval -> newval
 *
 * Sets the width value of the rectangle.
 */
static VALUE rb_rect_m_set_width(VALUE self, VALUE newval) {
  struct Rect *ptr = rb_rect_data_mut(self);
  ptr->width = NUM2INT(newval);
  return newval;
}

/*
 * call-seq:
 *    rectangle.height -> integer
 *
 * Returns the height value of the rectangle.
 */
static VALUE rb_rect_m_height(VALUE self) {
  const struct Rect *ptr = rb_rect_data(self);
  // Note: RGSS wrongly wraps it with 31bit.
  return INT2FIX_E(ptr->height);
}

/*
 * call-seq:
 *    rectangle.height = newval -> newval
 *
 * Sets the height value of the rectangle.
 */
static VALUE rb_rect_m_set_height(VALUE self, VALUE newval) {
  struct Rect *ptr = rb_rect_data_mut(self);
  ptr->height = NUM2INT(newval);
  return newval;
}

/*
 * call-seq:
 *    rectangle.to_s -> string
 *
 * Returns the string representation of the rectangle.
 *
 * Same as <code>"(%d, %d, %d, %d)" % [x, y, width, height]</code>.
 */
static VALUE rb_rect_m_to_s(VALUE self) {
  const struct Rect *ptr = rb_rect_data(self);
  char s[60];
  snprintf(s, sizeof(s), "(%d, %d, %d, %d)",
      ptr->x, ptr->y, ptr->width, ptr->height);
  return rb_str_new2(s);
}

/*
 * call-seq:
 *   Rect._load(str) -> rectangle
 *
 * Loads a rectangle from <code>str</code>. Used in <code>Marshal.load</code>.
 *
 * Same as <code>Rect.new(*str.unpack("l<l<l<l<"))</code>.
 */
static VALUE rb_rect_s_old_load(VALUE klass, VALUE str) {
  (void) klass;
  VALUE ret = rect_alloc(rb_cRect);
  struct Rect *ptr = rb_rect_data_mut(ret);
  StringValue(str);
  // Note: original RGSS doesn't check types.
  Check_Type(str, T_STRING);
  const char *s = RSTRING_PTR(str);
  // Note: original RGSS doesn't check length.
  if(RSTRING_LEN(str) != sizeof(int32_t)*4) {
    rb_raise(rb_eArgError, "Corrupted marshal data for Rect.");
  }
  if(!s) return ret;
  ptr->x = read_int32(s+sizeof(int32_t)*0);
  ptr->y = read_int32(s+sizeof(int32_t)*1);
  ptr->width = read_int32(s+sizeof(int32_t)*2);
  ptr->height = read_int32(s+sizeof(int32_t)*3);
  return ret;
}

/*
 * call-seq:
 *   rectangle._dump(limit) -> string
 *
 * Dumps a rectangle to a string. Used in <code>Marshal.dump</code>.
 *
 * Same as <code>[x, y, width, height].pack("l<l<l<l<")</code>.
 */
static VALUE rb_rect_m_old_dump(VALUE self, VALUE limit) {
  (void) limit;
  const struct Rect *ptr = rb_rect_data(self);
  char s[sizeof(int32_t)*4];
  write_int32(s+sizeof(int32_t)*0, ptr->x);
  write_int32(s+sizeof(int32_t)*1, ptr->y);
  write_int32(s+sizeof(int32_t)*2, ptr->width);
  write_int32(s+sizeof(int32_t)*3, ptr->height);
  VALUE ret = rb_str_new(s, sizeof(s));
  return ret;
}

/* static END */

VALUE rb_rect_new(int32_t x, int32_t y, int32_t width, int32_t height) {
  VALUE ret = rect_alloc(rb_cRect);
  struct Rect *ptr = rb_rect_data_mut(ret);
  ptr->x = x;
  ptr->y = y;
  ptr->width = width;
  ptr->height = height;
  return ret;
}

VALUE rb_rect_new2(void)
{
maxrectc++;
 return rb_rect_new(0, 0, 0, 0);
}

void Init_Rect() {
  rb_cRect = rb_define_class("Rect", rb_cObject);
  rb_define_alloc_func(rb_cRect, rect_alloc);
  rb_define_private_method(rb_cRect, "initialize", rb_rect_m_initialize, -1);
  rb_define_private_method(rb_cRect, "initialize_copy",
      rb_rect_m_initialize_copy, 1);
  rb_define_method(rb_cRect, "==", rb_rect_m_equal, 1);
  rb_define_method(rb_cRect, "===", rb_rect_m_equal, 1);
  rb_define_method(rb_cRect, "eql?", rb_rect_m_equal, 1);
  rb_define_method(rb_cRect, "set", rb_rect_m_set, -1);
  rb_define_method(rb_cRect, "empty", rb_rect_m_empty, 0);
  rb_define_method(rb_cRect, "x", rb_rect_m_x, 0);
  rb_define_method(rb_cRect, "x=", rb_rect_m_set_x, 1);
  rb_define_method(rb_cRect, "y", rb_rect_m_y, 0);
  rb_define_method(rb_cRect, "y=", rb_rect_m_set_y, 1);
  rb_define_method(rb_cRect, "width", rb_rect_m_width, 0);
  rb_define_method(rb_cRect, "width=", rb_rect_m_set_width, 1);
  rb_define_method(rb_cRect, "height", rb_rect_m_height, 0);
  rb_define_method(rb_cRect, "height=", rb_rect_m_set_height, 1);
  rb_define_method(rb_cRect, "to_s", rb_rect_m_to_s, 0);
  rb_define_singleton_method(rb_cRect, "_load", rb_rect_s_old_load, 1);
  rb_define_method(rb_cRect, "_dump", rb_rect_m_old_dump, 1);
}

bool rb_rect_data_p(VALUE obj) {
  if(TYPE(obj) != T_DATA) return false;
  return RDATA(obj)->dmark == (void(*)(void*))rect_mark;
}

const struct Rect *rb_rect_data(VALUE obj) {
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))rect_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Rect",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Rect *ret;
  Data_Get_Struct(obj, struct Rect, ret);
  return ret;
}

struct Rect *rb_rect_data_mut(VALUE obj) {
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Rect");
  return (struct Rect *)rb_rect_data(obj);
}

void rect_set(
    struct Rect *ptr, int32_t newx, int32_t newy,
    int32_t newwidth, int32_t newheight) {
  ptr->x = newx;
  ptr->y = newy;
  ptr->width = newwidth;
  ptr->height = newheight;
}

void rb_rect_set2(VALUE self, VALUE other) {
  struct Rect *ptr = rb_rect_data_mut(self);
  const struct Rect *other_ptr = rb_rect_data(other);
  ptr->x = other_ptr->x;
  ptr->y = other_ptr->y;
  ptr->width = other_ptr->width;
  ptr->height = other_ptr->height;
}
