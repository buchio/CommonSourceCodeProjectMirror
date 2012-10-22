/*
	SHARP X1twin Emulator 'eX1twin'
	Skelton for retropc emulator

	Origin : Ootake (joypad)
	       : xpce (psg)
	       : MESS (vdc)
	Author : Takeda.Toshiya
	Date   : 2009.03.11-

	[ PC-Eninge ]
*/

#ifndef _PCE_H_
#define _PCE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define VDC_WPF		684	/* width of a line in frame including blanking areas */
#define VDC_LPF		262	/* number of lines in a single frame */

class HUC6280;

class PCE : public DEVICE
{
private:
	HUC6280* d_cpu;
	
	// memory
	uint8 ram[0x2000];	// ram 8kb
	uint8 cart[0x400000];	// max 4mb
//	uint8 backup[0x800];
	uint32 bank;
	uint8 buffer;
	
	// vdc
	struct {
		int dvssr_write;		/* Set when the DVSSR register has been written to */
		int physical_width;		/* Width of the display */
		int physical_height;		/* Height of the display */
		uint16 sprite_ram[64*4];	/* Sprite RAM */
		int curline;			/* the current scanline we're on */
		int current_segment;		/* current segment of display */
		int current_segment_line;	/* current line inside a segment of display */
		int vblank_triggered;		/* to indicate whether vblank has been triggered */
		int raster_count;		/* counter to compare RCR against */
		int satb_countdown;		/* scanlines to wait to trigger the SATB irq */
		uint8 vram[0x10000];
		uint8 inc;
		uint8 vdc_register;
		uint8 vdc_latch;
		pair vdc_data[32];
		int status;
		int y_scroll;
	} vdc;
	struct {
		uint8 vce_control;		/* VCE control register */
		pair vce_address;		/* Current address in the palette */
		pair vce_data[512];		/* Palette data */
		int current_bitmap_line;	/* The current line in the display we are on */
		//bitmap_ind16 *bmp;
		scrntype bmp[VDC_LPF][VDC_WPF];
		scrntype palette[1024];
	} vce;
	void pce_interrupt();
	void vdc_reset();
	void vdc_advance_line();
	void draw_black_line(int line);
	void draw_overscan_line(int line);
	void vram_write(uint32 offset, uint8 data);
	uint8 vram_read(uint32 offset);
	void vdc_w(uint16 offset, uint8 data);
	uint8 vdc_r(uint16 offset);
	void vce_w(uint16 offset, uint8 data);
	uint8 vce_r(uint16 offset);
	void pce_refresh_line(int line, uint8 *drawn, scrntype *line_buffer);
	void conv_obj(int i, int l, int hf, int vf, char *buf);
	void pce_refresh_sprites(int line, uint8 *drawn, scrntype *line_buffer);
	void vdc_do_dma();
	
	// psg
	typedef struct {
		// registers
		uint8 regs[8];
		uint8 wav[32];
		uint8 wavptr;
		// sound gen
		uint32 genptr;
		uint32 remain;
		bool noise;
		uint32 randval;
	} psg_t;
	psg_t psg[8];
	uint8 psg_ch, psg_vol, psg_lfo_freq, psg_lfo_ctrl;
	int sample_rate;
	void psg_reset();
	void psg_write(uint16 addr, uint8 data);
	uint8 psg_read(uint16 addr);
	
	// joypad
	uint8 *joy_stat, *key_stat;
	uint8 joy_count, joy_nibble, joy_second;
	void joy_reset();
	void joy_write(uint16 addr, uint8 data);
	uint8 joy_read(uint16 addr);
	
public:
	PCE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PCE() {}
	
	// common functions
	void initialize();
	void reset();
	void event_vline(int v, int clock);
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void mix(int32* buffer, int cnt);
	
	// unique functions
	void set_context_cpu(HUC6280* device) {
		d_cpu = device;
	}
	void initialize_sound(int rate) {
		sample_rate = rate;
	}
	void open_cart(_TCHAR* file_path);
	void close_cart();
	void draw_screen();
};

#endif
