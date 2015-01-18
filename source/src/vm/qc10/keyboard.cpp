/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.13 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../z80sio.h"
#include "../../fileio.h"

/*
	F1-F10				F1-F10
	BREAK,PAUSE,HELP,SCRN		F11,F12,PU,PD
	SF1-SF4				SHIFT + F1-F4?
	ESC,TAB,CTRL,SHIFT,CAPS		ESC,TAB,CTRL,SHIFT,CAPS
	BS,LF,ENTER,SHIFT,GRAPH		BS,SHIFT + ENTER,ENTER,SHIFT,GRAPH
	HOME,CLS,INS,DEL		HOME,END,INS,DEL
	NUMPAD				

	SHIFT + ENTER	-> VK_F20
	SHIFT + F1	-> VK_F21
	SHIFT + F2	-> VK_F22
	SHIFT + F3	-> VK_F23
	SHIFT + F4	-> VK_F24
*/

// pc key code -> qc-10 key code
static const int key_map[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x6d,0x77,0x00,0x00, 0x00,0x4e,0x00,0x00,	// 0
	0x87,0x8b,0x8d,0x00, 0x31,0x00,0x00,0x00, 0x00,0x00,0x00,0x75, 0x00,0x00,0x00,0x00,	// 1
	0x32,0x0c,0x0b,0x6f, 0x6e,0x3d,0x3c,0x3e, 0x3f,0x00,0x00,0x00, 0x00,0x5e,0x5f,0x00,	// 2
	0x69,0x76,0x61,0x62, 0x63,0x64,0x65,0x66, 0x67,0x68,0x00,0x00, 0x00,0x00,0x00,0x00,	// 3
	0x00,0x43,0x37,0x35, 0x45,0x53,0x46,0x47, 0x48,0x58,0x49,0x4a, 0x4b,0x39,0x38,0x59,	// 4
	0x5a,0x51,0x54,0x44, 0x55,0x57,0x36,0x52, 0x34,0x56,0x33,0x00, 0x00,0x00,0x00,0x00,	// 5
	0x17,0x27,0x26,0x25, 0x1b,0x1a,0x19,0x2b, 0x2a,0x29,0x2d,0x18, 0x2f,0x28,0x16,0x2e,	// 6
	0x73,0x72,0x71,0x01, 0x02,0x03,0x04,0x05, 0x06,0x07,0x09,0x0a, 0x00,0x00,0x00,0x00,	// 7
	0x00,0x00,0x00,0x11, 0x0f,0x1f,0x1e,0x1d, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 8
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 9
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// a
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x4d,0x4c, 0x3a,0x6a,0x3b,0x4f,	// b
	0x5b,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// c
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x5c, 0x6c,0x5d,0x6b,0x00,	// d
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// e
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00	// f
};

void KEYBOARD::initialize()
{
	for(int i = 0; i < 8; i++) {
		led[i] = false;
	}
	repeat = enable = true;
	key_stat = emu->key_buffer();
}

void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	// rec command
	process_cmd(data & 0xff);
}

void KEYBOARD::key_down(int code)
{
	if(enable) {
		if(key_stat[0x10]) {
			if(code == 0x0d) code = 0x83;	// SHIFT + ENTER
			if(code == 0x70) code = 0x84;	// SHIFT + F1
			if(code == 0x71) code = 0x85;	// SHIFT + F2
			if(code == 0x72) code = 0x86;	// SHIFT + F3
			if(code == 0x73) code = 0x87;	// SHIFT + F4
		}
		if(code = key_map[code]) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, code, 0xff);
		}
	}
}

void KEYBOARD::key_up(int code)
{
	if(enable) {
		// key break
		if(code == 0x10) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x86, 0xff);	// shift break
		} else if(code == 0x11) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8a, 0xff);	// ctrl break
		} else if(code == 0x12) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8c, 0xff);	// graph break
		}
	}
}

void KEYBOARD::process_cmd(uint8 val)
{
	switch(val & 0xe0) {
	case 0x00:
		// repeat starting time set:
		break;
	case 0x20:
		// repeat interval set
		break;
	case 0xa0:
		// repeat control
		repeat = ((val & 1) != 0);
		break;
	case 0x40:
		// key_led control
		led[(val >> 1) & 7] = ((val & 1) != 0);
		break;
	case 0x60:
		// key_led status read
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH0, 1, 1);
		for(int i = 0; i < 8; i++) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0xc0 | (i << 1) | (led[i] ? 1: 0), 0xff);
		}
		break;
	case 0x80:
		// key sw status read
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH0, 1, 1);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x80, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x82, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x84, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x86 | (key_stat[0x10] ? 1: 0), 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x88, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8a | (key_stat[0x11] ? 1: 0), 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8c | (key_stat[0x12] ? 1: 0), 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8e, 0xff);
		break;
	case 0xc0:
		// keyboard enable
		enable = ((val & 1) != 0);
		break;
	case 0xe0:
		// reset
		for(int i = 0; i < 8; i++) {
			led[i] = false;
		}
		repeat = enable = true;
		// diagnosis
		if(!(val & 1)) {
			d_sio->write_signal(SIG_Z80SIO_CLEAR_CH0, 1, 1);
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0, 0xff);
		}
		break;
	}
}

#define STATE_VERSION	1

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(led, sizeof(led), 1);
	state_fio->FputBool(repeat);
	state_fio->FputBool(enable);
}

bool KEYBOARD::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(led, sizeof(led), 1);
	repeat = state_fio->FgetBool();
	enable = state_fio->FgetBool();
	return true;
}

