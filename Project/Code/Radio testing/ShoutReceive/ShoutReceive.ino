#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);
const byte rxAddr[6] = "00001";

void setup()
{
  while (!Serial);
  Serial.begin(9600);
  
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);  
  radio.setPALevel(RF24_PA_MIN);
  radio.setCRCLength(RF24_CRC_DISABLED);  
  radio.setChannel(114);
  radio.openReadingPipe(0, rxAddr);
  
  radio.startListening();
}

void loop()
{
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    
    String packetString = String(text);
    String p2 = String(millis());
    
    Serial.println(p2+packetString);
  }
}
