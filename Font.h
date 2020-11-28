// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

struct Font
{
  VALUE name, size;
  VALUE color;
/*
#if RGSS == 3
*/
  VALUE out_color;
  bool outline;
/*#endif RGSS == 3 */
  bool bold, italic;
/*#if RGSS > 1*/
  bool shadow;
/*#endif */
  TTF_Font *cache;
};

TTF_Font *rb_font_to_sdl(VALUE self);
VALUE rb_font_new(void);
bool rb_font_data_p(VALUE obj);
const struct Font *rb_font_data(VALUE obj);
struct Font *rb_font_data_mut(VALUE obj);
void Init_Font(void);
void rb_font_set(VALUE self, VALUE other);

unsigned int maxfontc;
