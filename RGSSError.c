// Copyright 2017 Masaki Hara. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include "rubyfill.h"
#include "RGSSError.h"

VALUE rb_eRGSSError;

/*
 * Raised from some RGSS methods.
 */
void Init_RGSSError(void) {
  rb_eRGSSError = rb_define_class("RGSSError", rb_eStandardError);
}
