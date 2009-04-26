/*
 * trk_log.h - Configurable error, debug and tracing for programs.
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


#ifndef COPYNESL_ERROR_HANDLING_H
#define COPYNESL_ERROR_HANDLING_H 1

enum trk_severity { TRK_NONE=0,TRK_FATAL,TRK_ERROR,TRK_WARNING,TRK_VERBOSE,TRK_DEBUG,TRK_DEBUGVERBOSE };
/* extern enum trace_level;
*/
/*extern char* program_name;
*/

extern void trk_log(enum trk_severity, const char* message, ...);
extern int trk_set_program_name (char* argv0);
extern void trk_set_tracelevel (enum trk_severity level);

#endif
