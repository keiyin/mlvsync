CC=colorgcc
CFLAGS=-ansi -Wall
LDFLAGS=-lrt -pthread
EXEC=MLVSync
BIN=bin/
SRC=src/
INCLUDE=include/

$(EXEC) : $(BIN)main.o $(BIN)sync_comm.o $(BIN)walk_tree.o $(BIN)sync_log.o $(BIN)sync_opt.o $(BIN)sync_network.o
	$(CC) -o $@ $^ $(LDFLAGS)
	
test : $(BIN)test.o $(BIN)sync_comm.o $(BIN)walk_tree.o $(BIN)sync_log.o
	$(CC) -o $@ $^ $(LDFLAGS) 
	
$(BIN)%.o : $(SRC)%.c $(include)%.h
	$(CC) -g -o $@ -c $< $(CFLAGS)	

$(BIN)%.o : $(SRC)%.c
	$(CC) -g -o $@ -c $< $(CFLAGS)
	
clean :
	rm -f $(BIN)*.o
