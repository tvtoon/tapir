// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <stdbool.h>
#include <stdint.h>

#include "rubyfill.h"
#include "Table.h"
#include "misc.h"

static VALUE rb_cTable;
static unsigned int tabc = 0;
unsigned int maxtabc = 0;

/*
 Note: original RGSS doesn't check overflow.
 Note 2: the original checking made no sense, this one does.
*/
static int multiply_size( int xsize, int ysize, int zsize)
{

 if ( ( xsize > 16384 ) || ( ysize > 16384 ) || ( zsize > 4 ) )
{
  rb_raise(rb_eArgError, "Multiplied table size is too large.");
  return(0);
}
 else
{
  return( xsize * ysize * zsize );
}

}

static void table_resize( struct Table *ptr, int new_dim, int new_xsize, int new_ysize, int new_zsize )
{
 int new_size = 0, x = 0, xsize_min = 0, y = 0, ysize_min = 0, z = 0, zsize_min = 0;
 unsigned short *new_data = 0;

 if ( ( new_xsize < 1 ) || ( new_ysize < 1 ) || ( new_zsize < 1 ) )
{
  new_dim = 0;
  new_xsize = 0;
  new_ysize = 0;
  new_zsize = 0;
}
 else
{
  new_size = multiply_size(new_xsize, new_ysize, new_zsize);

  if ( new_size != 0 )
{
   new_data = ALLOC_N(unsigned short, new_size);

   if (ptr->data)
{
    for ( ; z < new_size; z++ ) new_data[z] = 0;

    xsize_min = ptr->xsize < new_xsize ? ptr->xsize : new_xsize;
    ysize_min = ptr->ysize < new_ysize ? ptr->ysize : new_ysize;
    zsize_min = ptr->zsize < new_zsize ? ptr->zsize : new_zsize;

    for ( z = 0; z < zsize_min; ++z )
{

     for ( y = 0; y < ysize_min; ++y )
{

      for ( x = 0; x < xsize_min; ++x )
{
       new_data[(z*new_ysize+y)*new_xsize+x] = ptr->data[(z*ptr->ysize+y)*ptr->xsize+x];
}

}

}

    xfree(ptr->data);
}

}

}

 ptr->dim = new_dim;
 ptr->xsize = new_xsize;
 ptr->ysize = new_ysize;
 ptr->zsize = new_zsize;
 ptr->size = new_size;
 ptr->data = new_data;
}

/*
 * A table is 1, 2, or 3-dimensional array of <tt>unsigned short</tt> values.
 *
 * == Bugs
 *
 * - Table.new doesn't check multiplication overflow. Therefore a code like
 *   <tt>Table.new(65536, 65536)</tt> will generate an ill-formed table.
 */

static void table_mark(struct Table *ptr) {
  (void) ptr;
}

static void table_free(struct Table *ptr) {
  if(ptr->data)
{
xfree(ptr->data);
  tabc--;
}

  xfree(ptr);
}

static VALUE table_alloc(VALUE klass)
{
 struct Table *ptr = ALLOC(struct Table);
 VALUE ret = Qnil;
 ptr->dim = 0;
 ptr->xsize = 0;
 ptr->ysize = 0;
 ptr->zsize = 0;
 ptr->size = 0;
 ptr->data = NULL;
 ret = Data_Wrap_Struct(klass, table_mark, table_free, ptr);
 tabc++;

 if ( tabc > maxtabc ) maxtabc = tabc;

 return ret;
}

/*
 * call-seq:
 *   Table.new(xsize) -> table
 *   Table.new(xsize, ysize) -> table
 *   Table.new(xsize, ysize, zsize) -> table
 *
 * Creates a new table of 1, 2, or 3 dimension.
 *
 * <code>xsize</code>, <code>ysize</code>, and <code>zsize</code> are
 * interpreted as <tt>int</tt> values.
 * Negative values are adjusted to zero.
 */
static VALUE rb_table_m_initialize(int argc, VALUE *argv, VALUE self)
{
 struct Table *ptr = rb_table_data_mut(self);
 int newx = 1, newy = 1, newz = 1;

 if ( ptr->data )
{
  xfree(ptr->data);
  ptr->data = NULL;
}
// if ( 1 <= argc && argc <= 3 )
 if ( ( argc > 0 ) && ( argc < 4 ) )
{
  newx = NUM2INT(argv[0]);

  if ( argc > 1 )
{
   newy = NUM2INT(argv[1]);

   if ( argc > 2 ) newz = NUM2INT(argv[2]);
}
//  table_resize( ptr, argc, 0 < argc ? NUM2INT(argv[0]): 1, 1 < argc ? NUM2INT(argv[1]) : 1, 2 < argc ? NUM2INT(argv[2]) : 1);
  table_resize( ptr, argc, newx, newy, newz );
}
 else
{
// Note: original RGSS has wrong messages.
  rb_raise( rb_eArgError, "wrong number of arguments (%d for 1..3)", argc );
}

 return Qnil;
}

static VALUE rb_table_m_initialize_copy(VALUE self, VALUE orig)
{
 struct Table *ptr = rb_table_data_mut(self);
 const struct Table *orig_ptr = rb_table_data(orig);
 int i = 0;

 ptr->dim = orig_ptr->dim;
 ptr->xsize = orig_ptr->xsize;
 ptr->ysize = orig_ptr->ysize;
 ptr->zsize = orig_ptr->zsize;
 ptr->size = orig_ptr->size;
 ptr->data = ALLOC_N(unsigned short, ptr->size);

 for ( ; i < ptr->size; i++ )
{
  ptr->data[i] = orig_ptr->data[i];
}

 return Qnil;
}

/*
 * call-seq:
 *   resize(xsize) -> table
 *   resize(xsize, ysize) -> table
 *   resize(xsize, ysize, zsize) -> table
 *
 * Resizes the table to a specified size.
 *
 * <code>xsize</code>, <code>ysize</code>, and <code>zsize</code> are
 * interpreted as <tt>int</tt> values.
 * Negative values are adjusted to zero.
 *
 * It preserves existing elements. When expanding, new areas are filled by 0.
 *
 * It can also change the dimension of the table.
 * In this case, <code>y</code> and <code>z</code> defaults to 0
 * if necessary.
 *
 * It returns the table itself.
 */
static VALUE rb_table_m_resize(int argc, VALUE *argv, VALUE self)
{
 struct Table *ptr = rb_table_data_mut(self);
 int new_dim = argc, new_xsize = 1, new_ysize = 1, new_zsize = 1;

 if ( ( argc == 0 ) || ( argc > 3 ) )
{
// Note: original RGSS has wrong messages.
  rb_raise(rb_eArgError, "wrong number of arguments (%d for 1..3)", argc);
}
 else
{
/*
 if ( argc > 1 )
{
*/
  new_xsize = NUM2INT(argv[0]);

  if ( argc > 1 )
{
   new_ysize = NUM2INT(argv[1]);

   if ( argc > 2 )
{
    new_zsize = NUM2INT(argv[2]);
}

}

//}
// if(1 <= argc && argc <= 3)
//  table_resize( ptr, argc, 0 < argc ? NUM2INT(argv[0]) : 1, 1 < argc ? NUM2INT(argv[1]) : 1, 2 < argc ? NUM2INT(argv[2]) : 1);
  table_resize( ptr, new_dim, new_xsize, new_ysize, new_zsize );
  printf( "Resize with %i:%i:%i:%i.\n", new_dim, new_xsize, new_ysize, new_zsize );
}

  return self;
}

/*
 * call-seq:
 *    table.xsize -> integer
 *
 * Returns the first component of the size.
 */
static VALUE rb_table_m_xsize(VALUE self)
{
 const struct Table *ptr = rb_table_data(self);
 return INT2NUM(ptr->xsize);
}

/*
 * call-seq:
 *    table.ysize -> integer
 *
 * Returns the second component of the size.
 * If the dimension is 1, it returns 1.
 */
static VALUE rb_table_m_ysize(VALUE self)
{
 const struct Table *ptr = rb_table_data(self);
 return INT2NUM(ptr->ysize);
}

/*
 * call-seq:
 *    table.zsize -> integer
 *
 * Returns the third component of the size.
 * If the dimension is 1 or 2, it returns 1.
 */
static VALUE rb_table_m_zsize(VALUE self)
{
 const struct Table *ptr = rb_table_data(self);
 return INT2NUM(ptr->zsize);
}

/*
 * call-seq:
 *    table[x] -> integer
 *    table[x, y] -> integer
 *    table[x, y, z] -> integer
 *
 * Returns an element of the table.
 * <code>x</code>, <code>y</code> and <code>z</code> are
 * interpreted as <tt>int</tt> values.
 *
 * The number of the arguments must match the dimension of the table.
 *
 * If the index is out of bounds, <code>nil</code> is returned.
 */
static VALUE rb_table_m_aref(int argc, VALUE *argv, VALUE self)
{
 VALUE retv = Qnil;
 const struct Table *ptr = rb_table_data(self);
 int x = 0, y = 0, z = 0;

 if ( argc == ptr->dim )
{
/*
  int x = 0 < argc ? NUM2INT(argv[0]) : 0;
  int y = 1 < argc ? NUM2INT(argv[1]) : 0;
  int z = 2 < argc ? NUM2INT(argv[2]) : 0;
*/
  x = NUM2INT(argv[0]);

  if ( argc > 1 )
{
   y = NUM2INT(argv[1]);

   if ( argc > 2 ) z = NUM2INT(argv[2]);
}

  if ( 0 <= x && x < ptr->xsize && 0 <= y && y < ptr->ysize && 0 <= z && z < ptr->zsize )
{
   retv = INT2NUM(ptr->data[((z * ptr->ysize) + y) * ptr->xsize + x]);
}

}
 else
{
// Note: original RGSS has wrong messages.
  rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)", argc, ptr->dim);
}

 return retv;
}

/*
 * call-seq:
 *    table[] = newval -> newval
 *    table[x] = newval -> newval
 *    table[x, y] = newval -> newval
 *    table[x, y, z] = newval -> newval
 *
 * Modifies an element of the table.
 * <code>x</code>, <code>y</code> and <code>z</code> are
 * interpreted as <tt>int</tt> values.
 *
 * <code>newval</code> is first interpreted as an <tt>int</tt> value,
 * and then casted to <tt>unsigned short</tt> value by wrapping.
 *
 * Usually, the number of the arguments must be exactly one more than
 * the dimension of the table.
 * However, there is an RGSS bug that this method also accepts one
 * less arguments. In this case, the last index is assumed to be 0.
 *
 * If the index is out of bounds, it does nothing.
 */
static VALUE rb_table_m_aset(int argc, VALUE *argv, VALUE self)
{
 VALUE retv = Qnil;
 struct Table *ptr = rb_table_data_mut(self);
 int x = 0, y = 0, z = 0;
 unsigned short val = 0;

// Note: original RGSS wrongly accepts one less arguments.
// if ( argc == ptr->dim || argc == ptr->dim+1 )
 if ( ( argc > 0 ) && ( argc < ( ptr->dim + 2 ) ) )
{
/*
  int x = 0 < argc-1 ? NUM2INT(argv[0]) : 0;
  int y = 1 < argc-1 ? NUM2INT(argv[1]) : 0;
  int z = 2 < argc-1 ? NUM2INT(argv[2]) : 0;
  unsigned short val = (unsigned short)NUM2INT(argv[argc-1]);
*/
  if ( argc > 1 )
{
   x = NUM2INT(argv[0]);

   if ( argc > 2 )
{
    y = NUM2INT(argv[1]);

    if ( argc > 3 ) z = NUM2INT(argv[2]);
}

}

  val = (unsigned short)NUM2INT(argv[argc-1]);

  if ( 0 <= x && x < ptr->xsize && 0 <= y && y < ptr->ysize && 0 <= z && z < ptr->zsize )
{
   ptr->data[((z * ptr->ysize) + y) * ptr->xsize + x] = val;
   retv = INT2NUM(val);
}

}
 else
{
// Note: original RGSS has wrong messages.
  rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)", argc, ptr->dim+1);
}

 return retv;
}

/*
 * call-seq:
 *   Table._load(str) -> table
 *
 * Loads a table from <code>str</code>. Used in <code>Marshal.load</code>.
 *
 * See Table#_dump for the format.
 */
static VALUE rb_table_s_old_load(VALUE klass, VALUE str)
{
 VALUE ret = table_alloc(rb_cTable);
 const char *s = 0;
 int i = 0;
 long s_len = 0;
 struct Table *ptr = rb_table_data_mut(ret);

 (void) klass;
 StringValue(str);
// Note: original RGSS doesn't check types.
 Check_Type(str, T_STRING);
 s = RSTRING_PTR(str);
// Note: original RGSS doesn't check length.
 s_len = RSTRING_LEN(str);

 if ( s_len < (long)(sizeof(int) * 5 ) )
{
  rb_raise(rb_eArgError, "Corrupted marshal data for Table.");
}
 else
{
  if (!s) s = "";

  ptr->dim = read_int32(s+sizeof(int)*0);
  ptr->xsize = read_int32(s+sizeof(int)*1);
  ptr->ysize = read_int32(s+sizeof(int)*2);
  ptr->zsize = read_int32(s+sizeof(int)*3);
  ptr->size = read_int32(s+sizeof(int)*4);
// Note: original RGSS doesn't check dimension or size.
  if ( ptr->dim < 1 || ptr->dim > 3 || ptr->xsize < 0 || ptr->ysize < 0 || ptr->zsize < 0 )
{
   rb_raise(rb_eArgError, "Corrupted marshal data for Table.");
}
  else
{
// Note: original RGSS doesn't check total size.
   if ( ptr->size != multiply_size(ptr->xsize, ptr->ysize, ptr->zsize) )
{
    rb_raise(rb_eArgError, "Corrupted marshal data for Table.");
}
   else
{
// Note: original RGSS doesn't check length.
    if ( s_len - sizeof(int)*5 != sizeof(unsigned short)*ptr->size )
{
     rb_raise(rb_eArgError, "Corrupted marshal data for Table.");
}
    else
{
     if (ptr->data) xfree(ptr->data);

     ptr->data = ALLOC_N(unsigned short, ptr->size);

     for ( ; i < ptr->size; i++ )
{
      ptr->data[i] = read_int16(s+sizeof(int)*5+sizeof(unsigned short)*i);
}

 printf( "%i*%i*%i=%i(%i) ", ptr->xsize, ptr->ysize, ptr->zsize, ptr->size, ptr->dim );
}

}

}

}

  return ret;
}

/*
 * call-seq:
 *   table._dump(limit) -> string
 *
 * Dumps a table to a string. Used in <code>Marshal.dump</code>.
 *
 * Same as <code>[xsize, ysize, zsize, size, *contents].pack("l<l<l<l<s*<")</code>, where
 *
 * - <code>size = xsize * ysize * zsize</code>
 * - <code>contents[x + xsize * (y + ysize * z)] = table[x, y, z]</code> (if dimension is 3)
 * - <code>contents[x + xsize * y] = table[x, y]</code> (if dimension is 2)
 * - <code>contents[x] = table[x]</code> (if dimension is 1)
 */
static VALUE rb_table_m_old_dump(VALUE self, VALUE limit)
{
 VALUE ret = Qnil;
 const struct Table *ptr = rb_table_data(self);
 int i = 0;
 size_t dumpsize = sizeof(int)*5+sizeof(unsigned short)*(ptr->size);
 char *s = (char *)xmalloc(dumpsize);

 if ( s != 0 )
{
  (void) limit;
  write_int32(s, ptr->dim);
  write_int32(s+4, ptr->xsize);
  write_int32(s+8, ptr->ysize);
  write_int32(s+12, ptr->zsize);
  write_int32(s+16, ptr->size);

  for ( ; i < ptr->size; i++ )
{
   write_int16(s+20+i*2, ptr->data[i]);
}

  ret = rb_str_new(s, dumpsize);
  xfree(s);
}
 else
{
  rb_raise(rb_eArgError, "No more memory for strings, dumping Table.");
}

 return ret;
}

/* static END */

bool rb_table_data_p(VALUE obj) {
  if(TYPE(obj) != T_DATA) return false;
  return RDATA(obj)->dmark == (void(*)(void*))table_mark;
}

const struct Table *rb_table_data(VALUE obj) {
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))table_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Table",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Table *ret;
  Data_Get_Struct(obj, struct Table, ret);
  return ret;
}

struct Table *rb_table_data_mut(VALUE obj) {
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Table");
  return (struct Table *)rb_table_data(obj);
}

void Init_Table() {
  rb_cTable = rb_define_class("Table", rb_cObject);
  rb_define_alloc_func(rb_cTable, table_alloc);
  rb_define_private_method(rb_cTable, "initialize", rb_table_m_initialize, -1);
  rb_define_private_method(rb_cTable, "initialize_copy", rb_table_m_initialize_copy, 1);
  rb_define_method(rb_cTable, "resize", rb_table_m_resize, -1);
  rb_define_method(rb_cTable, "xsize", rb_table_m_xsize, 0);
  rb_define_method(rb_cTable, "ysize", rb_table_m_ysize, 0);
  rb_define_method(rb_cTable, "zsize", rb_table_m_zsize, 0);
  rb_define_method(rb_cTable, "[]", rb_table_m_aref, -1);
  rb_define_method(rb_cTable, "[]=", rb_table_m_aset, -1);
  rb_define_singleton_method(rb_cTable, "_load", rb_table_s_old_load, 1);
  rb_define_method(rb_cTable, "_dump", rb_table_m_old_dump, 1);
}
