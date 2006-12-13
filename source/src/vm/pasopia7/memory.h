/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_I8255_1_A	0
#define SIG_MEMORY_I8255_1_B	1
#define SIG_MEMORY_I8255_1_C	2

class MEMORY : public DEVICE
{
private:
	DEVICE *dev_io, *dev_pio0, *dev_pio2;
	int dev_io_id, dev_pio0_id, dev_pio2_id;
	
	uint8 bios[0x4000];
	uint8 basic[0x8000];
	uint8 ram[0x10000];
	uint8 vram[0x10000];	// blue, red, green + text, attribute
	uint8 pal[0x10];
	uint8 wdmy[0x10000];
	uint8 rdmy[0x10000];
	uint8* wbank[16];
	uint8* rbank[16];
	
	uint8 plane, attr_data, attr_latch;
	bool vram_sel, pal_sel, attr_wrap;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_io(DEVICE* device, int id) { dev_io = device; dev_io_id = id; }
	void set_context_pio0(DEVICE* device, int id) { dev_pio0 = device; dev_pio0_id = id; }
	void set_context_pio2(DEVICE* device, int id) { dev_pio2 = device; dev_pio2_id = id; }
	uint8* get_ram() { return ram; }
	uint8* get_vram() { return vram; }
	uint8* get_pal() { return pal; }
};

#endif

