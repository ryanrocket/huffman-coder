CFLAGS = -std=c11 -fsanitize=address -g -Wall --pedantic -Wshadow -Wvla -Werror -Wunreachable-code
OBJS = main.o huffman.o writer.o
APP = compress

$(APP): $(OBJS)
	$(CC) $(CFLAGS) *.o -o $(APP)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	/bin/rm -rf *.o
	/bin/rm -rf $(APP)
