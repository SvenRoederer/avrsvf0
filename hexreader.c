/* avrsvf0
 * hexreader.c
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
 * 28.06.2014 ASR  Ignore type-03 records; fix line counter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debuginfo.h"


/* Memory for the hex file */
#define HEXFILE_BYTES (128*1024)
static unsigned char s_data[HEXFILE_BYTES];

/* Last address read from file */
static unsigned s_adr9 = 0;

unsigned lastAddress() {
	return s_adr9;
}

char *getBytes(unsigned start) {
	return (char *) &s_data[start];
}

void readFile(char *fileName) {
	char hexdata[1024];
	FILE *hexfile;
	unsigned i, dlen, dadr, dtype, b, curLine = 1;
    unsigned dadr_segment_offset = 0;
	int dadr_old = -1;
	
	/* clear the memory first by initializing with FF */
	for (i=0; i<HEXFILE_BYTES; i++) {
		s_data[i] = 0xFF;
	}
	
	hexfile = fopen(fileName, "r");
	if (!hexfile) {
		fprintf(stderr, "failed to read file '%s', exiting\n", fileName);
		exit(-1);
	}

	while (fgets(hexdata, 1024, hexfile)) {
        debugInfo("read data: %s\n", hexdata);
		if (hexdata[0] == ':') {
			if (!strncmp(hexdata, ":00000001FF", 11)) {
				// end of file
				break;
			}
			
			if (3 == sscanf(hexdata, ":%02x%04x%02x", &dlen, &dadr, &dtype)) {
				debugInfo("parsed len=%d, adr=%d, type=%d, offset=%d\n",
                          dlen, dadr, dtype, dadr_segment_offset);
                
				if (dtype == 0) {
					if (dadr_old >= 0 && dadr_old != dadr) {
						fprintf(stderr, "dadr=%08x dadr_old=%08x dadr_segment_offset=%08x\n",
                                dadr, dadr_old, dadr_segment_offset);
						fprintf(stderr, "discontiguous start address on line %d, exiting\n", curLine);
						exit(-1);
					}
					i = 9;
					while (dlen-- > 0) {
						sscanf(&hexdata[i], "%02x", &b);
						s_data[dadr + dadr_segment_offset] = b;
						i += 2;
						dadr++;
					}
					dadr_old = dadr;
					debugInfo("dadr_old = %d\n", dadr_old);
                    
                } else if (dtype == 2) {
                    sscanf(&hexdata[9], "%04x", &b);
                    dadr_segment_offset = 16 * b;
                    debugInfo("extended_segment_address = %d\n", dadr_segment_offset);
                    dadr_old = -1; // don't check next 00 record for continuity of address
                    
				} else if (dtype == 3) {
					// start segment address record, ignore
                    
				} else {
					fprintf(stderr, "unrecognized record type (%d) on line %d, exiting\n", dtype, curLine);
					exit(-1);
				}
			} else {
				fprintf(stderr, "failed to parse data on line %d, exiting; %s\n", curLine, hexdata);
				exit(-1);
			}			
		} else {
			fprintf(stderr, "illegal start character on line %d, exiting; %s\n", curLine, hexdata);
			exit(-1);
		}
        
		curLine++;
	}
	s_adr9 = dadr_segment_offset + dadr_old - 1;
	debugInfo("s_adr9 = %d\n", s_adr9);
}
