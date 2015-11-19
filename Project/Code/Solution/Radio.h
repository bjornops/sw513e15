#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "iRadio.h"

class nRF24Radio : public iRadio
{
private:
    RF24 _radio = NULL
    
    const byte _rxAddr[6] = "00001";
    const int _channel = 114;
    const int _readingPipe = 0;
    
    //UNO
    /*
    const int _cePin = 7;
    const int _csPin = 8;
    */
    //MEGA 
    /*
    const int _cePin = 53;
    const int _csPin = 48; 
    */
  
    //RPI
    /*
    const int _cePin = 22;
    const int _csPin = 0;
    */
    
public:
    nRF24Radio(int, int);
    void broadcast(char *);
    char *listen();
    char *listenFor(int);
};