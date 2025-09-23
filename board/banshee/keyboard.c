/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "common.h"
#include "ec_commands.h"
#include "keyboard_scan.h"
#include "timer.h"
#include <stddef.h>

__override const struct ec_response_keybd_config *
board_vivaldi_keybd_config(void)
{
	return NULL;
}
