# Makefile for MemTest-86
#
# Author:		Chris Brady
# Created:		January 1, 1996

#
# Path for the floppy disk device
#
FDISK=/dev/fd0

CC=gcc
#
# gcc compiler options, these settings should suffice
#
CCFLAGS=-Wall -m32 -march=i486 -Os -fomit-frame-pointer -fno-builtin -ffreestanding

AS=as -32

OBJS= head.o reloc.o main.o test.o init.o lib.o patn.o screen_buffer.o config.o linuxbios.o memsize.o pci.o controller.o extra.o random.o 

all: memtest.bin memtest

reloc.o: reloc.c
	$(CC) -c -m32 -march=i486 -fPIC -Wall -g -O2 -fno-strict-aliasing reloc.c

test.o: test.c test.h defs.h config.h
	$(CC) -c $(CCFLAGS) test.c

main.o: main.c test.h defs.h
	$(CC) -c $(CCFLAGS) -fPIC main.c

init.o: init.c test.h defs.h io.h config.h controller.h pci.h
	$(CC) -c $(CCFLAGS) -fPIC init.c

linuxbios.o: linuxbios.c test.h linuxbios_tables.h defs.h config.h
	$(CC) -c $(CCFLAGS) -fPIC linuxbios.c

memsize.o: memsize.c test.h defs.h config.h
	$(CC) -c $(CCFLAGS) -fPIC memsize.c

lib.o: lib.c test.h defs.h io.h screen_buffer.h serial.h config.h
	$(CC) -c $(CCFLAGS) -fPIC lib.c

screen_buffer.o: screen_buffer.c screen_buffer.h test.h config.h
	$(CC) -c $(CCFLAGS) -fPIC screen_buffer.c

random.o: random.c
	$(CC) -c $(CCFLAGS) -fPIC random.c

patn.o: patn.c
	$(CC) -c $(CCFLAGS) -fPIC patn.c

config.o: config.c test.h controller.h screen_buffer.h
	$(CC) -c $(CCFLAGS) -fPIC config.c

pci.o: pci.c pci.h io.h
	$(CC) -c $(CCFLAGS) -fPIC pci.c

controller.o: controller.c defs.h config.h test.h pci.h controller.h
	$(CC) -c $(CCFLAGS) -fPIC controller.c

extra.o: config.c test.h screen_buffer.h extra.h
	$(CC) -c $(CCFLAGS) -fPIC extra.c

controller.s: controller.c defs.h config.h test.h pci.h controller.h
	$(CC) -S $(CCFLAGS) -fPIC controller.c

head.s: head.S
	$(CC) -E -m32 -traditional $< -o $@

head.o: head.s
	$(AS) -o $@ $<

makedefs: makedefs.c defs.h
	$(CC) $(CCFLAGS) makedefs.c -o $@


# Link it statically once so I know I don't have undefined
# symbols and then link it dynamically so I have full
# relocation information
memtest_shared: $(OBJS) memtest_shared.lds Makefile
	$(LD) --warn-constructors --warn-common -static -T memtest_shared.lds -o $@ $(OBJS) && \
	$(LD) -shared -Bsymbolic -T memtest_shared.lds -o $@ $(OBJS)

memtest_shared.bin: memtest_shared
	objcopy -O binary $< memtest_shared.bin

memtest: memtest_shared.bin memtest.lds
	$(LD) -s -T memtest.lds -b binary memtest_shared.bin -o $@

bootsect.s: bootsect.S defs.h
	$(CC) -E -traditional $< -o $@

bootsect.o: bootsect.s
	$(AS) -o $@ $<

bootsect: bootsect.o
	$(LD) -Ttext 0x00 -s --oformat binary -e _main --just-symbols=memtest_shared.o -o $@ $<

setup.s: setup.S config.h defs.h
	$(CC) -E -traditional $< -o $@

setup.o: setup.s
	$(AS) -o $@ $<


memtest.bin: memtest_shared.bin bootsect.o setup.o memtest.bin.lds
	$(LD) -T memtest.bin.lds bootsect.o setup.o -b binary memtest_shared.bin -o memtest.bin

clean:
	rm -f *.o *.s memtest.bin bootsect setup low_mapfile high_mapfile \
		memtest memtest.out makedefs defs.lds memtest_shared memtest_shared.bin

wormkill: 
	rm -f *~

install: all
	dd <memtest.bin >$(FDISK) bs=8192

install-bin:
	dd <precomp.bin >$(FDISK) bs=8192
