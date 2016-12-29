/*
	COLECO ColecoVision Emulator 'yaCOLECOVISION'

	Author : tanam
	Date   : 2016.08.14-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_cpu;
	
	const uint8_t* key_stat;
	const uint32_t* joy_stat;
	bool tenkey;
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void event_frame();
	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);	

	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
};

#endif
