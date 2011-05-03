/*
	SEGA SC-3000 Emulator 'eSC-3000'
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.17-

	[ memory ]
*/

#include "memory.h"
#include "../../fileio.h"

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

void MEMORY::initialize()
{
	// load ipl
	_memset(cart, 0xff, sizeof(cart));
	_memset(ipl, 0xff, sizeof(ipl));
	_memset(ram, 0, sizeof(ram));
	_memset(rdmy, 0xff, sizeof(rdmy));
	
	_TCHAR app_path[_MAX_PATH], file_path[_MAX_PATH];
	emu->application_path(app_path);
	FILEIO* fio = new FILEIO();
	
	_stprintf(file_path, _T("%sSF7000.ROM"), app_path);
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, wdmy, ipl);
	SET_BANK(0x2000, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0xffff, ram + 0x4000, ram + 0x4000);
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(data & mask) {
		SET_BANK(0x0000, 0x3fff, ram, ram);
	}
	else {
		// ROM
		SET_BANK(0x0000, 0x1fff, wdmy, ipl);
		SET_BANK(0x2000, 0x3fff, wdmy, rdmy);
	}
}

void MEMORY::open_cart(_TCHAR* filename)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(filename, FILEIO_READ_BINARY)) {
		_memset(cart, 0xff, sizeof(cart));
		fio->Fread(cart, sizeof(cart), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x7fff, wdmy, cart);
	SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
}

void MEMORY::close_cart()
{
	_memset(cart, 0xff, sizeof(cart));
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, wdmy, ipl);
	SET_BANK(0x2000, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0xffff, ram + 0x4000, ram + 0x4000);
}
