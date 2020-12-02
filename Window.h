// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

struct Window
{
 VALUE viewport, windowskin, contents, cursor_rect;
 VALUE bdispose;
 bool visible, active, pause;
 int x, y, z, width, height;
 int ox, oy;
 int opacity, back_opacity, contents_opacity;
//#if RGSS == 3
 VALUE tone;
 bool arrows_visible;
 int padding, padding_bottom;
//#endif
//#if RGSS > 1
 int openness;
//#endif
 int cursor_tick;
 int pause_tick;
 unsigned short rendid;
/* RGSS1 */
 unsigned char task;
 unsigned char stretch;
};
/*
bool rb_window_data_p(VALUE obj);
const struct Window *rb_window_data(VALUE obj);
struct Window *rb_window_data_mut(VALUE obj);
*/
int initWindowSDL(void);
void Init_Window(void);
void deinitWindowSDL(void);

void prepareRenderWindow( const unsigned short index, const unsigned short rindex );
void renderWindow( const unsigned short index, const int vportox, const int vportoy );
void renderWindowRGSS1( const unsigned short index, const int vportox, const int vportoy );

unsigned short maxwindowc;
