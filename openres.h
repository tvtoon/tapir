// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
/*
#if RGSS > 1
#define NUM_RTP_SLOTS 1
#else
#define NUM_RTP_SLOTS 3
#endif

*/
VALUE rb_load_data(VALUE self, VALUE path);
int configure_rtp_path(struct ini_section *game_section);
size_t loadfile_withrtp( char *pato, const char *filen, const char (*extsa)[5], const size_t filens, const size_t extotal, const size_t extrtp );
unsigned char build_rtppath( /*char ** */char (*patob)[PATH_MAX + 1], const char *newdir, const size_t newdirs );
//void deconfigure_rtp_path(void);
// TODO: this is a dirty hack -- only the first call of `rb_load_data`
//       after this function is considered "script loading", that means
//       it will only look into the archive provided it exists.
void flag_script_loading(void);
