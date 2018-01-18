CC = gcc
BINDIR = ../bin
OBJDIR = ../obj/crank
CFLAGS = -c -DUNIX -DPARANOID -DUSE_LOGGING -DUSE_QUERY
LDFLAGS = -pthread -lm
ifeq ($(BUILD),release)
	PROG = $(BINDIR)/crank
	OBJDIR += /release
	CFLAGS += -DNDEBUG -O3 -fno-exceptions
	LDFLAGS += -O3 -fno-exceptions
	EXTRA = strip $(PROG)
else
	PROG = $(BINDIR)/crank-debug
	OBJDIR += /debug
	CFLAGS += -D_DEBUG -DSUPERDEBUG -g -Wall
	LDFLAGS += -g
	EXTRA =
endif
OBJS = $(OBJDIR)/crank.o $(OBJDIR)/generate.o $(OBJDIR)/hash.o $(OBJDIR)/irc.o $(OBJDIR)/objects.o $(OBJDIR)/query.o $(OBJDIR)/sockets.o $(OBJDIR)/util.o

all: crank

crank: $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS) 
	$(EXTRA)

$(OBJDIR)/crank.o: crank.c crank.h query.h
	$(CC) $(CFLAGS) crank.c -o $(OBJDIR)/crank.o

$(OBJDIR)/query.o: query.c query.h crank.h
	$(CC) $(CFLAGS) query.c -o $(OBJDIR)/query.o

$(OBJDIR)/generate.o: generate.c crank.h query.h
	$(CC) $(CFLAGS) generate.c -o $(OBJDIR)/generate.o

$(OBJDIR)/irc.o: irc.c crank.h query.h
	$(CC) $(CFLAGS) irc.c -o $(OBJDIR)/irc.o

$(OBJDIR)/objects.o: objects.c crank.h query.h
	$(CC) $(CFLAGS) objects.c -o $(OBJDIR)/objects.o

$(OBJDIR)/sockets.o: sockets.c sockets.h crank.c
	$(CC) $(CFLAGS) sockets.c -o $(OBJDIR)/sockets.o

$(OBJDIR)/hash.o: hash.c hash.h
	$(CC) $(CFLAGS) hash.c -o $(OBJDIR)/hash.o

$(OBJDIR)/util.o: util.c util.h
	$(CC) $(CFLAGS) util.c -o $(OBJDIR)/util.o

clean:
	rm $(OBJDIR)/*.o $(PROG)

