// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
struct Viewport
{
 VALUE rect, color, tone;
 VALUE bdispose;
 int ox, oy, z;
 unsigned char visible;
 unsigned short ownid;
 unsigned short rendid;
};

const struct Viewport *rb_viewport_data(VALUE obj);
void Init_Viewport(void);
unsigned short maxvportqc;
struct Viewport *rb_getvports( const unsigned short id );
