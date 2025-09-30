/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "common.h"
#include "gpio.h"
#include "hooks.h"
#include "keyboard_8042_sharedlib.h"
#include "keyboard_config.h"
#include "keyboard_customization.h"
#include "keyboard_protocol.h"
#include "keyboard_raw.h"
#include "keyboard_scan.h"
#include "system.h"

enum gpio_signal signal;
static int colinv;

uint16_t scancode_set2[KEYBOARD_COLS_MAX][KEYBOARD_ROWS] = {
	{ 0x0021, 0x007B, 0x0079, 0x0072, 0x007A, 0x0071, 0x0069, 0xe04A },
	{ 0xe071, 0xe070, 0x007D, 0xe01f, 0x006c, 0xe06c, 0xe07d, 0x0077 },
	{ 0x0015, 0x0070, 0x00ff, 0x000D, 0x000E, 0x0016, 0x0067, 0x001c },
	{ 0xe011, 0x0011, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
	{ 0xe05a, 0x0029, 0x0024, 0x000c, 0x0058, 0x0026, 0x0004, 0xe07a },
	{ 0x0022, 0x001a, 0x0006, 0x0005, 0x001b, 0x001e, 0x001d, 0x0076 },
	{ 0x002A, 0x0032, 0x0034, 0x002c, 0x002e, 0x0025, 0x002d, 0x002b },
	{ 0x003a, 0x0031, 0x0033, 0x0035, 0x0036, 0x003d, 0x003c, 0x003b },
	{ 0x0049, 0xe072, 0x005d, 0x0044, 0x0009, 0x0046, 0x0078, 0x004b },
	{ 0x0059, 0x0012, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
	{ 0x0041, 0x007c, 0x0083, 0x000b, 0x0003, 0x003e, 0x0043, 0x0042 },
	{ 0x0013, 0x0064, 0x0075, 0x0001, 0x0051, 0x0061, 0xe06b, 0xe02f },
	{ 0xe014, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
	{ 0x004a, 0xe075, 0x004e, 0x0007, 0x0045, 0x004d, 0x0054, 0x004c },
	{ 0x0052, 0x005a, 0xe03c, 0xe069, 0x0055, 0x0066, 0x005b, 0x0023 },
	{ 0x006a, 0x000a, 0xe074, 0xe054, 0x0000, 0x006b, 0x0073, 0x0074 },
};

uint16_t get_scancode_set2(uint8_t row, uint8_t col)
{
	if (col < KEYBOARD_COLS_MAX && row < KEYBOARD_ROWS)
		return scancode_set2[col][row];
	return 0;
}

void set_scancode_set2(uint8_t row, uint8_t col, uint16_t val)
{
	if (col < KEYBOARD_COLS_MAX && row < KEYBOARD_ROWS)
		scancode_set2[col][row] = val;
}

struct keyboard_type key_typ = {
	.col_esc = KEYBOARD_COL_ESC,
	.row_esc = KEYBOARD_ROW_ESC,
	.col_down = KEYBOARD_COL_DOWN,
	.row_down = KEYBOARD_ROW_DOWN,
	.col_left_shift = KEYBOARD_COL_LEFT_SHIFT,
	.row_left_shift = KEYBOARD_ROW_LEFT_SHIFT,
	.col_refresh = KEYBOARD_COL_REFRESH,
	.row_refresh = KEYBOARD_ROW_REFRESH,
	.col_right_alt = KEYBOARD_COL_RIGHT_ALT,
	.row_right_alt = KEYBOARD_ROW_RIGHT_ALT,
	.col_left_alt = KEYBOARD_COL_LEFT_ALT,
	.row_left_alt = KEYBOARD_ROW_LEFT_ALT,
	.col_key_r = KEYBOARD_COL_KEY_R,
	.row_key_r = KEYBOARD_ROW_KEY_R,
	.col_key_h = KEYBOARD_COL_KEY_H,
	.row_key_h = KEYBOARD_ROW_KEY_H,
};

void board_id_keyboard_col_inverted(int board_id)
{
	if (board_id == 0) {
		/* keyboard_col2_inverted on board id 0 */
		signal = GPIO_EC_KSO_02_INV;
		colinv = 2;
	} else if (board_id == 1) {
		/* keyboard_col4_inverted on board id 1 */
		signal = GPIO_EC_KSO_04_INV;
		colinv = 4;
	} else {
		/* keyboard_col5_inverted on board id 2 and later */
		signal = GPIO_EC_KSO_05_INV;
		colinv = 5;
	}
}

void board_keyboard_drive_col(int col)
{
	/* Drive all lines to high */
	if (col == KEYBOARD_COLUMN_NONE)
		gpio_set_level(signal, 0);

	/* Set KBSOUT to zero to detect key-press */
	else if (col == KEYBOARD_COLUMN_ALL)
		gpio_set_level(signal, 1);

	/* Drive one line for detection */
	else {
		if (col == colinv)
			gpio_set_level(signal, 1);
		else
			gpio_set_level(signal, 0);
	}
}

#ifdef CONFIG_KEYBOARD_DEBUG
static uint8_t keycap_label[KEYBOARD_COLS_MAX][KEYBOARD_ROWS] = {
	{ 'c', KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO,
	  KLLI_UNKNO, KLLI_UNKNO },
	{ KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO,
	  KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO },
	{ 'q', KLLI_UNKNO, KLLI_UNKNO, KLLI_TAB, '`', '1', KLLI_UNKNO, 'a' },
	{ KLLI_R_ALT, KLLI_L_ALT, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO,
	  KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO },
	{ KLLI_UNKNO, KLLI_SPACE, 'e', KLLI_F4, KLLI_SEARC, '3', KLLI_F3,
	  KLLI_UNKNO },
	{ 'x', 'z', KLLI_F2, KLLI_F1, 's', '2', 'w', KLLI_ESC },
	{ 'v', 'b', 'g', 't', '5', '4', 'r', 'f' },
	{ 'm', 'n', 'h', 'y', '6', '7', 'u', 'j' },
	{ '.', KLLI_DOWN, '\\', 'o', KLLI_F10, '9', KLLI_UNKNO, 'l' },
	{ KLLI_R_SHT, KLLI_L_SHT, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO,
	  KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO },
	{ ',', KLLI_UNKNO, KLLI_F7, KLLI_F6, KLLI_F5, '8', 'i', 'k' },
	{ KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO, KLLI_F9, KLLI_UNKNO, KLLI_UNKNO,
	  KLLI_LEFT, KLLI_UNKNO },
	{ KLLI_R_CTR, KLLI_L_CTR, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO,
	  KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO },
	{ '/', KLLI_UP, '-', KLLI_UNKNO, '0', 'p', '[', ';' },
	{ '\'', KLLI_ENTER, KLLI_UNKNO, KLLI_UNKNO, '=', KLLI_B_SPC, ']', 'd' },
	{ KLLI_UNKNO, KLLI_F8, KLLI_RIGHT, KLLI_UNKNO, KLLI_UNKNO, KLLI_UNKNO,
	  KLLI_UNKNO, KLLI_UNKNO },
};

uint8_t get_keycap_label(uint8_t row, uint8_t col)
{
	if (col < KEYBOARD_COLS_MAX && row < KEYBOARD_ROWS)
		return keycap_label[col][row];
	return KLLI_UNKNO;
}

void set_keycap_label(uint8_t row, uint8_t col, uint8_t val)
{
	if (col < KEYBOARD_COLS_MAX && row < KEYBOARD_ROWS)
		keycap_label[col][row] = val;
}
#endif

#ifdef CONFIG_KEYBOARD_CUSTOMIZATION_COMBINATION_KEY
#define FN_PRESSED BIT(0)
#define FN_LOCKED BIT(1)
static uint8_t Fn_key;
static uint32_t fn_key_table_media;
static uint32_t fn_key_table;

int fn_table_media_set(int8_t pressed, uint32_t fn_bit)
{
	if (pressed) {
		fn_key_table_media |= fn_bit;
		return true;
	} else if (!pressed && (fn_key_table_media & fn_bit)) {
		fn_key_table_media &= ~fn_bit;
		return true;
	}

	return false;
}

int fn_table_set(int8_t pressed, uint32_t fn_bit)
{
	if (pressed && (Fn_key & FN_PRESSED)) {
		fn_key_table |= fn_bit;
		return true;
	} else if (!pressed && (fn_key_table & fn_bit)) {
		fn_key_table &= ~fn_bit;
		return true;
	}

	return false;
}

static void fnkey_shutdown(void)
{
	uint8_t current_kb = 0;

	if (Fn_key & FN_LOCKED) {
		current_kb |= 0x80;
	}
	system_set_bbram(SYSTEM_BBRAM_IDX_KBSTATE, current_kb);

	Fn_key &= ~FN_LOCKED;
	Fn_key &= ~FN_PRESSED;
}
DECLARE_HOOK(HOOK_CHIPSET_SUSPEND, fnkey_shutdown, HOOK_PRIO_DEFAULT);

static void fnkey_startup(void)
{
	uint8_t current_kb = 0;

	if (system_get_bbram(SYSTEM_BBRAM_IDX_KBSTATE, &current_kb) ==
	    EC_SUCCESS) {
		if (current_kb & 0x80) {
			Fn_key |= FN_LOCKED;
		}
	}
}
DECLARE_HOOK(HOOK_CHIPSET_STARTUP, fnkey_startup, HOOK_PRIO_DEFAULT);

int hotkey_F1_F6(uint16_t *key_code, uint16_t fn, int8_t pressed)
{
	const uint16_t prss_key = *key_code;

	if (!(Fn_key & FN_LOCKED) && (fn & FN_PRESSED))
		return EC_SUCCESS;
	else if (Fn_key & FN_LOCKED && !(fn & FN_PRESSED) &&
		 !fn_key_table_media)
		return EC_SUCCESS;
	else if (!fn_key_table_media && !pressed)
		return EC_SUCCESS;

	switch (prss_key) {
	case SCANCODE_F1: /* SPEAKER_MUTE */
		if (fn_table_media_set(pressed, KB_FN_F1))
			*key_code = SCANCODE_VOLUME_MUTE;
		break;
	case SCANCODE_F2: /* VOLUME_DOWN */
		if (fn_table_media_set(pressed, KB_FN_F2))
			*key_code = SCANCODE_VOLUME_DOWN;
		break;
	case SCANCODE_F3: /* VOLUME_UP */
		if (fn_table_media_set(pressed, KB_FN_F3))
			*key_code = SCANCODE_VOLUME_UP;
		break;
	case SCANCODE_F4: /* PREVIOUS_TRACK */
		if (fn_table_media_set(pressed, KB_FN_F4))
			*key_code = SCANCODE_PREV_TRACK;
		break;
	case SCANCODE_F5: /* PLAY_PAUSE */
		if (fn_table_media_set(pressed, KB_FN_F5))
			*key_code = 0xe034;
		break;
	case SCANCODE_F6: /* NEXT_TRACK */
		if (fn_table_media_set(pressed, KB_FN_F6))
			*key_code = SCANCODE_NEXT_TRACK;
		break;

	default:
		return EC_SUCCESS;
	}
	return EC_SUCCESS;
}

int functional_hotkey(uint16_t *key_code, int8_t pressed)
{
	const uint16_t prss_key = *key_code;

	switch (prss_key) {
	case SCANCODE_ESC: /* TODO: FUNCTION_LOCK */
		if (fn_table_set(pressed, KB_FN_ESC)) {
			if (pressed) {
				if (Fn_key & FN_LOCKED)
					Fn_key &= ~FN_LOCKED;
				else
					Fn_key |= FN_LOCKED;
			}
			return EC_ERROR_UNIMPLEMENTED;
		}
		break;
	}
	return EC_SUCCESS;
}

enum ec_error_list keyboard_scancode_callback(uint16_t *make_code,
					      int8_t pressed)
{
	const uint16_t pressed_key = *make_code;
	int r = 0;

	if (pressed_key == SCANCODE_FN && pressed) {
		Fn_key |= FN_PRESSED;
		return EC_ERROR_UNIMPLEMENTED;
	} else if (pressed_key == SCANCODE_FN && !pressed) {
		Fn_key &= ~FN_PRESSED;
		return EC_ERROR_UNIMPLEMENTED;
	}

	r = hotkey_F1_F6(make_code, Fn_key, pressed);
	if (r != EC_SUCCESS)
		return r;
	/*
	 * If the function key is not held then
	 * we pass through all events without modifying them
	 * but if last time have press FN still need keep that
	 */
	if (!(Fn_key & FN_PRESSED) && !fn_key_table)
		return EC_SUCCESS;

	r = functional_hotkey(make_code, pressed);
	if (r != EC_SUCCESS)
		return r;

	return EC_SUCCESS;
}
#endif
