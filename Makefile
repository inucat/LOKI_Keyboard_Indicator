# -mwindows hides console when .exe double-clicked

SRC=Main.cpp
OBJS=$(SRC:.cpp=.o)
EXEC=loki.exe

CC=g++
CFLAGS=-Wall -O3 -ladvapi32
LDFLAGS=-mwindows
# Opt out `-mwindows` for console debug.

WINDRES=windres
RC=AppResources.rc
OBJS+=$(RC:.rc=.o)

RM=del


${EXEC}:	${OBJS}
	${CC} ${OBJS} ${LDFLAGS} -o ${EXEC}

run:	${EXEC}
	./${EXEC}

%.o:	%.c
	${CC} ${CFLAGS} -o $@ -c $<

%.o:	%.rc
	${WINDRES} ${RC} $*.o

clean:
	${RM} ${OBJS} ${EXEC}
