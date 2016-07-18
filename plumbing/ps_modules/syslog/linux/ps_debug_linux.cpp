/*
 * ps_debug_linux.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */
#include <errno.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <mutex>

using namespace std;

#include "ps_common.h"
#include "ps_config.h"

//used by PS_DEBUG and PS_ERROR macros
static FILE *plumbing_debug_file;
mutex	debugMtx;	//debug file mutex

//defaults to plumbing.log
void print_debug_message(const char *text)
{
    if (!plumbing_debug_file) plumbing_debug_file = fopen_logfile("plumbing");

    print_debug_message_to_file(plumbing_debug_file, text);
    print_debug_message_to_file(stdout, text);
}
void print_error_message(const char *text)
{
    if (!plumbing_debug_file) plumbing_debug_file = fopen_logfile("plumbing");
    
    print_debug_message_to_file(plumbing_debug_file, text);
    print_debug_message_to_file(stderr, text);
}

void print_debug_message_to_file(FILE *dbgfile, const char *text)
{
	char printBuff[30];
	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	snprintf(printBuff, 30, "%02i:%02i:%02i ",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);

    debugMtx.lock();
    
	fprintf(dbgfile, "%s", printBuff);
	fprintf(dbgfile, "%s", text);
	fprintf(dbgfile, "\n");
	fflush(dbgfile);
    
    debugMtx.unlock();
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
        char printBuff[30];
        const time_t now = time(NULL);
        struct tm *timestruct = localtime(&now);
        
        snprintf(printBuff, 30, "%02i/%02i/%04i %02i:%02i:%02i ",
                 timestruct->tm_mon, timestruct->tm_mday, timestruct->tm_year + 1900,
                 timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);

        fprintf (dbg, "%s", printBuff);
    	fprintf (dbg, "Started %s logfile\n", name);
    	fprintf (stdout, "Opened %s logfile\n", name);
    	return dbg;
    }
}
