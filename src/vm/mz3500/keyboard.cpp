/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../ls244.h"
#include "../../fifo.h"

#define PHASE_RESET		-1
#define PHASE_IDLE		0
#define PHASE_SEND_START_H	1
#define PHASE_SEND_START_L	2
#define PHASE_SEND_BIT_7_H	3
#define PHASE_SEND_BIT_7_L	4
#define PHASE_SEND_BIT_6_H	5
#define PHASE_SEND_BIT_6_L	6
#define PHASE_SEND_BIT_5_H	7
#define PHASE_SEND_BIT_5_L	8
#define PHASE_SEND_BIT_4_H	9
#define PHASE_SEND_BIT_4_L	10
#define PHASE_SEND_BIT_3_H	11
#define PHASE_SEND_BIT_3_L	12
#define PHASE_SEND_BIT_2_H	13
#define PHASE_SEND_BIT_2_L	14
#define PHASE_SEND_BIT_1_H	15
#define PHASE_SEND_BIT_1_L	16
#define PHASE_SEND_BIT_0_H	17
#define PHASE_SEND_BIT_0_L	18
#define PHASE_SEND_COMMAND_H	19
#define PHASE_SEND_COMMAND_L	20
#define PHASE_SEND_PARITY_H	21
#define PHASE_SEND_PARITY_L	22
#define PHASE_SEND_FINISH	23
#define PHASE_SEND_WAIT_ACK	24
#define PHASE_RECV_START	30
#define PHASE_RECV_BIT_2	31
#define PHASE_RECV_BIT_1	32
#define PHASE_RECV_BIT_0	33
#define PHASE_RECV_PARITY	34
#define PHASE_RECV_ACK_H	35
#define PHASE_RECV_ACK_L	36

#define EVENT_DRIVE		0
#define EVENT_DATA		1
#define EVENT_ACK		2

// http://www.8bity.cz/2013/adapter-pro-pripojeni-ps2-klavesnice-k-sharp-mz-3500/

// CMD		Escape or Kanji
// CL		Back Space
// 00		Alt + 0 or Num0
// RUN		Alt + F1
// EDIT		End
// DEB		F11
// CONT		F12
// PRO		Page Up
// OP		Page Down

static const uint16_t key_table[256] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x00f, 0x1f4, 0x000, 0x000, 0x000, 0x00d, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0fe, 0x000, 0x0fe, 0x000, 0x000, 0x000, 0x000,
	0x020, 0x000, 0x000, 0x1f7, 0x00b, 0x01f, 0x01c, 0x01e, 0x01d, 0x000, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x030, 0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x041, 0x042, 0x043, 0x044, 0x045, 0x046, 0x047, 0x048, 0x049, 0x04a, 0x04b, 0x04c, 0x04d, 0x04e, 0x04f,
	0x050, 0x051, 0x052, 0x053, 0x054, 0x055, 0x056, 0x057, 0x058, 0x059, 0x05a, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x030, 0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039, 0x02a, 0x02b, 0x02c, 0x02d, 0x0fd, 0x02f,
	0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008, 0x009, 0x00a, 0x1f5, 0x1fb, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x03a, 0x03b, 0x02c, 0x02d, 0x02e, 0x02f,
	0x040, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x05b, 0x05c, 0x05d, 0x05e, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

static const uint16_t key_shift_table[256] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x00f, 0x1f4, 0x000, 0x000, 0x000, 0x00d, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0fe, 0x000, 0x0fe, 0x000, 0x000, 0x000, 0x000,
	0x020, 0x000, 0x000, 0x000, 0x00b, 0x01f, 0x01c, 0x01e, 0x01d, 0x000, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x000, 0x021, 0x022, 0x1f7, 0x024, 0x025, 0x026, 0x027, 0x028, 0x029, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x061, 0x062, 0x063, 0x064, 0x065, 0x066, 0x067, 0x068, 0x069, 0x06a, 0x06b, 0x06c, 0x06d, 0x06e, 0x06f,
	0x070, 0x071, 0x072, 0x073, 0x074, 0x075, 0x076, 0x077, 0x078, 0x079, 0x07a, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x030, 0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039, 0x02a, 0x02b, 0x02c, 0x02d, 0x0fd, 0x02f,
	0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008, 0x009, 0x00a, 0x1f5, 0x1fb, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x02a, 0x02b, 0x03c, 0x03d, 0x03e, 0x03f,
	0x060, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x07b, 0x07c, 0x07d, 0x07e, 0x000,
	0x000, 0x000, 0x05f, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

static const uint16_t key_ctrl_table[256] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x00f, 0x1f4, 0x000, 0x000, 0x000, 0x00d, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x1fc, 0x000, 0x1fc, 0x000, 0x000, 0x000, 0x000,
	0x020, 0x000, 0x000, 0x1f7, 0x01b, 0x01f, 0x01c, 0x01e, 0x01d, 0x000, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x081, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087, 0x088, 0x089, 0x08a, 0x08b, 0x08c, 0x08d, 0x08e, 0x08f,
	0x090, 0x091, 0x092, 0x093, 0x094, 0x095, 0x096, 0x097, 0x098, 0x099, 0x09a, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198, 0x199, 0x02a, 0x02b, 0x02c, 0x02d, 0x0fd, 0x02f,
	0x011, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017, 0x018, 0x019, 0x01a, 0x1f6, 0x1f3, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x080, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x09b, 0x09c, 0x09d, 0x09e, 0x000,
	0x000, 0x000, 0x09f, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

static const uint16_t key_ctrl_shift_table[256] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x00f, 0x1f4, 0x000, 0x000, 0x000, 0x00d, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x1fc, 0x000, 0x1fc, 0x000, 0x000, 0x000, 0x000,
	0x020, 0x000, 0x000, 0x1f7, 0x01b, 0x01f, 0x01c, 0x01e, 0x01d, 0x000, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x0e1, 0x0e2, 0x0e3, 0x0e4, 0x0e5, 0x0e6, 0x0e7, 0x0e8, 0x0e9, 0x0ea, 0x0eb, 0x0ec, 0x0ed, 0x0ee, 0x0ef,
	0x0f0, 0x0f1, 0x0f2, 0x0f3, 0x0f4, 0x0f5, 0x0f6, 0x0f7, 0x0f8, 0x0f9, 0x0fa, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x02a, 0x02b, 0x02c, 0x02d, 0x0fd, 0x02f,
	0x011, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017, 0x018, 0x019, 0x01a, 0x1f6, 0x1f3, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x0e0, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0fb, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

static const uint16_t key_kana_table[256] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x00f, 0x1f4, 0x000, 0x000, 0x000, 0x00d, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0fe, 0x000, 0x0fe, 0x000, 0x000, 0x000, 0x000,
	0x020, 0x000, 0x000, 0x1f7, 0x00b, 0x01f, 0x01c, 0x01e, 0x01d, 0x000, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x0dc, 0x0c7, 0x0cc, 0x0b1, 0x0b3, 0x0b4, 0x0b5, 0x0d4, 0x0d5, 0x0d6, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x0c1, 0x0ba, 0x0bf, 0x0bc, 0x0b2, 0x0ca, 0x0b7, 0x0b8, 0x0c6, 0x0cf, 0x0c9, 0x0d8, 0x0d3, 0x0d0, 0x0d7,
	0x0be, 0x0c0, 0x0bd, 0x0c4, 0x0b6, 0x0c5, 0x0cb, 0x0c3, 0x0bb, 0x0dd, 0x0c2, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x030, 0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039, 0x02a, 0x02b, 0x02c, 0x02d, 0x0fd, 0x02f,
	0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008, 0x009, 0x00a, 0x1f5, 0x1fb, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0b9, 0x0da, 0x0c8, 0x0ce, 0x0d9, 0x0d2,
	0x0de, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0df, 0x0b0, 0x0d1, 0x0cd, 0x000,
	0x000, 0x000, 0x0db, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

static const uint16_t key_kana_shift_table[256] = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x00f, 0x1f4, 0x000, 0x000, 0x000, 0x00d, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0fe, 0x000, 0x0fe, 0x000, 0x000, 0x000, 0x000,
	0x020, 0x000, 0x000, 0x1f7, 0x00b, 0x01f, 0x01c, 0x01e, 0x01d, 0x000, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x0a6, 0x000, 0x000, 0x0a7, 0x0a9, 0x0aa, 0x0ab, 0x0ac, 0x0ad, 0x0ae, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x0a8, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x1f9, 0x1f8, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0af, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x030, 0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039, 0x02a, 0x02b, 0x02c, 0x02d, 0x0fd, 0x02f,
	0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x008, 0x009, 0x00a, 0x1f5, 0x1fb, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0a4, 0x000, 0x0a1, 0x0a5,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x0a2, 0x000, 0x0a3, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	key_buf = new FIFO(64);
	
	caps = kana = false;
	pro_mode = true;
	
	register_frame_event(this);
}

void KEYBOARD::release()
{
	key_buf->release();
	delete key_buf;
}

void KEYBOARD::reset()
{
	key_buf->clear();
	emu->out_message(pro_mode ? _T("PRO mode") : _T("OP mode"));
	key_buf->write(pro_mode ? 0x1f2 : 0x1f0);
	
	phase = PHASE_RESET;
	drive();
	
	stc_clock = get_current_clock();
	stc = dc = false;
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_KEYBOARD_ACKC) {
		if(data & mask) {
			if(phase == PHASE_SEND_WAIT_ACK) {
				key_buf->read();
			}
		}
	} else if(id == SIG_KEYBOARD_STC) {
		bool old = stc;
		stc = ((data & mask) != 0);
		if(old && !stc) {
			stc_clock = get_current_clock();
		} else if(!old && stc) {
			switch(phase) {
			case PHASE_RESET:
				phase = PHASE_IDLE;
				drive();
				break;
			case PHASE_IDLE:
				if(get_passed_usec(stc_clock) < 7.5) {
					phase = PHASE_IDLE;
				} else {
					phase = PHASE_RECV_START;
				}
				drive();
				break;
			case PHASE_RECV_BIT_2:
			case PHASE_RECV_BIT_1:
			case PHASE_RECV_BIT_0:
			case PHASE_RECV_PARITY:
				if(get_passed_usec(stc_clock) < 17.5) {
					phase = PHASE_IDLE;
				}
				drive();
				break;
			}
		}
	} else if(id == SIG_KEYBOARD_DC) {
		dc = ((data & mask) != 0);
	}
}

void KEYBOARD::event_callback(int event_id, int err)
{
	if(event_id == EVENT_DRIVE) {
		drive();
	} else if(event_id == EVENT_DATA) {
		set_dk((send_data & 0x200) != 0);
		send_data <<= 1;
	} else if(event_id == EVENT_ACK) {
		set_dk(recv_ok);
	}
}

void KEYBOARD::event_frame()
{
	if(phase == PHASE_IDLE) {
		if(!key_buf->empty()) {
			send_data = (uint16_t)key_buf->read_not_remove(0);
			send_data = ((send_data & 0xff) << 2) | ((send_data & 0x100) >> 7);
			int parity = 0;//1;
			if(send_data & 0x200) parity++;
			if(send_data & 0x100) parity++;
			if(send_data & 0x080) parity++;
			if(send_data & 0x040) parity++;
			if(send_data & 0x020) parity++;
			if(send_data & 0x010) parity++;
			if(send_data & 0x008) parity++;
			if(send_data & 0x004) parity++;
			if(send_data & 0x002) parity++;
			send_data |= (parity & 1);
			phase = PHASE_SEND_START_H;
			drive();
		}
	}
}

void KEYBOARD::drive()
{
	switch(phase) {
	case PHASE_RESET:
	case PHASE_IDLE:
		set_dk(false);
		set_stk(false);
		break;
	case PHASE_SEND_START_H:
		set_dk(false);
		set_stk(true);
		phase++;
		register_event(this, EVENT_DRIVE, 12.5, false, NULL);
		break;
	case PHASE_SEND_START_L:
		set_stk(false);
		phase++;
		register_event(this, EVENT_DATA, 32.0, false, NULL);
		register_event(this, EVENT_DRIVE, 32.5, false, NULL);
		break;
	case PHASE_SEND_BIT_7_H:
	case PHASE_SEND_BIT_6_H:
	case PHASE_SEND_BIT_5_H:
	case PHASE_SEND_BIT_4_H:
	case PHASE_SEND_BIT_3_H:
	case PHASE_SEND_BIT_2_H:
	case PHASE_SEND_BIT_1_H:
	case PHASE_SEND_BIT_0_H:
	case PHASE_SEND_COMMAND_H:
	case PHASE_SEND_PARITY_H:
		set_stk(true);
		phase++;
		register_event(this, EVENT_DRIVE, 17.5, false, NULL);
		break;
	case PHASE_SEND_BIT_7_L:
	case PHASE_SEND_BIT_6_L:
	case PHASE_SEND_BIT_5_L:
	case PHASE_SEND_BIT_4_L:
	case PHASE_SEND_BIT_3_L:
	case PHASE_SEND_BIT_2_L:
	case PHASE_SEND_BIT_1_L:
	case PHASE_SEND_BIT_0_L:
	case PHASE_SEND_COMMAND_L:
		set_stk(false);
		phase++;
		register_event(this, EVENT_DATA, 49.5, false, NULL);
		register_event(this, EVENT_DRIVE, 50.0, false, NULL);
		break;
	case PHASE_SEND_PARITY_L:
		set_stk(false);
		phase++;
		register_event(this, EVENT_DRIVE, 50.0, false, NULL);
		break;
	case PHASE_SEND_FINISH:
		set_dk(false);
		phase++;
		register_event(this, EVENT_DRIVE, 300.0, false, NULL);
		break;
	case PHASE_SEND_WAIT_ACK:
		phase = PHASE_IDLE;
		break;
	case PHASE_RECV_START:
		recv_data = 0;
		phase++;
		break;
	case PHASE_RECV_BIT_2:
		recv_data |= dc ? 4 : 0;
		phase++;
		break;
	case PHASE_RECV_BIT_1:
		recv_data |= dc ? 2 : 0;
		phase++;
		break;
	case PHASE_RECV_BIT_0:
		recv_data |= dc ? 1 : 0;
		phase++;
		break;
	case PHASE_RECV_PARITY:
		{
			int parity = 0;//1;
			if(recv_data & 4) parity++;
			if(recv_data & 2) parity++;
			if(recv_data & 1) parity++;
			recv_ok = ((parity & 1) == (dc ? 1 : 0));
			phase++;
			register_event(this, EVENT_ACK, 69.5, false, NULL);
			register_event(this, EVENT_DRIVE, 70.0, false, NULL);
		}
		break;
	case PHASE_RECV_ACK_H:
		set_stk(true);
		phase++;
		register_event(this, EVENT_DRIVE, 17.5, false, NULL);
		break;
	case PHASE_RECV_ACK_L:
		set_dk(false);
		set_stk(false);
		phase = PHASE_IDLE;
		break;
	}
}

void KEYBOARD::key_down(int code)
{
	bool shift = (key_stat[0x10] != 0);
	bool ctrl = (key_stat[0x11] != 0);
	bool alt = (key_stat[0x12] != 0);
	
	if(code == 0x14) {
		caps = !caps;
		return;
	} else if(code == 0x15) {
		kana = !kana;
		return;
	} else if(code == 0x21) {
		if(!pro_mode) {
			pro_mode = true;
			emu->out_message(_T("PRO mode"));
			key_buf->write(0x1f2);
		}
		return;
	} else if(code == 0x22) {
		if(pro_mode) {
			pro_mode = false;
			emu->out_message(_T("OP mode"));
			key_buf->write(0x1f0);
		}
		return;
	} else if(code == 0x70 && alt) {
		// ALT + F11 -> RUN
		key_buf->write(0x1fa);
		return;
	} else if((code == 0x30 || code == 0x60) && alt) {
		// Alt + 0 -> 00
		key_buf->write(0x0fc);
		return;
	}
	if(ctrl && shift) {
		code = key_ctrl_shift_table[code];
	} else if(ctrl) {
		code = key_ctrl_table[code];
	} else if(kana && shift) {
		code = key_kana_shift_table[code];
	} else if(kana) {
		code = key_kana_table[code];
	} else if(shift) {
		code = key_shift_table[code];
	} else {
		code = key_table[code];
	}
	if(code != 0) {
		if(caps) {
			if(code >= 'a' && code <= 'z') {
				code += 'A' - 'a';
			} else if(code >= 'A' && code <= 'Z') {
				code += 'a' - 'A';
			}
		}
		key_buf->write(code);
	}
}

void KEYBOARD::key_up(int code)
{
}

void KEYBOARD::set_stk(bool value)
{
	d_ls244->write_signal(SIG_LS244_INPUT, value ? 0xffffffff : 0, 0x40);
	d_subcpu->write_signal(SIG_CPU_IRQ, value ? 0xffffffff : 0, 1);
}

void KEYBOARD::set_dk(bool value)
{
	d_ls244->write_signal(SIG_LS244_INPUT, value ? 0xffffffff : 0, 0x20);
}

#define STATE_VERSION	3

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!key_buf->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateInt32(phase);
	state_fio->StateUint16(send_data);
	state_fio->StateUint32(stc_clock);
	state_fio->StateUint8(recv_data);
	state_fio->StateBool(recv_ok);
	state_fio->StateBool(stc);
	state_fio->StateBool(dc);
	state_fio->StateBool(caps);
	state_fio->StateBool(kana);
	state_fio->StateBool(pro_mode);
	return true;
}

