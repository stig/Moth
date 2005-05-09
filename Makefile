
CC=cc
EXECUTABLE = moth
SOURCES = moth.c
OBJS = moth.o

CFLAGS = -W -Wall -O -ansi -pedantic
LDFLAGS = -lggtl
LINK = $(CC) $(LDFLAGS)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LINK) $(OBJS) -o $(EXECUTABLE) 

%.o : %.c
	$(CC) -c $< 


clean:
	-rm -f $(OBJS) core


distclean: clean
	-rm -f $(EXECUTABLE)
