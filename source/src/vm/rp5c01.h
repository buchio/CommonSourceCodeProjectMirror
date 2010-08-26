/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.11-

	[ RP-5C01 ]
*/

#ifndef _RP5C01_H_
#define _RP5C01_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class RP5C01 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_alarm;
	outputs_t outputs_pulse;
	
	uint8 regs[16];
	uint8 ram[26];
	int time[8];
	bool alarm, pulse_1hz, pulse_16hz;
	
public:
	RP5C01(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		init_output_signals(&outputs_alarm);
		init_output_signals(&outputs_pulse);
	}
	~RP5C01() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void event_frame();
	
	// unique functions
	void set_context_alarm(DEVICE* device, int id, uint32 mask) {
		regist_output_signal(&outputs_alarm, device, id, mask);
	}
	void set_context_pulse(DEVICE* device, int id, uint32 mask) {
		regist_output_signal(&outputs_pulse, device, id, mask);
	}
};

#endif

