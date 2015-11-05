#include <iostream>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

class iRadio
{
  virtual void broadcast(string packetAsString);
  virtual string listen(void);
}

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
      _radio.setAutoAck(false);
      _radio.setDataRate(RF24_250KBPS);
      _radio.setPALevel(RF24_PA_MIN);
      _radio.setCRCLength(RF24_CRC_DISABLED);
      _radio.setChannel(_channel);
      _radio.openReadingPipe(_readingPipe, _rxAddr);
      _radio.begin();
    }

    // Sender pakke ud som string.
    void broadcast(string packetAsString)
    {
      _radio.stopListening();
      _radio.write(&packetAsString, strlen(packetAsStrin);
    }

    // Lyt indtil en pakke er klar
    string listen(void)
    {
      if (_radio.available())
      {
        string returnString = {0};
        _radio.startListening();
        _radio.read(&returnString,sizeof(returnString));
        return returnString;
      }
      return "RADIO NOT AVAILABLE!";
    }
}