default: bcm2835.o spi.o RF24.o NRF24Radio.o Node.o Packet.o
	g++ -std=c++11 -o solution bcm2835.o spi.o RF24.o NRF24Radio.o Packet.o Node.o

bcm2835.o: bcm2835.h bcm2835.c
	gcc -c bcm2835.h bcm2835.c

spi.o: spi.h spi.cpp
	g++ -c spi.h spi.cpp

RF24.o:
	g++ -c RF24.h RF24.cpp

NRF24Radio.o: iRadio.h NRF24Radio.h NRF24Radio.cpp
	g++ -std=c++11 -c iRadio.h NRF24Radio.h NRF24Radio.cpp

Node.o: Packet.o Node.h Node.cpp
	g++ -std=c++11 -c Node.h Node.cpp

Packet.o: Packet.h Packet.cpp
	g++ -std=c++11 -c Packet.h Packet.cpp

semi-clean:
	rm -f bcm2835.o bcm2835.h.gch spi.o spi.h.gch RF24.o RF24.h.gch NRF24Radio.o NRF24Radio.h.gch Packet.o Packet.h.gch Node.o Node.h.gch iRadio.h.gch

clean:
	rm -f bcm2835.o bcm2835.h.gch spi.o spi.h.gch RF24.o RF24.h.gch NRF24Radio.o NRF24Radio.h.gch Packet.o Packet.h.gch Node.o Node.h.gch iRadio.h.gch solution
