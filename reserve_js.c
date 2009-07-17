#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_JOYSTICKS 10
#define WAIT_TIME 10

void register_devices() {
	int i, j;
	struct uinput_user_dev dev;
	int fd[NUM_JOYSTICKS];
	for (i=0; i<NUM_JOYSTICKS; i++) {
		fd[i] = open("/dev/misc/uinput", O_WRONLY);
		ioctl(fd[i], UI_SET_EVBIT, EV_KEY);
		ioctl(fd[i], UI_SET_EVBIT, EV_ABS);
		ioctl(fd[i], UI_SET_ABSBIT, ABS_X);
		ioctl(fd[i], UI_SET_ABSBIT, ABS_Y);
		ioctl(fd[i], UI_SET_ABSBIT, ABS_RUDDER);
		ioctl(fd[i], UI_SET_KEYBIT, BTN_JOYSTICK+0);
		ioctl(fd[i], UI_SET_KEYBIT, BTN_JOYSTICK+1);

		memset(&dev, 0, sizeof(dev));
		sprintf(dev.name, "JOYMAP Joystick %d", i);

		dev.id.bustype = BUS_VIRTUAL;
		dev.id.vendor = 0x00ff;
		dev.id.product = 0x0001;
		dev.id.version = 0x0000;
		dev.ff_effects_max = 0;

		for (j = 0; j < ABS_MAX + 1; j++) {
			dev.absmax[j] = 32767;
			dev.absmin[j] = -32767;
			dev.absfuzz[i] = 0;
			dev.absflat[j] = 0;
		}
		write(fd[i], &dev, sizeof(dev));

		ioctl(fd[i], UI_DEV_CREATE);
	}

	sleep(WAIT_TIME);

	for (i=0; i<NUM_JOYSTICKS; i++) {
		ioctl(fd[i], UI_DEV_DESTROY);
		close(fd[i]);
	}
}

int main() {
	if (fork()==0) {
		register_devices();
	}
	sleep(1);
	return 0;
}
