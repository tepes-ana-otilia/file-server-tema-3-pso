all: build

build: client server
	gcc server.c -o server -pthread
        gcc client.c -o client

clean:
        rm client | rm server