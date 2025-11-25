/*
 * log.c
 *
 *  Created on: Jan 5, 2019
 *      Author: Thinpv
 */
#include "Log.h"
#include <stdio.h>
#include <string.h>

printf_like_t s_log_print_func = &printf;

log_level_t log_level = LOG_INFO;

static char time_str[16];
char *timestr()
{
	time_t t = time(NULL);
	struct tm *timeinfo;
	timeinfo = localtime(&t);
	sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return time_str;
}

void log_set_level(log_level_t log_level_)
{
	log_level = log_level_;
}

void log_set_printf(printf_like_t func)
{
	s_log_print_func = func;
}

char *log_cut_str(char *full_path, uint8_t len)
{
	uint8_t k;
	char *ptr;
	k = strlen(full_path);

	if (k <= len)
		return full_path;

	ptr = full_path + (k - len);
	return ptr;
}
