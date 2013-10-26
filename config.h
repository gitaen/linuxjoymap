#ifndef __config_h
#define __config_h

#define CONFIG_MAX 10
#define UINPUT_DEV 0

char *get_config(int key);
char *set_config(int key, char *value);

#endif
