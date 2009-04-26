/*
 * expand_var.c - expand variables in settings and config files.
 *                (non-working)
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


static char* 
expand(char* val)
{
	char* val_copy;
	const char* cur = NULL;
	int i = 0;
	int newlen = 0;
	int origlen = 0;
	int testnewlen = 0;
	char* newval;
	int j = 0;
	
	cur = val;

	origlen = strlen(val);
	newlen = origlen;
	testnewlen = newlen;
	for (i = 0; i < origlen; i++) {
		if (val[i] == '$') {
			if (val[++i] == '(') {
				char* varpos = &val[i];
				int varnamelen = 0;
				while(val[i] && val[i++] != ')') varnamelen++;
				if (val[i - 1] == ')') {
					const char* setting_val;
					val[i - 1] = '\0';
					setting_val = get_string_setting(varpos);
					if (!setting_val) {
						setting_val = getenv(varpos);
					}
					if (!setting_val) {
						setting_val = strdup("");
						if (!setting_val) return NULL;
					};
					newlen += strlen(setting_val);
					val[i - 1] = ')';
				} else {
					return NULL;
				}
			}
		}
	}
	newval = (char*)malloc(newlen + 1);
	for (i = 0; i < origlen; i++) {
		if (val[i] == '$') {
			i++;
			if (val[i] == '(') {
				char* varpos = &val[i];
				const char* setting_val = NULL;
				char* setting_val_len = 0;
				setting_val = get_string_setting(varpos);
				if (!setting_val) {
					setting_val = getenv(varpos);
				}
				if (!setting_val) {
					setting_val = malloc(1);
					setting_val strcpy(setting_val, "");
				}
				setting_val_len = strlen(setting_val);
				testnewlen += strlen(setting_val);
				if (testnewlen <= newlen) { /* extra check to make sure */
					strcpy(&newval[j], setting_val);
					i += strlen(varpos) + 1;
					j += setting_val_len;
				}
			}
		}
	}
	free(val);
	return newval;
}


