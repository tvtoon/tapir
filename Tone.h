// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

struct Tone {  double red, green, blue, gray; };

VALUE rb_tone_new(double red, double green, double blue, double gray);
VALUE rb_tone_new2(void);
bool rb_tone_data_p(VALUE obj);
const struct Tone *rb_tone_data(VALUE obj);
struct Tone *rb_tone_data_mut(VALUE obj);
void Init_Tone(void);
void rb_tone_set2(VALUE self, VALUE other);
void tone_set( struct Tone *ptr, double newred, double newgreen, double newblue, double newgray);

unsigned int maxtonec;
