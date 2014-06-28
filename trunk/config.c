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

#include "debuginfo.h"


char *s_version;

char s_device[16] = "";
char s_infile[256] = "";
char s_outfile[256] = "";

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

void printUsage() {
	printf("avrsvf0 %s (C) 2009-2014 A. Schweizer\n", s_version);
	printf("\n");
	printf("Command Line Switches:\n");
	printf("        [-d device name] [-m s|p] [-if infile] [-ov outfile]\n");
	printf("        [-s] [-e] [-p f|e|b] [-v f|e|b] [-f value]\n");
	printf("        [-F] [-c hd|td|hi|ti] [-verbose] [-h|?]\n");
	printf("\n");
	printf("Parameters:\n");
	printf("d       Device name. See list of supported devices below.\n");
	printf("m       Select programming mode; serial (s) or pageload (p). Pageload is the\n");
	printf("        most efficient, but can only be used when the target AVR is alone in\n");
	printf("        the JTAG chain. Default is Serial programming mode.\n");
	printf("if      Name of FLASH input file. Required for programming or verification\n");
	printf("        of the FLASH memory. The file format is Intel Extended HEX.\n");
	printf("ov      Name of SVF output file.\n");
	printf("s       Verify signature bytes.\n");
	printf("e       Erase device. If applied with another programming parameter, the\n");
	printf("        device will be erased before any other programming takes place.\n");
	printf("p       Program device; FLASH (f), EEPROM (e) or both (b). Corresponding\n");
	printf("        input files are required.\n");
	printf("          ** currently, only f is supported in avrsvf0 **\n");
	printf("v       Verify device; FLASH (f), EEPROM (e) or both (b). Can be used with\n");
	printf("        -p or standalone. Corresponding input files are required.\n");
	printf("          ** currently, only f is supported in avrsvf0 **\n");
	printf("f       Set fuse bytes. 'value' is a 16(/24)-bit hex value describing the\n");
	printf("        settings for the (extended,) upper and lower fuse.\n");
	printf("F       Verify fuse bytes.\n");
	printf("c##     JTAG chain information (default is all set to NULL)\n");
	printf(" chd     HDR - Number of devices after the current device.\n");
	printf(" ctd     TDR - Number of devices ahead of the current device.\n");
	printf(" chi     HIR - Total number of bits in instruction registers after the\n");
	printf("               current device.\n");
	printf(" cti     TIR - Total number of bits in instruction registers ahead of the\n");
	printf("               current device.\n");
	printf("          ** currently, only HDR and HIR are supported in avrsvf0 **\n");
	printf("verbose Display debugging information during execution.\n");
	printf("h       Help information, this page. (overrides all other settings)\n");
	printf("\n");
	printf("Supported devices:\n");
	printf(" ATmega16  ATmega128  ATmega1284P (serial only)\n");
}

void parseArguments(int argc, char *argv[]) {
	unsigned fuseBytes = 0;
	char *originalArgument;
	int c;
	while (--argc > 0 && (*++argv)[0] == '-') {
		originalArgument = *argv;
		c = *++argv[0];
		switch (c) {
			case 'h':
				printUsage();
				exit(0);
				break;				
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
				break;
			case 's':
				if (!*++argv[0]) {
					s_verifySignByte = 1;
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'e':
				if (!*++argv[0]) {
					s_eraseDevice = 1;
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'i':
				c = *++argv[0];
				if (c == 'f') {
					strncpy(s_infile, ++argv[0], 256);
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'p': 
				c = *++argv[0];
				if (c == 'f' && !*++argv[0]) {
					s_programDevice = 1;
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'v':
				if (!strcmp(originalArgument, "-verbose")) {
					setVerbose(1);
				} else {
					c = *++argv[0];
					if (c == 'f' && !*++argv[0]) {
						s_verifyDevice = 1;
					} else {
						fprintf(stderr, "invalid argument: %s\n", originalArgument);
						exit(1);
					}
				}
				break;
			case 'F':
				if (!*++argv[0]) {
					s_verifyFuses = 1;
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;
			case 'o':
				c = *++argv[0];
				if (c == 'v') {
					strncpy(s_outfile, ++argv[0], 256);
				} else {
					fprintf(stderr, "invalid argument: %s\n", originalArgument);
					exit(1);
				}
				break;	
			case 'm':
				c = *++argv[0];
				if (!*++argv[0]) {
                    if (c == 'p') {
                        s_pageloadProgramming = 1;
                    } else if (c == 's') {
                        s_pageloadProgramming = 0;
                    } else {
                        fprintf(stderr, "invalid argument: %s\n", originalArgument);
                        exit(1);
                    }
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
				break;
			default:
				fprintf(stderr, "invalid argument: %s\n", originalArgument);
				exit(1);
				break;
		}
	}
}
