// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
/*
#include <stdbool.h>
#include <ruby.h>
#include "sdl_misc.h"
*/

struct Window {
  struct Renderable renderable;
  VALUE viewport, windowskin, contents, cursor_rect;
  bool visible, active, pause;
#if RGSS == 1
  bool stretch;
#endif
  int x, y, z, width, height;
  int ox, oy;
  int opacity, back_opacity, contents_opacity;
#if RGSS == 3
  VALUE tone;
  bool arrows_visible;
  int padding, padding_bottom;
#endif
#if RGSS > 1
  int openness;
#endif
  int cursor_tick;
  int pause_tick;
};

bool rb_window_data_p(VALUE obj);
const struct Window *rb_window_data(VALUE obj);
struct Window *rb_window_data_mut(VALUE obj);
void Init_Window(void);
void deinitWindowSDL(void);
void initWindowSDL(void);
