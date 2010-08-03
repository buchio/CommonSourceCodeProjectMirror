/*
	SANYO PHC-25 Emulator 'ePHC-25'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ keyboard ]
*/

#include "keyboard.h"

static const uint8 key_map[9][8] = {
	{0x31, 0x57, 0x53, 0x58, 0x26, 0x2e, 0xba, 0x00},
	{0x1b, 0x51, 0x41, 0x5a, 0x28, 0x0d, 0xbb, 0xbf},
	{0x33, 0x52, 0x46, 0x56, 0x25, 0xde, 0xdb, 0x00},
	{0x32, 0x45, 0x44, 0x43, 0x27, 0xdc, 0xdd, 0x20},
	{0x35, 0x59, 0x48, 0x4e, 0x72, 0x30, 0x50, 0x00},
	{0x34, 0x54, 0x47, 0x42, 0x73, 0xbd, 0xc0, 0x00},
	{0x36, 0x55, 0x4a, 0x4d, 0x71, 0x39, 0x4f, 0x00},
	{0x37, 0x49, 0x4b, 0xbc, 0x70, 0x38, 0x4c, 0xbe},
	{0x00, 0x12, 0x10, 0x11, 0x00, 0xf0, 0x00, 0x00}
};

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	
	// regist event to update the key status
	vm->regist_frame_event(this);
}

uint32 KEYBOARD::read_io8(uint32 addr)
{
	return status[addr & 0x0f];
}

void KEYBOARD::event_frame()
{
	_memset(status, 0, sizeof(status));
	
	for(int i = 0; i < 9; i++) {
		uint8 val = 0;
		for(int j = 0; j < 8; j++) {
			val |= key_stat[key_map[i][j]] ? (1 << j) : 0;
		}
		status[i] = ~val;
	}
}
