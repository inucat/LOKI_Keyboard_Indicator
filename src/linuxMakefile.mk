export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-c++
export LD=x86_64-w64-mingw32-ld
export AR=x86_64-w64-mingw32-ar
export AS=x86_64-w64-mingw32-as
export NM=x86_64-w64-mingw32-nm
export STRIP=x86_64-w64-mingw32-strip
export RANLIB=x86_64-w64-mingw32-ranlib
export DLLTOOL=x86_64-w64-mingw32-dlltool
export OBJDUMP=x86_64-w64-mingw32-objdump
export RESCOMP=x86_64-w64-mingw32-windres
export WINDRES=x86_64-w64-mingw32-windres

SRC		= Main.c
OBJS	= $(SRC:.c=.o)
EXEC	= LOKI.exe

CFLAGS	= -Wall -O3 -static
LDFLAGS	= -lshlwapi
LDFLAGS += -mwindows	# Comment out this to enable console debug.

RC		= AppResources.rc
OBJS	+= $(RC:.rc=.o)

RM		= del


${EXEC}:	${OBJS}
	${CXX} ${OBJS} ${LDFLAGS} -o ${EXEC}

force:	clean ${EXEC}

run:	${EXEC}
	./${EXEC}

%.o:	%.c
	${CXX} ${CFLAGS} -o $@ -c $<

%.o:	%.rc
	${WINDRES} ${RC} $*.o

all: clean ${EXEC}

clean:
	${RM} ${OBJS} ${EXEC} *.ini
