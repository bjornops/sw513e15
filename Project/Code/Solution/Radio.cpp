#include "Interfaces.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


class nrf24radio : iRadio
{
  private:
    RF24 _radio = NULL
    
    const byte _rxAddr[6] = "00001";
    const int _channel = 114;
    const int _readingPipe = 0;
    
    //UNO
    const int _cePin = 7;
    const int _csPin = 8;
  
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
    //Sætter hardware op.
    nrf24radio(void)
    {
        _radio = RF24(_cePin,_csPin);
        _radio.begin();      
        _radio.setAutoAck(false);
        _radio.setDataRate(RF24_250KBPS);
        _radio.setPALevel(RF24_PA_MIN);
        _radio.setCRCLength(RF24_CRC_DISABLED);
        _radio.setChannel(_channel);
        _radio.openReadingPipe(_readingPipe, _rxAddr);
        _radio.startListening();
    }

    // Sender pakke ud som string.
    void broadcast(string packetAsString)
    {
        _radio.stopListening();
        _radio.write(&packetAsString, strlen(packetAsStrin);
    }
    
    // Lyt indtil en pakke modtages, eller tiden er gået
    char *listenFor(unsigned long ms)
    {
        _radio.startListening();
        unsigned long firstMs = millis();
        
        while(true)
        {
            if(_radio.available()) // Læs og returner data
            {
                char returnString[32] = {0};
                _radio.read(&returnString, 32*sizeof(char));
                
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
    char *listen(void)
    {
        _radio.startListening();
        
        while(true)
        {
            if (_radio.available()) // Læs og returner data
            {
                char returnString[32] = {0};
                _radio.read(&returnString, 32*sizeof(char));
            
                return returnString;
            }
        }
    }
}