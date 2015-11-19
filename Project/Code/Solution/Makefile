CC = g++
CFLAGS  = -g -Wall

default: Node.o Radio.o MoistureSensor.o Packet.o MoistureSensor.o 
	$(CC) $(CFLAGS) -o wasp Node.o Radio.o MoistureSensor.o Packet.o MoistureSensor.o 

Node.o: Node.cpp Node.h Radio.cpp MoistureSensor.h Packet.h
	$(CC) $(CFLAGS) -c Node.cpp    

#ingen radio.h findes
Radio.o: Radio.cpp iRadio.cpp Arduino.h SPI.h
	$(CC) $(CFLAGS) -c SPI.h Radio.cpp  

MoistureSensor.o: MoistureSensor.cpp MoistureSensor.h  
	$(CC) $(CFLAGS) -c MoistureSensor.cpp  

Packet.o: Packet.cpp Packet.h
	$(CC) $(CFLAGS) -c Packet.cpp  

clean:
	$(RM) wasp Node.o Radio.o MoistureSensor.o Packet.o MoistureSensor.o 
