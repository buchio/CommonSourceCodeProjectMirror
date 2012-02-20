/*
	NEC PC-98DO Emulator 'ePC-98DO'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2011.12.29-

	[ PC-8801 ]
*/

#include "pc88.h"
#include "../beep.h"
#include "../event.h"
#include "../pcm1bit.h"
#include "../upd1990a.h"
#include "../ym2203.h"
#include "../../config.h"

#define EVENT_TIMER	0
#define EVENT_BUSREQ	1

#define IRQ_SERIAL	0
#define IRQ_VRTC	1
#define IRQ_TIMER	2
#define IRQ_SOUND	4

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} \
		else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} \
		else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

#define SET_BANK_W(s, e, w) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} \
		else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
	} \
}

#define SET_BANK_R(s, e, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} \
		else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

static const int key_table[15][8] = {
	{ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 },
	{ 0x68, 0x69, 0x6a, 0x6b, 0x00, 0x00, 0x6e, 0x0d },
	{ 0xc0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 },
	{ 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f },
	{ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 },
	{ 0x58, 0x59, 0x5a, 0xdb, 0xdc, 0xdd, 0xde, 0xbd },
	{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
	{ 0x38, 0x39, 0xba, 0xbb, 0xbc, 0xbe, 0xbf, 0xe2 },
	{ 0x24, 0x26, 0x27, 0x2e, 0x12, 0x15, 0x10, 0x11 },
	{ 0x13, 0x70, 0x71, 0x72, 0x73, 0x74, 0x20, 0x1b },
	{ 0x09, 0x28, 0x25, 0x23, 0x7b, 0x6d, 0x6f, 0x14 },
	{ 0x21, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x75, 0x76, 0x77, 0x78, 0x79, 0x08, 0x2d, 0x2e },
	{ 0x1c, 0x1d, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0d, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

static const int key_conv_table[6][2] = {
	{0x2d, 0x2e}, // INS	-> SHIFT + DEL
	{0x75, 0x70}, // F6	-> SHIFT + F1
	{0x76, 0x71}, // F7	-> SHIFT + F2
	{0x77, 0x72}, // F8	-> SHIFT + F3
	{0x78, 0x73}, // F9	-> SHIFT + F4
	{0x79, 0x74}, // F10	-> SHIFT + F5
};

void PC88::initialize()
{
	memset(rdmy, 0xff, sizeof(rdmy));
//	memset(ram, 0, sizeof(ram));
#ifdef PC88_EXRAM_BANKS
	memset(exram, 0, sizeof(exram));
#endif
	memset(gvram, 0, sizeof(gvram));
	memset(tvram, 0, sizeof(tvram));
	memset(n88rom, 0xff, sizeof(n88rom));
	memset(n88exrom, 0xff, sizeof(n88exrom));
	memset(n80rom, 0xff, sizeof(n80rom));
	memset(kanji1, 0xff, sizeof(kanji1));
	memset(kanji2, 0xff, sizeof(kanji2));
#ifdef SUPPORT_PC88_DICTIONARY
	memset(dicrom, 0xff, sizeof(dicrom));
#endif
	
	// load font data
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sN88.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(n88rom, 0x8000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sN88_0.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x0000, 0x2000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sN88_1.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x2000, 0x2000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sN88_2.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x4000, 0x2000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sN88_3.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x6000, 0x2000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sN80.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(n80rom, 0x8000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sKANJI1.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(kanji1, 0x20000, 1);
		fio->Fclose();
	}
	_stprintf(file_path, _T("%sKANJI2.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(kanji2, 0x20000, 1);
		fio->Fclose();
	}
#ifdef SUPPORT_PC88_DICTIONARY
	_stprintf(file_path, _T("%sJISYO.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(dicrom, 0x80000, 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	// memory pattern (from M88)
	for(int i = 0; i < 0x10000; i += 0x80) {
		uint8 b = ((i >> 7) ^ i) & 0x100 ? 0x00 : 0xff;
		memset(ram + i, b, 0x40);
		memset(ram + i + 0x40, ~b, 0x40);
		ram[i + 0x7f] = b;
	}
	ram[0xffff] = 0;
	
	// create semi graphics pattern
	for(int i = 0; i < 256; i++) {
		uint8 *dest = sg_pattern + 8 * i;
		dest[0] = dest[1] = ((i & 1) ? 0xf0 : 0) | ((i & 0x10) ? 0x0f : 0);
		dest[2] = dest[3] = ((i & 2) ? 0xf0 : 0) | ((i & 0x20) ? 0x0f : 0);
		dest[4] = dest[5] = ((i & 4) ? 0xf0 : 0) | ((i & 0x40) ? 0x0f : 0);
		dest[6] = dest[7] = ((i & 8) ? 0xf0 : 0) | ((i & 0x80) ? 0x0f : 0);
	}
	
	for(int i = 0; i < 8; i++) {
		palette_text_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	palette_text_pc[8] = 0;
	
	line200 = false;
	
#ifdef SUPPORT_PC88_HIGH_CLOCK
	cpu_clock_low = config.cpu_clock_low;
#else
	cpu_clock_low = true;
#endif
	
//	int mem_wait_clock = ((config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N) && cpu_clock_low) ? 0 : 1;
	int mem_wait_clock= (config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N) ? 1 : cpu_clock_low ? 0 : 1;
	for(int i = 0; i < 16; i++) {
		mem_wait_clocks[i] = mem_wait_clock;
	}
	
	key_status = emu->key_buffer();
#ifdef SUPPORT_PC88_JOYSTICK
	joy_status = emu->joy_buffer();
#endif
	
	register_frame_event(this);
	register_vline_event(this);
	register_event(this, EVENT_TIMER, 1000000 / 600, true, NULL);
}

void PC88::reset()
{
	// memory
	rm_mode = 0;
	exrom_sel = exram_sel = exram_bank = 0;
	exrom_bank = 0xff;
	text_window = 0x80; // ???
	gvram_plane = gvram_sel = 0;
	tvram_sel = 0x10;
#ifdef SUPPORT_PC88_DICTIONARY
	dicrom_sel = 1;
	dicrom_bank = 0;
#endif
	
	SET_BANK(0x0000, 0x7fff, ram + 0x0000, n88rom);
	SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
	
	port32 = 0;	// ???
	portE2 = 0xff;	// ???
	alu_ctrl1 = alu_ctrl2 = 0;
	ghs_mode = 0;
	opn_busy = true;
	
	// crtc
	crtc_cmd = crtc_ptr = 0;
	crtc_status = 0;
	
	text_attrib = 0;
	attrib_num = 20;
	
	text_width = 80;
	text_height = 25;
	char_height = 16;
	skip_line = true;
	
	cursor_on = cursor_blink = false;
	cursor_x = cursor_y = cursor_line = 0;
	
	blink_on = false;
	blink_rate = 24;
	blink_counter = 0;
	
	text_mode = graph_mode = 0;
	disp_ctrl = 0;
	
	if(!line200) {
		set_frames_per_sec(60);
		set_lines_per_frame(260);
		line200 = true;
	}
	
	for(int i = 0; i <= 8; i++) {
		palette[i].b = (i & 1) ? 7 : 0;
		palette[i].r = (i & 2) ? 7 : 0;
		palette[i].g = (i & 4) ? 7 : 0;
	}
	update_palette = true;
	
	// dma
	memset(dma_reg, 0, sizeof(dma_reg));
	dma_mode = dma_status = 0;
	dma_hl = false;
	
	// keyboard
	kana = caps = 0;
	
	// kanji rom
	kanji1_addr.d = kanji2_addr.d = 0;
	
	// interrupt
	intr_req = intr_mask1 = 0;
	intr_mask2 = 0xff;
	
	// fdd i/f
	d_pio->write_io8(1, 0);
	d_pio->write_io8(2, 0);
}

#ifdef Z80_MEMORY_WAIT
void PC88::write_data8w(uint32 addr, uint32 data, int* wait)
{
#else
void PC88::write_data8(uint32 addr, uint32 data)
{
	int wait_tmp;
	int *wait = &wait_tmp;
#endif
	addr &= 0xffff;
	*wait = mem_wait_clocks[addr >> 12];
	
	if((addr & 0xfc00) == 0x8000) {
		// text window
		if(rm_mode == 0) {
			addr = (text_window << 8) + (addr & 0x3ff);
		}
		ram[addr & 0xffff] = data;
		return;
	}
	else if((addr & 0xc000) == 0xc000) {
		switch(gvram_sel) {
		case 1:
			*wait = gvram_wait_clocks;
			gvram[(addr & 0x3fff) | 0x0000] = data;
			return;
		case 2:
			*wait = gvram_wait_clocks;
			gvram[(addr & 0x3fff) | 0x4000] = data;
			return;
		case 4:
			*wait = gvram_wait_clocks;
			gvram[(addr & 0x3fff) | 0x8000] = data;
			return;
		case 8:
			*wait = alu_wait_clocks;
			addr &= 0x3fff;
			switch(alu_ctrl2 & 0x30) {
			case 0x00:
				for(int i = 0; i < 3; i++) {
					switch((alu_ctrl1 >> i) & 0x11) {
					case 0x00:	// reset
						gvram[addr | (0x4000 * i)] &= ~data;
						break;
					case 0x01:	// set
						gvram[addr | (0x4000 * i)] |= data;
						break;
					case 0x10:	// reverse
						gvram[addr | (0x4000 * i)] ^= data;
						break;
					}
				}
				break;
			case 0x10:
				gvram[addr | 0x0000] = alu_reg[0];
				gvram[addr | 0x4000] = alu_reg[1];
				gvram[addr | 0x8000] = alu_reg[2];
				break;
			case 0x20:
				gvram[addr | 0x0000] = alu_reg[1];
				break;
			case 0x30:
				gvram[addr | 0x4000] = alu_reg[0];
				break;
			}
			return;
		}
	}
	wbank[addr >> 12][addr & 0xfff] = data;
}

#ifdef Z80_MEMORY_WAIT
uint32 PC88::read_data8w(uint32 addr, int* wait)
{
#else
uint32 PC88::read_data8(uint32 addr)
{
	int wait_tmp;
	int *wait = &wait_tmp;
#endif
	addr &= 0xffff;
	*wait = mem_wait_clocks[addr >> 12];
	
	if((addr & 0xfc00) == 0x8000) {
		// text window
		if(rm_mode == 0) {
			addr = (text_window << 8) + (addr & 0x3ff);
		}
		return ram[addr & 0xffff];
	}
	else if((addr & 0xc000) == 0xc000) {
		uint8 b, r, g;
		switch(gvram_sel) {
		case 1:
			*wait = gvram_wait_clocks;
			return gvram[(addr & 0x3fff) | 0x0000];
		case 2:
			*wait = gvram_wait_clocks;
			return gvram[(addr & 0x3fff) | 0x4000];
		case 4:
			*wait = gvram_wait_clocks;
			return gvram[(addr & 0x3fff) | 0x8000];
		case 8:
			*wait = alu_wait_clocks;
			addr &= 0x3fff;
			alu_reg[0] = gvram[addr | 0x0000];
			alu_reg[1] = gvram[addr | 0x4000];
			alu_reg[2] = gvram[addr | 0x8000];
			b = alu_reg[0]; if(!(alu_ctrl2 & 1)) b ^= 0xff;
			r = alu_reg[1]; if(!(alu_ctrl2 & 2)) r ^= 0xff;
			g = alu_reg[2]; if(!(alu_ctrl2 & 4)) g ^= 0xff;
			return b & r & g;
		}
#ifdef SUPPORT_PC88_DICTIONARY
		if(dicrom_sel == 0) {
			return dicrom[(addr & 0x3fff) | (0x4000 * dicrom_bank)];
		}
#endif
	}
	return rbank[addr >> 12][addr & 0xfff];
}

void PC88::write_io8(uint32 addr, uint32 data)
{
	addr &= 0xff;
	switch(addr) {
	case 0x10:
		d_rtc->write_signal(SIG_UPD1990A_C0, data, 1);
		d_rtc->write_signal(SIG_UPD1990A_C1, data, 2);
		d_rtc->write_signal(SIG_UPD1990A_C2, data, 4);
		d_rtc->write_signal(SIG_UPD1990A_DIN, data, 8);
		break;
	case 0x20:
	case 0x21:
		d_sio->write_io8(addr, data);
		break;
	case 0x30:
		text_mode = data;
		break;
	case 0x31:
		if(line200 && !(data & 0x11)) {
			set_frames_per_sec(55.4);
			set_lines_per_frame(448);
			line200 = false;
		}
		else if(!line200 && (data & 0x11)) {
			set_frames_per_sec(60);
			set_lines_per_frame(260);
			line200 = true;
		}
		if(rm_mode != (data & 6)) {
			rm_mode = data & 6;
			update_low_memmap();
		}
		if((graph_mode & 0x10) != (data & 0x10)) {
			update_palette = true;
		}
		graph_mode = data;
		break;
	case 0x32:
		if(exrom_sel != (data & 3)) {
			exrom_sel = data & 3;
			if(exrom_bank == 0xfe) {
				update_low_memmap();
			}
		}
		if(tvram_sel != (data & 0x10)) {
			tvram_sel = data & 0x10;
			if(config.boot_mode == MODE_PC88_V1H || config.boot_mode == MODE_PC88_V2) {
				update_tvram_memmap();
			}
		}
//		if((port32 & 0x20) != (data & 0x20)) {
//			update_palette = true;
//		}
		if((port32 & 0x80) != (data & 0x80)) {
			intr_mask2 = (intr_mask2 & ~0x10) | ((data & 0x80) ? 0 : 0x10);
			update_intr();
		}
		port32 = data;
		update_gvram_sel();
		break;
	case 0x34:
		alu_ctrl1 = data;
		break;
	case 0x35:
		alu_ctrl2 = data;
		update_gvram_sel();
		break;
	case 0x40:
		// bit0: printer strobe
		d_rtc->write_signal(SIG_UPD1990A_STB, ~data, 2);
		d_rtc->write_signal(SIG_UPD1990A_CLK, data, 4);
		// bit3: crtc i/f sync mode
		if(ghs_mode != (data & 0x10)) {
			ghs_mode = data & 0x10;
			update_gvram_wait();
		}
		d_beep->write_signal(SIG_BEEP_ON, data, 0x20);
		// bit6: joystick port pin#8
		d_pcm->write_signal(SIG_PCM1BIT_SIGNAL, data, 0x80);
		break;
	case 0x44:
	case 0x45:
#ifdef HAS_YM2608
	case 0x46:
	case 0x47:
#endif
		d_opn->write_io8(addr, data);
		break;
	case 0x50:
		switch(crtc_cmd) {
		case 0:
			switch(crtc_ptr) {
			case 0:
				text_width = (data & 0x7f) + 2;
				break;
			case 1:
				text_height = (data & 0x3f) + 1;
				blink_rate = 8 * ((data >> 6) + 1);
				break;
			case 2:
				char_height = (data & 0x1f) + 1;
				cursor_blink = ((data & 0x20) != 0);
				cursor_line = (data & 0x40) ? 0 : 7;
				skip_line = ((data & 0x80) != 0);
				break;
			case 4:
				attrib_num = (data & 0x20) ? 0 : (data & 0x1f) + 1;
				break;
			}
			break;
		case 4:
			switch(crtc_ptr) {
			case 0:
				cursor_x = data;
				break;
			case 1:
				cursor_y = data;
				break;
			}
			break;
		}
		crtc_ptr++;
		break;
	case 0x51:
		crtc_cmd = (data >> 5) & 7;
		crtc_ptr = 0;
		switch(crtc_cmd) {
		case 0:	// reset CRTC
			crtc_status = 0;
			break;
		case 1:	// start display
			crtc_status |= 0x10;
			crtc_status &= ~8;
			break;
		case 2:	// set irq mask
			if(!(data & 1)) {
				crtc_status = 0;
			}
			break;
		case 3:	// read light pen
			crtc_status &= ~1;
			break;
		case 4:	// load cursor position ON/OFF
			cursor_on = ((data & 1) != 0);
			break;
		case 5:	// reset IRQ
			break;
		case 6:	// reset counters
			crtc_status = 0;
			break;
		}
		break;
	case 0x52:
		palette[8].b = (data & 0x10) ? 7 : 0;
		palette[8].r = (data & 0x20) ? 7 : 0;
		palette[8].g = (data & 0x40) ? 7 : 0;
		update_palette = true;
		break;
	case 0x53:
		disp_ctrl = data;
		break;
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
		if(port32 & 0x20) {
			int n = (data & 0x80) ? 8 : (addr - 0x54);
			if(data & 0x40) {
				palette[n].g = data & 7;
			}
			else {
				palette[n].b = data & 7;
				palette[n].r = (data >> 3) & 7;
			}
		}
		else {
			int n = addr - 0x54;
			palette[n].b = (data & 1) ? 7 : 0;
			palette[n].r = (data & 2) ? 7 : 0;
			palette[n].g = (data & 4) ? 7 : 0;
		}
		update_palette = true;
		break;
	case 0x5c:
		gvram_plane = 1;
		update_gvram_sel();
		break;
	case 0x5d:
		gvram_plane = 2;
		update_gvram_sel();
		break;
	case 0x5e:
		gvram_plane = 4;
		update_gvram_sel();
		break;
	case 0x5f:
		gvram_plane = 0;
		update_gvram_sel();
		break;
	case 0x60:
	case 0x62:
	case 0x64:
	case 0x66:
		if(!dma_hl) {
			dma_reg[(addr >> 1) & 3].start.b.l = data;
		}
		else {
			dma_reg[(addr >> 1) & 3].start.b.h = data;
		}
		dma_hl = !dma_hl;
		break;
	case 0x61:
	case 0x63:
	case 0x65:
	case 0x67:
		if(!dma_hl) {
			dma_reg[(addr >> 1) & 3].length.b.l = data;
		}
		else {
			dma_reg[(addr >> 1) & 3].length.b.h = data & 0x3f;
		}
		dma_hl = !dma_hl;
		break;
	case 0x68:
		dma_mode = data;
		dma_hl = false;
		break;
	case 0x70:
		text_window = data;
		break;
	case 0x71:
		if(exrom_bank != data) {
			exrom_bank = data;
			update_low_memmap();
		}
		break;
	case 0x78:
		text_window++;
		break;
	case 0xe2:
		if(exram_sel != (data & 0x11)) {
			exram_sel = data & 0x11;
			update_low_memmap();
		}
		portE2 = data ^ 0x11;
		break;
	case 0xe3:
		if(exram_bank != data) {
			exram_bank = data;
			update_low_memmap();
		}
		break;
	case 0xe4:
		intr_mask1 = (data < 8) ? ~(0xff << data) : 0xff;
		update_intr();
		break;
	case 0xe6:
		intr_mask2 = (intr_mask2 & ~7) | ((data & 1) << 2) | (data & 2) | ((data & 4) >> 2);
		update_intr();
		break;
	case 0xe8:
		kanji1_addr.b.l = data;
		break;
	case 0xe9:
		kanji1_addr.b.h = data;
		break;
	case 0xec:
		kanji2_addr.b.l = data;
		break;
	case 0xed:
		kanji2_addr.b.h = data;
		break;
#ifdef SUPPORT_PC88_DICTIONARY
	case 0xf0:
		dicrom_bank = data & 0x1f;
		break;
	case 0xf1:
		dicrom_sel = data & 1;
		break;
#endif
	case 0xfc:
		d_pio->write_io8(0, data);
		break;
	case 0xfd:
	case 0xfe:
	case 0xff:
		d_pio->write_io8(addr, data);
		break;
	}
}

uint32 PC88::read_io8(uint32 addr)
{
	uint32 val = 0xff;
	
	addr &= 0xff;
	switch(addr) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
		if(addr == 8 || addr == 9) {
			// INS or F6-F10 -> SHIFT + DEL or F1-F5
			key_status_bak[6] = key_status[0x10];
			key_status_bak[7] = key_status[0x15];
			for(int i = 0; i < 6; i++) {
				key_status_bak[i] = key_status[key_conv_table[i][1]];
				if(key_status[key_conv_table[i][0]]) {
					key_status[key_conv_table[i][1]] = 1;
					key_status[0x10] = 1;
				}
			}
			key_status[0x15] = kana;
		}
		else if(addr == 0x0a) {
			key_status_bak[6] = key_status[0x14];
			key_status[0x14] = caps;
		}
		for(int i = 0; i < 8; i++) {
			if(key_status[key_table[addr & 0x0f][i]]) {
				val &= ~(1 << i);
			}
		}
		if(addr == 8 || addr == 9) {
			for(int i = 0; i < 6; i++) {
				key_status[key_conv_table[i][1]] = key_status_bak[i];
			}
			key_status[0x10] = key_status_bak[6];
			key_status[0x15] = key_status_bak[7];
		}
		else if(addr == 0x0a) {
			key_status[0x14] = key_status_bak[6];
		}
		else if(addr == 0x0e) {
			val &= ~0x80; // http://www.maroon.dti.ne.jp/youkan/pc88/iomap.html
		}
		return val;
	case 0x20:
	case 0x21:
		return d_sio->read_io8(addr);
	case 0x30:
		// 80x25, BASIC mode
		return (config.boot_mode == MODE_PC88_N ? 0 : 1) | 0xc2;
	case 0x31:
		return (config.boot_mode == MODE_PC88_V2 ? 0 : 0x80) | (config.boot_mode == MODE_PC88_V1S ? 0 : 0x40);
	case 0x32:
		return port32;
	case 0x40:
		return (vblank ? 0x20 : 0) | (d_rtc->read_signal(0) ? 0x10 : 0) | 0xc0;
	case 0x44:
		val = d_opn->read_io8(addr);
		if(opn_busy) {
			// show busy flag for first access (for ALPHA)
			val |= 0x80;
			opn_busy = false;
		}
		return val;
	case 0x45:
#ifdef HAS_YM2608
	case 0x46:
	case 0x47:
#endif
		return d_opn->read_io8(addr);
	case 0x50:
		if(crtc_status & 8) {
			// dma underrun
			return (crtc_status & ~0x10);
		}
		return crtc_status;
	case 0x51:
		return 0xff;
	case 0x5c:
		return gvram_plane | 0xf8;
	case 0x68:
		return dma_status;
	case 0x6e:
		return (cpu_clock_low ? 0x80 : 0) | 0x7f;
	case 0x70:
		return text_window;
	case 0x71:
		return exrom_bank;
	case 0xe2:
		return portE2;
	case 0xe3:
		return exram_bank;
	case 0xe8:
		return kanji1[kanji1_addr.w.l * 2 + 1];
	case 0xe9:
		return kanji1[kanji1_addr.w.l * 2];
	case 0xec:
		return kanji2[kanji2_addr.w.l * 2 + 1];
	case 0xed:
		return kanji2[kanji2_addr.w.l * 2];
	case 0xfc:
	case 0xfd:
	case 0xfe:
		return d_pio->read_io8(addr);
	}
	return 0xff;
}

void PC88::write_dma_data8(uint32 addr, uint32 data)
{
	if((addr & 0xf000) == 0xf000 && (config.boot_mode == MODE_PC88_V1H || config.boot_mode == MODE_PC88_V2)) {
		tvram[addr & 0xfff] = data;
		return;
	}
	ram[addr & 0xffff] = data;
}

uint32 PC88::read_dma_data8(uint32 addr)
{
	if((addr & 0xf000) == 0xf000 && (config.boot_mode == MODE_PC88_V1H || config.boot_mode == MODE_PC88_V2)) {
		return tvram[addr & 0xfff];
	}
	return ram[addr & 0xffff];
}

void PC88::write_dma_io8(uint32 addr, uint32 data)
{
	// to crtc
	crtc_buffer[(crtc_buffer_ptr++) & 0x3fff] = data;
}

void PC88::update_gvram_wait()
{
	// from M88 memory wait table
	if(config.boot_mode == MODE_PC88_V1H || config.boot_mode == MODE_PC88_V2) {
		if(vblank) {
//			gvram_wait_clocks = cpu_clock_low ? 1 : 3;
			gvram_wait_clocks = cpu_clock_low ? 2 : 3;
		}
		else {
//			gvram_wait_clocks = cpu_clock_low ? 2 : 5;
			gvram_wait_clocks = cpu_clock_low ? 3 : 5;
		}
	}
	else if(ghs_mode) {
		if(vblank) {
			gvram_wait_clocks = cpu_clock_low ? 3 : 4;
		}
		else {
			gvram_wait_clocks = cpu_clock_low ? 4 : 8;
		}
	}
	else {
		if(vblank) {
			gvram_wait_clocks = cpu_clock_low ? 3 : 7;
		}
		else {
			gvram_wait_clocks = cpu_clock_low ? 30 : 72;
		}
	}
	alu_wait_clocks = gvram_wait_clocks + 4; // ???
}

void PC88::update_gvram_sel()
{
	if(port32 & 0x40) {
		if(alu_ctrl2 & 0x80) {
			gvram_sel = 8;
		}
		else {
			gvram_sel = 0;
		}
		gvram_plane = 0; // from M88
	}
	else {
		gvram_sel = gvram_plane;
	}
}

void PC88::update_low_memmap()
{
	// read
	if(exram_sel & 1) {
#ifdef PC88_EXRAM_BANKS
		if(exram_bank < PC88_EXRAM_BANKS) {
			SET_BANK_R(0x0000, 0x7fff, exram + 0x8000 * exram_bank);
		}
		else {
#endif
			SET_BANK_R(0x0000, 0x7fff, rdmy);
#ifdef PC88_EXRAM_BANKS
		}
#endif
	}
	else if(rm_mode & 2) {
		// 64K RAM
		SET_BANK_R(0x0000, 0x7fff, ram);
	}
	else if(rm_mode & 4) {
		// N-BASIC
		SET_BANK_R(0x0000, 0x7fff, n80rom);
	}
	else {
		// N-88BASIC
		SET_BANK_R(0x0000, 0x5fff, n88rom);
		if(exrom_bank == 0xff) {
			SET_BANK_R(0x6000, 0x7fff, n88rom + 0x6000);
		}
		else if(exrom_bank == 0xfe) {
			SET_BANK_R(0x6000, 0x7fff, n88exrom + 0x2000 * exrom_sel);
		}
		else {
			SET_BANK_R(0x6000, 0x7fff, rdmy);
		}
	}
	
	// write
	if(exram_sel & 0x10) {
#ifdef PC88_EXRAM_BANKS
		if(exram_bank < PC88_EXRAM_BANKS) {
			SET_BANK_W(0x0000, 0x7fff, exram + 0x8000 * exram_bank);
		}
		else {
#endif
			SET_BANK_W(0x0000, 0x7fff, wdmy);
#ifdef PC88_EXRAM_BANKS
		}
#endif
	}
	else {
		SET_BANK_W(0x0000, 0x7fff, ram);
	}
}

void PC88::update_tvram_memmap()
{
	if(tvram_sel == 0) {
		SET_BANK(0xf000, 0xffff, tvram, tvram);
		mem_wait_clocks[15] = mem_wait_clocks[0] + 1;
	}
	else {
		SET_BANK(0xf000, 0xffff, ram + 0xf000, ram + 0xf000);
		mem_wait_clocks[15] = mem_wait_clocks[0] + 0;
	}
}

void PC88::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_PC88_SOUND_IRQ) {
		request_intr(IRQ_SOUND, ((data & mask) != 0));
	}
}

void PC88::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TIMER) {
		request_intr(IRQ_TIMER, true);
	}
	else if(event_id == EVENT_BUSREQ) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
	}
}

void PC88::event_frame()
{
	// update blink counter
	if(!(blink_counter++ < blink_rate)) {
		blink_on = !blink_on;
		blink_counter = 0;
	}
	
#ifdef SUPPORT_PC88_JOYSTICK
	// update joystick status
	d_opn->write_signal(SIG_YM2203_PORT_A, ~(joy_status[0] >> 0), 0x0f);	// direction
	d_opn->write_signal(SIG_YM2203_PORT_B, ~(joy_status[0] >> 4), 0x03);	// button
#endif
}

void PC88::event_vline(int v, int clock)
{
	int disp_line = line200 ? 200 : 400;
	
	if(v == 0) {
		if((crtc_status & 0x10) && (dma_mode & 4)) {
			// start dma transfer to crtc
			dma_status &= ~4;
			
			// dma wait cycles: 9clocks/byte from quasi88
			busreq_clocks = (int)((double)(dma_reg[2].length.sd + 1) * 9.0 / (double)disp_line + 0.5);
		}
		memset(crtc_buffer, 0, sizeof(crtc_buffer));
		crtc_buffer_ptr = 0;
		
		vblank = false;
		request_intr(IRQ_VRTC, false);
		update_gvram_wait();
	}
	if(v < disp_line) {
		if((crtc_status & 0x10) && (dma_mode & 4) && !(dma_status & 4)) {
			// bus request
			if(config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				register_event_by_clock(this, EVENT_BUSREQ, busreq_clocks, false, NULL);
			}
		}
	}
	else if(v == disp_line) {
		if((crtc_status & 0x10) && (dma_mode & 4) && !(dma_status & 4)) {
			// run dma transfer to crtc
			uint16 addr = dma_reg[2].start.w.l;
			for(int i = 0; i <= dma_reg[2].length.sd; i++) {
				write_dma_io8(0, read_dma_data8(addr++));
			}
			dma_status |= 4;
		}
		vblank = true;
		request_intr(IRQ_VRTC, true);
		update_gvram_wait();
	}
}

void PC88::key_down(int code, bool repeat)
{
	if(!repeat) {
		if(code == 0x14) {
			caps ^= 1;
		}
		else if(code == 0x15) {
			kana ^= 1;
		}
	}
}

void PC88::draw_screen()
{
	memset(text, 0, sizeof(text));
	memset(graph, 0, sizeof(graph));
	bool attribs_expanded = false;
	
	// render text screen
	if(!(disp_ctrl & 1) && (crtc_status & 0x10)) {
		expand_attribs();
		draw_text();
		attribs_expanded = true;
	}
	
	// render graph screen
	if(graph_mode & 8) {
		if(graph_mode & 0x10) {
			draw_color_graph();
		}
		else if(line200) {
			draw_mono_graph();
		}
		else {
			if(text_mode & 2) {
				memset(attribs, 0xe0, sizeof(attribs));
			}
			else if(!attribs_expanded) {
				expand_attribs();
			}
			draw_mono_hires_graph();
		}
	}
	
	// update palette
	if(update_palette) {
		if(graph_mode & 0x10) {
			for(int i = 0; i < 8; i++) {
				palette_graph_pc[i] = RGB_COLOR(palette[i].r << 5, palette[i].g << 5, palette[i].b << 5);
			}
		}
		else {
			palette_graph_pc[0] = RGB_COLOR(palette[8].r << 5, palette[8].g << 5, palette[8].b << 5);
			palette_graph_pc[1] = RGB_COLOR(255, 255, 255);
		}
		update_palette = false;
	}
	
	// copy to screen buffer
	if(line200) {
		for(int y = 0; y < 200; y++) {
			scrntype* dest0 = emu->screen_buffer(y * 2);
			scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
			uint8* src_t = text[y];
			uint8* src_g = graph[y];
			
			for(int x = 0; x < 640; x++) {
				uint8 t = src_t[x];
				dest0[x] = t ? palette_text_pc[t] : palette_graph_pc[src_g[x]];
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype));
			}
			else {
				for(int x = 0; x < 640; x++) {
					dest1[x] = dest0[x];
				}
//				memcpy(dest1, dest0, 640 * sizeof(scrntype));
			}
		}
	}
	else {
		for(int y = 0; y < 400; y++) {
			scrntype* dest = emu->screen_buffer(y);
			uint8* src_t = text[y >> 1];
			uint8* src_g = graph[y];
			
			for(int x = 0; x < 640; x++) {
				uint8 t = src_t[x];
				dest[x] = palette_text_pc[t ? t : src_g[x]];
			}
		}
	}
}

uint8 PC88::get_crtc_buffer(int ofs)
{
	if(ofs < crtc_buffer_ptr) {
		return crtc_buffer[ofs];
	}
	// dma underrun occurs !!!
	crtc_status |= 8;
//	crtc_status &= ~0x10;
	return 0;
}

void PC88::expand_attribs()
{
	if(attrib_num == 0) {
		if(text_mode & 2) {
			memset(attribs, 0, sizeof(attribs));
		}
		else {
			memset(attribs, 0xe0, sizeof(attribs));
		}
		return;
	}
	
	int char_height_tmp = char_height;
	
	if(!line200 || !skip_line) {
		char_height_tmp >>= 1;
	}
	for(int cy = 0, ytop = 0, ofs = 0; cy < text_height && ytop < 200; cy++, ytop += char_height_tmp, ofs += 80 + attrib_num * 2) {
		if(attrib_num != 0) {
			if(attrib_num == 20 && crtc_buffer[ofs + 80] == 0 && crtc_buffer[ofs + 81] == 120 && crtc_buffer[ofs + 82] == 1 && crtc_buffer[ofs + 83] == 2) {
				// XXX: ugly patch for alpha :-(
				memset(attribs[cy], 0, 80);
			}
			else {
				uint8 flags[128];
				memset(flags, 0, sizeof(flags));
				for(int i = 2 * (attrib_num - 1); i >= 0; i -= 2) {
					flags[crtc_buffer[ofs + i + 80] & 0x7f] = 1;
				}
				for(int cx = 0, pos = 0; cx < text_width && cx < 80; cx++) {
					if(flags[cx]) {
						text_attrib = crtc_buffer[ofs + pos + 81];
						pos += 2;
					}
					attribs[cy][cx] = text_attrib;
				}
			}
		}
		
	}
}

void PC88::draw_text()
{
	bool cursor_draw = cursor_on && !(cursor_blink && !blink_on);
	int char_height_tmp = char_height;
	
	if(!line200 || !skip_line) {
		char_height_tmp >>= 1;
	}
	
	crtc_status &= ~8; // clear dma underrun
	
	for(int cy = 0, ytop = 0, ofs = 0; cy < text_height && ytop < 200; cy++, ytop += char_height_tmp, ofs += 80 + attrib_num * 2) {
		for(int x = 0, cx = 0; cx < text_width && cx < 80; x += 8, cx++) {
			if(!(text_mode & 1) && (cx & 1)) {
				continue;
			}
			uint8 code = get_crtc_buffer(ofs + cx);
			uint8 attrib = attribs[cy][cx];
			
			bool under_line = false, upper_line = false, reverse = false, blink = false, secret = false;
			uint8 color = 7;
			uint8 *pattern = kanji1 + 0x1000 + code * 8;
			
			if(!(text_mode & 2) && (attrib & 8)) {
				if(attrib & 0x10) {
					pattern = sg_pattern + code * 8;
				}
				color = (attrib & 0xe0) ? (attrib >> 5) : 8;
			}
			else {
				if(attrib & 0x80) {
					pattern = sg_pattern + code * 8;
				}
				under_line = ((attrib & 0x20) != 0);
				upper_line = ((attrib & 0x10) != 0);
				reverse = ((attrib & 4) != 0);
				blink = ((attrib & 2) != 0) && blink_on;
				secret = ((attrib & 1) != 0);
			}
			bool cursor_now = (cursor_draw && cx == cursor_x && cy == cursor_y);
			
			for(int l = 0, y = ytop; l < char_height_tmp && y < 200; l++, y++) {
				uint8 pat = (l < 8) ? pattern[l] : 0;
				if(secret) {
					pat = 0;
				}
				if((upper_line && l == 0) || (under_line && l >= 7)) {
					pat = 0xff;
				}
				if(blink || reverse || (cursor_now && l >= cursor_line)) {
					pat = ~pat;
				}
				
				uint8 *dest = &text[y][x];
				if(text_mode & 1) {
					dest[0] = (pat & 0x80) ? color : 0;
					dest[1] = (pat & 0x40) ? color : 0;
					dest[2] = (pat & 0x20) ? color : 0;
					dest[3] = (pat & 0x10) ? color : 0;
					dest[4] = (pat & 0x08) ? color : 0;
					dest[5] = (pat & 0x04) ? color : 0;
					dest[6] = (pat & 0x02) ? color : 0;
					dest[7] = (pat & 0x01) ? color : 0;
				}
				else {
					dest[ 0] = dest[ 1] = (pat & 0x80) ? color : 0;
					dest[ 2] = dest[ 3] = (pat & 0x40) ? color : 0;
					dest[ 4] = dest[ 5] = (pat & 0x20) ? color : 0;
					dest[ 6] = dest[ 7] = (pat & 0x10) ? color : 0;
					dest[ 8] = dest[ 9] = (pat & 0x08) ? color : 0;
					dest[10] = dest[11] = (pat & 0x04) ? color : 0;
					dest[12] = dest[13] = (pat & 0x02) ? color : 0;
					dest[14] = dest[15] = (pat & 0x01) ? color : 0;
				}
			}
		}
		if(crtc_status & 8) {
			// dma underrun occurs !!!
			memset(text, 0, sizeof(text));
			break;
		}
	}
}

void PC88::draw_color_graph()
{
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8 b = gvram[addr | 0x0000];
			uint8 r = gvram[addr | 0x4000];
			uint8 g = gvram[addr | 0x8000];
			addr++;
			uint8 *dest = &graph[y][x];
			dest[0] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
			dest[1] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
			dest[2] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
			dest[3] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
			dest[4] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
			dest[5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04)     );
			dest[6] = ((b & 0x02) >> 1) | ((r & 0x02)     ) | ((g & 0x02) << 1);
			dest[7] = ((b & 0x01)     ) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
		}
	}
}

void PC88::draw_mono_graph()
{
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8 b = (disp_ctrl & 2) ? 0 : gvram[addr | 0x0000];
			uint8 r = (disp_ctrl & 4) ? 0 : gvram[addr | 0x4000];
			uint8 g = (disp_ctrl & 8) ? 0 : gvram[addr | 0x8000];
			addr++;
			uint8 *dest = &graph[y][x];
			dest[0] = ((b | r | g) & 0x80) >> 7;
			dest[1] = ((b | r | g) & 0x40) >> 6;
			dest[2] = ((b | r | g) & 0x20) >> 5;
			dest[3] = ((b | r | g) & 0x10) >> 4;
			dest[4] = ((b | r | g) & 0x08) >> 3;
			dest[5] = ((b | r | g) & 0x04) >> 2;
			dest[6] = ((b | r | g) & 0x02) >> 1;
			dest[7] = ((b | r | g) & 0x01)     ;
		}
	}
}

void PC88::draw_mono_hires_graph()
{
	int char_height_tmp = char_height ? char_height : 16;
	int shift = (text_mode & 1) ? 0 : 1;
	
	for(int y = 0, addr = 0; y < 200; y++) {
		int cy = y / char_height_tmp;
		for(int x = 0, cx = 0; x < 640; x += 8, cx++) {
			uint8 b = (disp_ctrl & 2) ? 0 : gvram[addr | 0x0000];
			uint8 c = attribs[cy][cx >> shift] >> 5;
			addr++;
			uint8 *dest = &graph[y][x];
			dest[0] = (b & 0x80) ? c : 0;
			dest[1] = (b & 0x40) ? c : 0;
			dest[2] = (b & 0x20) ? c : 0;
			dest[3] = (b & 0x10) ? c : 0;
			dest[4] = (b & 0x08) ? c : 0;
			dest[5] = (b & 0x04) ? c : 0;
			dest[6] = (b & 0x02) ? c : 0;
			dest[7] = (b & 0x01) ? c : 0;
		}
	}
	for(int y = 200, addr = 0; y < 400; y++) {
		int cy = y / char_height_tmp;
		for(int x = 0, cx = 0; x < 640; x += 8, cx++) {
			uint8 r = (disp_ctrl & 4) ? 0 : gvram[addr | 0x4000];
			uint8 c = attribs[cy][cx >> shift] >> 5;
			addr++;
			uint8 *dest = &graph[y][x];
			dest[0] = (r & 0x80) ? c : 0;
			dest[1] = (r & 0x40) ? c : 0;
			dest[2] = (r & 0x20) ? c : 0;
			dest[3] = (r & 0x10) ? c : 0;
			dest[4] = (r & 0x08) ? c : 0;
			dest[5] = (r & 0x04) ? c : 0;
			dest[6] = (r & 0x02) ? c : 0;
			dest[7] = (r & 0x01) ? c : 0;
		}
	}
}

void PC88::request_intr(int level, bool status)
{
	uint8 bit = 1 << level;
	
	if(status) {
//		if(!(intr_req & bit)) {
			intr_req |= bit;
			update_intr();
//		}
	}
	else {
//		if(intr_req & bit) {
			intr_req &= ~bit;
			update_intr();
//		}
	}
}

void PC88::update_intr()
{
	if(intr_req & intr_mask1 & intr_mask2) {
		d_cpu->set_intr_line(true, true, 0);
	}
	else {
		d_cpu->set_intr_line(false, true, 0);
	}
}

uint32 PC88::intr_ack()
{
	for(int i = 0; i < 8; i++) {
		uint8 bit = 1 << i;
		if(intr_req & intr_mask1 & intr_mask2 & bit) {
			intr_req &= ~bit;
			intr_mask1 = 0;
			return i * 2;
		}
	}
	return 0;
}

