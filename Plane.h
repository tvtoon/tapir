// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

//bool rb_plane_data_p(VALUE obj);
int initPlaneSDL(void);
void Init_Plane(void);
void deinitPlaneSDL(void);

void prepareRenderPlane( const unsigned short index, const unsigned short rindex );
void renderPlane( const unsigned short index, const int vportox, const int vportoy );

unsigned short maxplanec;
