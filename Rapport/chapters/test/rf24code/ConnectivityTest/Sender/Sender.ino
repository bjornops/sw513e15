#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

const byte rxAddr[6] = "00001";
int sentPackets;
const int maxPackets = 10000;

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  sentPackets = 0;
  
  while(!Serial);
  Serial.begin(9600);
  
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);
  
  radio.stopListening();
}

void loop()
{
  if(sentPackets < maxPackets)
  {
    const char packet[] = "P";
    radio.write(&packet, sizeof(packet));
    sentPackets += 1;
    
    
//    delay(20);
  }
}
