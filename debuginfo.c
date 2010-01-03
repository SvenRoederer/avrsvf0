/* avrsvf0
 * debuginfo.c
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
 * 02.01.2010 ASR  First version, extracted from main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "debuginfo.h"


static int s_verbose;

void setVerbose(int verbose) {
	s_verbose = verbose;
}

void debugInfo(const char *formatString, ...) {
        va_list ellipsisArgs;
        if (!s_verbose) {
                return;
        }
        va_start(ellipsisArgs, formatString);
        vprintf((char *)formatString, ellipsisArgs);
        va_end(ellipsisArgs);   
}

