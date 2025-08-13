CFLAGS = -std=c11 -fsanitize=address -g -Wall --pedantic -Wshadow -Wvla -Werror -Wunreachable-code

# Common object files
COMMON_OBJS = huffman.o coder.o

# App-specific object files
COMPRESS_OBJS = compress.o $(COMMON_OBJS)
DECOMPRESS_OBJS = decompress.o $(COMMON_OBJS)

APPS = compress decompress

all: $(APPS)

compress: $(COMPRESS_OBJS)
	$(CC) $(CFLAGS) $(COMPRESS_OBJS) -o $@

decompress: $(DECOMPRESS_OBJS)
	$(CC) $(CFLAGS) $(DECOMPRESS_OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	/bin/rm -f *.o $(APPS)

