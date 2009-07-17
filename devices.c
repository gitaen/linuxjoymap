#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mapper.h"

#define NUM_JOYSTICKS 10

struct joystick_device {
	int axes;
	int buttons;
	int fd;
};

static struct joystick_device devices[NUM_JOYSTICKS]={
		{0,0,-1},
		{0,0,-1},
		{0,0,-1},
		{0,0,-1},
		{0,0,-1},
		{0,0,-1},
		{0,0,-1},
		{0,0,-1},
		{0,0,-1},
		{0,0,-1}
};

static int mouse_fd;
static int kbd_fd;
static int code_fd;
static int njoysticks;
static int x;
static int y;
static int dx;
static int dy;
static int dwheel;

void set_num_joysticks(int num) {
	njoysticks=num;
}

void set_num_axes(int js, int axes) {
	if ((js<0)||(js>NUM_JOYSTICKS)) return;
	devices[js].axes=axes;
}

void set_num_buttons(int js, int buttons) {
	if ((js<0)||(js>NUM_JOYSTICKS)) return;
	devices[js].buttons=buttons;
}

void register_devices() {
	int i, j;
	struct uinput_user_dev dev;
	for (i=0; i<NUM_JOYSTICKS; i++) {
		devices[i].fd = open("/dev/misc/uinput", O_WRONLY);
		ioctl(devices[i].fd, UI_SET_EVBIT, EV_KEY);
		ioctl(devices[i].fd, UI_SET_EVBIT, EV_ABS);
		for (j=0; j<devices[i].axes; j++)
			ioctl(devices[i].fd, UI_SET_ABSBIT, j);
		for (j=0; j<devices[i].buttons; j++)
			ioctl(devices[i].fd, UI_SET_KEYBIT, BTN_JOYSTICK+j);

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
		write(devices[i].fd, &dev, sizeof(dev));

		ioctl(devices[i].fd, UI_DEV_CREATE);
	}

	//now the mouse
	memset(&dev, 0, sizeof(dev));
	mouse_fd = open("/dev/misc/uinput", O_WRONLY);
	ioctl(mouse_fd, UI_SET_EVBIT, EV_KEY);
	ioctl(mouse_fd, UI_SET_EVBIT, EV_REL);
	ioctl(mouse_fd, UI_SET_KEYBIT, BTN_LEFT);
	ioctl(mouse_fd, UI_SET_KEYBIT, BTN_MIDDLE);
	ioctl(mouse_fd, UI_SET_KEYBIT, BTN_RIGHT);
	ioctl(mouse_fd, UI_SET_RELBIT, REL_X);
	ioctl(mouse_fd, UI_SET_RELBIT, REL_Y);
	ioctl(mouse_fd, UI_SET_RELBIT, REL_WHEEL);

	sprintf(dev.name, "JOYMAP Mouse");
	dev.id.bustype = BUS_VIRTUAL;
	dev.id.vendor = 0x00ff;
	dev.id.product = 0x0002;
	dev.id.version = 0x0000;
	dev.ff_effects_max = 0;
	write(mouse_fd, &dev, sizeof(dev));

	ioctl(mouse_fd, UI_DEV_CREATE);
	
	//now the keyboard
	memset(&dev, 0, sizeof(dev));
	kbd_fd = open("/dev/misc/uinput", O_WRONLY);
	ioctl(kbd_fd, UI_SET_EVBIT, EV_KEY);
	ioctl(kbd_fd, UI_SET_EVBIT, EV_REP);
	ioctl(kbd_fd, UI_SET_EVBIT, EV_MSC);
	ioctl(kbd_fd, UI_SET_EVBIT, EV_LED);
	ioctl(kbd_fd, UI_SET_LEDBIT, LED_NUML);
	ioctl(kbd_fd, UI_SET_LEDBIT, LED_CAPSL);
	ioctl(kbd_fd, UI_SET_LEDBIT, LED_SCROLLL);
	for (i=0; i<512; i++) 
		ioctl(kbd_fd, UI_SET_KEYBIT, i);

	sprintf(dev.name, "JOYMAP Keyboard");
	dev.id.bustype = BUS_VIRTUAL;
	dev.id.vendor = 0x00ff;
	dev.id.product = 0x0002;
	dev.id.version = 0x0000;
	dev.ff_effects_max = 0;
	write(kbd_fd, &dev, sizeof(dev));

	ioctl(kbd_fd, UI_DEV_CREATE);

	//and the code device
	memset(&dev, 0, sizeof(dev));
	code_fd = open("/dev/misc/uinput", O_WRONLY);
	ioctl(code_fd, UI_SET_EVBIT, EV_KEY);
	ioctl(code_fd, UI_SET_EVBIT, EV_ABS);
	for (j=0; j<ABS_MAX; j++)
		ioctl(code_fd, UI_SET_ABSBIT, j);
	for (j=0; j<KEY_MAX; j++)
		if (j>=BTN_JOYSTICK)
		if ((j!=BTN_TOUCH)&&(j!=BTN_TOOL_FINGER)) //make sure we don't get matched as some other device
			ioctl(code_fd, UI_SET_KEYBIT, j);

	sprintf(dev.name, "JOYMAP Code Device");

	dev.id.bustype = BUS_VIRTUAL;
	dev.id.vendor = 0x00ff;
	dev.id.product = 0x0000;
	dev.id.version = 0x0000;
	dev.ff_effects_max = 0;

	for (i = 0; i < ABS_MAX + 1; i++) {
		dev.absmax[i] = 32767;
		dev.absmin[i] = -32767;
		dev.absfuzz[i] = 0;
		dev.absflat[i] = 0;
	}
	write(code_fd, &dev, sizeof(dev));

	ioctl(code_fd, UI_DEV_CREATE);

	mapper_code_install();
}

void unregister_devices();

void press_key(int code, int value) {
	struct input_event event;
	event.type=EV_KEY;
	event.code=code;
	event.value=value;
	write(kbd_fd, &event, sizeof(event));
}

void release_keys(void) {
	int i;
	for (i=0; i<512; i++) {
		press_key(i, 0);
	}
}

void press_mouse_button(int code, int value) {
	struct input_event event;
	event.type=EV_KEY;
	event.code=code+BTN_LEFT;
	event.value=value;
	write(mouse_fd, &event, sizeof(event));
}

void set_mouse_pos(int cx, int cy) {
	dx=cx-x;
	dy=cy-y;
	move_mouse_x(dx);
	move_mouse_y(dy);
	x=cx;
	y=cy;
	dx=0;
	dy=0;
}

void move_mouse_x(int rdx) {
	struct input_event event;
	dx=rdx;
	x+=dx;
	event.type=EV_REL;
	event.code=REL_X;
	event.value=dx;
	write(mouse_fd, &event, sizeof(event));
}

void move_mouse_wheel(int rdw) {
	struct input_event event;
	dwheel=rdw;
	event.type=EV_REL;
	event.code=REL_WHEEL;
	event.value=dx;
	write(mouse_fd, &event, sizeof(event));
}

void move_mouse_y(int rdy) {
	struct input_event event;
	dy=rdy;
	y+=dy;
	event.type=EV_REL;
	event.code=REL_Y;
	event.value=dy;
	write(mouse_fd, &event, sizeof(event));
}

void move_mouse(int rdx, int rdy) {
	struct input_event event;
	dx=rdx;
	dy=rdy;
	x+=dx;
	y+=dy;
	event.type=EV_REL;
	event.code=REL_X;
	event.value=dx;
	write(mouse_fd, &event, sizeof(event));
	event.type=EV_REL;
	event.code=REL_Y;
	event.value=dy;
	write(mouse_fd, &event, sizeof(event));
}

void release_mouse_buttons(void) {
	press_mouse_button(0,0);
	press_mouse_button(1,0);
	press_mouse_button(2,0);
}

void press_joy_button(int j, int code, int value) {
	struct input_event event;
	if (j==255) {
		event.type=EV_KEY;
		event.code=code;
		event.value=value;
		write(code_fd, &event, sizeof(event));
	}
	if ((j<0)||(j>=NUM_JOYSTICKS)) return;
	event.type=EV_KEY;
	event.code=code;
	event.value=value;
	write(devices[j].fd, &event, sizeof(event));
}

void set_joy_axis(int j, int axis, int value) {
	struct input_event event;
	if (j==255) {
		event.type=EV_ABS;
		event.code=axis;
		event.value=value;
		write(code_fd, &event, sizeof(event));
	}
	if ((j<0)||(j>=NUM_JOYSTICKS)) return;
	event.type=EV_ABS;
	event.code=axis;
	event.value=value;
	write(devices[j].fd, &event, sizeof(event));
}
