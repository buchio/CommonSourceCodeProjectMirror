/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2017.06.22-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

class DISPLAY;

class MEMBUS : public MEMORY
{
private:
	DISPLAY *d_display;
	
#if defined(SUPPORT_32BIT_ADDRESS)
	uint8_t ram[0x800000]; // 8MB
#elif defined(SUPPORT_24BIT_ADDRESS)
	uint8_t ram[0x400000]; // 4MB
#else
	uint8_t ram[0x100000]; // 1MB
#endif
#if !defined(SUPPORT_HIRESO)
	uint8_t ipl[0x18000];
#else
	uint8_t ipl[0x10000];
#endif
#if defined(SUPPORT_ITF_ROM)
	uint8_t itf[0x8000];
	bool itf_selected;
#endif
#if defined(_PC9801) || defined(_PC9801E)
	uint8_t fd_bios_2hd[0x1000];
	uint8_t fd_bios_2dd[0x1000];
#endif
#if !defined(SUPPORT_HIRESO)
#if defined(SUPPORT_SASI)
	uint8_t sasi_bios[0x1000];
#endif
	uint8_t sound_bios[0x4000];
#endif
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	uint8_t dma_access_ctrl;
	uint32_t window_80000h;
	uint32_t window_a0000h;
#endif
	
public:
	MEMBUS(VM* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);
	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data32(uint32_t addr);
	void write_data32(uint32_t addr, uint32_t data);
	uint32_t read_dma_data8(uint32_t addr);
	void write_dma_data8(uint32_t addr, uint32_t data);
	uint32_t read_dma_data16(uint32_t addr);
	void write_dma_data16(uint32_t addr, uint32_t data);
	uint32_t read_dma_data32(uint32_t addr);
	void write_dma_data32(uint32_t addr, uint32_t data);
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_display(DISPLAY* device)
	{
		d_display = device;
	}
};

#endif
