/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ virtual machine ]
*/

#ifndef _FMR30_H_
#define _FMR30_H_

#define DEVICE_NAME		"FUJITSU FMR-30"
#define CONFIG_NAME		"fmr30"

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CPU_CLOCKS		8000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		2
#define MAX_SCSI		8
#define MAX_MEMCARD		2
//#define HAS_I286
#define HAS_I86
#define I86_PSEUDO_BIOS
#define HAS_I8254
#define I8259_MAX_CHIPS		2
//#define SINGLE_MODE_DMA
#define MB8877_NO_BUSY_AFTER_SEEK
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_ACCESS_LAMP
#define USE_SOUND_VOLUME	1
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8237;
class I8251;
class I8253;
class I8259;
class I286;
class IO;
class MB8877;
class SN76489AN;

class BIOS;
class CMOS;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class RTC;
//class SCSI;
class SERIAL;
class SYSTEM;
class TIMER;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8237* dma;
	I8251* sio_kb;
	I8251* sio_sub;
	I8251* sio_ch1;
	I8251* sio_ch2;
	I8253* pit;
	I8259* pic;
	I286* cpu;
	IO* io;
	MB8877* fdc;
	SN76489AN* psg;
	
	BIOS* bios;
	CMOS* cmos;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	RTC* rtc;
//	SCSI* scsi;
	SERIAL* serial;
	SYSTEM* system;
	TIMER* timer;
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	void run();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	int access_lamp();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void set_disk_protected(int drv, bool value);
	bool get_disk_protected(int drv);
	bool now_skip();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
