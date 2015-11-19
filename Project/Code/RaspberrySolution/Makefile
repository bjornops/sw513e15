default: bcm2835.o spi.o RF24.o Radio.o Node.o Packet.o
	g++ -std=c++11 -o solution bcm2835.o spi.o RF24.o Radio.o Packet.o Node.o

bcm2835.o:
	gcc -c bcm2835.h bcm2835.c

spi.o:
	g++ -c spi.h spi.cpp

RF24.o:
	g++ -c RF24.h RF24.cpp

Radio.o:
	g++ -std=c++11 -c iRadio.h Radio.h Radio.cpp

Node.o: Packet.o
	g++ -std=c++11 -c Packet.h Node.h Node.cpp

Packet.o:
	g++ -std=c++11 -c Packet.h Packet.cpp

semi-clean:
	rm -f bcm2835.o bcm2835.h.gch spi.o spi.h.gch RF24.o RF24.h.gch Radio.o Radio.h.gch Node.o Node.h.gch

clean:
	rm -f bcm2835.o bcm2835.h.gch spi.o spi.h.gch RF24.o RF24.h.gch Radio.o Radio.h.gch Packet.o Packet.h.gch Node.o Node.h.gch iRadio.h.gch solution
