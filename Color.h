// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

// Color class in RGSS.

struct Color { double red, green, blue, alpha; };

VALUE rb_color_new(double red, double green, double blue, double alpha);
VALUE rb_color_new2(void);
bool rb_color_data_p(VALUE obj);
const struct Color *rb_color_data(VALUE obj);
struct Color *rb_color_data_mut(VALUE obj);
void Init_Color(void);
void color_set( struct Color *ptr, double newred, double newgreen, double newblue, double newalpha);
void rb_color_set2(VALUE self, VALUE other);

unsigned int maxcolorc;
