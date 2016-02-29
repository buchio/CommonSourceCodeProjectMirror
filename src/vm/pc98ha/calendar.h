/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.08.14 -

	[ calendar ]
*/

#ifndef _CALENDAR_H_
#define _CALENDAR_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class CALENDAR : public DEVICE
{
private:
	DEVICE *d_rtc;
#ifdef _PC98HA
	uint8_t ch;
#endif
	
public:
	CALENDAR(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CALENDAR() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#ifdef _PC98HA
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
#endif
	
	// unique function
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
};

#endif

