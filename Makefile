
CC=cc
EXECUTABLE = moth
SOURCES = moth.c
OBJS = moth.o

CFLAGS = -W -Wall -O2 -ansi -pedantic
LDFLAGS = -lggtl

COMPILE = $(CC) $(CFLAGS)
LINK = $(COMPILE) $(LDFLAGS)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LINK) $(OBJS) -o $(EXECUTABLE) 

%.o : %.c
	$(COMPILE) -c $< 


clean:
	-rm -f $(OBJS) core


distclean: clean
	-rm -f $(EXECUTABLE)
