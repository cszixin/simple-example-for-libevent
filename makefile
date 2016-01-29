all:server client
server:server.cpp
	g++ -o server -levent server.cpp
client:client.cpp
	g++ -o client client.cpp
