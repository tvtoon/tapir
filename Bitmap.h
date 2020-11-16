// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

struct Bitmap
{
 SDL_Surface *surface;
 GLuint texture_id;
 bool texture_invalidated;
 VALUE font;
 VALUE rect;
 VALUE pixcol;
};

VALUE rb_bitmap_new(int width, int height);
VALUE rb_bitmap_rect(VALUE self);
bool rb_bitmap_data_p(VALUE obj);
const struct Bitmap *rb_bitmap_data(VALUE obj);
struct Bitmap *rb_bitmap_data_mut(VALUE obj);
void bitmapBindTexture(struct Bitmap *ptr);
void Init_Bitmap(void);

unsigned int maxbitmapc;
