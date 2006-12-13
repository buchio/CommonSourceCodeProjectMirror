/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_COLUMN	0

static const int key_map[14][8] = {
	{0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77},
	{0x78, 0x79, 0x68, 0x69, 0x6c, 0x6e, 0x6b, 0x6d},
	{0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67},
	{0x09, 0x20, 0x0d, 0x26, 0x28, 0x25, 0x27, 0x13},
	{0xbf, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47},
	{0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f},
	{0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57},
	{0x58, 0x59, 0x5a, 0xde, 0xdc, 0xe2, 0xbe, 0xbc},
	{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37},
	{0x38, 0x39, 0xba, 0xbb, 0xbd, 0xc0, 0xdb, 0x00},
	{0xdd, 0x7b, 0x24, 0x2e, 0x08, 0x1b, 0x6a, 0x6f},
	{0x12, 0x14, 0x10, 0x15, 0x11, 0x00, 0x00, 0x00},
	{0x1d, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x19, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

class KEYBOARD : public DEVICE
{
private:
	DEVICE* pio0;	// i8255
	DEVICE* pio1;	// z80pio
	int pio0_id, pio1_id;
	
	// keyboard
	uint8* key_stat;
	uint8 keys[16];
	uint8 column;
	void create_keystat();
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unique function
	void set_context_pio0(DEVICE* device, int id) {
		pio0 = device;
		pio0_id = id;
	}
	void set_context_pio1(DEVICE* device, int id) {
		pio1 = device;
		pio1_id = id;
	}
};

#endif

