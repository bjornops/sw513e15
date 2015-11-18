default: bcm2835.o spi.o RF24.o Radio.o Node.o
	g++ -std=c++11 -o solution bcm2835.o spi.o RF24.o Radio.o Node.o

bcm2835.o:
	gcc -c bcm2835.h bcm2835.c

spi.o:
	g++ -c spi.h spi.cpp

RF24.o:
	g++ -c RF24.h RF24.cpp

Radio.o:
	g++ -std=c++11 -c Radio.h Radio.h Radio.cpp

Node.o:
	g++ -std=c++11 -c Node.h Node.cpp
