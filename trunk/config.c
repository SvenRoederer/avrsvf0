/* avrsvf0
 * config.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * 10.02.2009 ASR  First version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char s_device[16];
char s_infile[256];
char s_outfile[256];

unsigned s_fuseLowByte;
unsigned s_fuseHighByte;
unsigned s_fuseExtraByte;
int s_programFuses;

int s_verifySignByte;
int s_verifyFuses;

int s_eraseDevice;
int s_programDevice;
int s_verifyDevice;

int s_pageloadProgramming;
int s_chd;
int s_ctd;
int s_chi;
int s_cti;

void parseArguments(int argc, char *argv[]) {
	unsigned fuseBytes = 0;
	char *originalArgument;
	int c;
	while (--argc > 0 && (*++argv)[0] == '-') {
		originalArgument = *argv;
		c = *++argv[0];
		switch (c) {
			case 'c':
				if (1 == sscanf(argv[0], "chd%d", &s_chd)) {
					break;
				} else if (1 == sscanf(argv[0], "ctd%d", &s_ctd)) {
					break;
				} else if (1 == sscanf(argv[0], "chi%d", &s_chi)) {
					break;
				} else if (1 == sscanf(argv[0], "cti%d", &s_cti)) {
					break;
				}
				break;
			case 'd':
				strncpy(s_device, ++argv[0], 16);
				printf("device = %s\n", s_device);
				break;
			case 's':
				if (!*++argv[0]) {
					s_verifySignByte = 1;
					printf("verify sign byte = 1\n");
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'e':
				if (!*++argv[0]) {
					s_eraseDevice = 1;
					printf("erase device = 1\n");
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'i':
				c = *++argv[0];
				if (c == 'f') {
					strncpy(s_infile, ++argv[0], 256);
					printf("infile = %s\n", s_infile);
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
				}
				break;
			case 'p': 
				c = *++argv[0];
				if (c == 'f' && !*++argv[0]) {
					s_programDevice = 1;
					printf("program device = 1\n");
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'v':
				c = *++argv[0];
				if (c == 'f' && !*++argv[0]) {
					s_verifyDevice = 1;
					printf("verify device = 1\n");
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'F':
				if (!*++argv[0]) {
					s_verifyFuses = 1;
					printf("verify fuses = 1\n");
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'o':
				strncpy(s_outfile, ++argv[0], 256);
				printf("outfile = %s\n", s_outfile);
				break;	
			case 'm':
				c = *++argv[0];
				if (c == 'p' && !*++argv[0]) {
					s_pageloadProgramming = 1;
					printf("pageload programming = 1\n");
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'f':
				sscanf(++argv[0], "0x%08x", &fuseBytes);
				s_fuseExtraByte = (fuseBytes >> 16) & 0xFF;
				s_fuseHighByte = (fuseBytes >> 8) & 0xFF;
				s_fuseLowByte = fuseBytes & 0xFF;
				s_programFuses = 1;
				printf("program fuses %02x %02x\n", s_fuseHighByte, s_fuseLowByte);
				break;
			default:
				fprintf(stderr, "invalid argument: %s\n", originalArgument);
				exit(1);
				break;
		}
	}
}
