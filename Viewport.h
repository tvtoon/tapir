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
//  struct Renderable renderable;
//  struct RenderQueue viewport_queue;
  VALUE rect, color, tone;
 VALUE bdispose;
  bool visible;
  int ox, oy, z;
 unsigned short rendid;
};

bool rb_viewport_data_p(VALUE obj);
const struct Viewport *rb_viewport_data(VALUE obj);
struct Viewport *rb_viewport_data_mut(VALUE obj);
void Init_Viewport(void);

void prepareRenderViewport( const unsigned short index );
void renderViewport( const unsigned short index, const struct RenderViewport *viewport );

unsigned short maxvportqc;
