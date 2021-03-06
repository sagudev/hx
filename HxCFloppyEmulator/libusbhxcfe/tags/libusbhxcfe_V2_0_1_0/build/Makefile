#CC=i386-pc-linux-gcc
CC=gcc
AR=ar

UNAME := $(shell uname)

BASEDIR=../sources
INCLUDES = -I$(BASEDIR)/ -I ../../../libhxcfe/trunk/sources -I ../sources/win32 -I ../../../libhxcadaptor/trunk/sources

DEBUG ?= 0

ifeq ($(DEBUG), 1)
	CFLAGS=-O0 $(INCLUDES) -Wall -g -DDEBUG -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS= -shared -lc -lm -ldl -lasan
else
	CFLAGS=-O3 $(INCLUDES) -Wall
	LDFLAGS= -shared -lc -lm -ldl -s
endif

EXEC=libusbhxcfe.so

ifeq ($(UNAME), Linux)
CFLAGS += -fPIC -Wl,-Map,foo.map
endif

ifeq ($(UNAME), Darwin)
CFLAGS += -arch i386 -mmacosx-version-min=10.5
LDFLAGS += -arch i386 -dynamiclib -current_version 2.0 -install_name @executable_path/../Frameworks/libusbhxcfe.dylib -mmacosx-version-min=10.5
EXEC=libusbhxcfe.dylib
endif

ifeq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
EXEC=libusbhxcfe.dll
endif

all: $(EXEC)

libusbhxcfe.dll: usb_hxcfloppyemulator.o variablebitrate.o ftdi.o
	$(AR) r $@    $^ 
	cp $@ ../../../build

libusbhxcfe.dylib: usb_hxcfloppyemulator.o variablebitrate.o ftdi.o
	$(AR) r $@    $^ 
	cp $@ ../../../build

libusbhxcfe.a: usb_hxcfloppyemulator.o variablebitrate.o ftdi.o
	$(AR) r $@    $^ 
	cp $@ ../../../build

libusbhxcfe.so: usb_hxcfloppyemulator.o variablebitrate.o ftdi.o
	$(CC) -o $@    $^ $(LDFLAGS)
	cp $@ ../../../build

usb_hxcfloppyemulator.o: $(BASEDIR)/usb_hxcfloppyemulator.c
	$(CC) -o $@ -c $< $(CFLAGS)

variablebitrate.o: $(BASEDIR)/variablebitrate.c
	$(CC) -o $@ -c $< $(CFLAGS)

ftdi.o: $(BASEDIR)/ftdi.c
	$(CC) -o $@ -c $< $(CFLAGS)

ftdi_d2xxmode.o : ftdi.c ftdi.h ftd2xx.h WinTypes.h
	$(CC) -o $@ -c $< $(CFLAGS)

ftdi_libftdimode.o : ftdi.c ftdi.h ftd2xx.h WinTypes.h
	$(CC) -o $@ -c $< $(CFLAGS) -DFTDILIB

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper
