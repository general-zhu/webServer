src=$(wildcard ./*.c)
object=$(patsubst %.c, %.o, $(src))
target=main
CC=gcc
CPPFLAGS=-I./

$(target):$(object)
	$(CC) -o $@ $^ -lpthread
%.o:%.c
	$(CC) -o $@ -c $< $(CPPFLAGS) -lpthread

.PHONY:clean
clean:
	-rm -f $(target) $(object)
