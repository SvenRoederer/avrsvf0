/* avrsvf0
 * config.h
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

#ifndef CONFIG_H
#define CONFIG_H 1

extern char s_version[];

extern char s_device[];
extern char s_infile[];
extern char s_outfile[];

extern int s_verbose;
extern unsigned s_fuseLowByte;
extern unsigned s_fuseHighByte;
extern unsigned s_fuseExtraByte;
extern int s_programFuses;

extern int s_verifySignByte;
extern int s_verifyFuses;

extern int s_eraseDevice;
extern int s_programDevice;
extern int s_verifyDevice;

extern int s_pageloadProgramming;

extern int s_chd;
extern int s_ctd;
extern int s_chi;
extern int s_cti;

void parseArguments(int argc, char *argv[]);

#endif
