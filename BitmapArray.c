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
#include <SDL_opengl.h>

#include "rubyfill.h"
#include "sdl_misc.h"
#include "Bitmap.h"
#include "BitmapArray.h"
#include "Tilemap.h"
#include "misc.h"
/*
#if RGSS > 1
#define BITMAP_ARRAY_LENGTH 9
#define BITMAP_ARRAY_CLASS "Tilemap::BitmapArray"
#else
#define BITMAP_ARRAY_LENGTH 7
#define BITMAP_ARRAY_CLASS "TilemapAutotiles"
#endif
*/
static const char bitmapastrv[4][22] = { "TilemapAutotiles", "TilemapAutotiles", "Tilemap::BitmapArray", "Tilemap::BitmapArray" };
static VALUE rb_cBitmapArray;
static unsigned char bitmapalen = 9;

/*
 * Document-class:  Tilemap::BitmapArray
 *
 * Special array class used in Tilemap (RGSS2 and RGSS3 only; for RGSS1, see TilemapAutotiles)
 *
 * The length of the array is always 9 for RGSS2 and RGSS3.
 */
/*
 * Document-class:  TilemapAutotiles
 *
 * Special array class used in Tilemap (RGSS1 only; for RGSS2 and RGSS3, see Tilemap::BitmapArray)
 *
 * The length of the array is always 7 for RGSS1.
 */

static void bitmaparray_mark(struct BitmapArray *ptr)
{
 int i = 0;

 for( ; i < bitmapalen; ++i)
{
  rb_gc_mark(ptr->data[i]);
}

}

static VALUE bitmaparray_alloc(VALUE klass)
{
 struct BitmapArray *ptr = ALLOC(struct BitmapArray);

 for(int i = 0; i < bitmapalen; ++i)
{
  ptr->data[i] = Qnil;
}

 VALUE ret = Data_Wrap_Struct(klass, bitmaparray_mark, -1, ptr);
 return ret;
}

/* call-seq:
 *   bitmaparray[index]
 *
 * Returns the <code>index</code>-th element of the BitmapArray, or <code>nil</code> when the index is out of bounds.
 */
static VALUE rb_bitmaparray_m_aref(VALUE self, VALUE index)
{
 const struct BitmapArray *ptr = rb_bitmaparray_data(self);
 int iindex = NUM2INT(index);

 if(iindex < 0 || bitmapalen <= iindex) return Qnil;

 return ptr->data[iindex];
}

/* call-seq:
 *   bitmaparray[index] = newval -> newval
 *
 * Modifies the <code>index</code>-th element of the BitmapArray. Does nothing when the index is out of bounds.
 *
 * An element must be an instance of Bitmap or +nil+.
 */
static VALUE rb_bitmaparray_m_aset(VALUE self, VALUE index, VALUE newval)
{
 struct BitmapArray *ptr = rb_bitmaparray_data_mut(self);
 int iindex = NUM2INT(index);

 if (iindex < 0 || bitmapalen <= iindex) return Qnil;

 if (newval != Qnil)
{
  rb_bitmap_data(newval);
  ptr->data[iindex] = newval;
}

 return newval;
}

/* static END */

VALUE rb_bitmaparray_new()
{
 VALUE ret = bitmaparray_alloc(rb_cBitmapArray);
 return ret;
}

bool rb_bitmaparray_data_p(VALUE obj)
{
 if(TYPE(obj) != T_DATA) return false;

 return RDATA(obj)->dmark == (void(*)(void*))bitmaparray_mark;
}

const struct BitmapArray *rb_bitmaparray_data(VALUE obj)
{
 Check_Type(obj, T_DATA);
// Note: original RGSS doesn't check types.
 if(RDATA(obj)->dmark != (void(*)(void*))bitmaparray_mark)
{
  rb_raise(rb_eTypeError, "can't convert %s into %s", rb_class2name(rb_obj_class(obj)), bitmapastrv[rgssver] );
}

 struct BitmapArray *ret;
 Data_Get_Struct(obj, struct BitmapArray, ret);
 return ret;
}

struct BitmapArray *rb_bitmaparray_data_mut(VALUE obj)
{
// Note: original RGSS doesn't check frozen.
 if(OBJ_FROZEN(obj)) rb_error_frozen( bitmapastrv[rgssver] );

 return (struct BitmapArray *)rb_bitmaparray_data(obj);
}

void Init_BitmapArray(void)
{
/*
#if 0
  // To fake rdoc
  rb_cTilemap = rb_define_class("Tilemap", rb_cObject);
  rb_cTilemapAutotiles = rb_define_class("TilemapAutotiles", rb_cObject);
  rb_define_method(rb_cTilemapAutotiles, "[]", rb_bitmaparray_m_aref, 1);
  rb_define_method(rb_cTilemapAutotiles, "[]=", rb_bitmaparray_m_aset, 2);
#endif
*/

 if ( rgssver > 1 )
{
  rb_cBitmapArray = rb_define_class_under( rb_cTilemap, "BitmapArray", rb_cObject);
}
 else
{
  rb_cBitmapArray = rb_define_class("TilemapAutotiles", rb_cObject);
  bitmapalen = 7;
}

 rb_define_method(rb_cBitmapArray, "[]", rb_bitmaparray_m_aref, 1);
 rb_define_method(rb_cBitmapArray, "[]=", rb_bitmaparray_m_aset, 2);
}

void rb_bitmaparray_set2(VALUE self, VALUE other)
{
 const struct BitmapArray *other_ptr = rb_bitmaparray_data(other);
 struct BitmapArray *ptr = rb_bitmaparray_data_mut(self);
 int i = 0;

 for ( ; i < bitmapalen; i++ )
{
  ptr->data[i] = other_ptr->data[i];
}

}
