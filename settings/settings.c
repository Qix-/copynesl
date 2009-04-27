/*
 * settings.c - Store and restore settings for programs.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <settings/settings.h>
#include <trk_log/trk_log.h>
#include "array.h"

struct stored_settings {
  enum settings_type type;
  char* long_opt;
  void* value;
  struct stored_settings* next;
};

enum settings_command {
  INIT,
  GET,
  SET,
  LIST,
  FREE
};

/* prototypes */
static void load_defaults(struct settings_init* init, int initlen);
static int load_commandline(int argc, char** argv, struct settings_init* init, int initlen);
static void load_environment(struct settings_init* init, int initlen);
static void print_usage(FILE* fp, char* program_name, struct settings_init* init, int initlen);
static void print_version(void);
static void load_configfile(struct settings_init* init, int initlen, char* filepath, ...);

/* setting storage prototypes */
static int settings(enum settings_command cmd, enum settings_type* type, const char* key, void** value);
static struct stored_settings* settings_init(struct stored_settings** settings, const char* key);
static void settings_list(struct stored_settings* settings);
static int settings_set(struct stored_settings** settings, enum settings_type type, const char* key, void* value);
static enum settings_type settings_get(struct stored_settings* settings, const char* key, void** value);
static int settings_free(struct stored_settings* settings);
void setting_free(struct stored_settings* setting);

/* external functions */
int 
load_settings(int argc, char** argv, struct settings_init* init)
{
	int errorcode = 0;
	int len = 0;
	struct settings_init* cur = NULL;

	cur = init;
	while(cur->long_opt) {
		len++;
		cur++;
	}

	load_defaults(init, len);
	if (errorcode) return errorcode;

	load_configfile(init, len, "/etc/%s/config", PACKAGE);
	load_configfile(init, len, "%s/.%s/config", getenv("HOME"), PACKAGE);

	load_environment(init, len);
	
	errorcode = load_commandline(argc, argv, init, len);
	if (errorcode) return errorcode;

	trk_log(TRK_DEBUG, "All Settings loaded successfully.");

	return errorcode;
}

void 
settings_usage(char* program_name, struct settings_init* init)
{
	int len = 0;
	struct settings_init* cur = NULL;
	cur = init;
	while(cur->long_opt) {
		len++;
		cur++;
	}
	print_usage(stdout, program_name, init, len);
}

enum settings_type
get_setting(const char* key, void** value)
{

	enum settings_type type;
	int errorcode = 0;
	errorcode = settings(GET, &type, key, value);
	if (errorcode) {
		return 0;
	} else {
		return type;
	}

}

int
set_setting(enum settings_type type, const char* key, void* value)
{
	int errorcode = 0;
	errorcode = settings(SET, &type, key, (void*) &value);
	return errorcode;
}

void
free_settings()
{
	settings(FREE, NULL, NULL, NULL);
}

int 
get_bool_setting(const char* setting)
{
	enum settings_type type;
	void* val = NULL;
	type = get_setting(setting, &val);
	if ((type == BOOLEAN_SETTING) && ((int)val)) {
		return 1;
	} else {
		return 0;
	}
}

int 
get_int_setting(const char* setting)
{
	enum settings_type type;
	void* val = NULL;
	type = get_setting(setting, &val);
	if ((type == INT_SETTING)) {
		return (int)val;
	} else {
		return -1;
	}
}

const char* 
get_string_setting(const char* setting)
{
	enum settings_type type;
	void* val = NULL;
	type = get_setting(setting, &val);
	if ((type == STRING_SETTING) && (val)) {
		return (const char*)val;
	} else if ((type == STR_ARRAY_SETTING) && (val)) {
		return get_str_from_array(val);
	} else {
		return NULL;
	}
}

int reset_string_setting(const char* setting)
{
	enum settings_type type;
	void* val = NULL;
	type = get_setting(setting, &val);
	if ((type == STR_ARRAY_SETTING) && (val)) {
		reset_array(val);
		return 0;
	} else {
		return -1;
	}
}

/* internal functions */

static void 
load_defaults(struct settings_init* init, int initlen)
{
	int i = 0;
	for (i = 0;i < initlen;i++) {
		if (init[i].default_val) {
			set_setting(init[i].val_type, init[i].long_opt, init[i].default_val);	
		}
	};
}

static int 
load_commandline(int argc, char** argv, struct settings_init* init, int initlen)
{
#ifdef HAVE_GETOPT_LONG
	struct option* lopt;
#endif /* HAVE_GETOPT_LONG */
	char* sopt;
	char c = 0;
	int i = 0;
	int j = 0;
	int errorcode = 0;

	/* reset to 1 in case there are multiple calls to load_commandline */
	optind = 1;

	sopt = (char*)malloc(initlen * 3 + 2); /* up to 3 chars per option */

	/* if any options are looking for character code 1
	 * then add a '-' to the front of the optstring. */
	for (i = 0; i < initlen; i++) {
		if (init[i].has_arg >= 0 && init[i].short_opt == 1) {
			sopt[j++] = '-';
		}
	}

	for (i = 0; i < initlen; i++) {
		sopt[j] = init[i].short_opt;
		if (init[i].has_arg == 1) { 
			sopt[++j] = ':'; 
		} else if (init[i].has_arg == 2) {
			sopt[++j] = ':';
			sopt[++j] = ':';
		}
		j++;
	}
	sopt[j] = '\0';
	trk_log(TRK_DEBUG, "parsing command line arguments. Short options string used: %s", sopt);
#ifdef HAVE_GETOPT_LONG
		lopt = (struct option*)malloc(initlen * sizeof (struct option));
		for (i = 0; i < initlen; i++) {
			lopt[i].name = init[i].long_opt;
			lopt[i].has_arg = init[i].has_arg;
			lopt[i].flag = NULL;
			lopt[i].val = init[i].short_opt; 
		}
#endif

  /* TODO when we have a gui interface, set option ui to appropriately. 
   * according to the program name. */


	while(c >= 0) {
		if (errorcode) break;
#ifdef HAVE_GETOPT_LONG
		c = getopt_long(argc, argv, sopt, lopt, NULL);
#else /* !HAVE_GETOPT_LONG */
		c = getopt(argc, argv, sopt);
#endif /* HAVE_GETOPT_LONG */
		if (c >= 0) {
			int match = 0;
			for (i = 0; i < initlen; i++) {
				if (init[i].has_arg >= 0 && c == init[i].short_opt) {
					match = 1;
					if (c == 'h') {
						print_usage(stdout, argv[0], init, initlen);
						errorcode = 1;
						break;
					} else if (c == 'V') {
						print_version();
						errorcode = 2;
						break;
					} else {
						/* depending on the val_type, we have different arg types */
						if (optarg) {
							if ((init[i].val_type == BOOLEAN_SETTING) || (init[i].val_type == INT_SETTING)) {
								set_setting(init[i].val_type, init[i].long_opt, (void*)atoi(optarg));
							} else if (init[i].val_type == STRING_SETTING || init[i].val_type == STR_ARRAY_SETTING) {
								set_setting(init[i].val_type, init[i].long_opt, (void*)optarg);
							} else {
								errorcode = -1;
								break;
							}
						} else {
							if (init[i].val_type == BOOLEAN_SETTING) {
								set_setting(init[i].val_type, init[i].long_opt, (void*)1);
							} else if (init[i].val_type == INT_SETTING) {
								set_setting(init[i].val_type, init[i].long_opt, (void*)-1);
							} else if (init[i].val_type == STRING_SETTING || init[i].val_type == STR_ARRAY_SETTING) {
								set_setting(init[i].val_type, init[i].long_opt, "");
							} else {
								errorcode = -1;
								break;
							}
						}
					}
				}
			}
			if (errorcode) break;
			if (!match) {
				fprintf(stderr, "Invalid command line argument encountered.\n");
				print_usage(stderr, argv[0], init, initlen);
				errorcode = -1;
				break;
			}
		}
	}

	free(sopt);
#ifdef HAVE_GETOPT_LONG
	free(lopt);
#endif
  	return errorcode;
}

static void 
load_environment(struct settings_init* init, int initlen)
{
	int i = 0;
	char* setting = NULL;
	for (i = 0; i < initlen; i++) {
		if (init[i].env_var) {
			setting = getenv(init[i].env_var);
			if (setting) {
				set_setting(init[i].val_type, init[i].long_opt, (void*)setting);
			}
		}

	}
}

static void 
print_usage(FILE* fp, char* program_name, struct settings_init* init, int initlen)
{
	int i = 0;
	fprintf(fp, "Usage: %s [OPTION(S)] \n"
		    "\n"
		    "Valid options are:\n"
	, program_name);

	for (i = 0; i < initlen; i++) {
		if (init[i].has_arg >= 0) {
			if (init[i].usage_long) fprintf(fp, init[i].usage_long);
		}
	}
	fprintf(fp, "\n");
}

static void
print_version(void)
{
	fprintf(stdout, "%s version %s\n", PACKAGE, VERSION);
}

static void 
load_configfile(struct settings_init* init, int initlen, char* filepath, ...)
{
	char configfilepath[MAX_CONFIGFILE_PATH];

	va_list ap;
	va_start(ap, filepath);
	vsprintf(configfilepath, filepath, ap);
	va_end(ap);
}


/*
 * Generic function used by settings functions to access settings.
 * required parameters vary depending on command.  
 * unrequired parameters can be NULL.
 *
 * @param cmd - type of call. INIT, GET, SET, LIST, FREE
 * @param type - pointer to the type of the parameter.  this should be full for set, empty for get, null otherwise.
 * @param key - the setting key (equivilent to the long option value passed in)
 * @param value - empty fot get, full for set.
 *
 * @return negative on error.
 */
static int
settings(enum settings_command cmd, enum settings_type* type, const char* key, void** value)
{
	static struct stored_settings* settings = NULL;

	switch(cmd) {
		case INIT:
			settings_init(&settings, NULL);
			return 0;
		case GET:
			*type = settings_get(settings, key, value); 
			return 0;
		case SET:
			return settings_set(&settings, *type, key, *value);
		case LIST:
			settings_list(settings);
			return 0;
		case FREE:			
			settings_free(settings);
			free(settings);
			return 0;
	}

	return -1;
}

/* - initialize setting with the given key and return a pointer to the initialized setting.
 * - if key is null, just ensure that *settings is properly initialized.
 */
static struct stored_settings*
settings_init(struct stored_settings** settings, const char* key)
{
	struct stored_settings* cur = NULL;
	if (!(*settings)) {
	    *settings = (struct stored_settings*)malloc(sizeof(struct stored_settings));
	    cur = *settings;
	} else if (key) { 
	    cur = *settings;
	    while (cur->next && strcmp(cur->long_opt, key)) cur = cur->next;
	    if (!strcmp(cur->long_opt, key)) {
		return cur;
	    } else {
		cur->next = (struct stored_settings*)malloc(sizeof(struct stored_settings));
		cur = cur->next;
	    }
	}
	if(cur == NULL) { /* malloc failed */
	    	return NULL;
	}
	cur->type = 0;
	if (key) {
		cur->long_opt = (char*)malloc(strlen(key) + 1);
		if(cur->long_opt == NULL) return NULL;
		strcpy(cur->long_opt, key);
	} else {
		cur->long_opt = NULL;
	}
	cur->value = NULL;
    	cur->next = NULL;

	/* cur will now point either to the found setting, or to a new setting,
	 * properly initialized.
	 */
	return cur;
}

static int
settings_set(struct stored_settings** settings, enum settings_type type, const char* key, void* value)
{

  struct stored_settings* cur;
  int errorcode = 0;

  cur = settings_init(settings, key);

  if(!cur || (cur->type && cur->type != type)) { /* invalid type */
    return -1;
  }

  cur->type = type;

  if (type == BOOLEAN_SETTING || type == INT_SETTING) {
	trk_log(TRK_DEBUGVERBOSE, "setting int setting %d", value);
  	cur->value = value;
  } else if (type == STRING_SETTING) {
  	cur->value = (char*)malloc(strlen( (const char*) value) + 1);
	strcpy((char*)cur->value, (const char*)value);
  } else if (type == STR_ARRAY_SETTING) {
  	errorcode = add_to_array((struct val_array**)&cur->value, (const char*)value); 
  } else { 
  	return -1;
  }

  return 0;
}

static enum settings_type 
settings_get(struct stored_settings* settings, const char* key, void** value)
{
	struct stored_settings* cur = settings;
	
	if (!cur) return 0;
	while (cur && strcmp(cur->long_opt, key)) cur = cur->next;
	if (!cur) {
		*value = NULL;
		return 0;
	} else {
		if (cur->type == BOOLEAN_SETTING || cur->type == INT_SETTING) {
			*value = cur->value;
		} else {
			*value = cur->value;
		}
		return cur->type;
	}
}

/* dump settings to stdout (for debugging) */
static void
settings_list(struct stored_settings* settings)
{
	struct stored_settings* cur = settings;
	while (cur) {
		if (cur->value) {
			if (cur->type == BOOLEAN_SETTING) {
				fprintf(stdout, "key: %s, type: bool, val: true", cur->long_opt);
			} else if (cur->type == INT_SETTING) {
				fprintf(stdout, "key: %s, type: int, val: %d", cur->long_opt, (int)cur->value);
			} else if (cur->type == STRING_SETTING) {
				fprintf(stdout, "key: %s, type: string, val: %s", cur->long_opt, (char*)cur->value);
			} else {
				fprintf(stdout, "key: %s, type: invalid, val: unknown", cur->long_opt);
			}
		}
		cur = cur->next;
	}
	return;
}

static int
settings_free(struct stored_settings* settings)
{
  struct stored_settings* cur;
  struct stored_settings* tmp;
  
  cur = settings;
  while(cur) {
    tmp = cur;
    cur = tmp->next;
    setting_free(tmp);
    free(tmp);
  }

  free(settings);

  settings = NULL;
  return 0;
}

void 
setting_free(struct stored_settings* setting)
{
  if (setting->type == STRING_SETTING && setting->value) {
  	free(setting->value);
  } else if (setting->type == STR_ARRAY_SETTING && setting->value) {
  	free_array(setting->value);
  }
  if(setting->long_opt) free(setting->long_opt);
  setting->long_opt = NULL;
  setting->value = NULL;
}



