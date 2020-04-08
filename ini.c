// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

#include "ini.h"

// 932 is the default code page for Japanese Windows environment.
static const int ansi_code_page = 932;

struct buffer {
  size_t /*capacity,*/ size;
/*  char *buf;*/
  char buf[65536 + 1];
};

/*
static struct buffer read_all(FILE *f) {
  struct buffer buf;
  buf.capacity = 1024;
  buf.size = 0;
  buf.buf = malloc(buf.capacity);
  while(true) {
    if(buf.size >= buf.capacity) {
      buf.capacity += buf.capacity / 2;
      buf.buf = realloc(buf.buf, buf.capacity);
    }
    size_t numread = fread(buf.buf + buf.size, 1, buf.capacity - buf.size, f);
    // TODO: error handling
    if(numread == 0) break;
    buf.size += numread;
  }
  return buf;
}

struct buffer convert_all(
    struct buffer inbuf, const char *tocode, const char *fromcode) {
  iconv_t cd = iconv_open(tocode, fromcode);
  struct buffer outbuf;
  outbuf.capacity = 1024;
  outbuf.size = 0;
  outbuf.buf = malloc(outbuf.capacity);
  size_t inpos = 0;
  while(inpos < inbuf.size) {
    char *iconv_inbuf = inbuf.buf + inpos;
    size_t iconv_inbytesleft = inbuf.size - inpos;
    char *iconv_outbuf = outbuf.buf + outbuf.size;
    size_t iconv_outbytesleft = outbuf.capacity - outbuf.size;
    size_t irrev = iconv(
        cd, &iconv_inbuf, &iconv_inbytesleft, &iconv_outbuf,
        &iconv_outbytesleft);
    // TODO: error handling
    if(irrev == (size_t)(-1)) break;
    if(iconv_inbuf == inbuf.buf + inpos) {
      outbuf.capacity += outbuf.capacity / 2;
      outbuf.buf = realloc(outbuf.buf, outbuf.capacity);
      continue;
    }
    inpos = iconv_inbuf - inbuf.buf;
    outbuf.size = iconv_outbuf - outbuf.buf;
  }
  return outbuf;
}
*/
struct ini *load_ini(const char *filename, int codepage) {
  FILE *f = fopen(filename, "rb");
  struct buffer buf = { 0, "\0" };

  if(!f) return NULL;

  if(codepage == 0) {
    codepage = ansi_code_page;
  }

/*  struct buffer buf_before_encoding = read_all(f);*/
  buf.size = fread( buf.buf, 1, 65536, f );
  fclose(f);
/*
fprintf( stderr, "BEFORE CONVERSION!\n" );
  struct buffer buf = convert_all(buf_before_encoding, "utf-8", "cp932");
fprintf( stderr, "GO CRASH!\n" );
  free(buf_before_encoding.buf);
*/
  struct ini *data = new_ini();
  struct ini_section *section = new_ini_section("");
  add_ini_section(data, section);

  size_t pos = 0;
  while(pos < buf.size) {
    size_t eolpos = pos;
    while(eolpos < buf.size && buf.buf[eolpos] != '\n' &&
        buf.buf[eolpos] != '\r') ++eolpos;
    while(pos < eolpos && buf.buf[eolpos] == ' ') ++pos;
    if(pos < eolpos && buf.buf[pos] == ';') {
    } else if(pos < eolpos && buf.buf[pos] == '[') {
      ++pos;
      while(pos < eolpos && buf.buf[pos] == ' ') ++pos;
      size_t sectionend = pos;
      while(sectionend < eolpos && buf.buf[sectionend] != ']') ++sectionend;
      char *section_name = malloc(sectionend - pos + 1);
      memcpy(section_name, buf.buf + pos, sectionend - pos);
      section_name[sectionend - pos] = '\0';
      section = new_ini_section(section_name);
      add_ini_section(data, section);
      free(section_name);
    } else if(pos < eolpos) {
      size_t eqpos = pos;
      while(eqpos < eolpos && buf.buf[eqpos] != '=') ++eqpos;
      if(buf.buf[eqpos] == '=') {
        size_t keyend = eqpos;
        while(keyend > pos && buf.buf[keyend-1] == ' ') --keyend;
        char *key = malloc(keyend - pos + 1);
        memcpy(key, buf.buf + pos, keyend - pos);
        key[keyend - pos] = '\0';

        pos = eqpos + 1;
        while(pos < eolpos && buf.buf[pos] == ' ') ++pos;
        size_t valueend = eolpos;
        while(valueend > pos && buf.buf[valueend-1] == ' ') --valueend;
        if(valueend - pos >= 2 &&
            buf.buf[pos] == '\'' && buf.buf[valueend-1] == '\'') {
          ++pos;
          --valueend;
        } else if(valueend - pos >= 2 &&
            buf.buf[pos] == '\"' && buf.buf[valueend-1] == '\"') {
          ++pos;
          --valueend;
        }
        char *value = malloc(valueend - pos + 1);
        memcpy(value, buf.buf + pos, valueend - pos);
        value[valueend - pos] = '\0';

        add_ini_entry(section, key, value);
        free(key);
        free(value);
      }
    }
    pos = eolpos;
    while(pos < buf.size && (buf.buf[pos] == '\n' || buf.buf[pos] == '\r')) {
      ++pos;
    }
  }

  return data;
}

bool save_ini(struct ini *data, const char *filename, int codepage) {
 FILE *f = fopen(filename, "wb");
 char *cur = '\0';
 size_t buflen = 0, i = 0;
 struct buffer buf = { 0, "\0" };

 cur = buf.buf;

 if(!f) return false;

  if(codepage == 0) {
    codepage = ansi_code_page;
  }

  for( i = 0; i < data->size; ++i) {
    struct ini_section *section = data->sections[i];
    if(i > 0 || strcmp(section->name, "")) {
      buflen += 4 + strlen(section->name); // [section]\r\n
    }
    for(size_t j = 0; j < section->size; ++j) {
      struct ini_entry entry = section->entries[j];
      // key=value\r\n
      buflen += 3 + strlen(entry.key) + strlen(entry.value);
    }
  }
/*
  struct buffer buf_before_encoding = {
    buflen, buflen, malloc(buflen)
  };
*/
 buf.size = buflen;

/*  char *cur = buf_before_encoding.buf;*/
  for( i = 0; i < data->size; ++i)
{
    struct ini_section *section = data->sections[i];

    if(i > 0 || strcmp(section->name, ""))
{
      size_t namelen = strlen(section->name);
      *cur++ = '[';
      memcpy(cur, section->name, namelen);
      cur += namelen;
      *cur++ = ']';
      *cur++ = '\r';
      *cur++ = '\n';
}

    for(size_t j = 0; j < section->size; ++j) {
      struct ini_entry entry = section->entries[j];
      size_t keylen = strlen(entry.key);
      size_t valuelen = strlen(entry.value);
      memcpy(cur, entry.key, keylen);
      cur += keylen;
      *cur++ = '=';
      memcpy(cur, entry.value, valuelen);
      cur += valuelen;
      *cur++ = '\r';
      *cur++ = '\n';
    }
  }
/*
  struct buffer buf = convert_all(buf_before_encoding, "cp932", "utf-8");
  free(buf_before_encoding.buf);
*/
  i = fwrite(buf.buf, 1, buf.size, f);
  fclose(f);
/*  free(buf.buf);*/
  return i == buf.size;
}

struct ini_section *new_ini_section(const char *name) {
  struct ini_section *section = malloc(sizeof(*section));
  section->name = malloc(strlen(name) + 1);
  strcpy(section->name, name);
  section->capacity = 10;
  section->size = 0;
  section->entries = malloc(sizeof(*section->entries) * section->capacity);
  return section;
}

void free_ini_section(struct ini_section *section) {
  for(size_t i = 0; i < section->size; ++i) {
    free(section->entries[i].key);
    free(section->entries[i].value);
  }
  free(section->entries);
  free(section->name);
  free(section);
}

void add_ini_entry(
    struct ini_section *section, const char *key, const char *value) {
  if(section->size >= section->capacity) {
    section->capacity += section->capacity / 2;
    section->entries = realloc(section->entries,
        sizeof(*section->entries) * section->capacity);
  }
  char *new_key = malloc(strlen(key) + 1);
  strcpy(new_key, key);
  char *new_value = malloc(strlen(value) + 1);
  strcpy(new_value, value);
  section->entries[section->size].key = new_key;
  section->entries[section->size].value = new_value;
  section->size++;
}

void set_ini_entry(
    struct ini_section *section, const char *key, const char *value) {
  for(size_t i = 0; i < section->size; ++i) {
    if(!strcmp(section->entries[i].key, key)) {
      free(section->entries[i].value);
      section->entries[i].value = strdup(value);
      return;
    }
  }
  add_ini_entry(section, key, value);
}

const char *find_ini_entry(struct ini_section *section, const char *key) {
  for(size_t i = 0; i < section->size; ++i) {
    if(!strcmp(section->entries[i].key, key)) {
      return section->entries[i].value;
    }
  }
  return NULL;
}

struct ini *new_ini(void) {
  struct ini *data = malloc(sizeof(*data));
  data->capacity = 10;
  data->size = 0;
  data->sections = malloc(sizeof(*data->sections) * data->capacity);
  return data;
}

void free_ini(struct ini *data) {
  for(size_t i = 0; i < data->size; ++i) {
    free_ini_section(data->sections[i]);
  }
  free(data->sections);
  free(data);
}

void add_ini_section(struct ini *data, struct ini_section *section) {
  if(data->size >= data->capacity) {
    data->capacity += data->capacity / 2;
    data->sections = realloc(data->sections,
        sizeof(*data->sections) * data->capacity);
  }
  data->sections[data->size++] = section;
}

struct ini_section *find_ini_section(struct ini *data, const char *name) {
  for(size_t i = 0; i < data->size; ++i) {
    if(!strcmp(data->sections[i]->name, name)) {
      return data->sections[i];
    }
  }
  return NULL;
}

struct ini_section *find_ini_section_or_insert(
    struct ini *data, const char *name) {
  struct ini_section *section = find_ini_section(data, name);
  if(section) return section;
  section = new_ini_section(name);
  add_ini_section(data, section);
  return section;
}


void unix_inifix_forGJ( struct ini *data )
{
 int testi = 0;
 size_t gamei = (size_t)-1, keyns = 0, scripti = (size_t)-1, ui = 0, uj = 0, uk = 0;

 for ( ; ui < data->size; ui++ )
{
#ifdef __DEBUG__
  printf( "Section before: \"%s\". ", data->sections[ui]->name );
#endif
  data->sections[ui]->name[0] = toupper( data->sections[ui]->name[0] );
  keyns = strlen( data->sections[ui]->name );

  for ( uk = 1; uk < keyns; uk++ )
{
   data->sections[ui]->name[uk] = tolower( data->sections[ui]->name[uk] );
}

  testi = strncmp( data->sections[ui]->name, "Game", keyns );

  if ( testi == 0 )
{
   gamei = ui;
}

#ifdef __DEBUG__
  printf( "Section after: \"%s\".\n", data->sections[ui]->name );
#endif

  for ( uj = 0; uj < data->sections[ui]->size; uj++ )
{
#ifdef __DEBUG__
   printf( "Key before: \"%s\". ", data->sections[ui]->entries[uj].key );
#endif
   keyns = strlen( data->sections[ui]->entries[uj].key );
   data->sections[ui]->entries[uj].key[0] = toupper( data->sections[ui]->entries[uj].key[0] );
/* RTP, everything else lower case... */
   if ( data->sections[ui]->entries[uj].key[0] == 'R' )
{

    for ( uk = 1; uk < keyns; uk++ )
{
     data->sections[ui]->entries[uj].key[uk] = toupper( data->sections[ui]->entries[uj].key[uk] );
}

}
   else
{

    for ( uk = 1; uk < keyns; uk++ )
{
     data->sections[ui]->entries[uj].key[uk] = tolower( data->sections[ui]->entries[uj].key[uk] );
}

}

#ifdef __DEBUG__
   printf( "Key after: \"%s\".\n", data->sections[ui]->entries[uj].key );
#endif
   testi = strncmp( data->sections[ui]->entries[uj].key, "Scripts", keyns );

   if ( testi == 0 )
{
    scripti = uj;
}

}

}

 if ( ( gamei != (size_t)-1 ) && ( scripti != (size_t)-1 ) )
{
/* PATH FIX! */
  keyns = strlen(data->sections[gamei]->entries[scripti].value);

  for ( ui = 0; ui < keyns; ui++ )
{
   if ( data->sections[gamei]->entries[scripti].value[ui] == '\\' ) data->sections[gamei]->entries[scripti].value[ui] = '/';
}

}
#ifdef __DEBUG__
 else
{
  printf( "Invalid shit, fail before it is too late!\n" );
}
#endif

}
