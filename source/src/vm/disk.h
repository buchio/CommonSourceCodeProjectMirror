/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 image handler ]
*/

#ifndef _DISK_H_
#define _DISK_H_

#include "vm.h"
#include "../emu.h"

// d88 media type
#define MEDIA_TYPE_2D	0x00
#define MEDIA_TYPE_2DD	0x10
#define MEDIA_TYPE_2HD	0x20
#define MEDIA_TYPE_144	0x30
#define MEDIA_TYPE_UNK	0xff

#define DRIVE_TYPE_2D	MEDIA_TYPE_2D
#define DRIVE_TYPE_2DD	MEDIA_TYPE_2DD
#define DRIVE_TYPE_2HD	MEDIA_TYPE_2HD
#define DRIVE_TYPE_144	MEDIA_TYPE_144
#define DRIVE_TYPE_UNK	MEDIA_TYPE_UNK

// this value will be stored to the state file,
// so don't change these definitions
#define SPECIAL_DISK_X1TURBO_ALPHA	 1
#define SPECIAL_DISK_X1_BATTEN		 2
#define SPECIAL_DISK_FM7_GAMBLER	11
#define SPECIAL_DISK_FM7_DEATHFORCE	12

// d88 constant
#define DISK_BUFFER_SIZE	0x380000	// 3.5MB
#define TRACK_BUFFER_SIZE	0x080000	// 0.5MB

class FILEIO;

class DISK
{
protected:
	EMU* emu;
private:
	FILEIO* fi;
	uint8 buffer[DISK_BUFFER_SIZE + TRACK_BUFFER_SIZE];
	_TCHAR orig_path[_MAX_PATH];
	_TCHAR dest_path[_MAX_PATH];
	_TCHAR temp_path[_MAX_PATH];
	pair file_size;
	int file_bank;
	uint32 crc32;
	bool trim_required;
	bool temporary;
	
	bool is_1dd_image;
	bool is_solid_image;
	bool is_fdi_image;
	uint8 fdi_header[4096];
	int solid_ncyl, solid_nside, solid_nsec, solid_size;
	
	void set_sector_info(uint8 *t);
	void trim_buffer();
	
	// teledisk image decoder (td0)
	bool teledisk_to_d88();
	
	// imagedisk image decoder (imd)
	bool imagedisk_to_d88();
	
	// cpdread image decoder (dsk)
	bool cpdread_to_d88(int extended);
	
	// solid image decoder (fdi/tfd/2d/img/sf7)
	bool solid_to_d88(int type, int ncyl, int nside, int nsec, int size);
	
public:
	DISK(EMU* parent_emu) : emu(parent_emu)
	{
		inserted = ejected = write_protected = changed = false;
		file_size.d = 0;
		sector_size.sd = sector_num.sd = 0;
		sector = NULL;
		drive_type = DRIVE_TYPE_UNK;
		drive_rpm = 0;
		drive_mfm = true;
		static int num = 0;
		drive_num = num++;
	}
	~DISK()
	{
		if(inserted) {
			close();
		}
	}
	
	void open(const _TCHAR* file_path, int bank);
	void close();
	bool get_track(int trk, int side);
	bool make_track(int trk, int side);
	bool get_sector(int trk, int side, int index);
	void set_deleted(bool value);
	void set_data_crc_error(bool value);
	void set_data_mark_missing();
	
	bool format_track(int trk, int side);
	void insert_sector(uint8 c, uint8 h, uint8 r, uint8 n, bool deleted, bool data_crc_error, uint8 fill_data, int length);
	void sync_buffer();
	
	int get_rpm();
	int get_track_size();
	double get_usec_per_track();
	double get_usec_per_bytes(int bytes);
	int get_bytes_per_usec(double usec);
	bool check_media_type();
	
	bool inserted;
	bool ejected;
	bool write_protected;
	bool changed;
	uint8 media_type;
	int is_special_disk;
	
	// track
	uint8 track[TRACK_BUFFER_SIZE];
	pair sector_num;
	bool invalid_format;
	bool no_skew;
	int cur_track, cur_side;
	
	int sync_position[256];
	int id_position[256];
	int data_position[256];
	
	// sector
	uint8* sector;
	pair sector_size;
	uint8 id[6];
	uint8 density;
	bool deleted;
	bool addr_crc_error;
	bool data_crc_error;
	
	// drive
	uint8 drive_type;
	int drive_rpm;
	bool drive_mfm;
	int drive_num;
	bool correct_timing()
	{
		if(drive_num < array_length(config.correct_disk_timing)) {
			return config.correct_disk_timing[drive_num];
		}
		return false;
	}
	bool ignore_crc()
	{
		if(drive_num < array_length(config.ignore_disk_crc)) {
			return config.ignore_disk_crc[drive_num];
		}
		return false;
	}
	
	// state
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

