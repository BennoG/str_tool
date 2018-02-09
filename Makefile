CC=gcc
CPP=g++

ifeq ($(MAKECMDGOALS),debug)
  DEBUG = -g -D_DEBUG
else
  DEBUG = -O3
endif

UNAME_P := $(shell uname -p)
ifneq ($(filter unknow%,$(UNAME_P)),)
  UNAME_P := $(shell cat /proc/cpuinfo | grep -i "model name" | awk '{print $4;}' | tail -1)
endif

ifneq ($(filter ARM%,$(UNAME_P)),)
  INSTDIR = ../../libraries/Arm/
  OBJDIR = obj-arm
endif
ifneq ($(filter arm%,$(UNAME_P)),)
  INSTDIR = ../../libraries/Arm/
  OBJDIR = obj-arm
endif
ifneq ($(filter aarch64%,$(UNAME_P)),)
  INSTDIR = ../libs-arm64/
  OBJDIR = obj-arm64
endif
ifneq ($(filter %86,$(UNAME_P)),)
  INSTDIR = ../libs-x86/
  ARCH=-m32
  OBJDIR = obj-x32
endif
ifneq ($(filter %86_64,$(UNAME_P)),)
  INSTDIR = ../libs-x64/
  ARCH=-m64
  OBJDIR = obj-x64
endif


#CFLAGS=$(DEBUG) -fPIC -Wall -c -D_REENTRANT -I. -Wno-pointer-sign
CFLAGS=$(DEBUG) -fPIC $(ARCH) -Wall -c -D_REENTRANT -I. -I/usr/include/libxml2/
LIBS=-lpthread

PROGS = libstr_tool
VPATH = .:$(OBJDIR)

# object files excluding destination path
OBJS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.c,%.o,$(wildcard *.c))
# object files including destination path
OBJSP = $(patsubst %.o, $(OBJDIR)/%.o, $(OBJS))

all:  depend $(PROGS).a
#  $(PROGS).so

.c.o:
	@mkdir -p $(OBJDIR)
	@echo $*.c
	@$(CC) $(CFLAGS) $*.c -o $(OBJDIR)/$*.o

.cpp.o:
	@mkdir -p $(OBJDIR)
	@echo $*.cpp
	@${CPP} ${CFLAGS} ${CPPFLAGS} ${INCLUDE} -c $*.cpp -o $(OBJDIR)/$*.o


$(PROGS).a: $(OBJS)
	@ar rcs $(PROGS).a $(OBJSP)

$(PROGS).so: $(OBJS)
	@g++ $(ARCH) -shared -Wl,-soname,$(PROGS).so.1 -o $(PROGS).so.1.0.0 $(OBJSP) -lc

clean:
	@rm -f $(PROGS).a $(PROGS).so $(PROGS).so.1.0.0 $(OBJSP)

install: depend $(PROGS).a
	@mkdir -p $(INSTDIR)
	@mkdir -p ../../includes/include/
	cp -p $(PROGS).a    $(INSTDIR)
	@rm -f $(PROGS).a
	cp -p stl*.h    ../../includes/include/

debug: install

depend:
	@mkdir -p $(OBJDIR)
	@rm -f .depend
	@$(CPP) -MM $(INCLDIR) *.c >> .depend
	@$(CPP) -E -MM $(INCLDIR) *.cpp >> .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif

