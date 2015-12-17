#include "iRadio.h"
#include "spi.h"
#include "RF24.h"
#include <stdlib.h>

#ifndef RADIO_H
#define RADIO_H

class NRF24Radio : public iRadio
{
private:
    RF24 *_radio; 
    
    const uint64_t _rxAddr = 0xF0F0F0F0E1LL;
    const int _channel = 114;
    const int _readingPipe = 0;
    
    //RPI
    const int _cePin = 22;
    const int _csPin = 0;

    char defaultMessage[32];
    
public:
    char lastMessage[32];
    
    NRF24Radio();
    
    void broadcast(char *);
    
    char *listen();
    char *listenFor(unsigned long);
};

#endif
