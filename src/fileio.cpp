/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ file i/o ]
*/

#ifdef _WIN32
#include <windows.h>
#endif
#include "fileio.h"

FILEIO::FILEIO()
{
	fp = NULL;
}

FILEIO::~FILEIO(void)
{
	Fclose();
}

bool FILEIO::IsFileExisting(const _TCHAR *file_path)
{
#ifdef _WIN32
	DWORD attr = GetFileAttributes(file_path);
	if(attr == -1) {
		return false;
	}
	return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
	return (_taccess(file_path, 0) == 0);
#endif
}

bool FILEIO::IsFileProtected(const _TCHAR *file_path)
{
#ifdef _WIN32
	return ((GetFileAttributes(file_path) & FILE_ATTRIBUTE_READONLY) != 0);
#else
	return (_taccess(file_path, 2) != 0);
#endif
}

bool FILEIO::RemoveFile(const _TCHAR *file_path)
{
#ifdef _WIN32
	return (DeleteFile(file_path) != 0);
#else
	return (_tremove(file_path) == 0);
#endif
}

bool FILEIO::RenameFile(const _TCHAR *existing_file_path, const _TCHAR *new_file_path)
{
#ifdef _WIN32
	return (MoveFile(existing_file_path, new_file_path) != 0);
#else
	return (_trename(existing_file_path, new_file_path) == 0);
#endif
}

bool FILEIO::Fopen(const _TCHAR *file_path, int mode)
{
	Fclose();
	
	switch(mode) {
	case FILEIO_READ_BINARY:
		return ((fp = _tfopen(file_path, _T("rb"))) != NULL);
	case FILEIO_WRITE_BINARY:
		return ((fp = _tfopen(file_path, _T("wb"))) != NULL);
	case FILEIO_READ_WRITE_BINARY:
		return ((fp = _tfopen(file_path, _T("r+b"))) != NULL);
	case FILEIO_READ_WRITE_NEW_BINARY:
		return ((fp = _tfopen(file_path, _T("w+b"))) != NULL);
	case FILEIO_READ_ASCII:
		return ((fp = _tfopen(file_path, _T("r"))) != NULL);
	case FILEIO_WRITE_ASCII:
		return ((fp = _tfopen(file_path, _T("w"))) != NULL);
	case FILEIO_WRITE_APPEND_ASCII:
		return ((fp = _tfopen(file_path, _T("a"))) != NULL);
	case FILEIO_READ_WRITE_ASCII:
		return ((fp = _tfopen(file_path, _T("r+"))) != NULL);
	case FILEIO_READ_WRITE_NEW_ASCII:
		return ((fp = _tfopen(file_path, _T("w+"))) != NULL);
	case FILEIO_READ_WRITE_APPEND_ASCII:
		return ((fp = _tfopen(file_path, _T("a+"))) != NULL);
	}
	return false;
}

void FILEIO::Fclose()
{
	if(fp) {
		fclose(fp);
	}
	fp = NULL;
}

uint32_t FILEIO::FileLength()
{
	long pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return (uint32_t)len;
}

#define GET_VALUE(type) \
	uint8_t buffer[sizeof(type)]; \
	fread(buffer, sizeof(buffer), 1, fp); \
	return *(type *)buffer

#define PUT_VALUE(type, v) \
	fwrite(&v, sizeof(type), 1, fp)

bool FILEIO::FgetBool()
{
	GET_VALUE(bool);
}

void FILEIO::FputBool(bool val)
{
	PUT_VALUE(bool, val);
}

uint8_t FILEIO::FgetUint8()
{
	GET_VALUE(uint8_t);
}

void FILEIO::FputUint8(uint8_t val)
{
	PUT_VALUE(uint8_t, val);
}

uint16_t FILEIO::FgetUint16()
{
	GET_VALUE(uint16_t);
}

void FILEIO::FputUint16(uint16_t val)
{
	PUT_VALUE(uint16_t, val);
}

uint32_t FILEIO::FgetUint32()
{
	GET_VALUE(uint32_t);
}

void FILEIO::FputUint32(uint32_t val)
{
	PUT_VALUE(uint32_t, val);
}

uint64_t FILEIO::FgetUint64()
{
	GET_VALUE(uint64_t);
}

void FILEIO::FputUint64(uint64_t val)
{
	PUT_VALUE(uint64_t, val);
}

int8_t FILEIO::FgetInt8()
{
	GET_VALUE(int8_t);
}

void FILEIO::FputInt8(int8_t val)
{
	PUT_VALUE(int8_t, val);
}

int16_t FILEIO::FgetInt16()
{
	GET_VALUE(int16_t);
}

void FILEIO::FputInt16(int16_t val)
{
	PUT_VALUE(int16_t, val);
}

int32_t FILEIO::FgetInt32()
{
	GET_VALUE(int32_t);
}

void FILEIO::FputInt32(int32_t val)
{
	PUT_VALUE(int32_t, val);
}

int64_t FILEIO::FgetInt64()
{
	GET_VALUE(int64_t);
}

void FILEIO::FputInt64(int64_t val)
{
	PUT_VALUE(int64_t, val);
}

float FILEIO::FgetFloat()
{
	GET_VALUE(float);
}

void FILEIO::FputFloat(float val)
{
	PUT_VALUE(float, val);
}

double FILEIO::FgetDouble()
{
	GET_VALUE(double);
}

void FILEIO::FputDouble(double val)
{
	PUT_VALUE(double, val);
}

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h, l;
#else
		uint8_t l, h;
#endif
	} b;
	uint16_t u16;
	int16_t s16;
} pair16_t;

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h3, h2, h, l;
#else
		uint8_t l, h, h2, h3;
#endif
	} b;
	uint32_t u32;
	int32_t s32;
} pair32_t;

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		uint8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} b;
	uint64_t u64;
	int64_t s64;
} pair64_t;

uint16_t FILEIO::FgetUint16_LE()
{
	pair16_t tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.u16;
}

void FILEIO::FputUint16_LE(uint16_t val)
{
	pair16_t tmp;
	tmp.u16 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
}

uint32_t FILEIO::FgetUint32_LE()
{
	pair32_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.u32;
}

void FILEIO::FputUint32_LE(uint32_t val)
{
	pair32_t tmp;
	tmp.u32 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
}

uint64_t FILEIO::FgetUint64_LE()
{
	pair64_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.u64;
}

void FILEIO::FputUint64_LE(uint64_t val)
{
	pair64_t tmp;
	tmp.u64 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h7);
}

int16_t FILEIO::FgetInt16_LE()
{
	pair16_t tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.s16;
}

void FILEIO::FputInt16_LE(int16_t val)
{
	pair16_t tmp;
	tmp.s16 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
}

int32_t FILEIO::FgetInt32_LE()
{
	pair32_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.s32;
}

void FILEIO::FputInt32_LE(int32_t val)
{
	pair32_t tmp;
	tmp.s32 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
}

int64_t FILEIO::FgetInt64_LE()
{
	pair64_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.s64;
}

void FILEIO::FputInt64_LE(int64_t val)
{
	pair64_t tmp;
	tmp.s64 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h7);
}

uint16_t FILEIO::FgetUint16_BE()
{
	pair16_t tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.u16;
}

void FILEIO::FputUint16_BE(uint16_t val)
{
	pair16_t tmp;
	tmp.u16 = val;
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

uint32_t FILEIO::FgetUint32_BE()
{
	pair32_t tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.u32;
}

void FILEIO::FputUint32_BE(uint32_t val)
{
	pair32_t tmp;
	tmp.u32 = val;
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

uint64_t FILEIO::FgetUint64_BE()
{
	pair64_t tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.u64;
}

void FILEIO::FputUint64_BE(uint64_t val)
{
	pair64_t tmp;
	tmp.u64 = val;
	FputUint8(tmp.b.h7);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int16_t FILEIO::FgetInt16_BE()
{
	pair16_t tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.s16;
}

void FILEIO::FputInt16_BE(int16_t val)
{
	pair16_t tmp;
	tmp.s16 = val;
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int32_t FILEIO::FgetInt32_BE()
{
	pair32_t tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.s32;
}

void FILEIO::FputInt32_BE(int32_t val)
{
	pair32_t tmp;
	tmp.s32 = val;
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int64_t FILEIO::FgetInt64_BE()
{
	pair64_t tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.s64;
}

void FILEIO::FputInt64_BE(int64_t val)
{
	pair64_t tmp;
	tmp.s64 = val;
	FputUint8(tmp.b.h7);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int FILEIO::Fgetc()
{
	return fgetc(fp);
}

int FILEIO::Fputc(int c)
{
	return fputc(c, fp);
}

char *FILEIO::Fgets(char *str, int n)
{
	return fgets(str, n, fp);
}

int FILEIO::Fprintf(const char* format, ...)
{
	va_list ap;
	char buffer[1024];
	
	va_start(ap, format);
	my_vsprintf_s(buffer, 1024, format, ap);
	va_end(ap);
	
	return my_fprintf_s(fp, "%s", buffer);
}

uint32_t FILEIO::Fread(void* buffer, uint32_t size, uint32_t count)
{
	return fread(buffer, size, count, fp);
}

uint32_t FILEIO::Fwrite(void* buffer, uint32_t size, uint32_t count)
{
	return fwrite(buffer, size, count, fp);
}

uint32_t FILEIO::Fseek(long offset, int origin)
{
	switch(origin) {
	case FILEIO_SEEK_CUR:
		return fseek(fp, offset, SEEK_CUR);
	case FILEIO_SEEK_END:
		return fseek(fp, offset, SEEK_END);
	case FILEIO_SEEK_SET:
		return fseek(fp, offset, SEEK_SET);
	}
	return 0xFFFFFFFF;
}

uint32_t FILEIO::Ftell()
{
	return ftell(fp);
}
