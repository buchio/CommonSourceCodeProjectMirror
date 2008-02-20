#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

// variable scope of 'for' loop for microsoft visual c++ 6.0 and embedded visual c++ 4.0
#if (defined(_MSC_VER) && (_MSC_VER == 1200)) || defined(_WIN32_WCE)
#define for if(0);else for
#endif
// disable warnings C4189, C4995 and C4996 for microsoft visual c++ 2005
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )
#endif

// type definition
#ifndef uint8
typedef unsigned char uint8;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef int8
typedef signed char int8;
#endif
#ifndef int16
typedef signed short int16;
#endif
#ifndef int32
typedef signed int int32;
#endif

void main(int argc, char *argv[])
{
	if(argc < 3) {
		printf("D88FIX : MERGE 2 D88 IMAGES TO FIX CRC ERR\n");
		printf("\n");
		printf("USAGE : D88FIX 1ST.D88 2ND.D88\n");
		return;
	}
	uint8* buf1 = (uint8*)malloc(0x200000);
	uint8* buf2 = (uint8*)malloc(0x200000);
	uint8 head[688];
	FILE* fp;
	
	fp = fopen(argv[1], "rb");
	fread(buf1, 0x200000, 1, fp);
	fclose(fp);
	fp = fopen(argv[2], "rb");
	fread(buf2, 0x200000, 1, fp);
	fclose(fp);
	
	fp = fopen("OUT.D88", "wb");
	memcpy(head, buf1, sizeof(head));
	fwrite(head, sizeof(head), 1, fp);
	
	uint32 ofs = sizeof(head);
	int cnt1 = 0, cnt2 = 0, err = 0;
	for(int trk = 0; trk < 164; trk++) {
		// get header 1
		uint32 ofs1 = buf1[0x20 + 4 * trk];
		ofs1 |= buf1[0x20 + 4 * trk + 1] << 8;
		ofs1 |= buf1[0x20 + 4 * trk + 2] << 16;
		ofs1 |= buf1[0x20 + 4 * trk + 3] << 24;
		uint8* t1 = buf1 + ofs1;
		int sec1 = t1[4] | (t1[5] << 8);
		
		// get header 2
		uint32 ofs2 = buf2[0x20 + 4 * trk];
		ofs2 |= buf2[0x20 + 4 * trk + 1] << 8;
		ofs2 |= buf2[0x20 + 4 * trk + 2] << 16;
		ofs2 |= buf2[0x20 + 4 * trk + 3] << 24;
		uint8* t2 = buf2 + ofs2;
		int sec2 = t2[4] | (t2[5] << 8);
		
		// set header
		if(ofs1 && ofs2) {
			head[0x20 + 4 * trk + 0] = (ofs >>  0) & 0xff;
			head[0x20 + 4 * trk + 1] = (ofs >>  8) & 0xff;
			head[0x20 + 4 * trk + 2] = (ofs >> 16) & 0xff;
			head[0x20 + 4 * trk + 3] = (ofs >> 24) & 0xff;
		}
		else {
			head[0x20 + 4 * trk + 0] = 0;
			head[0x20 + 4 * trk + 1] = 0;
			head[0x20 + 4 * trk + 2] = 0;
			head[0x20 + 4 * trk + 3] = 0;
			sec1 = sec2 = 0;
		}
		
		// compare
		if(sec1 >= sec2) {
			printf("trk %d : secs = %d\n", trk, sec1);
			for(int s1 = 0; s1 < sec1; s1++) {
				int size1 = t1[0xe] | (t1[0xf] << 8);
				
				uint8* tmp = t2;
				for(int s2 = 0; s2 < sec2; s2++) {
					int size2 = tmp[0xe] | (tmp[0xf] << 8);
					if(t1[0] == tmp[0] && t1[1] == tmp[1] && t1[2] == tmp[2] && t1[3] == tmp[3]) {
						if(t1[8] != 0 && tmp[8] == 0 && size1 == size2) {
							memcpy(t1 + 6, tmp + 6, size1 + 10);
							cnt1++;
						}
						break;
					}
					tmp += size2 + 16;
				}
				printf("\tC=%2x H=%2x R=%2x N=%2x\tSTATUS=%2x\n", t1[0], t1[1], t1[2], t1[3], t1[8]);
				if(t1[8] != 0) err++;
				fwrite(t1, size1 + 16, 1, fp);
				t1 += size1 + 16;
				ofs += size1 + 16;
			}
		}
		else {
			printf("trk %d : secs = %d\n", trk, sec2);
			for(int s2 = 0; s2 < sec2; s2++) {
				int size2 = t2[0xe] | (t2[0xf] << 8);
				
				uint8* tmp = t1;
				for(int s1 = 0; s1 < sec1; s1++) {
					int size1 = tmp[0xe] | (tmp[0xf] << 8);
					if(t2[0] == tmp[0] && t2[1] == tmp[1] && t2[2] == tmp[2] && t2[3] == tmp[3]) {
						if(t2[8] != 0 && tmp[8] == 0 && size2 == size1) {
							memcpy(t2 + 6, tmp + 6, size2 + 10);
							cnt1++;
						}
						break;
					}
					tmp += size1 + 16;
				}
				printf("\tC=%2x H=%2x R=%2x N=%2x\tSTATUS=%2x\n", t2[0], t2[1], t2[2], t2[3], t2[8]);
				if(t2[8] != 0) err++;
				fwrite(t2, size2 + 16, 1, fp);
				t2 += size2 + 16;
				ofs += size2 + 16;
			}
		}
		if(sec1 != sec2)
			cnt2++;
	}
	head[0x1c] = (ofs >>  0) & 0xff;
	head[0x1d] = (ofs >>  8) & 0xff;
	head[0x1e] = (ofs >> 16) & 0xff;
	head[0x1f] = (ofs >> 24) & 0xff;
	fseek(fp, 0, SEEK_SET);
	fwrite(head, sizeof(head), 1, fp);
	fclose(fp);
	
	printf("\n");
	printf("FIXED CRC ERR = %d\n", cnt1);
	printf("FIXED TRACKS = %d\n", cnt2);
	printf("REMAINED CRC ERR = %d\n", err);
}
