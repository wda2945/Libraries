/*
 * ps_debug_linux.c
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */
#include <errno.h>
#include <string.h>
#include <time.h>

#include "ps_common.h"
#include "ps_config.h"

//used by DEBUGPRINT and ERRORPRINT macros
void print_debug_message_to_file(FILE *dbgfile, const char *text)
{
	char printBuff[30];
	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	snprintf(printBuff, 30, "%02i:%02i:%02i ",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);

	fprintf(dbgfile, printBuff);
	fprintf(dbgfile, text);
	fprintf(dbgfile, "/n");
	fflush(dbgfile);
}

FILE *fopen_logfile(const char *name)
{
	FILE *dbg;
	char path[100];

	snprintf(path, 100, "%s/%s.log", LOGFILE_FOLDER, name);

	dbg = fopen(path, "w");

    if (dbg == NULL)
    {
        fprintf (stderr, "Couldn’t open %s; %s\n", path, strerror (errno));
        return (FILE*) 0;
    }
    else {
    	fprintf (dbg, "Started %s logfile\n", name);
    	fprintf (stdout, "Opened %s logfile\n", name);
    	return dbg;
    }
}
