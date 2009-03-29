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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "hexreader.h"


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

int main(int argc, char *argv[]) {
	int i, j=0;
	for (i=0; i<argc; i++) {
		sprintf(&cmd[j], "%s ", argv[i]);
		j = strlen(cmd);
	}

	parseArguments(argc, argv);
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

	outfile = fopen(s_outfile, "w");
	
	fprintf(outfile, "// avrsvf0 v0.1 (C) 2009 A. Schweizer\n");
	fprintf(outfile, "// This file was made: %s", ctime(&rawtime));
	fprintf(outfile, "// with this cmd: %s\n", cmd);
	fprintf(outfile, "TRST ABSENT;\n");
	fprintf(outfile, "ENDIR IDLE;\n");
	fprintf(outfile, "ENDDR IDLE;\n");
	
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
		writeEraseDevice();
	}
	
	if (s_verifySignByte) {
		writeVerifySignatureByte();
	}

	readFile(s_infile);
	
	if (s_programDevice) {
		writeWriteFlash();
	}
	
	if (s_verifyDevice) {
		writeVerifyFlash();
	}

	if (s_programFuses) {
		writeProgramFuses();
	}
	
	if (s_verifyFuses) {
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
	if (!strcmp(s_device, "atmega16")) {
		sig = "1e9403";
	} else if (!strcmp(s_device, "atmega128")) {
		sig = "1e9702";
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
	unsigned address = 0, address2;
	char *data, bytes[513];
	int i, n;

	if (!strcmp(s_device, "atmega16")) {
		n = 128;
	} else if (!strcmp(s_device, "atmega128")) {
		n = 256;
	} else {
		printf("unknown device: %s\n", s_device);
		exit(-1);	
	}
	
	/* see doc2466, p.263; 'enter flash write' */
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2310);\n");

	while (address < lastAddress()) {
		data = getBytes(address);

		/* address high: 07xx; address low: 03xx */
		address2 = address / 2;
		fprintf(outfile, "SDR 15 TDI(07%02x);\n", (address2 >> 8) & 0xFF);
		fprintf(outfile, "SDR 15 TDI(03%02x);\n", address2 & 0xFF);
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
		
		fprintf(outfile, "SDR 15 TDI(3700);\n"); /* write flash page */
		fprintf(outfile, "SDR 15 TDI(3500);\n");
		fprintf(outfile, "SDR 15 TDI(3700);\n");
		fprintf(outfile, "SDR 15 TDI(3700);\n");
		fprintf(outfile, "RUNTEST 7E-3 SEC;\n");

		address += n;
	}
}

void writeVerifyFlash() {
	unsigned address = 0, address2;
	char *data, bytes[513];
	int i, n;

	if (!strcmp(s_device, "atmega16")) {
		n = 128;
	} else if (!strcmp(s_device, "atmega128")) {
		n = 256;
	} else {
		printf("unknown device: %s\n", s_device);
		exit(-1);	
	}
	
	/* 'enter flash read' */
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2302);\n");
	
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

void writeProgramFuses() {
	fprintf(outfile, "SIR 4 TDI(5);\n");
	fprintf(outfile, "SDR 15 TDI(2340);\n"); /* enter fuse write */

	if (!strcmp(s_device, "atmega16")) {
		;
	} else if (!strcmp(s_device, "atmega128")) {
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
	
	if (!strcmp(s_device, "atmega128")) {
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
