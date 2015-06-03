CC=colorgcc
CFLAGS=-Wall -ansi
CCWIN=x86_64-w64-mingw32-gcc
LDFLAGS=-l ws2_32

wait : wait.c
	$(CC) -o $@ $< $(CFLAGS) -D LIN
	
wait.exe : wait.c
	$(CCWIN) -o $@ $< $(CFLAGS) $(LDFLAGS) -D WIN
