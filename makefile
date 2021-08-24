CC		=	gcc
INCLUDE	= -I llhttp/include -I C-Thread-Pool -I src -I e4c/src
DEFINES	= -DE4C_THREADSAFE
DEBUG	= -g3 -D_DEBUG
RELEASE	= -O2
CFLAGS	= -Wall -pthread $(INCLUDE) $(DEFINES)
OBJS	=	main.o src/utils.o src/map.o src/web.o src/url.o src/trie.o llhttp/src/api.o llhttp/src/http.o llhttp/src/llhttp.o C-Thread-Pool/thpool.o e4c/src/e4c.o
OUT		= main
RM		= rm -f

.PHONY:debug release
debug:CFLAGS+=$(DEBUG)
debug:build

release:CFLAGS+=$(RELEASE)
release:build

build:$(OBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS) $(LIBS)

map.o:src/utils.h
web.o:src/map.h src/utils.h
trie.o:src/utils.h src/map.h
main.o:main.c src/web.o src/utils.o
	$(CC) $(CFLAGS) -c main.c -o main.o

install-third-party-library: c-thread-pool

c-thread-pool:
	$(RM) -r C-Thread-Pool
	git clone https://github.com/Pithikos/C-Thread-Pool.git
	$(CC) -c C-Thread-Pool/thpool.c -pthread -o C-Thread-Pool/thpool.o

.PHONY:clean
clean:
	$(RM) $(OBJS) $(OUT)
