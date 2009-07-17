CFLAGS+=-Wall -Werror -g
all: loadmap reserve_js

loadmap: loadmap.o dictionary.o mapparser.o programparser.o validkeys.o events.o vm.o devices.o
	$(CC) -g -o loadmap loadmap.o dictionary.o mapparser.o programparser.o validkeys.o events.o vm.o devices.o

reserve_js: reserve_js.o
	$(CC) -g -o reserve_js reserve_js.o

clean:
	rm -f reserve_js reserve_js.o loadmap loadmap.o dictionary.o mapparser.o programparser.o validkeys.o events.o vm.o devices.o
