#include "Radio.h"

//Sætter hardware op.
NRF24Radio::NRF24Radio(int cePin, int csPin) 
{ 
    _radio = new RF24(cePin, csPin);
    _radio->begin();      
    _radio->setAutoAck(false);
    _radio->setDataRate(RF24_250KBPS);
    _radio->setPALevel(RF24_PA_MIN);
    _radio->setCRCLength(RF24_CRC_DISABLED);
    _radio->setChannel(_channel);
    _radio->openWritingPipe(_rxAddr);
    _radio->openReadingPipe(0, _rxAddr);
    
    _radio->startListening();
}  

// Sender pakke ud som string.
void NRF24Radio::broadcast(char packetAsString[32])
{
    _radio->stopListening();
    _radio->write(packetAsString, 32*sizeof(char));
}

// Lyt indtil en pakke modtages, eller tiden er gået
char *NRF24Radio::listenFor(unsigned long ms)
{
    _radio->startListening();
    unsigned long firstMs = millis();
    
    while(true)
    {
        if(_radio->available()) // Læs og returner data
        {
            memset(lastMessage, 0, 32);
            
            _radio->read(&lastMessage, 32*sizeof(char));
            _radio->stopListening();
            
            printf("Modtaget pakke i Radio!\n");
            return lastMessage;
        }
        
        // Er tiden gået?
        unsigned long thisMs = millis();
        if(thisMs-firstMs > ms)
        {
            printf("Ingen pakke, stopper.\n");
            return {(char)0};
        }
    }
}

// Lyt indtil en pakke er klar (Blokerer)
char *NRF24Radio::listen(void)
{
    _radio->startListening();
    
    while(true)
    {
        if (_radio->available()) // Læs og returner data
        {
            memset(lastMessage, 0, 32);
            
            _radio->read(&lastMessage, 32*sizeof(char));
            _radio->stopListening();
            
            return lastMessage;
        }
    }
}
