// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

/* BitmapArray too */
extern VALUE rb_cTilemap;

struct Tilemap
{
//#if RGSS > 1
 VALUE bitmaps, flags;
//#else
 VALUE tileset, autotiles, priorities;
//#endif
 VALUE map_data, flash_data, viewport;
 VALUE bdispose;
 bool visible;
 int ox, oy;
 int autotile_tick;
 unsigned short rendid;
 int jobz;
 unsigned short vportid;
 unsigned short flagsid;
 unsigned short flashid;
 unsigned short mapdataid;
 unsigned short prioritid;
 unsigned short bitmapid;
};

/*
bool rb_tilemap_data_p(VALUE obj);
const struct Tilemap *rb_tilemap_data(VALUE obj);
struct Tilemap *rb_tilemap_data_mut(VALUE obj);
*/
int initTilemapSDL(void);
void Init_Tilemap(void);
void deinitTilemapSDL(void);
//void renderTilemaps(int z_min, int z_max);

void prepareRenderTilemap( const unsigned short index, const unsigned short rindex );
void renderTilemap( const unsigned short index, const int vportox, const int vportoy );

unsigned short maxtmapc;
