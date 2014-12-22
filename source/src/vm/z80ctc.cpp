/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ Z80CTC ]
*/

#include "z80ctc.h"
#include "../fileio.h"

#define EVENT_COUNTER	0
#define EVENT_TIMER	4

void Z80CTC::reset()
{
	for(int ch = 0; ch < 4; ch++) {
		counter[ch].count = counter[ch].constant = 256;
		counter[ch].clocks = 0;
		counter[ch].control = 0;
		counter[ch].slope = false;
		counter[ch].prescaler = 256;
		counter[ch].freeze = counter[ch].start = counter[ch].latch = false;
		counter[ch].clock_id = counter[ch].sysclock_id = -1;
		counter[ch].first_constant = true;
		// interrupt
		counter[ch].req_intr = false;
		counter[ch].in_service = false;
		counter[ch].vector = ch << 1;
	}
	iei = oei = true;
}

void Z80CTC::write_io8(uint32 addr, uint32 data)
{
	int ch = addr & 3;
	if(counter[ch].latch) {
		// time constant
		counter[ch].constant = data ? data : 256;
		counter[ch].latch = false;
		if(counter[ch].freeze || counter[ch].first_constant) {
			counter[ch].count = counter[ch].constant;
			counter[ch].clocks = 0;
			counter[ch].freeze = false;
			counter[ch].first_constant = false;
			update_event(ch, 0);
		}
	} else {
		if(data & 1) {
			// control word
			counter[ch].prescaler = (data & 0x20) ? 256 : 16;
			counter[ch].latch = ((data & 4) != 0);
			counter[ch].freeze = ((data & 2) != 0);
			counter[ch].start = (counter[ch].freq || !(data & 8));
			counter[ch].control = data;
			counter[ch].slope = ((data & 0x10) != 0);
			if(!(data & 0x80) && counter[ch].req_intr) {
				counter[ch].req_intr = false;
				update_intr();
			}
			update_event(ch, 0);
		} else if(ch == 0) {
			// vector
			counter[0].vector = (data & 0xf8) | 0;
			counter[1].vector = (data & 0xf8) | 2;
			counter[2].vector = (data & 0xf8) | 4;
			counter[3].vector = (data & 0xf8) | 6;
		}
	}
}

uint32 Z80CTC::read_io8(uint32 addr)
{
	int ch = addr & 3;
	// update counter
	if(counter[ch].clock_id != -1) {
		int passed = passed_clock(counter[ch].prev);
		uint32 input = (uint32)(counter[ch].freq * passed / cpu_clocks);
		if(counter[ch].input <= input) {
			input = counter[ch].input - 1;
		}
		if(input > 0) {
			input_clock(ch, input);
			// cancel and re-register event
			cancel_event(this, counter[ch].clock_id);
			counter[ch].input -= input;
			counter[ch].period -= passed;
			counter[ch].prev = current_clock();
			register_event_by_clock(this, EVENT_COUNTER + ch, counter[ch].period, false, &counter[ch].clock_id);
		}
	} else if(counter[ch].sysclock_id != -1) {
		int passed = passed_clock(counter[ch].prev);
#ifdef Z80CTC_CLOCKS
		uint32 input = (uint32)(passed * Z80CTC_CLOCKS / cpu_clocks);
#else
		uint32 input = passed;
#endif
		if(counter[ch].input <= input) {
			input = counter[ch].input - 1;
		}
		if(input > 0) {
			input_sysclock(ch, input);
			// cancel and re-register event
			cancel_event(this, counter[ch].sysclock_id);
			counter[ch].input -= passed;
			counter[ch].period -= passed;
			counter[ch].prev = current_clock();
			register_event_by_clock(this, EVENT_TIMER + ch, counter[ch].period, false, &counter[ch].sysclock_id);
		}
	}
	return counter[ch].count & 0xff;
}

void Z80CTC::event_callback(int event_id, int err)
{
	int ch = event_id & 3;
	if(event_id & 4) {
		input_sysclock(ch, counter[ch].input);
		counter[ch].sysclock_id = -1;
	} else {
		input_clock(ch, counter[ch].input);
		counter[ch].clock_id = -1;
	}
	update_event(ch, err);
}

void Z80CTC::write_signal(int id, uint32 data, uint32 mask)
{
	int ch = id & 3;
#if 1
	if(data & mask) {
		input_clock(ch, 1);
		update_event(ch, 0);
	}
#else
	// more correct implements...
	bool next = ((data & mask) != 0);
	if(counter[ch].prev_in != next) {
		if(counter[ch].slope == next) {
			input_clock(ch, 1);
			update_event(ch, 0);
		}
		counter[ch].prev_in = next;
	}
#endif
}

void Z80CTC::input_clock(int ch, int clock)
{
	if(!(counter[ch].control & 0x40)) {
		// now timer mode, start timer and quit !!!
		counter[ch].start = true;
		return;
	}
	if(counter[ch].freeze) {
		return;
	}
	
	// update counter
	counter[ch].count -= clock;
	while(counter[ch].count <= 0) {
		counter[ch].count += counter[ch].constant;
		if(counter[ch].control & 0x80) {
			counter[ch].req_intr = true;
			update_intr();
		}
		write_signals(&counter[ch].outputs, 0xffffffff);
		write_signals(&counter[ch].outputs, 0);
	}
}

void Z80CTC::input_sysclock(int ch, int clock)
{
	if(counter[ch].control & 0x40) {
		// now counter mode, quit !!!
		return;
	}
	if(!counter[ch].start || counter[ch].freeze) {
		return;
	}
	counter[ch].clocks += clock;
	int input = counter[ch].clocks >> (counter[ch].prescaler == 256 ? 8 : 4);
	counter[ch].clocks &= counter[ch].prescaler - 1;
	
	// update counter
	counter[ch].count -= input;
	while(counter[ch].count <= 0) {
		counter[ch].count += counter[ch].constant;
		if(counter[ch].control & 0x80) {
			counter[ch].req_intr = true;
			update_intr();
		}
		write_signals(&counter[ch].outputs, 0xffffffff);
		write_signals(&counter[ch].outputs, 0);
	}
}

void Z80CTC::update_event(int ch, int err)
{
	if(counter[ch].control & 0x40) {
		// counter mode
		if(counter[ch].sysclock_id != -1) {
			cancel_event(this, counter[ch].sysclock_id);
		}
		counter[ch].sysclock_id = -1;
		
		if(counter[ch].freeze) {
			if(counter[ch].clock_id != -1) {
				cancel_event(this, counter[ch].clock_id);
			}
			counter[ch].clock_id = -1;
			return;
		}
		if(counter[ch].clock_id == -1 && counter[ch].freq) {
			counter[ch].input = counter[ch].count;
			counter[ch].period = (uint32)(cpu_clocks * counter[ch].input / counter[ch].freq) + err;
			counter[ch].prev = current_clock() + err;
			register_event_by_clock(this, EVENT_COUNTER + ch, counter[ch].period, false, &counter[ch].clock_id);
		}
	} else {
		// timer mode
		if(counter[ch].clock_id != -1) {
			cancel_event(this, counter[ch].clock_id);
		}
		counter[ch].clock_id = -1;
		
		if(!counter[ch].start || counter[ch].freeze) {
			if(counter[ch].sysclock_id != -1) {
				cancel_event(this, counter[ch].sysclock_id);
			}
			counter[ch].sysclock_id = -1;
			return;
		}
		if(counter[ch].sysclock_id == -1) {
			counter[ch].input = counter[ch].count * counter[ch].prescaler - counter[ch].clocks;
#ifdef Z80CTC_CLOCKS
			counter[ch].period = (uint32)(counter[ch].input * cpu_clocks / Z80CTC_CLOCKS) + err;
#else
			counter[ch].period = counter[ch].input + err;
#endif
			counter[ch].prev = current_clock() + err;
			register_event_by_clock(this, EVENT_TIMER + ch, counter[ch].period, false, &counter[ch].sysclock_id);
		}
	}
}

void Z80CTC::set_intr_iei(bool val)
{
	if(iei != val) {
		iei = val;
		update_intr();
	}
}

#define set_intr_oei(val) { \
	if(oei != val) { \
		oei = val; \
		if(d_child) { \
			d_child->set_intr_iei(oei); \
		} \
	} \
}

void Z80CTC::update_intr()
{
	bool next;
	
	// set oei signal
	if((next = iei) == true) {
		for(int ch = 0; ch < 4; ch++) {
			if(counter[ch].in_service) {
				next = false;
				break;
			}
		}
	}
	set_intr_oei(next);
	
	// set int signal
	if((next = iei) == true) {
		next = false;
		for(int ch = 0; ch < 4; ch++) {
			if(counter[ch].in_service) {
				break;
			}
			if(counter[ch].req_intr) {
				next = true;
				break;
			}
		}
	}
	if(d_cpu) {
		d_cpu->set_intr_line(next, true, intr_bit);
	}
}

uint32 Z80CTC::intr_ack()
{
	// ack (M1=IORQ=L)
	for(int ch = 0; ch < 4; ch++) {
		if(counter[ch].in_service) {
			// invalid interrupt status
			return 0xff;
		} else if(counter[ch].req_intr) {
			counter[ch].req_intr = false;
			counter[ch].in_service = true;
			update_intr();
			return counter[ch].vector;
		}
	}
	if(d_child) {
		return d_child->intr_ack();
	}
	return 0xff;
}

void Z80CTC::intr_reti()
{
	// detect RETI
	for(int ch = 0; ch < 4; ch++) {
		if(counter[ch].in_service) {
			counter[ch].in_service = false;
			counter[ch].req_intr = false; // ???
			update_intr();
			return;
		}
	}
	if(d_child) {
		d_child->intr_reti();
	}
}

#define STATE_VERSION	1

void Z80CTC::save_state(FILEIO* fio)
{
	fio->FputUint32(STATE_VERSION);
	fio->FputInt32(this_device_id);
	
	for(int i = 0; i < 4; i++) {
		fio->FputUint8(counter[i].control);
		fio->FputBool(counter[i].slope);
		fio->FputUint16(counter[i].count);
		fio->FputUint16(counter[i].constant);
		fio->FputUint8(counter[i].vector);
		fio->FputInt32(counter[i].clocks);
		fio->FputInt32(counter[i].prescaler);
		fio->FputBool(counter[i].freeze);
		fio->FputBool(counter[i].start);
		fio->FputBool(counter[i].latch);
		fio->FputBool(counter[i].prev_in);
		fio->FputBool(counter[i].first_constant);
		fio->FputUint64(counter[i].freq);
		fio->FputInt32(counter[i].clock_id);
		fio->FputInt32(counter[i].sysclock_id);
		fio->FputUint32(counter[i].input);
		fio->FputUint32(counter[i].period);
		fio->FputUint32(counter[i].prev);
		fio->FputBool(counter[i].req_intr);
		fio->FputBool(counter[i].in_service);
	}
	fio->FputUint64(cpu_clocks);
	fio->FputBool(iei);
	fio->FputBool(oei);
	fio->FputUint32(intr_bit);
}

bool Z80CTC::load_state(FILEIO* fio)
{
	if(fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < 4; i++) {
		counter[i].control = fio->FgetUint8();
		counter[i].slope = fio->FgetBool();
		counter[i].count = fio->FgetUint16();
		counter[i].constant = fio->FgetUint16();
		counter[i].vector = fio->FgetUint8();
		counter[i].clocks = fio->FgetInt32();
		counter[i].prescaler = fio->FgetInt32();
		counter[i].freeze = fio->FgetBool();
		counter[i].start = fio->FgetBool();
		counter[i].latch = fio->FgetBool();
		counter[i].prev_in = fio->FgetBool();
		counter[i].first_constant = fio->FgetBool();
		counter[i].freq = fio->FgetUint64();
		counter[i].clock_id = fio->FgetInt32();
		counter[i].sysclock_id = fio->FgetInt32();
		counter[i].input = fio->FgetUint32();
		counter[i].period = fio->FgetUint32();
		counter[i].prev = fio->FgetUint32();
		counter[i].req_intr = fio->FgetBool();
		counter[i].in_service = fio->FgetBool();
	}
	cpu_clocks = fio->FgetUint64();
	iei = fio->FgetBool();
	oei = fio->FgetBool();
	intr_bit = fio->FgetUint32();
	return true;
}

