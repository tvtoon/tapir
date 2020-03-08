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
#include "Tone.h"
#include "misc.h"

static VALUE rb_cTone;

/*
 * A tone has four Float values in its RDATA, each representing red, green,
 * blue and gray values.
 *
 * == Clamping
 *
 * RGB values should be within <code>(-255..255)</code> range. Gray values should
 * be within <code>(0..255)</code>.
 *
 * This range is enforced by the following functions: Tone.new, Tone#set,
 * Tone#red=, Tone#green=, Tone#blue=, Tone#gray=.
 *
 * == Update Hook
 *
 * == Bugs
 *
 * - Negative values for Tone#alpha= is clamped to +0.0+,
 *   but +-0.0+ is accepted as-is.
 * - NaN is clamped to NaN.
 * - Any floating point values can be injected using Tone._load.
 * - In RGSS1 and RGSS2, Tone#==, Tone#=== or Tone#eql? raises TypeError
 *   when objects other than Tone is given.
 * - Tone#set with 0 arguments is not on the official documentation.
 */

static void tone_mark(struct Tone *ptr) {
  (void) ptr;
}

static VALUE tone_alloc(VALUE klass) {
  struct Tone *ptr = ALLOC(struct Tone);
  ptr->red = 0.0;
  ptr->green = 0.0;
  ptr->blue = 0.0;
  ptr->gray = 0.0;
  VALUE ret = Data_Wrap_Struct(klass, tone_mark, -1, ptr);
  return ret;
}

/*
 * call-seq:
 *   Tone.new(red, green, blue, gray=0.0)
 *   Tone.new (RGSS3 only)
 *
 * Returns a new tone. In the second form, it initializes all fields by 0.0.
 */
static VALUE rb_tone_m_initialize(int argc, VALUE *argv, VALUE self) {
  switch(argc) {
    case 3:
      tone_set(
          rb_tone_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          0.0);
      break;
    case 4:
      tone_set(
          rb_tone_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          NUM2DBL(argv[3]));
      break;
#if RGSS == 3
    case 0:
      tone_set(rb_tone_data_mut(self), 0.0, 0.0, 0.0, 0.0);
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

static VALUE rb_tone_m_initialize_copy(VALUE self, VALUE orig) {
  if (TYPE(self) != TYPE(orig) || rb_obj_class(self) != rb_obj_class(orig)) {
    rb_raise(rb_eTypeError, "wrong argument class");
  }
  rb_tone_set2(self, orig);
  return Qnil;
}

/*
 * call-seq:
 *    color == other -> bool
 *    color === other -> bool
 *    color.eql?(other) -> bool
 *
 * Compares it with another tone, using exact comparison of IEEE754 floating
 * point numbers.
 */
static VALUE rb_tone_m_equal(VALUE self, VALUE other) {
#if RGSS == 3
  if(!rb_tone_data_p(other)) return Qfalse;
#else
  // RGSS <= 2 fails comparison when different objects are given.
  rb_tone_data(other);
#endif
  const struct Tone *ptr = rb_tone_data(self);
  const struct Tone *other_ptr = rb_tone_data(other);
  bool equal =
    ptr->red == other_ptr->red &&
    ptr->green == other_ptr->green &&
    ptr->blue == other_ptr->blue &&
    ptr->gray == other_ptr->gray;
  return equal ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *    tone.set(red, green, blue, gray=0.0) -> tone
 *    tone.set(other) -> tone (RGSS3 only)
 *    tone.set -> tone (RGSS3 only)
 *
 * Sets all fields. In the second form, it copies all fields from
 * <code>other</code>. In the third form, it initializes all fields by 0.0.
 *
 * It returns the tone itself.
 */
static VALUE rb_tone_m_set(int argc, VALUE *argv, VALUE self) {
  switch(argc) {
    // Note: original RGSS wrongly accepts empty argument list.
    // In this case, it works as set(0.0, 0.0, 0.0, 0.0).
    case 3:
      tone_set(
          rb_tone_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          0.0);
      break;
    case 4:
      tone_set(
          rb_tone_data_mut(self),
          NUM2DBL(argv[0]),
          NUM2DBL(argv[1]),
          NUM2DBL(argv[2]),
          NUM2DBL(argv[3]));
      break;
#if RGSS == 3
    case 0:
      // Undocumented, but implemented in RGSS.
      tone_set(rb_tone_data_mut(self), 0.0, 0.0, 0.0, 0.0);
      break;
    case 1:
      rb_tone_set2(self, argv[0]);
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
 *   tone.red -> float
 *
 * Returns the red value of the tone.
 */
static VALUE rb_tone_m_red(VALUE self) {
  const struct Tone *ptr = rb_tone_data(self);
  return DBL2NUM(ptr->red);
}

/*
 * call-seq:
 *   tone.red = newval -> newval
 *
 * Sets the red value of the tone, clamping the value within
 * <code>(-255..255)</code>.
 */
static VALUE rb_tone_m_set_red(VALUE self, VALUE newval) {
  struct Tone *ptr = rb_tone_data_mut(self);
  ptr->red = clamp_double(NUM2DBL(newval), -255.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   tone.green -> float
 *
 * Returns the green value of the tone.
 */
static VALUE rb_tone_m_green(VALUE self) {
  const struct Tone *ptr = rb_tone_data(self);
  return DBL2NUM(ptr->green);
}

/*
 * call-seq:
 *   tone.green = newval -> newval
 *
 * Sets the green value of the tone, clamping the value within
 * <code>(-255..255)</code>.
 */
static VALUE rb_tone_m_set_green(VALUE self, VALUE newval) {
  struct Tone *ptr = rb_tone_data_mut(self);
  ptr->green = clamp_double(NUM2DBL(newval), -255.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   tone.blue -> float
 *
 * Returns the blue value of the tone.
 */
static VALUE rb_tone_m_blue(VALUE self) {
  const struct Tone *ptr = rb_tone_data(self);
  return DBL2NUM(ptr->blue);
}

/*
 * call-seq:
 *   tone.blue = newval -> newval
 *
 * Sets the blue value of the tone, clamping the value within
 * <code>(-255..255)</code>.
 */
static VALUE rb_tone_m_set_blue(VALUE self, VALUE newval) {
  struct Tone *ptr = rb_tone_data_mut(self);
  ptr->blue = clamp_double(NUM2DBL(newval), -255.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   tone.gray -> float
 *
 * Returns the gray value of the tone.
 */
static VALUE rb_tone_m_gray(VALUE self) {
  const struct Tone *ptr = rb_tone_data(self);
  return DBL2NUM(ptr->gray);
}

/*
 * call-seq:
 *   tone.gray = newval -> newval
 *
 * Sets the gray value of the tone, clamping the value within
 * <code>(0..255)</code>.
 */
static VALUE rb_tone_m_set_gray(VALUE self, VALUE newval) {
  struct Tone *ptr = rb_tone_data_mut(self);
  ptr->gray = clamp_double(NUM2DBL(newval), 0.0, 255.0);
  return newval;
}

/*
 * call-seq:
 *   tone.to_s -> string
 *
 * Returns the string representation of the tone.
 *
 * Same as <code>"(%f, %f, %f, %f)" % [red, green, blue, gray]</code>.
 */
static VALUE rb_tone_m_to_s(VALUE self) {
  const struct Tone *ptr = rb_tone_data(self);
  char s[100];
  snprintf(s, sizeof(s), "(%f, %f, %f, %f)",
      ptr->red, ptr->green, ptr->blue, ptr->gray);
  return rb_str_new2(s);
}

/*
 * call-seq:
 *   Tone._load(str) -> tone
 *
 * Loads a tone from <code>str</code>. Used in <code>Marshal.load</code>.
 *
 * Same as <code>Tone.new(*str.unpack("EEEE"))</code> except clamping.
 */
static VALUE rb_tone_s_old_load(VALUE klass, VALUE str) {
  (void) klass;

  VALUE ret = tone_alloc(rb_cTone);
  struct Tone *ptr = rb_tone_data_mut(ret);
  StringValue(str);
  // Note: original RGSS doesn't check types.
  Check_Type(str, T_STRING);
  const char *s = RSTRING_PTR(str);
  // Note: original RGSS doesn't check length.
  if(RSTRING_LEN(str) != sizeof(double)*4) {
    rb_raise(rb_eArgError, "Corrupted marshal data for Tone.");
  }
  if(!s) return ret;
  // Note: values should be clamped, but not in the original RGSS.
  ptr->red = read_double(s+sizeof(double)*0);
  ptr->green = read_double(s+sizeof(double)*1);
  ptr->blue = read_double(s+sizeof(double)*2);
  ptr->gray = read_double(s+sizeof(double)*3);
  // ptr->red = clamp_double(read_double(s+sizeof(double)*0), -255.0, 255.0);
  // ptr->green = clamp_double(read_double(s+sizeof(double)*1), -255.0, 255.0);
  // ptr->blue = clamp_double(read_double(s+sizeof(double)*2), -255.0, 255.0);
  // ptr->gray = clamp_double(read_double(s+sizeof(double)*3), 0.0, 255.0);
  return ret;
}

/*
 * call-seq:
 *   tone._dump(limit) -> string
 *
 * Dumps a tone to a string. Used in <code>Marshal.dump</code>.
 *
 * Same as <code>[red, green, blue, gray].pack("EEEE")</code>.
 */
static VALUE rb_tone_m_old_dump(VALUE self, VALUE limit) {
  (void) limit;

  const struct Tone *ptr = rb_tone_data(self);
  char s[sizeof(double)*4];
  write_double(s+sizeof(double)*0, ptr->red);
  write_double(s+sizeof(double)*1, ptr->green);
  write_double(s+sizeof(double)*2, ptr->blue);
  write_double(s+sizeof(double)*3, ptr->gray);
  VALUE ret = rb_str_new(s, sizeof(s));
  return ret;
}

/* static END */

VALUE rb_tone_new(double red, double green, double blue, double gray) {
  VALUE ret = tone_alloc(rb_cTone);
  struct Tone *ptr = rb_tone_data_mut(ret);
  ptr->red = clamp_double(red, -255.0, 255.0);
  ptr->green = clamp_double(green, -255.0, 255.0);
  ptr->blue = clamp_double(blue, -255.0, 255.0);
  ptr->gray = clamp_double(gray, 0.0, 255.0);
  return ret;
}

VALUE rb_tone_new2(void) {
  return rb_tone_new(0.0, 0.0, 0.0, 0.0);
}

bool rb_tone_data_p(VALUE obj) {
  if(TYPE(obj) != T_DATA) return false;
  return RDATA(obj)->dmark == (void(*)(void*))tone_mark;
}

const struct Tone *rb_tone_data(VALUE obj) {
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))tone_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Tone",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Tone *ret;
  Data_Get_Struct(obj, struct Tone, ret);
  return ret;
}

struct Tone *rb_tone_data_mut(VALUE obj) {
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Tone");
  return (struct Tone *)rb_tone_data(obj);
}

void Init_Tone(void) {
  rb_cTone = rb_define_class("Tone", rb_cObject);
  rb_define_alloc_func(rb_cTone, tone_alloc);
  rb_define_private_method(rb_cTone, "initialize", rb_tone_m_initialize, -1);
  rb_define_private_method(rb_cTone, "initialize_copy", rb_tone_m_initialize_copy, 1);
  rb_define_method(rb_cTone, "==", rb_tone_m_equal, 1);
  rb_define_method(rb_cTone, "===", rb_tone_m_equal, 1);
  rb_define_method(rb_cTone, "eql?", rb_tone_m_equal, 1);
  rb_define_method(rb_cTone, "set", rb_tone_m_set, -1);
  rb_define_method(rb_cTone, "red", rb_tone_m_red, 0);
  rb_define_method(rb_cTone, "red=", rb_tone_m_set_red, 1);
  rb_define_method(rb_cTone, "green", rb_tone_m_green, 0);
  rb_define_method(rb_cTone, "green=", rb_tone_m_set_green, 1);
  rb_define_method(rb_cTone, "blue", rb_tone_m_blue, 0);
  rb_define_method(rb_cTone, "blue=", rb_tone_m_set_blue, 1);
  rb_define_method(rb_cTone, "gray", rb_tone_m_gray, 0);
  rb_define_method(rb_cTone, "gray=", rb_tone_m_set_gray, 1);
  rb_define_method(rb_cTone, "to_s", rb_tone_m_to_s, 0);
  rb_define_singleton_method(rb_cTone, "_load", rb_tone_s_old_load, 1);
  rb_define_method(rb_cTone, "_dump", rb_tone_m_old_dump, 1);
}

void rb_tone_set2(VALUE self, VALUE other) {
  struct Tone *ptr = rb_tone_data_mut(self);
  const struct Tone *other_ptr = rb_tone_data(other);
  ptr->red = other_ptr->red;
  ptr->green = other_ptr->green;
  ptr->blue = other_ptr->blue;
  ptr->gray = other_ptr->gray;
  // ptr->red = clamp_double(other_ptr->red, -255.0, 255.0);
  // ptr->green = clamp_double(other_ptr->green, -255.0, 255.0);
  // ptr->blue = clamp_double(other_ptr->blue, -255.0, 255.0);
  // ptr->gray = clamp_double(other_ptr->gray, 0.0, 255.0);
}

void tone_set(
    struct Tone *ptr, double newred, double newgreen, double newblue,
    double newgray) {
  ptr->red = clamp_double(newred, -255.0, 255.0);
  ptr->green = clamp_double(newgreen, -255.0, 255.0);
  ptr->blue = clamp_double(newblue, -255.0, 255.0);
  ptr->gray = clamp_double(newgray, 0.0, 255.0);
}
