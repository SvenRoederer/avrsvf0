/* avrsvf0
 * main.c
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
 * 24.03.2009 ASR  Extended for atmega128, HIR/HDR
 * 02.01.2010 ASR  Updated version for Autotools integration, 
 *                 moved debugInfo procedure to separate file.
 * 08.03.2013 ASR  Extended for atmega1284p
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>

#include "config.h"
#include "hexreader.h"
#include "debuginfo.h"


FILE *outfile;

void writeSvfFile();

void writeEraseDevice();
void writeVerifySignatureByte();

void writeWriteFlash();
void writeVerifyFlash();

void writeProgramFuses();
void writeVerifyFuses();

void writeNop();


char cmd[1024];
int resetHT = 0;
bool do_flash = false;

int main(int argc, char *argv[]) {
	int i, j=0;
	int invalidArgs = 0;
	
	s_version = "v0.2.1";
	
	if (argc <= 1) {
		printf("Use -h for help.\n");
		return -1;
	}
	
	for (i=0; i<argc; i++) {
		sprintf(&cmd[j], "%s ", argv[i]);
		j = strlen(cmd);
	}

	parseArguments(argc, argv);
	
	if (!strlen(s_outfile)) {
		fprintf(stderr, "No output file specified.\n");
		invalidArgs = 1;
	}
	if (!strlen(s_device)) {
		fprintf(stderr, "No device specified.\n");
		invalidArgs = 1;
	}
	if (invalidArgs) {
		fprintf(stderr, "Use -h to get help!\n");
		exit(1);
	}

	writeSvfFile();
}

void writeEnHeaderTrailer() {
	/* HEADER instruction and data register */
	if (s_chi > 0) {
		fprintf(outfile, "HIR %d TDI(ff);\n", s_chi);
		resetHT = 1;
	} else {
		fprintf(outfile, "HIR 0;\n");
	}
	if (s_chd > 0) {
		fprintf(outfile, "HDR %d TDI(0);\n", s_chd);
		resetHT = 1;
	} else {
		fprintf(outfile, "HDR 0;\n");
	}

	/* TRAILER instruction and data register */
	if (s_cti > 0) {
		printf("error, TIR not supported\n");
		resetHT = 1;
		exit(-1);
	} else {
		fprintf(outfile, "TIR 0;\n");
	}
	if (s_ctd > 0) {
		printf("error, TDR not supported\n");
		resetHT = 1;
		exit(-1);
	} else {
		fprintf(outfile, "TDR 0;\n");
	}
}

void writeDisHeaderTrailer() {
	fprintf(outfile, "HIR 0;\n");
	fprintf(outfile, "HDR 0;\n");
	fprintf(outfile, "TIR 0;\n");
	fprintf(outfile, "TDR 0;\n");		
}

void writeSvfFile() {
	time_t rawtime;
	time(&rawtime);

	debugInfo("opening output file: %s\n", s_outfile);
	outfile = fopen(s_outfile, "w");

	debugInfo("writing header\n");
	fprintf(outfile, "// avrsvf0 %s (C) 2009-2014 A. Schweizer\n", s_version);
	fprintf(outfile, "// This file was made: %s", ctime(&rawtime));
	fprintf(outfile, "// with this cmd: %s\n", cmd);
	fprintf(outfile, "TRST ABSENT;\n");
	fprintf(outfile, "ENDIR IDLE;\n");
	fprintf(outfile, "ENDDR IDLE;\n");
	
	debugInfo("writing EnHeaderTrailer\n");
	writeEnHeaderTrailer();

	fprintf(outfile, "STATE RESET;\n");
	fprintf(outfile, "STATE IDLE;\n");
	
	/* AVR-Reset register; enable reset */
	fprintf(outfile, "SIR 4 TDI(c);\n");
	fprintf(outfile, "SDR 1 TDI(1);\n");
	
	/* Programming-enable register is a 16-bit register; contents is compared */
	/* with 0xA370. Change back to 0 after programming. */
	fprintf(outfile, "SIR 4 TDI(4);\n"); /* prog-enable */
	fprintf(outfile, "SDR 16 TDI(a370);\n");
	
	if (s_eraseDevice) {
		debugInfo("writing EraseDevice\n");
		writeEraseDevice();
	}
	
	if (s_verifySignByte) {
		debugInfo("writing VerifySignatureByte\n");
		writeVerifySignatureByte();
	}

	if (do_flash) {
		printf("yes\n");
		debugInfo("reading input file: %s\n", s_infile);
		readFile(s_infile);
		
		if (s_programDevice) {
			debugInfo("writing WriteFlash\n");
			writeWriteFlash();
		}
		
		if (s_verifyDevice) {
		debugInfo("writing VerifyFlash\n");
		writeVerifyFlash();
		}
	} else {
		debugInfo("no flash-file provided - skipping\n");
	}

	if (s_programFuses) {
		debugInfo("writing ProgramFuses\n");
		writeProgramFuses();
	}
	
	if (s_verifyFuses) {
		debugInfo("writing VerifyFuses\n");
		writeVerifyFuses();
	}
	
	writeNop();
	
	/* write signature 0000 into JTAG programming-enable register to disable */
	fprintf(outfile, "SIR 4 TDI(4);\n"); /* prog-enable */
	fprintf(outfile, "SDR 16 TDI(0000);\n");
	
	/* AVR-Reset register; disable reset */
	fprintf(outfile, "SIR 4 TDI(c);\n");
	fprintf(outfile, "SDR 1 TDI(0);\n");
	
	fprintf(outfile, "STATE RESET;\n");
	
	debugInfo("closing output file\n");
	fclose(outfile);
}

void writeEraseDevice() {
	/* see doc2466, table 117 */
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2380);\n");
	fprintf(outfile, "SDR 15 TDI(3180);\n");
	fprintf(outfile, "SDR 15 TDI(3380);\n");
	fprintf(outfile, "SDR 15 TDI(3380);\n");
	fprintf(outfile, "RUNTEST 13E-3 SEC;\n");
}

void writeVerifySignatureByte() {
	/* see doc2466, p.261; signature byte for atmega16 is 00:1e 01:94 02:03 */
	/*  p264, table 112: 0000 1000  Read sign.byte and calibration byte */
	char *sig;
	if (!strcasecmp(s_device, "atmega16")) {
		sig = "1e9403";
	} else if (!strcasecmp(s_device, "atmega128")) {
		sig = "1e9702";
	} else if (!strcasecmp(s_device, "atmega1284p")) {
		sig = "1e9705";
	} else if (!strcasecmp(s_device, "atmega169p")) {
		sig = "1e9405";
	} else {
		printf("unknown device: %s\n", s_device);
		exit(-1);
	}
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2308);\n"); /* enter sign.byte read */
	fprintf(outfile, "SDR 15 TDI(0300);\n"); /* load address byte */
	fprintf(outfile, "SDR 15 TDI(3200);\n"); /* read signature byte */
	fprintf(outfile, "SDR 15 TDI(3300) TDO(00%.2s) MASK(00ff);\n", sig);
	fprintf(outfile, "SDR 15 TDI(0301);\n");
	fprintf(outfile, "SDR 15 TDI(3200);\n");
	fprintf(outfile, "SDR 15 TDI(3300) TDO(00%.2s) MASK(00ff);\n", &sig[2]);
	fprintf(outfile, "SDR 15 TDI(0302);\n");
	fprintf(outfile, "SDR 15 TDI(3200);\n");
	fprintf(outfile, "SDR 15 TDI(3300) TDO(00%.2s) MASK(00ff);\n", &sig[4]);
}

void writeWriteFlash() {
	unsigned address = 0, address2, oldHighByte;
	char *data, bytes[513];
	int i, n;

	if (!strcasecmp(s_device, "atmega16")
               || !strcasecmp(s_device, "atmega169p")) {
		n = 128;
	} else if (   !strcasecmp(s_device, "atmega128")
               || !strcasecmp(s_device, "atmega1284p")) {
		n = 256;
	} else {
		printf("unknown device: %s\n", s_device);
		exit(-1);	
	}

	/* prog_commands (doc2466, p.280) */
	fprintf(outfile, "SIR 4 TDI(5);\n"); /* enable programming commands */
	/* 'enter flash write' (doc2466, p.283) */
	fprintf(outfile, "SDR 15 TDI(2310);\n");

    if (!s_pageloadProgramming) {
        oldHighByte = -1;
        while (address < lastAddress()) {
            data = getBytes(address);
            
            /* address high: 07xx; address low: 03xx */
            address2 = address / 2;
            /* load adr high */
            if (oldHighByte != ((address2 >> 8) & 0xFF)) {
                oldHighByte = (address2 >> 8) & 0xFF;
                fprintf(outfile, "SDR 15 TDI(07%02x);\n", oldHighByte);
            }
            /* load adr low */
            fprintf(outfile, "SDR 15 TDI(03%02x);\n", address2 & 0xFF);
            
            /* load data low */
            fprintf(outfile, "SDR 15 TDI(13%02x);\n", data[0] & 0xFF);
            /* load data high */
            fprintf(outfile, "SDR 15 TDI(17%02x);\n", data[1] & 0xFF);

            /* latch data: 3700 7700 3700 */
            fprintf(outfile, "SDR 15 TDI(3700);\n");
            fprintf(outfile, "SDR 15 TDI(7700);\n");
            fprintf(outfile, "SDR 15 TDI(3700);\n");
            
            address += 2;
            
            /* write page at end */
            if (address % n == 0 || address >= lastAddress()) {
                fprintf(outfile, "SDR 15 TDI(3700);\n");
                fprintf(outfile, "SDR 15 TDI(3500);\n");
                fprintf(outfile, "SDR 15 TDI(3700);\n");
                fprintf(outfile, "SDR 15 TDI(3700);\n");
                
                fprintf(outfile, "RUNTEST 7E-3 SEC;\n");
                
                oldHighByte = -1;
            }
        }
    } else {
        while (address < lastAddress()) {
            data = getBytes(address);

            /* address high: 07xx; address low: 03xx */
            address2 = address / 2;
            /* load adr high */
            fprintf(outfile, "SDR 15 TDI(07%02x);\n", (address2 >> 8) & 0xFF);
            /* load adr low */
            fprintf(outfile, "SDR 15 TDI(03%02x);\n", address2 & 0xFF);
            
            /* prog_pageload */
            fprintf(outfile, "SIR 4 TDI(6);\n");
            
            if (resetHT) {
                writeDisHeaderTrailer();
            }

            fprintf(outfile, "SDR %d TDI", 8*n);
        
            // 1024 bit = 128 byte, in pairs of 2 bytes, in reverse byte order
            
            //:100000000C942A000C9445000C9445000C94450077
            //:100010000C9445000C9445000C9445000C9445004C
            //:100020000C9445000C9445000C9445000C94A203DC
            //:100030000C945B030C9445000C9445000C94450013
            //:100040000C9445000C9445000C9445000C9445001C
            //:100050000C94450011241FBECFE5D4E0DEBFCDBF18
            //:1000600011E0A0E6B0E0E4E2FEE102C005900D92EE
            //:10007000A230B107D9F712E0A2E0B1E001C01D92B1
            
            // : len=10 adr = 0000 typ=00(data) data=0C942A000C9445000C9445000C944500 cks=77
            
            // 921dc001e0b1e0a2e012f7d907b130a2
            // 920d9005c002e1fee2e4e0b0e6a0e011
            // bfcdbfdee0d4e5cfbe1f24110045940c
            // 0045940c0045940c0045940c0045940c
            // ...
            // 0045940c0045940c0045940c002a940c

            for (i=0; i<n; i++) {
                sprintf(&bytes[2*i], "%02x", data[n-1 - i] & 0xFF);
            }

//printf("adr = %d, data = %.256s\n", address, bytes);
		
            fprintf(outfile, "(%.218s\n", bytes);
            if (n == 128) {
                fprintf(outfile, "%.38s);\n", &bytes[218]);
            } else {
                fprintf(outfile, "%.230s\n", &bytes[218]);
                fprintf(outfile, "%.64s);\n", &bytes[448]);
            }
            
            if (resetHT) {
                writeEnHeaderTrailer();
            }
            fprintf(outfile, "SIR 4 TDI(5);\n");
            
            /* write flash page: 3700 3500 3700 3700 */
            fprintf(outfile, "SDR 15 TDI(3700);\n");
            fprintf(outfile, "SDR 15 TDI(3500);\n");
            fprintf(outfile, "SDR 15 TDI(3700);\n");
            fprintf(outfile, "SDR 15 TDI(3700);\n");
            
            fprintf(outfile, "RUNTEST 7E-3 SEC;\n");

            address += n;
        }
    }
}

void writeVerifyFlash() {
	unsigned address = 0, address2;
	char *data, bytes[513];
	int i, n;

	if (!strcasecmp(s_device, "atmega16")
		|| !strcasecmp(s_device, "atmega169p")) {
		n = 128;
	} else if (   !strcasecmp(s_device, "atmega128")
               || !strcasecmp(s_device, "atmega1284p")) {
		n = 256;
	} else {
		printf("unknown device: %s\n", s_device);
		exit(-1);	
	}
	
	/* 'enter flash read' */
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2302);\n");
	
    if (!s_pageloadProgramming) {
        while (address < lastAddress()) {
            data = getBytes(address);
            
            /* address high: 07xx; address low: 03xx */
            address2 = address / 2;
            fprintf(outfile, "SDR 15 TDI(07%02x);\n", (address2 >> 8) & 0xFF);
            fprintf(outfile, "SDR 15 TDI(03%02x);\n", address2 & 0xFF);
            
            /* read data low and high byte */
            fprintf(outfile, "SDR 15 TDI(3200);\n");
            fprintf(outfile, "SDR 15 TDI(3600) TDO(00%02x) MASK(00ff);\n", data[0] & 0xFF);
            fprintf(outfile, "SDR 15 TDI(3700) TDO(00%02x) MASK(00ff);\n", data[1] & 0xFF);
            
            address += 2;
        }
    } else {
        while (address < lastAddress()) {
            data = getBytes(address);

            for (i=0; i<n; i++) {
                sprintf(&bytes[2*i], "%02x", data[n-1 - i] & 0xFF);
            }

            /* address high: 07xx; address low: 03xx */
            address2 = address / 2;
            fprintf(outfile, "SDR 15 TDI(07%02x);\n", (address2 >> 8) & 0xFF);
            fprintf(outfile, "SDR 15 TDI(03%02x);\n", address2 & 0xFF);
            fprintf(outfile, "SIR 4 TDI(7);\n");
            fprintf(outfile, "SDR %d TDI(ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n", 8*n + 8);
            if (n == 128) {
                fprintf(outfile, "ffffffffffffffffffffffffffffffffffffffff) TDO(%.184s\n", bytes);
                fprintf(outfile, "%.72sff) MASK(ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n", &bytes[184]);
                fprintf(outfile, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00);\n");
            } else {
                fprintf(outfile, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
                fprintf(outfile, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) TDO(%.158s\n", bytes);
                fprintf(outfile, "%.230s\n", &bytes[158]);
                fprintf(outfile, "%.124sff) MASK(ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n", &bytes[388]);
                fprintf(outfile, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
                fprintf(outfile, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00);\n");
            }
            fprintf(outfile, "SIR 4 TDI(5);\n");

            address += n;
        }
    }
}

void writeProgramFuses() {
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2340);\n"); /* enter fuse write */

	if (!strcasecmp(s_device, "atmega16")) {
		;
	} else if (   !strcasecmp(s_device, "atmega128")
               || !strcasecmp(s_device, "atmega1284p")
               || !strcasecmp(s_device, "atmega169p")) {
		fprintf(outfile, "SDR 15 TDI(13%02x);\n", s_fuseExtraByte & 0xFF);
		fprintf(outfile, "SDR 15 TDI(3b00);\n");
		fprintf(outfile, "SDR 15 TDI(3900);\n");
		fprintf(outfile, "SDR 15 TDI(3b00);\n");
		fprintf(outfile, "SDR 15 TDI(3b00);\n");
		fprintf(outfile, "RUNTEST 7E-3 SEC;\n");
	} else {
		printf("unknown device: %s\n", s_device);
		exit(-1);	
	}

	fprintf(outfile, "SDR 15 TDI(13%02x);\n", s_fuseHighByte & 0xFF);
	fprintf(outfile, "SDR 15 TDI(3700);\n"); /* write fuse high byte */
	fprintf(outfile, "SDR 15 TDI(3500);\n");
	fprintf(outfile, "SDR 15 TDI(3700);\n");
	fprintf(outfile, "SDR 15 TDI(3700);\n");
	fprintf(outfile, "RUNTEST 7E-3 SEC;\n");
	
	fprintf(outfile, "SDR 15 TDI(13%02x);\n", s_fuseLowByte & 0xFF); /* load data low byte */
	fprintf(outfile, "SDR 15 TDI(3300);\n"); /* write fuse low byte */
	fprintf(outfile, "SDR 15 TDI(3100);\n");
	fprintf(outfile, "SDR 15 TDI(3300);\n");
	fprintf(outfile, "SDR 15 TDI(3300);\n");
	fprintf(outfile, "RUNTEST 7E-3 SEC;\n");	
}

void writeVerifyFuses() {
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2304);\n");
	
	if (   !strcasecmp(s_device, "atmega128")
        || !strcasecmp(s_device, "atmega1284p")
        || !strcasecmp(s_device, "atmega169p")) {
		/* extended fuse byte */
		fprintf(outfile, "SDR 15 TDI(3a00);\n");
		fprintf(outfile, "SDR 15 TDI(3b00) TDO(00%02x) MASK(00ff);\n", s_fuseExtraByte & 0xFF);
	}
	fprintf(outfile, "SDR 15 TDI(3e00);\n");
	fprintf(outfile, "SDR 15 TDI(3f00) TDO(00%02x) MASK(00ff);\n", s_fuseHighByte & 0xFF);
	fprintf(outfile, "SDR 15 TDI(3200);\n");
	fprintf(outfile, "SDR 15 TDI(3300) TDO(00%02x) MASK(00ff);\n", s_fuseLowByte & 0xFF);
}

void writeNop() {
	/* no-operation command */
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2300);\n");
	fprintf(outfile, "SDR 15 TDI(3300);\n");	
}
