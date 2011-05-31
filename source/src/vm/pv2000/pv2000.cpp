/*
	CASIO PV-2000 Emulator 'EmuGaki'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ virtual machine ]
*/

#include "pv2000.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../io.h"
#include "../memory.h"
#include "../sn76489an.h"
#include "../tms9918a.h"
#include "../z80.h"

#include "cmt.h"
#include "keyboard.h"
#include "printer.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	event->initialize();		// must be initialized first
	
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	psg = new SN76489AN(this, emu);
	vdp = new TMS9918A(this, emu);
	cpu = new Z80(this, emu);
	
	cmt = new CMT(this, emu);
	key = new KEYBOARD(this, emu);
	prt = new PRINTER(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	
	vdp->set_context_irq(cpu, SIG_CPU_NMI, 1);
	key->set_context_cpu(cpu);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
	
	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(ext, 0, sizeof(ext));
	memset(ipl, 0xff, sizeof(ipl));
	memset(cart, 0xff, sizeof(cart));
	
	memory->read_bios(_T("IPL.ROM"), ipl, sizeof(ipl));
	
	memory->set_memory_r(0x0000, 0x3fff, ipl);
	memory->set_memory_mapped_io_rw(0x4000, 0x4fff, vdp);
	memory->set_memory_rw(0x7000, 0x7fff, ram);
	memory->set_memory_rw(0x8000, 0xbfff, ext);
	memory->set_memory_r(0xc000, 0xffff, cart);
	
	// i/o bus
	io->set_iomap_single_w(0x00, cmt);
	io->set_iomap_single_r(0x10, key);
	io->set_iomap_single_rw(0x20, key);
	io->set_iomap_single_r(0x40, key);
	io->set_iomap_single_w(0x40, psg);
	io->set_iomap_single_rw(0x60, cmt);
	io->set_iomap_single_w(0x80, prt);
	io->set_iomap_single_r(0x90, prt);
	io->set_iomap_single_w(0xa0, prt);
	io->set_iomap_single_w(0xb0, prt);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id != event->this_device_id) {
			device->initialize();
		}
	}
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->release();
	}
}

DEVICE* VM::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id == id) {
			return device;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
}

void VM::run()
{
	event->drive();
}

// ----------------------------------------------------------------------------
// event manager
// ----------------------------------------------------------------------------

void VM::register_event(DEVICE* dev, int event_id, int usec, bool loop, int* register_id)
{
	event->register_event(dev, event_id, usec, loop, register_id);
}

void VM::register_event_by_clock(DEVICE* dev, int event_id, int clock, bool loop, int* register_id)
{
	event->register_event_by_clock(dev, event_id, clock, loop, register_id);
}

void VM::cancel_event(int register_id)
{
	event->cancel_event(register_id);
}

void VM::register_frame_event(DEVICE* dev)
{
	event->register_frame_event(dev);
}

void VM::register_vline_event(DEVICE* dev)
{
	event->register_vline_event(dev);
}

uint32 VM::current_clock()
{
	return event->current_clock();
}

uint32 VM::passed_clock(uint32 prev)
{
	uint32 current = event->current_clock();
	return (current > prev) ? current - prev : current + (0xffffffff - prev) + 1;
}

uint32 VM::get_prv_pc()
{
	return cpu->get_prv_pc();
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	vdp->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	psg->init(rate, 3579545, 8000);
}

uint16* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	key->key_down(code);
}

void VM::key_up(int code)
{
	key->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(_TCHAR* file_path)
{
	memset(cart, 0xff, sizeof(cart));
	memory->read_image(file_path, cart, sizeof(cart));
	reset();
}

void VM::close_cart()
{
	memset(cart, 0xff, sizeof(cart));
	reset();
}

void VM::play_datarec(_TCHAR* file_path)
{
	cmt->play_datarec(file_path);
}

void VM::rec_datarec(_TCHAR* file_path)
{
	cmt->rec_datarec(file_path);
}

void VM::close_datarec()
{
	cmt->close_datarec();
}

bool VM::now_skip()
{
	return cmt->skip();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

