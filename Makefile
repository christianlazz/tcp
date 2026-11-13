flags = -std=c2x -Wall 
cc = gcc

server: server.c
	$(cc) $(flags) -o server server.c

clean:
	rm -f server