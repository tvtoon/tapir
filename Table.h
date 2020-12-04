// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

struct Table {
// int32_t
 int dim, xsize, ysize, zsize, size;
// int16_t
 unsigned short *data;
};

bool rb_table_data_p(VALUE obj);
const struct Table *rb_table_data(VALUE obj);
struct Table *rb_table_data_mut(VALUE obj);
void Init_Table(void);

unsigned int maxtabc;
