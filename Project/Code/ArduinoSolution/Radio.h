#include "iRadio.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

class NRF24Radio : public iRadio
{
private:
    RF24 *_radio; 
    
    const uint64_t _rxAddr = 0xF0F0F0F0E1LL;
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
    NRF24Radio(int, int);
    
    void broadcast(char *);
    
    char *listen();
    char *listenFor(unsigned long);
};