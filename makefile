CC		=	gcc
INCLUDE	= -I llhttp/include -I C-Thread-Pool -I src
CFLAGS	= -Wall -D_DEBUG -g3 -pthread $(INCLUDE)
OBJS	=	main.o src/utils.o src/map.o src/web.o src/url.o src/trie.o llhttp/src/api.o llhttp/src/http.o llhttp/src/llhttp.o C-Thread-Pool/thpool.o
OUT		= main
RM		= rm -f


build:$(OBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS) $(LIBS)

map.o:utils.h
web.o:map.h utils.h
trie.o:utils.h map.h
threadpool.o:map.h utils.h trie.h web.h
main.o:main.c web.o utils.o
	$(CC) $(CFLAGS) -c main.c -o main.o
	

.PHONY:clean
clean:
	$(RM) $(OBJS) $(OUT)
