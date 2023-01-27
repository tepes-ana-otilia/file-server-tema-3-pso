all: build

build: client server
	gcc client.c -o client | gcc server.c -o server

clean:
	rm client | rm server
