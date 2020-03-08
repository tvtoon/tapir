// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

/*
#include <stdlib.h>
#include <stdbool.h>
*/

struct ini_entry {
  char *key, *value;
};

struct ini_section {
  char *name;
  size_t capacity, size;
  struct ini_entry *entries;
};

struct ini {
  size_t capacity, size;
  struct ini_section **sections;
};

bool save_ini(struct ini *data, const char *filename, int codepage);
const char *find_ini_entry(struct ini_section *section, const char *key);
struct ini *load_ini(const char *filename, int codepage);
struct ini *new_ini(void);
struct ini_section *find_ini_section(struct ini *data, const char *name);
struct ini_section *find_ini_section_or_insert( struct ini *data, const char *name);
struct ini_section *new_ini_section(const char *name);
void add_ini_entry( struct ini_section *section, const char *key, const char *value);
void add_ini_section(struct ini *data, struct ini_section *section);
void free_ini(struct ini *data);
void free_ini_section(struct ini_section *section);
void set_ini_entry( struct ini_section *section, const char *key, const char *value);
