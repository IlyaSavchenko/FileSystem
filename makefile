.PHONY: fileSystem

fileSystem: 
	gcc -c ./fileSystem.c -D_FILE_OFFSET_BITS=64 -Wall `pkg-config fuse --cflags --libs` -std=gnu99 -w 
		gcc -c ./fuseFunc.c  -D_FILE_OFFSET_BITS=64 -Wall `pkg-config fuse --cflags --libs` -std=gnu99 -w 
		gcc -o fs fuseFunc.o fileSystem.o -D_FILE_OFFSET_BITS=64 -Wall `pkg-config fuse --cflags --libs` -std=gnu99 -w
