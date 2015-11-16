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
    _radio->openReadingPipe(_readingPipe, _rxAddr);
    _radio->startListening();
}

// Sender pakke ud som string.
void NRF24Radio::broadcast(char *packetAsString)
{
    _radio->stopListening();
    _radio->write(&packetAsString, strlen(packetAsString));
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
            char returnString[32] = {0};
            _radio->read(&returnString, 32*sizeof(char));
            
            return returnString;
        }
        
        // Er tiden gået?
        unsigned long thisMs = millis();
        if(thisMs-firstMs > ms)
        {
            return "";
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
            char returnString[32] = {0};
            _radio->read(&returnString, 32*sizeof(char));
        
            return returnString;
        }
    }
}