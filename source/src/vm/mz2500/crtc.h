/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.03 -

	[ crtc ]
*/

#ifndef _CRTC_H_
#define _CRTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CRTC_COLUMN_SIZE	0
#define SIG_CRTC_PALLETE	1

#define EVENT_HSYNC	0
#define EVENT_BLINK	256

#define SCRN_640x400	1
#define SCRN_640x200	2
#define SCRN_320x200	3

class CRTC : public DEVICE
{
private:
	DEVICE *d_mem, *d_vblank, *d_pio, *d_fdc;
	int did0_mem, did1_mem, did_vblank, did_pio;
	
	// config
	bool scan_line, scan_tmp;
	bool monitor_200line;
	bool monitor_digital, monitor_tmp;
	
	// vram
	uint8 *vram_b, *vram_r, *vram_g, *vram_i;
	uint8 *tvram1, *attrib, *tvram2;
	// kanji rom, pcg
	uint8 *kanji1, *kanji2;
	uint8 *pcg0, *pcg1, *pcg2, *pcg3;
	
	// crtc
	void set_hsync(int h);
	uint8 textreg_num, textreg[32];
	uint8 cgreg_num, cgreg[32];
	uint8 scrn_size, cg_mask;
	bool font_size, column_size;
	uint8 latch[4];
	uint16 GDEVS, GDEVE;
	uint8 GDEHS, GDEHE;
	int GDEHSC, GDEHEC;
	bool hblank, vblank, blink;
	uint8 clear_flag;
	uint8 palette_reg[16];
	bool pal_select;
	
	// priority and palette
	uint8 priority16[16][9];
	uint16 palette16[16+8], palette4096[16];
	uint8 palette4096r[16], palette4096g[16], palette4096b[16];
	uint16 palette16txt[9], palette4096txt[9];
	uint16 palette16pri[16][9], palette4096pri[16][9];
	uint8 prev16;
	bool update16;
	
	uint16 priority256[256][16+64];
	uint16 palette256[256+16+64];
	uint16 palette256txt[16+64];
	uint16 palette256pri[256][16+64];
	uint16 prev256;
	bool update256;
	
	// draw text
	void draw_text();
	void draw_80column_screen();
	void draw_40column_screen();
	void draw_80column_font(uint16 src, int dest, int y);
	void draw_40column_font(uint16 src, int dest, int y);
	uint8 text[640*480*2];
	
	// draw cg
	void draw_cg();
	void draw_320x200x16screen(uint8 pl);
	void draw_320x200x256screen(uint8 pl);
	void draw_640x200x16screen(uint8 pl);
	void draw_640x400x4screen();
	void draw_640x400x16screen();
	void create_320x200map();
	void create_640x200map();
	void create_640x400map();
	uint8 cg[640*400*2];
	uint16 map_addr[400][80];
	uint8 map_hdsc[400][80];
	
	// speed optimize
	uint8 cg_matrix0[256][256][8];
	uint8 cg_matrix1[256][256][8];
	uint8 cg_matrix2[256][256][8];
	uint8 cg_matrix3[256][256][8];
	uint8 text_matrix[256][9][8];
	uint8 text_matrixw[256][9][16];
	uint8 trans_color;
	bool map_init, trans_init;
	
public:
	CRTC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CRTC() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void event_vsync(int v, int clock);
	void update_config();
	
	// unique function
	void set_context_mem(DEVICE* device, int id0, int id1) {
		d_mem = device; did0_mem = id0; did1_mem = id1;
	}
	void set_context_vblank(DEVICE* device, int id) {
		d_vblank = device; did_vblank = id;
	}
	void set_context_pio(DEVICE* device, int id) {
		d_pio = device; did_pio = id;
	}
	void set_context_fdc(DEVICE* device) {
		d_fdc = device;
	}
	void set_vram_ptr(uint8* ptr) {
		vram_b = ptr + 0x00000;
		vram_r = ptr + 0x08000;
		vram_g = ptr + 0x10000;
		vram_i = ptr + 0x18000;
	}
	void set_tvram_ptr(uint8* ptr) {
		tvram1 = ptr + 0x0000;
		attrib = ptr + 0x0800;
		tvram2 = ptr + 0x1000;
	}
	void set_kanji_ptr(uint8* ptr) {
		kanji1 = ptr + 0x00000;
		kanji2 = ptr + 0x20000;
	}
	void set_pcg_ptr(uint8* ptr) {
		pcg0 = ptr + 0x0000;
		pcg1 = ptr + 0x0800;
		pcg2 = ptr + 0x1000;
		pcg3 = ptr + 0x1800;
	}
	void draw_screen();
};

#endif

