#include "config.h"

char *current_config[CONFIG_MAX] = {
    /* UINPUT_DEV */
    "/dev/uinput",
    /* EVENT_DEV */
    "/dev/input/event",
};

char *cmdline_arg[CONFIG_MAX] = {
    /* UINPUT_DEV */
    "--uinput_dev",
    /* EVENT_DEV */
    "--event_dev",
};

char *get_config(int key) {
    if ((key >= CONFIG_MAX) || (key < 0)) {
        fprintf(stderr, "ERR: Invalid config key in code: %d\n", key);
        exit(1);
    }
    return current_config[key];
}

char *set_config(int key, char *value) {
    if ((key >= CONFIG_MAX) || (key < 0)) {
        fprintf(stderr, "ERR: Invalid config key in code: %d\n", key);
        exit(1);
    }
    current_config[key] = value;
}

void cmdline_config(int argc, char *argv[]) {
}
