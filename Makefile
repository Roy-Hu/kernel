#
#	Sample Makefile for COMP 421 Yalnix kernel and user programs.
#
#	The Yalnix kernel built will be named "yalnix".  *ALL* kernel
#	Makefiles for this lab must have a "yalnix" rule in them, and
#	must produce a kernel executable named "yalnix" -- we will run
#	your Makefile and will grade the resulting executable
#	named "yalnix".
#
#	Your project must be implemented using the C programming
#	language (e.g., not in C++ or other languages).
#

#
#	Define the list of everything to be made by this Makefile.
#	The list should include "yalnix" (the name of your kernel),
#	plus the list of user test programs you also want to be mae
#	by this Makefile.  For example, the definition below
#	specifies to make Yalnix test user programs test1, test2,
#	and test3.  You should modify this list to the list of your
#	own test programs.
#
#	For each user test program, the Makefile will make the
#	program out of a single correspondingly named sourc file.
#	For example, the Makefile will make test1 out of test1.c,
#	if you have a file named test1.c in this directory.
#
ALL = yalnix idle init console shell \
	  init1 init2 init3 bigstack blowstack brktest console delaytest \
	  forktest0 forktest1 forktest1b forktest2 forktest2b forktest3 forkwait0c forkwait0p \
	  forkwait1 forkwait1b forkwait1c forkwait1d shell trapillegal trapmath trapmemory \
	  ttyread1 ttywrite1 ttywrite2 ttywrite3

# forkwait0c cannot pass

#
#	You must modify the KERNEL_OBJS and KERNEL_SRCS definitions
#	below.  KERNEL_OBJS should be a list of the .o files that
#	make up your kernel, and KERNEL_SRCS should  be a list of
#	the corresponding source files that make up your kernel.
#
KERNEL_OBJS = source/global.o source/call.o source/pagetable.o source/trap.o source/init.o source/pcb.o yalnix.o source/terminal.o
KERNEL_SRCS = source/global.c source/call.c source/pagetable.c source/trap.c source/init.c source/pcb.c yalnix.c source/terminal.c

#
#	You should not have to modify anything else in this Makefile
#	below here.  If you want to, however, you may modify things
#	such as the definition of CFLAGS, for example.
#

PUBLIC_DIR = /clear/courses/comp421/pub
PUBLIC_DIR2 = .

CPPFLAGS = -I$(PUBLIC_DIR)/include -I$(PUBLIC_DIR2)/include
# CFLAGS = -g -Wall -Wextra -Werror
CFLAGS = -g -Wall -Wextra

LANG = gcc

%: %.o
	$(LINK.o) -o $@ $^ $(LOADLIBES) $(LDLIBS)

LINK.o = $(PUBLIC_DIR)/bin/link-user-$(LANG) $(LDFLAGS) $(TARGET_ARCH)

%: %.c
%: %.cc
%: %.cpp

all: $(ALL)

yalnix: $(KERNEL_OBJS)
	$(PUBLIC_DIR)/bin/link-kernel-$(LANG) -o yalnix $(KERNEL_OBJS)

clean:
	rm -f $(KERNEL_OBJS) $(ALL)

depend:
	$(CC) $(CPPFLAGS) -M $(KERNEL_SRCS) > .depend

#include .depend
