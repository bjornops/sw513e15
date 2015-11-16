CC = g++
CFLAGS  = -g -Wall

default: Node.o MoistureSensor.o Packet.o
	$(CC) $(CFLAGS) -o wasp Node.o MoistureSensor.o Packet.o

Node.o: Node.cpp MoistureSensor.h
	$(CC) $(CFLAGS) -c Node.cpp

MoistureSensor.o: MoistureSensor.cpp MoistureSensor.h
	$(CC) $(CFLAGS) -c MoistureSensor.cpp

Packet.o: Packet.cpp Packet.h
	$(CC) $(CFLAGS) -c Packet.cpp

clean:
	$(RM) MoistureSensor.o Packet.o Node.o wasp
