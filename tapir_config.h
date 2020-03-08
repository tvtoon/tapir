// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
//#include <libconfig.h>

#define KEYCONFIG_LEN 25

extern int key_config[KEYCONFIG_LEN];

const char *get_rtp_base_config(void);
const char *get_rtp_config(const char *rtp_name);
void deinit_tapir_config(void);
void init_tapir_config(void);
