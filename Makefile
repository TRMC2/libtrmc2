# Makefile for libtrmc2 on Linux. Use gnu make.


########################################################################
# Configure the variables below before installing

INSTALLDIR=/usr/local
LIBDIR=$(INSTALLDIR)/lib
INCLUDEDIR=$(INSTALLDIR)/include
CFLAGS=-O -Wall -Wextra


########################################################################
# Nothing should have to be configured below this point

# OS detection
OS_ID := $(shell grep '^ID=' /etc/os-release | sed 's/ID=//')

# Library name and version numbers
NAME  = libtrmc2
MAJOR = 2
MINOR = 0

# Command options
CC       = gcc
CFLAGS  += -fPIC
CPPFLAGS =
LDLIBS   =

# Use libgpiod on Raspbian. Otherwise assume this is a PC and use the
# control lines of one of the first two serial ports.
ifeq ($(OS_ID), raspbian)
    TrmcLin.o: CPPFLAGS += -DRASPBERRY_PI
    LDLIBS += -lgpiod
endif

# Files
SONAME=$(NAME).so.$(MAJOR)
LIB=$(SONAME).$(MINOR)
HEADERS=Trmc.h TrmcBoard.h TrmcBoardA.h TrmcBoardB.h TrmcBoardC.h \
	TrmcBoardD.h TrmcBoardE.h TrmcBoardF.h TrmcBoardG.h TrmcDac.h \
	TrmcDef.h TrmcPlatform.h TrmcProto.h TrmcRegul.h TrmcRunLib.h
SRC=TrmcAux.c TrmcBoard.c TrmcBoardA.c TrmcBoardADEF.c TrmcBoardB.c \
	 TrmcBoardC.c TrmcBoardD.c TrmcBoardE.c TrmcBoardF.c \
	 TrmcBoardG.c TrmcDac.c TrmcRegul.c TrmcRunLib.c TrmcLin.c
OBJS=$(SRC:%.c=%.o)

# Rules

$(LIB):	$(OBJS) Makefile
	$(CC) -shared -Wl,-soname,$(SONAME) $(OBJS) $(LDLIBS) -o $@

%.o:	%.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

tags:	$(HEADERS) $(SRC)
	ctags $^

clean:
	rm -f tags $(OBJS)

distclean:	cleanall

cleanall:
	rm -f tags $(OBJS) $(LIB)

install:	$(LIB)
		cp -a $^ $(LIBDIR)
		chown root.root $(LIBDIR)/$(LIB)
		(cd $(LIBDIR) && ln -sf $(LIB) $(SONAME))
		(cd $(LIBDIR) && ln -sf $(SONAME) $(NAME).so)
		/sbin/ldconfig
		cp -a Trmc.h $(INCLUDEDIR)
		chown root.root $(INCLUDEDIR)/Trmc.h

uninstall:
		rm -f $(LIBDIR)/$(NAME).so
		rm -f $(LIBDIR)/$(SONAME)
		rm -f $(LIBDIR)/$(LIB)
		rm -f $(INCLUDEDIR)/Trmc.h

# Dependencies

COMMON=Trmc.h TrmcRunLib.h TrmcDef.h TrmcProto.h

TrmcAux.o:		$(COMMON) TrmcPlatform.h
TrmcBoard.o:		$(COMMON) TrmcBoard.h
TrmcBoardA.o:		$(COMMON) TrmcBoard.h TrmcBoardA.h
TrmcBoardADEF.o:	$(COMMON) TrmcBoard.h
TrmcBoardB.o:		$(COMMON) TrmcBoard.h TrmcBoardB.h
TrmcBoardC.o:		$(COMMON) TrmcBoard.h TrmcBoardC.h
TrmcBoardD.o:		$(COMMON) TrmcBoard.h TrmcBoardD.h
TrmcBoardE.o:		$(COMMON) TrmcBoard.h TrmcBoardE.h
TrmcBoardF.o:		$(COMMON) TrmcBoard.h TrmcBoardF.h
TrmcBoardG.o:		$(COMMON) TrmcBoard.h TrmcBoardG.h
TrmcDac.o:		$(COMMON) TrmcBoard.h TrmcDac.h
TrmcRegul.o:		$(COMMON) TrmcBoard.h TrmcRegul.h
TrmcRunLib.o:		$(COMMON) TrmcPlatform.h TrmcRegul.h
TrmcLin.o:		$(COMMON) TrmcPlatform.h

# vim: set ts=8 sw=8:
