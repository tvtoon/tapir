// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <stdbool.h>

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "rubyfill.h"
#include "sdl_misc.h"
#include "Bitmap.h"
#include "BitmapArray.h"
#include "RGSSError.h"
#include "Table.h"
#include "Tilemap.h"
#include "Viewport.h"
#include "misc.h"
#include "gl_misc.h"

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

static GLuint shader;

VALUE rb_cTilemap;

static struct Tilemap *tmapspa[8] = { 0,0,0,0,0,0,0,0 };

static unsigned short cminindex = 0;
static unsigned short tmapc = 0;
unsigned short maxtmapc = 0;

static const int autotile_lookup_rgss23[4][96] =
{

{
 18,  2, 18,  2, 18,  2, 18,  2, 18,  2, 18,  2, 18,  2, 18,  2,
  16, 16, 16, 16, 10, 10, 10, 10, 18, 18,  2,  2, 18,  2, 18,  2,
  16, 10,  8,  8, 10, 10, 18,  2, 16, 16,  8,  8, 16, 10,  8,  0,
  10,  8,  2,  0, 10,  8,  2,  0, 10,  8,  2,  0, 10,  8,  2,  0
},
{
 17, 17,  3,  3, 17, 17,  3,  3, 17, 17,  3,  3, 17, 17,  3,  3,
 17,  3, 17,  3,  9,  9,  9,  9, 19, 19, 19, 19, 17, 17,  3,  3,
 19,  9,  9,  9, 11, 11, 19, 19, 17,  3, 11,  9, 19, 11, 11,  1,
  9,  9,  1,  1, 11, 11,  3,  3,  9,  9,  1,  1, 11, 11,  3,  3
},
{
 14, 14, 14, 14, 14, 14, 14, 14,  6,  6,  6,  6,  6,  6,  6,  6,
 12, 12, 12, 12, 14, 14,  6,  6, 14,  6, 14,  6, 22, 22, 22, 22,
 12, 22, 12, 12, 14,  6, 22, 22, 20, 20, 12, 20, 20, 22, 20,  4,
  6,  4,  6,  4,  6,  4,  6,  4, 14, 12, 14, 12, 14, 12, 14, 12
},
{
 13, 13, 13, 13,  7,  7,  7,  7, 13, 13, 13, 13,  7,  7,  7,  7,
 13, 13,  7,  7, 13,  7, 13,  7, 15, 15, 15, 15, 21, 21, 21, 21,
 15, 21, 13,  7, 15, 15, 23, 23, 21, 21, 15, 21, 23, 23, 23,  5,
  5,  5,  5,  5,  7,  7,  7,  7, 13, 13, 13, 13, 15, 15, 15, 15
}
};

static const int counter_alternatives[24] =
{
  -1, -1, -1, -1, 12, 15, 14, 13, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, 12, 13, 14, 15
};

static const int autotile_lookup_rgss1[4][48] =
{
{
 26,  4, 26,  4, 26,  4, 26,  4, 26,  4, 26,  4, 26,  4, 26,  4,
 24, 24, 24, 24, 14, 14, 14, 14, 28, 28,  4,  4, 38,  4, 38,  4,
 24, 14, 12, 12, 16, 16, 40,  4, 36, 36, 12, 12, 36, 16, 12,  0
},
{
 27, 27,  5,  5, 27, 27,  5,  5, 27, 27,  5,  5, 27, 27,  5,  5,
 25,  5, 25,  5, 15, 15, 15, 15, 29, 29, 29, 29, 39, 39,  5,  5,
 29, 15, 13, 13, 17, 17, 41, 41, 37,  5, 17, 13, 41, 17, 17,  1
},
{
 32, 32, 32, 32, 32, 32, 32, 32, 10, 10, 10, 10, 10, 10, 10, 10,
 30, 30, 30, 30, 20, 20, 10, 10, 34, 10, 34, 10, 44, 44, 44, 44,
 30, 44, 18, 18, 22, 10, 46, 46, 42, 42, 18, 42, 42, 46, 42,  6
},
{
 33, 33, 33, 33, 11, 11, 11, 11, 33, 33, 33, 33, 11, 11, 11, 11,
 31, 31, 11, 11, 21, 11, 21, 11, 35, 35, 35, 35, 45, 45, 45, 45,
 35, 45, 19, 11, 23, 23, 47, 47, 43, 43, 23, 43, 47, 47, 47,  7
}
};
/*
static const int counter_alternatives_rgss1[48] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
*/
static int autotilehalfx = 4;
static const int *autotile_lookup[4] = { autotile_lookup_rgss23[0], autotile_lookup_rgss23[1], autotile_lookup_rgss23[2], autotile_lookup_rgss23[3] };

static void tilemap_mark(struct Tilemap *ptr) {
//#if RGSS > 1
  rb_gc_mark(ptr->bitmaps);
  rb_gc_mark(ptr->flags);
//#else
  rb_gc_mark(ptr->autotiles);
  rb_gc_mark(ptr->tileset);
  rb_gc_mark(ptr->priorities);
//#endif
  rb_gc_mark(ptr->map_data);
  rb_gc_mark(ptr->flash_data);
  rb_gc_mark(ptr->viewport);
  rb_gc_mark(ptr->bdispose);
}

static const struct Tilemap *rb_tilemap_data(VALUE obj)
{
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))tilemap_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Tilemap",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Tilemap *ret;
  Data_Get_Struct(obj, struct Tilemap, ret);
  return ret;
}

static struct Tilemap *rb_tilemap_data_mut(VALUE obj)
{
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Tilemap");
  return (struct Tilemap *)rb_tilemap_data(obj);
}

void prepareRenderTilemap( const unsigned short index, const unsigned short rindex )
{
 struct Tilemap *ptr = tmapspa[index];
 struct RenderJob job;

 if ( ptr == 0 )
{
#ifdef __DEBUG__
  fprintf( stderr, "Tilemap NULL pointer at index %u!\n", index );
#endif
  rb_raise( rb_eRGSSError, "Tilemap NULL pointer at index %u!\n", index );
  return;
}

 if(!ptr->visible) return;

 job.reg = 1;
 job.t = rindex;
 job.rindex = index;
/*
 if ( ptr->viewport != Qnil )
{
  ptr->jobz = vppw->z;
  job.z = vppw->z;
}
 else
{
  ptr->jobz = 0;
  job.z = 0;
}
*/
 ptr->jobz = 0;
 job.z = 0;
 job.y = 0;
 queueRenderJob( job, ptr->vportid );
// if ( ptr->viewport == Qnil )
 job.z = 200;
// job.aux[0] = 1;
 queueRenderJob( job, ptr->vportid );
}

void prepareRenderTilemapRGSS1( const unsigned short index, const unsigned short rindex )
{
 const struct Table *map_data_ptr = 0, *priorities_ptr =  0;
 struct Tilemap *ptr = tmapspa[index];
 struct RenderJob job;
 int priority = 0, tile_id = 0, xi = 0, xii = 0, xsize = 0, x_end = 0, x_start = 0, yi = 0, yii = 0, ysize = 0, y_end = 0, y_start = 0, z = 0, zi = 0, zsize = 0;

 if ( ptr == 0 )
{
#ifdef __DEBUG__
  fprintf( stderr, "Tilemap NULL pointer at index %u!\n", index );
#endif
  rb_raise( rb_eRGSSError, "Tilemap NULL pointer at index %u!\n", index );
  return;
}

 if ( (!ptr->visible) || ( ptr->map_data == Qnil ) ) return;

 job.reg = 1;
 job.t = rindex;
 job.rindex = index;
// const struct Table *map_data_ptr = rb_table_data(ptr->map_data);
 map_data_ptr = rb_gettables(ptr->mapdataid);
 xsize = map_data_ptr->xsize;
 ysize = map_data_ptr->ysize;
 zsize = map_data_ptr->zsize;
// if(ptr->priorities != Qnil) rb_table_data(ptr->priorities);
 if(ptr->priorities != Qnil) priorities_ptr = rb_gettables(ptr->prioritid);

// TODO respect Viewport width
 x_start = ptr->ox >> 5;
 x_end = (ptr->ox + window_width + 31) >> 5;
 y_start = ptr->oy >> 5;
 y_end = (ptr->oy + window_height + 31) >> 5;

 if ( zi > 3 ) zi = 4;

 for ( ; zi < zsize; ++zi )
{

  for ( yi = y_start; yi <= y_end; ++yi )
{

   for ( xi = x_start; xi <= x_end; ++xi )
{
//    if ( zi > 3 ) continue;

    xii = (xi % xsize + xsize) % xsize;
    yii = (yi % ysize + ysize) % ysize;
    tile_id = map_data_ptr->data[(zi * ysize + yii) * xsize + xii];

    if ( priorities_ptr && 0 <= tile_id && tile_id < priorities_ptr->size )
{
     priority = priorities_ptr->data[tile_id];

     if ( priority > 0 )
{
      z = (1 + priority + yi) * 32 - ptr->oy;
}
     else
{
      priority = 0;
      z = 0;
}

}

    job.z = z;
    job.y = 0;
    job.aux[0] = xii;
    job.aux[1] = yii;
    job.aux[2] = zi;
    queueRenderJob( job, ptr->vportid );
}

}

}

}

static void renderTile( const struct Tilemap *ptr, int tile_id, int x, int y, const int vportox, const int vportoy )
{
 SDL_Surface *tileset_surface = 0;
 VALUE tileset = Qnil;
 const struct Bitmap *tileset_ptr = 0;
 const struct BitmapArray *bitmaparray_ptr = 0;
 const struct Table *flags_ptr = 0;
 bool is_counter = false;
 int autotile_shapes[2] = { 0, 0 };
 int autotile_id = 0, autotile_shape_id = -1, autotile_shape_orig = 0, autotile_x = 0, autotile_y = 0, bitmapid = -1, counter_repeat = 1, dst_x = 0, dst_y = 0, i = 0, id0 = 0, id1 = 0, id2 = 0, id3 = 0, j = 0, src_x = 0, src_xi = 0, src_y = 0, src_yi = 0;

// Tileset B, C, D, E
 if ( 0 < tile_id && tile_id < 1024 )
{
  bitmapid = 5 + ((tile_id>>8)&3);
  src_x = ((tile_id&7)|((tile_id>>4)&8)) * 32;
  src_y = ((tile_id>>3)&15) * 32;
}
// Tileset A5
 else if ( 1536 <= tile_id && tile_id < 1664 )
{
  bitmapid = 4;
  src_x = (tile_id - 1536) % 8 * 32;
  src_y = (tile_id - 1536) / 8 % 16 * 32;
}
// Autotile -- Tileset A1, A2, A3, A4
// Autotile has 47+1 shapes
 else if ( 2048 <= tile_id && tile_id < 8192 )
{
  autotile_id = (tile_id - 2048) / 48;
  autotile_shape_id = (tile_id - 2048) % 48;
//   if(ptr->flags != Qnil) flags_ptr = rb_table_data(ptr->flags);
  if ( ptr->flags != Qnil) flags_ptr = rb_gettables(ptr->flagsid );

  if ( flags_ptr && tile_id < flags_ptr->size ) is_counter = flags_ptr->data[tile_id] & 0x80;
// Tileset A1
  if ( autotile_id < 16 )
{
   bitmapid = 0;
// Swap autotile 1 and 2
   if(autotile_id == 1 || autotile_id == 2) autotile_id ^= 3;
// Complex reordering
   id0 = autotile_id % 2;
   id1 = autotile_id / 2 % 2;
   id2 = autotile_id / 4 % 2;
   id3 = autotile_id / 8;

   if ( id0 == 0 )
{
    autotile_x = id2 * 8 + ptr->autotile_tick % 3 * 2;
}
   else
{
    autotile_x = id2 * 8 + 3 * 2;
}

   autotile_y = (id1 + id3 * 2) * 3;
}
// Tileset A2
  else if ( autotile_id < 48 )
{
   bitmapid = 1;
   autotile_x = (autotile_id - 16) % 8 * 2;
   autotile_y = (autotile_id - 16) / 8 * 3;
}
// Tileset A3
  else if ( autotile_id < 80 )
{
   bitmapid = 2;
// Use a different autotile pattern
   autotile_shape_id += 48;
   autotile_x = (autotile_id - 48) % 8 * 2;
   autotile_y = (autotile_id - 48) / 8 % 4 * 2;
}
// Tileset A4
  else
{
   bitmapid = 3;
   id0 = (autotile_id - 80) % 8;
   id1 = (autotile_id - 80) / 8 % 2;
   id2 = (autotile_id - 80) / 16;
   autotile_x = id0 * 2;
   autotile_y = id2 * 5;

   if (id1)
{
// Use a different autotile pattern
    autotile_shape_id += 48;
    autotile_y += 3;
}

}

  src_x = autotile_x * 32;
  src_y = autotile_y * 32;
}
 else
{
  return;
}

 bitmaparray_ptr = rb_bitmaparray_data(ptr->bitmaps);
 tileset = bitmaparray_ptr->data[bitmapid];

 if ( tileset != Qnil )
{
//  const struct Bitmap *tileset_ptr = rb_bitmap_data(tileset);
  tileset_ptr = rb_bitmap_data(tileset);
  tileset_surface = tileset_ptr->surface;

  if ( tileset_surface != 0 )
{
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   glBlendEquation(GL_FUNC_ADD);
   glUseProgram(shader);
   glUniform1i(glGetUniformLocation(shader, "tex"), 0);
   glUniform2f(glGetUniformLocation(shader, "resolution"), window_width, window_height);
   glActiveTexture(GL_TEXTURE0);
   bitmapBindTexture((struct Bitmap *)tileset_ptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   if ( autotile_shape_id != -1 )
{

    for ( ; i < 4; ++i )
{
     autotile_shape_orig = autotile_lookup[i][autotile_shape_id];

     if ( is_counter && counter_alternatives[autotile_shape_orig] != -1 )
{
      autotile_shapes[0] = counter_alternatives[autotile_shape_orig];
      autotile_shapes[1] = autotile_shape_orig;
      counter_repeat = 2;
}
     else
{
      autotile_shapes[0] = autotile_shape_orig;
      autotile_shapes[1] = 0;
      counter_repeat = 1;
}

     for ( j = 0; j < counter_repeat; ++j )
{
      src_xi = src_x + autotile_shapes[j] % autotilehalfx * 16;
      src_yi = src_y + autotile_shapes[j] / autotilehalfx * 16;
      dst_x = x + (i % 2) * 16;
      dst_y = y + (i / 2) * 16;

      gl_draw_rect( -vportox + dst_x, dst_y + j * 8, -vportoy + dst_x + 16, dst_y + 16 + j * 8, -vportox + src_xi / (double)tileset_surface->w, -vportoy + src_yi / (double)tileset_surface->h, (src_xi + 16) / (double)tileset_surface->w, (src_yi + 16) / (double)tileset_surface->h);
}

}

}
   else
{
    gl_draw_rect( -vportox + x, -vportoy + y, -vportox + x + 32, -vportoy + y + 32, src_x / (double)tileset_surface->w, src_y / (double)tileset_surface->h, (src_x + 32) / (double)tileset_surface->w, (src_y + 32) / (double)tileset_surface->h );
}

}

}

}

static void renderTileRGSS1( const struct Tilemap *ptr, int tile_id, int x, int y, const int vportox, const int vportoy )
{
 SDL_Surface *tileset_surface = 0;
 VALUE tileset = Qnil;
 const struct Bitmap *tileset_ptr = 0;
 const struct BitmapArray *bitmaparray_ptr = 0;
 int autotile_shape_id = -1;
// int autotile_shapes[2] = { 0, 0 };
 int autotile_id = 0, autotile_shape_orig = 0, dst_x = 0, dst_y = 0, i = 0, j = 0, src_x = 0, src_xi = 0, src_y = 0, src_yi = 0;

// if ( tile_id < 48 ) return;
 if ( tile_id > 47 )
{

  if ( tile_id < 384 )
{
   autotile_id = tile_id / 48 - 1;
   autotile_shape_id = tile_id % 48;

   bitmaparray_ptr = rb_bitmaparray_data(ptr->autotiles);
   tileset = bitmaparray_ptr->data[autotile_id];
// TODO: animation
   src_x = 0;
   src_y = 0;
}
  else
{
   tileset = ptr->tileset;
   src_x = (tile_id - 384) % 8 * 32;
   src_y = (tile_id - 384) / 8 * 32;
}

  if ( tileset != Qnil )
{
//  const struct Bitmap *tileset_ptr = rb_bitmap_data(tileset);
   tileset_ptr = rb_bitmap_data(tileset);
   tileset_surface = tileset_ptr->surface;

   if ( tileset_surface != 0 )
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "tex"), 0);
    glUniform2f(glGetUniformLocation(shader, "resolution"), window_width, window_height);
    glActiveTexture(GL_TEXTURE0);
    bitmapBindTexture((struct Bitmap *)tileset_ptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if ( autotile_shape_id != -1 )
{

     for ( ; i < 4; ++i )
{
      autotile_shape_orig = autotile_lookup[i][autotile_shape_id];
/*
     if ( is_counter && counter_alternatives[autotile_shape_orig] != -1 )
{
      autotile_shapes[0] = counter_alternatives[autotile_shape_orig];
      autotile_shapes[1] = autotile_shape_orig;
      counter_repeat = 2;
}
     else
{
      autotile_shapes[0] = autotile_shape_orig;
      autotile_shapes[1] = 0;
      counter_repeat = 1;
}

     for ( j = 0; j < counter_repeat; ++j )
{
*/
      src_xi = src_x + autotile_shape_orig % autotilehalfx * 16;
      src_yi = src_y + autotile_shape_orig / autotilehalfx * 16;
      dst_x = x + (i % 2) * 16;
      dst_y = y + (i / 2) * 16;

      gl_draw_rect( -vportox + dst_x, dst_y + j * 8, -vportoy + dst_x + 16, dst_y + 16 + j * 8, -vportox + src_xi / (double)tileset_surface->w, -vportoy + src_yi / (double)tileset_surface->h, (src_xi + 16) / (double)tileset_surface->w, (src_yi + 16) / (double)tileset_surface->h);
//}

}

}
    else
{
     gl_draw_rect( -vportox + x, -vportoy + y, -vportox + x + 32, -vportoy + y + 32, src_x / (double)tileset_surface->w, src_y / (double)tileset_surface->h, (src_x + 32) / (double)tileset_surface->w, (src_y + 32) / (double)tileset_surface->h );
}

}

}

}

}

void renderTilemap( const unsigned short index, const int vportox, const int vportoy )
{
 struct Tilemap *ptr = tmapspa[index];
 const struct Table *flags_ptr = NULL;
 const struct Table *map_data_ptr = 0;
 const int x_end = (vportox + ptr->ox + window_width + 31) >> 5, x_start = (vportox + ptr->ox) >> 5, y_end = (vportoy + ptr->oy + window_height + 31) >> 5, y_start = (vportoy + ptr->oy) >> 5;
 int tile_id = 0, xi = 0, xii = 0, xsize = 0, yi = 0, yii = 0, ysize = 0, z = 0, zi = 0, zsize = 0;

 if ( ptr->map_data != Qnil )
{
// map_data_ptr = rb_table_data(ptr->map_data);
  map_data_ptr = rb_gettables(ptr->mapdataid);
  xsize = map_data_ptr->xsize;
  ysize = map_data_ptr->ysize;
  zsize = map_data_ptr->zsize;

  if ( zsize > 3 ) zsize = 3;
// if (ptr->flags != Qnil) flags_ptr = rb_table_data(ptr->flags);
  if (ptr->flags != Qnil) flags_ptr = rb_gettables(ptr->flagsid);

  for ( ; zi < zsize; ++zi )
{

   for ( yi = y_start; yi <= y_end; ++yi )
{

    for ( xi = x_start; xi <= x_end; ++xi )
{
/* TODO: shadow
    if ( zi == 3 ) continue;
*/
     xii = (xi % xsize + xsize) % xsize;
     yii = (yi % ysize + ysize) % ysize;
     tile_id = map_data_ptr->data[(zi * ysize + yii) * xsize + xii];

     if ( zi == 2 && flags_ptr && 0 <= tile_id && tile_id < flags_ptr->size )
{
      z = (flags_ptr->data[tile_id] & 0x10) ? 200 : 0;
}

     if ( z != ptr->jobz ) continue;

     renderTile( ptr, tile_id, xi * 32 - ptr->ox, yi * 32 - ptr->oy, vportox, vportoy );
}

}

}

  if ( ( ptr->jobz == 0 ) || ( ptr->jobz == 200 ) ) ptr->jobz ^= 200;
}

}

void renderTilemapRGSS1( const unsigned short index, const int vportox, const int vportoy )
{
 struct Tilemap *ptr = tmapspa[index];
// const struct Table *flags_ptr = NULL;
 const struct Table *map_data_ptr = 0;
// const int x_end = (vportox + ptr->ox + window_width + 31) >> 5, x_start = (vportox + ptr->ox) >> 5, y_end = (vportoy + ptr->oy + window_height + 31) >> 5, y_start = (vportoy + ptr->oy) >> 5;
 int tile_id = 0, xi = 0, xii = 0, xsize = 0, yi = 0, yii = 0, ysize = 0, zi = 0;

 if ( ptr->map_data != Qnil )
{
// map_data_ptr = rb_table_data(ptr->map_data);
  map_data_ptr = rb_gettables(ptr->mapdataid);
  xsize = map_data_ptr->xsize;
  ysize = map_data_ptr->ysize;
/*
  xi = job->aux[0];
  yi = job->aux[1];
  zi = job->aux[2];
*/
  xii = (xi % xsize + xsize) % xsize;
  yii = (yi % ysize + ysize) % ysize;
  tile_id = map_data_ptr->data[(zi * ysize + yii) * xsize + xii];
  renderTileRGSS1(ptr, tile_id, xi * 32 - ptr->ox, yi * 32 - ptr->oy, vportox, vportoy );
}

}

static void tilemap_free(struct Tilemap *ptr)
{
 unsigned short cindex = 0;

 if ( ptr->bdispose == Qfalse )
{
  cindex = NEWdisposeRenderable( ptr->rendid );
  ptr->bdispose = Qtrue;
  tmapspa[cindex] = 0;
  tmapc--;

  if ( cminindex > cindex )
{
   cminindex = cindex;
}

}

 xfree(ptr);
}

static VALUE tilemap_alloc(VALUE klass)
{
 VALUE ret = Qnil;
 struct Tilemap *ptr = 0;

 if ( cminindex == 8 )
{
#ifdef __DEBUG__
  fprintf( stderr, "Reached maximum tilemap count of 8!\n" );
#endif
  rb_raise( rb_eRGSSError, "Reached maximum tilemap count of 8!\n" );
}
 else
{
#ifdef __DEBUG__
  printf( "Allocating tilemap %u!\n", cminindex );
#endif
  ptr = ALLOC(struct Tilemap);
//#if RGSS > 1
  ptr->bitmaps = Qnil;
  ptr->flags = Qnil;
//#else
  ptr->tileset = Qnil;
  ptr->autotiles = Qnil;
  ptr->priorities = Qnil;
//#endif
  ptr->map_data = Qnil;
  ptr->flash_data = Qnil;
  ptr->viewport = Qnil;
  ptr->visible = true;
  ptr->ox = 0;
  ptr->oy = 0;
  ptr->autotile_tick = 0;
  ptr->bdispose = Qfalse;
  ret = Data_Wrap_Struct(klass, tilemap_mark, tilemap_free, ptr);
//#if RGSS > 1
  ptr->bitmaps = rb_bitmaparray_new();
//#else
  ptr->autotiles = rb_bitmaparray_new();
//#endif
  ptr->rendid = NEWregisterRenderable( cminindex, 1 );
  ptr->vportid = 255;
  ptr->flagsid = 4096;
  ptr->flashid = 4096;
  ptr->mapdataid = 4096;
  ptr->prioritid = 4096;
  tmapspa[cminindex] = ptr;

  for ( cminindex++; cminindex < 8; cminindex++ )
{
   if ( tmapspa[cminindex] == 0 ) break;
}

  tmapc++;

  if ( tmapc > maxtmapc ) maxtmapc = tmapc;
}

 return ret;
}

/*
 * call-seq:
 *   Tilemap.new
 *   Tilemap.new(viewport)
 *
 * Creates new tilemap object, possibly with viewport.
 */
static VALUE rb_tilemap_m_initialize(int argc, VALUE *argv, VALUE self)
{
 struct Tilemap *ptr = rb_tilemap_data_mut(self);

 if ( argc == 1 )
{
// rb_viewport_data(argv[0]);
  if ( argv[0] != Qnil )
{
   ptr->vportid = rb_viewport_data(argv[0])->ownid;
   ptr->viewport = argv[0];
}

}
 else
{
  if ( argc != 0 ) rb_raise(rb_eArgError, "wrong number of arguments (%d for 0..1)", argc);
}

 return Qnil;
}

static VALUE rb_tilemap_m_initialize_copy(VALUE self, VALUE orig) {
  struct Tilemap *ptr = rb_tilemap_data_mut(self);
  const struct Tilemap *orig_ptr = rb_tilemap_data(orig);

//#if RGSS > 1
  rb_bitmaparray_set2(ptr->bitmaps, orig_ptr->bitmaps);
  ptr->flags = orig_ptr->flags;
//#else
  ptr->priorities = orig_ptr->priorities;
  ptr->tileset = orig_ptr->tileset;
  rb_bitmaparray_set2(ptr->autotiles, orig_ptr->autotiles);
//#endif
  ptr->map_data = orig_ptr->map_data;
  ptr->flash_data = orig_ptr->flash_data;
  ptr->viewport = orig_ptr->viewport;
  ptr->vportid = orig_ptr->vportid;
  ptr->visible = orig_ptr->visible;
  ptr->ox = orig_ptr->ox;
  ptr->oy = orig_ptr->oy;
  ptr->autotile_tick = orig_ptr->autotile_tick;
  ptr->flagsid = orig_ptr->flagsid;
  ptr->flashid = orig_ptr->flashid;
  ptr->mapdataid = orig_ptr->mapdataid;
  ptr->prioritid = orig_ptr->prioritid;
  return Qnil;
}

static VALUE rb_tilemap_m_dispose(VALUE self)
{
 unsigned short cindex = 0;
 struct Tilemap *ptr = rb_tilemap_data_mut(self);

 if ( ptr->bdispose == Qfalse )
{
  cindex = NEWdisposeRenderable( ptr->rendid );
  ptr->bdispose = Qtrue;
  tmapspa[cindex] = 0;
  tmapc--;

  if ( cminindex > cindex )
{
   cminindex = cindex;
}
#ifdef __DEBUG__
  printf( "Disposing tilemap %u!\n", cindex );
#endif
}

 return Qnil;
}

static VALUE rb_tilemap_m_disposed_p(VALUE self)
{
 const struct Tilemap *ptr = rb_tilemap_data(self);
 return ptr->bdispose;
}

static VALUE rb_tilemap_m_update(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  (void) ptr;
  WARN_UNIMPLEMENTED("Tilemap#update");
  return Qnil;
}

static VALUE rb_tilemap_m_map_data(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->map_data;
}

static VALUE rb_tilemap_m_set_map_data(VALUE self, VALUE newval) {
 struct Tilemap *ptr = rb_tilemap_data_mut(self);
// const struct Table *ptrt = NULL;

 if ( ( newval != ptr->map_data ) && ( newval != Qnil ) )
{
  ptr->mapdataid = rb_table_data(newval)->ownid;
  ptr->map_data = newval;
//  printf( "Tilemap map data %i*%i*%i=%i(%i).\n", ptrt->xsize, ptrt->ysize, ptrt->zsize, ptrt->size, ptrt->dim );
}

 return newval;
}

static VALUE rb_tilemap_m_flash_data(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->flash_data;
}

static VALUE rb_tilemap_m_set_flash_data(VALUE self, VALUE newval) {
 struct Tilemap *ptr = rb_tilemap_data_mut(self);
// const struct Table *ptrt = NULL;

 if ( ( newval != ptr->flash_data ) && ( newval != Qnil ) )
{
  ptr->flashid = rb_table_data(newval)->ownid;
  ptr->flash_data = newval;
//  printf( "Tilemap flash data %i*%i*%i=%i(%i).\n", ptrt->xsize, ptrt->ysize, ptrt->zsize, ptrt->size, ptrt->dim );
}

 return newval;
}

static VALUE rb_tilemap_m_viewport(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->viewport;
}

static VALUE rb_tilemap_m_visible(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->visible ? Qtrue : Qfalse;
}

static VALUE rb_tilemap_m_set_visible(VALUE self, VALUE newval) {
  struct Tilemap *ptr = rb_tilemap_data_mut(self);
  ptr->visible = RTEST(newval);
  return newval;
}

static VALUE rb_tilemap_m_ox(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return INT2NUM(ptr->ox);
}

static VALUE rb_tilemap_m_set_ox(VALUE self, VALUE newval) {
  struct Tilemap *ptr = rb_tilemap_data_mut(self);
  ptr->ox = NUM2INT(newval);
  return newval;
}

static VALUE rb_tilemap_m_oy(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return INT2NUM(ptr->oy);
}

static VALUE rb_tilemap_m_set_oy(VALUE self, VALUE newval) {
  struct Tilemap *ptr = rb_tilemap_data_mut(self);
  ptr->oy = NUM2INT(newval);
  return newval;
}

//#if RGSS > 1
static VALUE rb_tilemap_m_bitmaps(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->bitmaps;
}

static VALUE rb_tilemap_m_flags(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->flags;
}

static VALUE rb_tilemap_m_set_flags(VALUE self, VALUE newval)
{
 struct Tilemap *ptr = rb_tilemap_data_mut(self);
// const struct Table *ptrt = NULL;

 if ( ( newval != ptr->flags ) && ( newval != Qnil ) )
{
  ptr->flagsid = rb_table_data(newval)->ownid;
  ptr->flags = newval;
//  printf( "Tilemap flags %i*%i*%i=%i(%i).\n", ptrt->xsize, ptrt->ysize, ptrt->zsize, ptrt->size, ptrt->dim );
}

 return newval;
}

static VALUE rb_tilemap_m_set_viewport(VALUE self, VALUE newval)
{
 struct Tilemap *ptr = rb_tilemap_data_mut(self);

 if ( ( newval != ptr->viewport ) && ( newval != Qnil ) )
{
// rb_viewport_data(newval);
  ptr->vportid = rb_viewport_data(newval)->ownid;
  ptr->viewport = newval;
}

 return newval;
}

//#else

static VALUE rb_tilemap_m_tileset(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->tileset;
}

static VALUE rb_tilemap_m_set_tileset(VALUE self, VALUE newval)
{
 struct Tilemap *ptr = rb_tilemap_data_mut(self);

 if ( ( newval != ptr->tileset ) && ( newval != Qnil ) )
{
//  if(newval != Qnil) rb_bitmap_data(newval);
  ptr->tileset = newval;
}

 return newval;
}

static VALUE rb_tilemap_m_autotiles(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->autotiles;
}

static VALUE rb_tilemap_m_priorities(VALUE self) {
  const struct Tilemap *ptr = rb_tilemap_data(self);
  return ptr->priorities;
}

static VALUE rb_tilemap_m_set_priorities(VALUE self, VALUE newval) {
 struct Tilemap *ptr = rb_tilemap_data_mut(self);
// const struct Table *ptrt = NULL;

 if ( ( newval != ptr->priorities ) && ( newval != Qnil ) )
{
  ptr->prioritid = rb_table_data(newval)->ownid;
  ptr->priorities = newval;
//  printf( "Tilemap priorities %i*%i*%i=%i(%i).\n", ptrt->xsize, ptrt->ysize, ptrt->zsize, ptrt->size, ptrt->dim );
}

 return newval;
}
//#endif RGSS > 1

/* static END */

int initTilemapSDL()
{

  static const char *fsh_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "\n"
    "void main(void) {\n"
    "    vec4 color = texture2D(windowskin, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));\n"
    "    gl_FragColor = color;\n"
    "    /* premultiplication */\n"
    "    gl_FragColor.rgb *= gl_FragColor.a;\n"
    "}\n";

 shader = compileShaders(fsh_source);

 if (shader == 0) return(1);

 return(0);
}

void Init_Tilemap(void)
{
 rb_cTilemap = rb_define_class("Tilemap", rb_cObject);
 rb_define_alloc_func(rb_cTilemap, tilemap_alloc);
 rb_define_private_method(rb_cTilemap, "initialize", rb_tilemap_m_initialize, -1);
 rb_define_private_method(rb_cTilemap, "initialize_copy", rb_tilemap_m_initialize_copy, 1);
 rb_define_method(rb_cTilemap, "dispose", rb_tilemap_m_dispose, 0);
 rb_define_method(rb_cTilemap, "disposed?", rb_tilemap_m_disposed_p, 0);
 rb_define_method(rb_cTilemap, "update", rb_tilemap_m_update, 0);
 rb_define_method(rb_cTilemap, "map_data", rb_tilemap_m_map_data, 0);
 rb_define_method(rb_cTilemap, "map_data=", rb_tilemap_m_set_map_data, 1);
 rb_define_method(rb_cTilemap, "flash_data", rb_tilemap_m_flash_data, 0);
 rb_define_method(rb_cTilemap, "flash_data=", rb_tilemap_m_set_flash_data, 1);
 rb_define_method(rb_cTilemap, "viewport", rb_tilemap_m_viewport, 0);
 rb_define_method(rb_cTilemap, "visible", rb_tilemap_m_visible, 0);
 rb_define_method(rb_cTilemap, "visible=", rb_tilemap_m_set_visible, 1);
 rb_define_method(rb_cTilemap, "ox", rb_tilemap_m_ox, 0);
 rb_define_method(rb_cTilemap, "ox=", rb_tilemap_m_set_ox, 1);
 rb_define_method(rb_cTilemap, "oy", rb_tilemap_m_oy, 0);
 rb_define_method(rb_cTilemap, "oy=", rb_tilemap_m_set_oy, 1);

 if ( rgssver > 1 )
{
  rb_define_method(rb_cTilemap, "bitmaps", rb_tilemap_m_bitmaps, 0);
  rb_define_method(rb_cTilemap, "viewport=", rb_tilemap_m_set_viewport, 1);

  if ( rgssver == 3 )
{
   rb_define_method(rb_cTilemap, "flags", rb_tilemap_m_flags, 0);
   rb_define_method(rb_cTilemap, "flags=", rb_tilemap_m_set_flags, 1);
}
  else
{
   rb_define_method(rb_cTilemap, "passages", rb_tilemap_m_flags, 0);
   rb_define_method(rb_cTilemap, "passages=", rb_tilemap_m_set_flags, 1);
}

}
 else
{
  rb_define_method(rb_cTilemap, "tileset", rb_tilemap_m_tileset, 0);
  rb_define_method(rb_cTilemap, "tileset=", rb_tilemap_m_set_tileset, 1);
  rb_define_method(rb_cTilemap, "autotiles", rb_tilemap_m_autotiles, 0);
  rb_define_method(rb_cTilemap, "priorities", rb_tilemap_m_priorities, 0);
  rb_define_method(rb_cTilemap, "priorities=", rb_tilemap_m_set_priorities, 1);
  autotilehalfx = 6;
  autotile_lookup[0] = autotile_lookup_rgss1[0];
  autotile_lookup[1] = autotile_lookup_rgss1[1];
  autotile_lookup[2] = autotile_lookup_rgss1[2];
  autotile_lookup[3] = autotile_lookup_rgss1[3];
}

}


void deinitTilemapSDL() {
  if(shader) glDeleteProgram(shader);
}
