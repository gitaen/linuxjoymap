#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "dictionary.h"
#include "program.h"
#include "keys.h"
#include "validkeys.h"
#include "parser.h"
#include "mapper.h"

int main(int argc, char *argv[]) {
	int i;
	int parse_err;
	
	++argv, --argc;  /* skip over program name */
	if ( argc > 0 )
		fmap = fopen( argv[0], "r" );
	else
		fmap = stdin;
	if (fmap==NULL) {
		perror("Failed to open map");
		return 1;
	}

	program.program=PROGRAM_CODE;
	program.code[0]=HALT;
	parse_err=parse_map();
	if (!parse_err) {
		printf("%d joysticks.\n", njoysticks);
		set_num_joysticks(njoysticks);
		for (i=0; i<njoysticks; i++) {
			set_num_axes(i, joysticks[i].axes);
			set_num_buttons(i, joysticks[i].buttons);
			printf("joystick%d axes=%d buttons=%d.\n", i, joysticks[i].axes, joysticks[i].buttons);
		}

		register_devices();
		sleep(1);
		install_event_handlers();
		printf("%d button assignments.\n", nbuttons);
		for (i=0; i<nbuttons; i++) 
			remap_button(&buttons[i]);
		printf("%d axes assignments.\n", naxes);
		for (i=0; i<naxes; i++) 
			remap_axis(&axes[i]);
		for (i=0; i<nscript; i++) {
			set_joystick_number(scriptassign[i].vendor, scriptassign[i].product, scriptassign[i].device);
		}
		code_set_program(&program);
	} else {
		printf("Error in map file, nothing done.\n");
		return 1;
	}	

	while (1) {
		poll_joystick_loop();
	}

	mapper_code_uninstall();
	unregister_devices();
	return 0;
}
