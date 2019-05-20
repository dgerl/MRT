# -*- mode: makefile-gmake; -*-
#
# Autor: M.Herhold
# Datum: Juni 2017
# Organisation: Institut für Automatisierungstechnik
# Projekt: ART1 - Makefile
# Version: r3

# Nutzung des Makefiles um den Quellcode zu kompilieren:
# 1.) geben Sie auf der Kommandozeile "make" oder "make all" ein um die hier aufgeführten Quellcode-Dateien zu einem Programm zu compilieren.
# 2.) geben Sie auf der Kommandozeile "make run" ein um das, erfolgreich compilierte, Programm in einem separaten Terminal zu starten
#	alternativ kann auch einfach das erstellte Binary aufgerufen werden z.B. mit: "./Tuersteuerung"
# 3.) geben Sie auf der Kommandozeile "make clean" ein um das Projektverzeichnis aufzuräumen und die Compilierung aller Dateien ganz von vorn zu starten

# Hinweise: zum Erweitern des Projekts um zusätzliche Quellcode- und Header-Dateien
# tragen Sie hier alle ihre Quellcode Dateien [inklusive (zum Makefile) relativer Pfadangabe] ein.
# Beachten Sie dabei:
# entweder
# 1.1) schreiben Sie alle Dateinamen auf eine Zeile
# 1.2) trennen sie die Dateinamen durch ein Leerzeichen voneinander
# oder
# 2.1) Sie schließen jede Zeile mit einem '\' ab.
# 2.2) Achten Sie peinlichst genau darauf, dass nach dem '\' keine weiteren Zeichen folgen, auch keine Leerzeichen!
# 2.3) rücken Sie die Dateinamen in jeder neuen Zeile mit einem <TAB> ein!

# in diese Variable gehören alle zu kompilierenden C++-Quellcode-Dateien
CC_FILES = \
	DoorInterface.cpp \
	DoorControl.cpp

# in diese Variable gehören alle zu kompilierenden C++-Header-Dateien
HH_FILES = \
	DoorInterface.h \
	DoorControl.h

# in diese Variable gehören alle zu kompilierenden C-Quellcode-Dateien
C_FILES = \
	Library/niusb6501.c

# in diese Variable gehören alle zu kompilierenden C-Header-Dateien
H_FILES = \
	Library/niusb6501.h

# der Name des zu erstellenden Programms
BINARY = Tuersteuerung

# --------------- snipp ---------------
# Make-Rules:
# alles nachfolgende brauchen Sie nicht zu editieren,
# tun Sie dies nur, wenn Sie sich mit Makefiles auskennen oder etwas dazulernen wollen.

OBJ_FILES = $(C_FILES:.c=.o) ${CC_FILES:.cpp=.o}

LDLIBS		= -lrt -lncurses -lusb -lm -lpthread
LDFLAGS		= -g

CXXFLAGSI	=  -Wall -g -O0
CFLAGSI		=  -Wall -g -O0

CSTD		= -std=gnu99
CXXSTD		= -std=c++11
.c.o:
	$(CC) ${CSTD} ${CFLAGSI} ${CFLAGS} -c $< -o $@

.cpp.o:
	${CXX} ${CXXSTD} ${CXXFLAGSI} ${CXXFLAGS} -c $< -o $@

${BINARY}: ${OBJ_FILES}
	${CXX} ${LDFLAGS} ${OBJ_FILES} ${LDLIBS} -o $@


all:	${BINARY}

clean:
	-rm ${OBJ_FILES} ${BINARY}

run:	${BINARY}
	@xterm -geometry 83x27 -e "./${BINARY}; echo 'press enter to continue'; read"

.SUFFIXES: .c .o .cpp
.PHONY: clean run


# dependencies

depend: .depend

.depend: ${C_FILES} ${CC_FILES}
	-@rm -f ./.depend 		2>/dev/null
	$(CXX) ${CXXSTD} -MM $^ -MF  ./.depend;

-include .depend
