SRC		= Main.cpp
OBJS	= $(SRC:.cpp=.o)
EXEC	= LOKI.exe

CC		= g++
CFLAGS	= -Wall -O3
LDFLAGS	= -lshlwapi
LDFLAGS += -mwindows	# Comment out this to enable console debug.

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
