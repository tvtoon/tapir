// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

struct BitmapArray { VALUE data[9]; };

VALUE rb_bitmaparray_new(void);

bool rb_bitmaparray_data_p(VALUE obj);
const struct BitmapArray *rb_bitmaparray_data(VALUE obj);
struct BitmapArray *rb_bitmaparray_data_mut(VALUE obj);
void Init_BitmapArray(void);
void rb_bitmaparray_set2(VALUE self, VALUE other);
