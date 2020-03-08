// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "misc.h"

int32_t clamp_int32(int32_t val, int32_t minval, int32_t maxval) {
  if(val <= minval) return minval;
  if(val >= maxval) return maxval;
  return val;
}

// Note: original RGSS seems to evaluate to_f twice or more.
double clamp_double(double val, double minval, double maxval) {
  if(val < minval) return minval;
  if(val > maxval) return maxval;
  return val;
}

double read_double(const char *ptr) {
  uint64_t u =
    ((uint64_t)(unsigned char)ptr[0])|
    ((uint64_t)(unsigned char)ptr[1]<<8)|
    ((uint64_t)(unsigned char)ptr[2]<<16)|
    ((uint64_t)(unsigned char)ptr[3]<<24)|
    ((uint64_t)(unsigned char)ptr[4]<<32)|
    ((uint64_t)(unsigned char)ptr[5]<<40)|
    ((uint64_t)(unsigned char)ptr[6]<<48)|
    ((uint64_t)(unsigned char)ptr[7]<<56);
  double val;
  memcpy(&val, &u, sizeof(val));
  return val;
}

void write_double(char *ptr, double val) {
  uint64_t u;
  memcpy(&u, &val, sizeof(val));
  ptr[0] = u;
  ptr[1] = u>>8;
  ptr[2] = u>>16;
  ptr[3] = u>>24;
  ptr[4] = u>>32;
  ptr[5] = u>>40;
  ptr[6] = u>>48;
  ptr[7] = u>>56;
}

int32_t read_int32(const char *ptr) {
  return
    ((uint32_t)(unsigned char)ptr[0])|
    ((uint32_t)(unsigned char)ptr[1]<<8)|
    ((uint32_t)(unsigned char)ptr[2]<<16)|
    ((uint32_t)(unsigned char)ptr[3]<<24);
}

void write_int32(char *ptr, int32_t val) {
  ptr[0] = ((uint32_t)val);
  ptr[1] = ((uint32_t)val)>>8;
  ptr[2] = ((uint32_t)val)>>16;
  ptr[3] = ((uint32_t)val)>>24;
}

int16_t read_int16(const char *ptr) {
  return
    ((uint16_t)(unsigned char)ptr[0])|
    ((uint16_t)(unsigned char)ptr[1]<<8);
}

void write_int16(char *ptr, int16_t val) {
  ptr[0] = ((uint16_t)val);
  ptr[1] = ((uint16_t)val)>>8;
}


void tryChdir(const char *path) {
  if(chdir(path)) {
    fprintf(stderr, "cannot chdir: %s\n", strerror(errno));
  }
}
