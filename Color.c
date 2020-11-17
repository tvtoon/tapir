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
#include "Color.h"
#include "misc.h"

static VALUE rb_cColor;
//static unsigned int colorc = 0;
unsigned int maxcolorc = 0;

/*
 * A color has four Float values in its RDATA, each representing red, green,
 * blue and alpha values.
 *
 * == Clamping
 *
 * RGB values should be within <code>(-255..255)</code> range. Alpha values should
 * be within <code>(0..255)</code>.
 *
 * This range is enforced by the following functions: Color.new, Color#set,
 * Color#red=, Color#green=, Color#blue=, Color#alpha=.
 *
 * == Update Hook
 *
 * A color generated by Font#color has a special hook on it.
 *
 * When the color is modified, it calls Font#color= of the parent object with self as an argument. (Not yet implemented by Tapir)
 *
 * == Bugs
 *
 * - RGB values should be clamped within <code>(0..255)</code>
 *   according to the official documentation. However, the actual clamp range
 *   is <code>(-255..255)</code>.
 * - Negative values for Color#alpha= is clamped to +0.0+,
 *   but +-0.0+ is accepted as-is.
 * - NaN is clamped to NaN.
 * - Any floating point values can be injected using Color._load.
 * - In RGSS1 and RGSS2, Color#==, Color#=== or Color#eql? raises TypeError
 *   when objects other than Color is given.
 * - Tone#set with 0 arguments is not on the official documentation.
 */

static void color_mark(struct Color *ptr) {
  (void) ptr;
}

static VALUE color_alloc(VALUE klass) {
  struct Color *ptr = ALLOC(struct Color);
  ptr->red = 0.0;
  ptr->green = 0.0;
  ptr->blue = 0.0;
  ptr->alpha = 0.0;
  VALUE ret = Data_Wrap_Struct(klass, color_mark, -1, ptr);
/*
 colorc++;

 if ( colorc > maxcolorc ) maxcolorc = colorc;
*/
 maxcolorc++;

 return ret;
}

/*
 * call-seq:
 *   Color.new(red, green, blue, alpha=255.0)
 *   Color.new (RGSS3 only)
 *
 * Returns a new color. In the second form, it initializes all fields by 0.0.
 */
static VALUE rb_color_m_initialize(int argc, VALUE *argv, VALUE self) {
  switch(argc) {
    case 3:
      color_set(
          rb_color_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          255.0);
      break;
    case 4:
      color_set(
          rb_color_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          NUM2DBL(argv[3]));
      break;
#if RGSS == 3
    case 0:
      color_set(rb_color_data_mut(self), 0.0, 0.0, 0.0, 0.0);
      break;
#endif
    default:
      // Note: original RGSS has wrong messages.
#if RGSS == 3
      rb_raise(rb_eArgError,
          "wrong number of arguments (%d for 0 or 3..4)", argc);
#else
      rb_raise(rb_eArgError,
          "wrong number of arguments (%d for 3..4)", argc);
#endif
      break;
  }
  return Qnil;
}

static VALUE rb_color_m_initialize_copy(VALUE self, VALUE orig) {
  if (TYPE(self) != TYPE(orig) || rb_obj_class(self) != rb_obj_class(orig)) {
    rb_raise(rb_eTypeError, "wrong argument class");
  }
  rb_color_set2(self, orig);
  return Qnil;
}

/*
 * call-seq:
 *    color == other -> bool
 *    color === other -> bool
 *    color.eql?(other) -> bool
 *
 * Compares it with another color, using exact comparison of IEEE754 floating
 * point numbers.
 */
static VALUE rb_color_m_equal(VALUE self, VALUE other) {
#if RGSS == 3
  if(!rb_color_data_p(other)) return Qfalse;
#else
  // RGSS <= 2 fails comparison when different objects are given.
  rb_color_data(other);
#endif
  const struct Color *ptr = rb_color_data(self);
  const struct Color *other_ptr = rb_color_data(other);
  bool equal =
    ptr->red == other_ptr->red &&
    ptr->green == other_ptr->green &&
    ptr->blue == other_ptr->blue &&
    ptr->alpha == other_ptr->alpha;
  return equal ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *    color.set(red, green, blue, alpha=255.0) -> color
 *    color.set(other) -> color (RGSS3 only)
 *    color.set -> color (RGSS3 only)
 *
 * Sets all fields. In the second form, it copies all fields from
 * <code>other</code>. In the third form, it initializes all fields by 0.0.
 *
 * It returns the color itself.
 */
static VALUE rb_color_m_set(int argc, VALUE *argv, VALUE self) {
  switch(argc) {
    // Note: original RGSS wrongly accepts empty argument list.
    // In this case, it works as set(0.0, 0.0, 0.0, 0.0).
    case 3:
      color_set(
          rb_color_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          255.0);
      break;
    case 4:
      color_set(
          rb_color_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          NUM2DBL(argv[3]));
      break;
#if RGSS == 3
    case 0:
      // Undocumented, but implemented in RGSS.
      color_set(rb_color_data_mut(self), 0.0, 0.0, 0.0, 0.0);
      break;
    case 1:
      rb_color_set2(self, argv[0]);
      break;
#endif
    default:
      // Note: original RGSS has wrong messages.
#if RGSS == 3
      rb_raise(rb_eArgError,
          "wrong number of arguments (%d for 0..1, 3..4)", argc);
#else
      rb_raise(rb_eArgError,
          "wrong number of arguments (%d for 3..4)", argc);
#endif
      break;
  }
  return self;
}

/*
 * call-seq:
 *   color.red -> float
 *
 * Returns the red value of the color.
 */
static VALUE rb_color_m_red(VALUE self) {
  const struct Color *ptr = rb_color_data(self);
  return DBL2NUM(ptr->red);
}

/*
 * call-seq:
 *   color.red = newval -> newval
 *
 * Sets the red value of the color, clamping the value within
 * <code>(-255..255)</code>.
 */
static VALUE rb_color_m_set_red(VALUE self, VALUE newval) {
  struct Color *ptr = rb_color_data_mut(self);
  // Note: RGB values are expected to be clamped within [0, 255].
  // but original RGSS wrongly uses [-255, 255].
  ptr->red = clamp_double(NUM2DBL(newval), -255.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   color.green -> float
 *
 * Returns the green value of the color.
 */
static VALUE rb_color_m_green(VALUE self) {
  const struct Color *ptr = rb_color_data(self);
  return DBL2NUM(ptr->green);
}

/*
 * call-seq:
 *   color.green = newval -> newval
 *
 * Sets the green value of the color, clamping the value within
 * <code>(-255..255)</code>.
 */
static VALUE rb_color_m_set_green(VALUE self, VALUE newval) {
  struct Color *ptr = rb_color_data_mut(self);
  // Note: RGB values are expected to be clamped within [0, 255].
  // but original RGSS wrongly uses [-255, 255].
  ptr->green = clamp_double(NUM2DBL(newval), -255.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   color.blue -> float
 *
 * Returns the blue value of the color.
 */
static VALUE rb_color_m_blue(VALUE self) {
  const struct Color *ptr = rb_color_data(self);
  return DBL2NUM(ptr->blue);
}

/*
 * call-seq:
 *   color.blue = newval -> newval
 *
 * Sets the blue value of the color, clamping the value within
 * <code>(-255..255)</code>.
 */
static VALUE rb_color_m_set_blue(VALUE self, VALUE newval) {
  struct Color *ptr = rb_color_data_mut(self);
  // Note: RGB values are expected to be clamped within [0, 255].
  // but original RGSS wrongly uses [-255, 255].
  ptr->blue = clamp_double(NUM2DBL(newval), -255.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   color.alpha -> float
 *
 * Returns the alpha value of the color.
 */
static VALUE rb_color_m_alpha(VALUE self) {
  const struct Color *ptr = rb_color_data(self);
  return DBL2NUM(ptr->alpha);
}

/*
 * call-seq:
 *   color.alpha = newval -> newval
 *
 * Sets the alpha value of the color, clamping the value within
 * <code>(0..255)</code>.
 */
static VALUE rb_color_m_set_alpha(VALUE self, VALUE newval) {
  struct Color *ptr = rb_color_data_mut(self);
  ptr->alpha = clamp_double(NUM2DBL(newval), 0.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   color.to_s -> string
 *
 * Returns the string representation of the color.
 *
 * Same as <code>"(%f, %f, %f, %f)" % [red, green, blue, alpha]</code>.
 */
static VALUE rb_color_m_to_s(VALUE self) {
  const struct Color *ptr = rb_color_data(self);
  char s[100];
  snprintf(s, sizeof(s), "(%f, %f, %f, %f)",
      ptr->red, ptr->green, ptr->blue, ptr->alpha);
  return rb_str_new2(s);
}

/*
 * call-seq:
 *   Color._load(str) -> color
 *
 * Loads a color from <code>str</code>. Used in <code>Marshal.load</code>.
 *
 * Same as <code>Color.new(*str.unpack("EEEE"))</code> except clamping.
 */
static VALUE rb_color_s_old_load(VALUE klass, VALUE str) {
  (void) klass;

  VALUE ret = color_alloc(rb_cColor);
  struct Color *ptr = rb_color_data_mut(ret);
  StringValue(str);
  // Note: original RGSS doesn't check types.
  Check_Type(str, T_STRING);
  const char *s = RSTRING_PTR(str);
  // Note: original RGSS doesn't check length.
  if(RSTRING_LEN(str) != sizeof(double)*4) {
    rb_raise(rb_eArgError, "Corrupted marshal data for Color.");
  }
  if(!s) return ret;
  // Note: values should be clamped, but not in the original RGSS.
  ptr->red = read_double(s+sizeof(double)*0);
  ptr->green = read_double(s+sizeof(double)*1);
  ptr->blue = read_double(s+sizeof(double)*2);
  ptr->alpha = read_double(s+sizeof(double)*3);
  // ptr->red = clamp_double(read_double(s+sizeof(double)*0), 0.0, 255.0);
  // ptr->green = clamp_double(read_double(s+sizeof(double)*1), 0.0, 255.0);
  // ptr->blue = clamp_double(read_double(s+sizeof(double)*2), 0.0, 255.0);
  // ptr->alpha = clamp_double(read_double(s+sizeof(double)*3), 0.0, 255.0);
  return ret;
}

/*
 * call-seq:
 *   color._dump(limit) -> string
 *
 * Dumps a color to a string. Used in <code>Marshal.dump</code>.
 *
 * Same as <code>[red, green, blue, alpha].pack("EEEE")</code>.
 */
static VALUE rb_color_m_old_dump(VALUE self, VALUE limit) {
  (void) limit;

  const struct Color *ptr = rb_color_data(self);
  char s[sizeof(double)*4];
  write_double(s+sizeof(double)*0, ptr->red);
  write_double(s+sizeof(double)*1, ptr->green);
  write_double(s+sizeof(double)*2, ptr->blue);
  write_double(s+sizeof(double)*3, ptr->alpha);
  VALUE ret = rb_str_new(s, sizeof(s));
  return ret;
}

/* static END */

VALUE rb_color_new(double red, double green, double blue, double alpha) {
  VALUE ret = color_alloc(rb_cColor);
  struct Color *ptr = rb_color_data_mut(ret);
  // Note: RGB values are expected to be clamped within [0, 255].
  // but original RGSS wrongly uses [-255, 255].
  ptr->red = clamp_double(red, -255.0, 255.0);
  ptr->green = clamp_double(green, -255.0, 255.0);
  ptr->blue = clamp_double(blue, -255.0, 255.0);
  ptr->alpha = clamp_double(alpha, 0.0, 255.0);
  return ret;
}

VALUE rb_color_new2(void) {
  return rb_color_new(0.0, 0.0, 0.0, 0.0);
}

bool rb_color_data_p(VALUE obj) {
  if(TYPE(obj) != T_DATA) return false;
  return RDATA(obj)->dmark == (void(*)(void*))color_mark;
}

const struct Color *rb_color_data(VALUE obj) {
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))color_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Color",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Color *ret;
  Data_Get_Struct(obj, struct Color, ret);
  return ret;
}

struct Color *rb_color_data_mut(VALUE obj) {
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Color");
  return (struct Color *)rb_color_data(obj);
}

void Init_Color(void) {
  rb_cColor = rb_define_class("Color", rb_cObject);
  rb_define_alloc_func(rb_cColor, color_alloc);
  rb_define_private_method(rb_cColor, "initialize", rb_color_m_initialize, -1);
  rb_define_private_method(rb_cColor, "initialize_copy",
      rb_color_m_initialize_copy, 1);
  rb_define_method(rb_cColor, "==", rb_color_m_equal, 1);
  rb_define_method(rb_cColor, "===", rb_color_m_equal, 1);
  rb_define_method(rb_cColor, "eql?", rb_color_m_equal, 1);
  rb_define_method(rb_cColor, "set", rb_color_m_set, -1);
  rb_define_method(rb_cColor, "red", rb_color_m_red, 0);
  rb_define_method(rb_cColor, "red=", rb_color_m_set_red, 1);
  rb_define_method(rb_cColor, "green", rb_color_m_green, 0);
  rb_define_method(rb_cColor, "green=", rb_color_m_set_green, 1);
  rb_define_method(rb_cColor, "blue", rb_color_m_blue, 0);
  rb_define_method(rb_cColor, "blue=", rb_color_m_set_blue, 1);
  rb_define_method(rb_cColor, "alpha", rb_color_m_alpha, 0);
  rb_define_method(rb_cColor, "alpha=", rb_color_m_set_alpha, 1);
  rb_define_method(rb_cColor, "to_s", rb_color_m_to_s, 0);
  rb_define_singleton_method(rb_cColor, "_load", rb_color_s_old_load, 1);
  rb_define_method(rb_cColor, "_dump", rb_color_m_old_dump, 1);
}

void color_set(
    struct Color *ptr, double newred, double newgreen, double newblue,
    double newalpha) {
  // Note: RGB values are expected to be clamped within [0, 255].
  // but original RGSS wrongly uses [-255, 255].
  ptr->red = clamp_double(newred, -255.0, 255.0);
  ptr->green = clamp_double(newgreen, -255.0, 255.0);
  ptr->blue = clamp_double(newblue, -255.0, 255.0);
  ptr->alpha = clamp_double(newalpha, 0.0, 255.0);
}

void rb_color_set2(VALUE self, VALUE other) {
  struct Color *ptr = rb_color_data_mut(self);
  const struct Color *other_ptr = rb_color_data(other);
  ptr->red = other_ptr->red;
  ptr->green = other_ptr->green;
  ptr->blue = other_ptr->blue;
  ptr->alpha = other_ptr->alpha;
  // ptr->red = clamp_double(other_ptr->red, 0.0, 255.0);
  // ptr->green = clamp_double(other_ptr->green, 0.0, 255.0);
  // ptr->blue = clamp_double(other_ptr->blue, 0.0, 255.0);
  // ptr->alpha = clamp_double(other_ptr->alpha, 0.0, 255.0);
}
