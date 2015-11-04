CC = g++
CFLAGS  = -g -Wall

default: Main.o MoistureSensor.o Packet.o PacketHandler.o
	$(CC) $(CFLAGS) -o wasp Main.o MoistureSensor.o Packet.o PacketHandler.o

Main.o: Main.cpp MoistureSensor.h PacketHandler.h
	$(CC) $(CFLAGS) -c Main.cpp

MoistureSensor.o: MoistureSensor.cpp MoistureSensor.h
	$(CC) $(CFLAGS) -c MoistureSensor.cpp

Packet.o: Packet.cpp Packet.h
	$(CC) $(CFLAGS) -c Packet.cpp

PacketHandler.o: PacketHandler.cpp PacketHandler.h Interfaces.h Packet.h PacketHandler.h
	$(CC) $(CFLAGS) -c PacketHandler.cpp

clean:
	$(RM) MoistureSensor.o Packet.o PacketHandler.o Main.o wasp
