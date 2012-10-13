CFLAGS=-std=c99 -g -O0 -I/usr/include
LDFLAGS=-l xcb
SRC=client_test
OBJS=client_test.o

client : $(OBJS)
	cc $(CFLAGS) $(LDFLAGS) -o client $(OBJS)
