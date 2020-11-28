// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
//#include <stdint.h>
#ifdef __DEBUG__
#define WARN_UNIMPLEMENTED(name) fprintf(stderr, "unimplemented: %s\n", name);
#else
#define WARN_UNIMPLEMENTED(name) 
#endif

double clamp_double(double val, double minval, double maxval);
double read_double(const char *ptr);
int16_t read_int16(const char *ptr);
int32_t clamp_int32(int32_t val, int32_t minval, int32_t maxval);
int32_t read_int32(const char *ptr);
void write_double(char *ptr, double val);
void write_int16(char *ptr, int16_t val);
void write_int32(char *ptr, int32_t val);

unsigned char rgssver;
