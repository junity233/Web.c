CC		=	gcc
INCLUDE	= -I llhttp/include -I C-Thread-Pool -I src
CFLAGS	= -Wall -D_DEBUG -g3 -pthread $(INCLUDE)
OBJS	=	main.o src/utils.o src/map.o src/web.o src/url.o src/trie.o llhttp/src/api.o llhttp/src/http.o llhttp/src/llhttp.o C-Thread-Pool/thpool.o
OUT		= main
RM		= rm -f


build:$(OBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS) $(LIBS)

map.o:src/utils.h
web.o:src/map.h src/utils.h
trie.o:src/utils.h src/map.h
main.o:main.c src/web.o src/utils.o
	$(CC) $(CFLAGS) -c main.c -o main.o
	

.PHONY:clean
clean:
	$(RM) $(OBJS) $(OUT)
