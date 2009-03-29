/* avrsvf0
 * hexreader.h
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

#ifndef HEXREADER_H
#define HEXREADER_H 1

void readFile(char *fileName);

unsigned lastAddress();
char *getBytes(unsigned start);

#endif
