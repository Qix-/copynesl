/*
 * trk_log.c - Configurable error, debug and tracing for programs.
 *
 * Copyright (C) Bjorn Hedin 2009 <cradelit@gmail.com>
 * Copyright (C) David Huseby 2009 <dave@linuxprogrammer.org>
 * 
 * This file is part of copynesl.
 *
 * copynesl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * copynesl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with copynesl.  If not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <libgen.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <trk_log/trk_log.h>

static void error (FILE* stream, int print_errno, int exit_status, const char* mode, const char* message, va_list ap);
char program_name[100];
static enum trk_severity tracelevel = TRK_WARNING;
extern int errno;

/*
 * errorcode ignored if 0, otherwise, used as errno.
 */
static void
error (FILE* stream, int print_errno, int exit_status, const char* mode, const char* message, va_list ap)
{
	char buf[256];

	fprintf(stream, "%s: %s: ", program_name, mode);
	vfprintf(stream, message, ap);
	if (print_errno && errno != 0) {
		fprintf(stream, ": %s", strerror(errno));
	}
	fprintf(stream, ".\n");

	if (exit_status >= 0) {
		exit (exit_status);
	}
}

extern void trk_log(enum trk_severity severity, const char* message, ...)
{
	if (tracelevel >= severity) {
		va_list ap;
		int print_errno = 0;
		int exit_status = -1;
		const char* mode = NULL;
		va_start(ap, message);
		switch (severity) {
			case TRK_FATAL:
				mode = "FATAL";
				print_errno = 1;
				exit_status = 1;
				break;
			case TRK_ERROR:
				mode = "ERROR";
				print_errno = 1;
				break;
			case TRK_WARNING:
				mode = "warning";
				break;
			case TRK_VERBOSE:
				mode="verbose";
				break;
			case TRK_DEBUG:
				mode = "debug";
				break;
			case TRK_DEBUGVERBOSE:
				mode = "debug verbose";
				break;

			default:
				mode = "invalid";
				break;
		}
		error (stdout, print_errno, exit_status, mode, message, ap);
		va_end(ap);
	}
}

int
trk_set_program_name (char* path)
{
	if (strlen (basename(path)) < 99) {
		strcpy(program_name, path);
		return 0;
	} else {
		return -1;
	}
}

void
trk_set_tracelevel (enum trk_severity mode)
{
	tracelevel = mode;
}
