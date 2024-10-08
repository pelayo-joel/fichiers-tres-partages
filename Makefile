CFLAGS := -Wall -Wextra -Wpedantic  -std=c++17

all:
	g++ -o lpf application/launch_client.cpp classes/src/FTP_Socket.cpp classes/src/FTP_Packet.cpp classes/src/client.cpp classes/src/logger.cpp $(CFLAGS)
	g++ -o lpf_server -lpthread application/launch_server.cpp classes/src/FTP_Socket.cpp classes/src/FTP_Packet.cpp classes/src/server.cpp classes/src/logger.cpp $(CFLAGS)

c:
	g++ -o lpf application/launch_client.cpp classes/src/FTP_Socket.cpp classes/src/client.cpp classes/src/logger.cpp $(CFLAGS)

s:
	g++ -o lpf_server -lpthread application/launch_server.cpp classes/src/FTP_Socket.cpp classes/src/server.cpp classes/src/logger.cpp $(CFLAGS)