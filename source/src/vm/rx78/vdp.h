/*
	BANDAI RX-78 Emulator 'eRX-78'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ vdp ]
*/

#ifndef _VDP_H_
#define _VDP_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#ifdef _WIN32_WCE
// RGB565
#define RGB_COLOR(r, g, b) (uint16)(((uint16)(r) << 11) | ((uint16)(g) << 6) | (uint16)(b))
#else
// RGB555
#define RGB_COLOR(r, g, b) (uint16)(((uint16)(r) << 10) | ((uint16)(g) << 5) | (uint16)(b))
#endif

class VDP : public DEVICE
{
private:
	DEVICE* dev;
	
	uint16 palette_pc[17];	// 8cols * 2 + bg
	uint8* vram[6];
	uint8 reg[6], bg, cmask, pmask;
	
	void create_pal();
	void create_bg();
	
public:
	VDP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~VDP() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	void event_vsync(int v, int clock);
	
	// unique function
	void set_context(DEVICE* device) { dev = device; }
	void set_vram_ptr(uint8* ptr) { for(int i = 0; i < 6; i++) vram[i] = ptr + 4416 * i; }
	void draw_screen();
};

#endif
