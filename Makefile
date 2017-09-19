all: client server

client:
	gcc -O3 client.c -lpthread -lncurses -o client

server:
	gcc -O3 server.c -o server

install:
	useradd -s/usr/sbin/nologin -d/ unixchat 2>/dev/null; true
	cp unixchat.service /etc/systemd/system/unixchat.service
	cp server /usr/local/bin/unixchat-server
	cp client /usr/local/bin/unixchat-client
	ln -s /usr/local/bin/unixchat-client /usr/local/bin/chat 2>/dev/null; true

clean:
	rm -rf server client *.o