# -mwindows hides console when .exe double-clicked
# Comment out `LDFLAGS=-mwindows` for console debug.

SRC		= Main.cpp
OBJS	= $(SRC:.cpp=.o)
EXEC	= LOKI.exe

CC		= g++
CFLAGS	= -Wall -O3 -ladvapi32
LDFLAGS	= -mwindows

WINDRES	= windres
RC		= AppResources.rc
OBJS	+= $(RC:.rc=.o)

RM		= del


${EXEC}:	${OBJS}
	${CC} ${OBJS} ${LDFLAGS} -o ${EXEC}

run:	${EXEC}
	./${EXEC}

%.o:	%.c
	${CC} ${CFLAGS} -o $@ -c $<

%.o:	%.rc
	${WINDRES} ${RC} $*.o

all: clean ${EXEC}

clean:
	${RM} ${OBJS} ${EXEC} *.ini
